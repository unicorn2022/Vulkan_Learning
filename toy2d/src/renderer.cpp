#include "renderer.h"
#include "math.h"
#include "context.h"

namespace toy2d {

Renderer::Renderer(int maxFlightCount){
	this->maxFlightCount_ = maxFlightCount;
	this->curFrame_ = 0;

	createFence();							// ����ͬ������
	createSemaphore();						// �����ź���
	createCmdBuffers();						// �����������
	
	
	createBuffers();						// ����������
	createUniformBuffer(maxFlightCount);	// ����Uniform������
	bufferRectData();							// �����ݿ�����������
	
	descriptorSets_ = DescriptorSetManager::Instance().AllocBufferSets(maxFlightCount);
	updateDescriptorSets();					// ������������, ��Uniform�������󶨵���������

	
	initMats();							// ��ʼ��MVP����
	createWhiteTexture();				// ������ɫ����
	SetDrawColor(Color({ 1, 1, 1 }));	// ���û�����ɫ
}

Renderer::~Renderer(){
	auto& device = Context::Instance().device;
	
	device.destroySampler(sampler);
	rectVerticesBuffer_.reset();
	rectIndicesBuffer_.reset();
	uniformBuffers_.clear();

	for (auto& sem : imageAvaliableSems_) 
		device.destroySemaphore(sem);
	for (auto& sem : renderFinishSems_) 
		device.destroySemaphore(sem);
	for (auto& fence : fences_) 
		device.destroyFence(fence);
	
}

void Renderer::DrawTexture(const Rect& rect, Texture& texture) {
	auto& ctx = Context::Instance();
	auto& device = Context::Instance().device;
	auto& cmd = cmdBufs_[curFrame_];

	bufferRectData();

	// 3.2.1 �� Pipeline
	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, ctx.renderProcess->graphicsPipelineWithTriangleTopology);

	// 3.2.2 ��vertex&index������
	vk::DeviceSize offset = 0;
	cmd.bindVertexBuffers(0, rectVerticesBuffer_->buffer, offset);
	cmd.bindIndexBuffer(rectIndicesBuffer_->buffer, 0, vk::IndexType::eUint32);
	
	// 3.2.3 ����������(uniform)
	auto& layout = Context::Instance().renderProcess->layout;
	cmd.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics, 
		layout, 
		0,	// ����������ƫ����
		{ descriptorSets_[curFrame_].set, texture.set.set }, // uniform������set, �����set
		{}
	);

	// 3.2.4 ����push constanst
	auto model = Mat4::CreateTranslate(rect.position).Mul(Mat4::CreateScale(rect.size));
	cmd.pushConstants(layout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(Mat4), model.GetData());
	cmd.pushConstants(layout, vk::ShaderStageFlagBits::eFragment, sizeof(Mat4), sizeof(Color), &drawColor_);
	
	// 3.2.4 ����������
	cmd.drawIndexed(6, 1, 0, 0, 0);
}

void Renderer::DrawLine(const Vec& p1, const Vec& p2) {
	auto& ctx = Context::Instance();
	auto& device = Context::Instance().device;
	auto& cmd = cmdBufs_[curFrame_];

	bufferLineData(p1, p2);

	// 3.2.1 �� Pipeline
	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, ctx.renderProcess->graphicsPipelineWithLineTopology);

	// 3.2.2 ��vertex������
	vk::DeviceSize offset = 0;
	cmd.bindVertexBuffers(0, lineVerticesBuffer_->buffer, offset);
	
	// 3.2.3 ����������(uniform)
	auto& layout = Context::Instance().renderProcess->layout;
	cmd.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics,
		layout,
		0,	// ����������ƫ����
		{ descriptorSets_[curFrame_].set, whiteTexture->set.set }, // uniform������set, �����set
		{}
	);

	// 3.2.4 ����push constanst
	auto model = Mat4::CreateIdentity();
	cmd.pushConstants(layout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(Mat4), model.GetData());
	cmd.pushConstants(layout, vk::ShaderStageFlagBits::eFragment, sizeof(Mat4), sizeof(Color), &drawColor_);

	// 3.2.4 ����ֱ��
	cmd.draw(2, 1, 0, 0);
}

void Renderer::StartRender() {
	auto& device = Context::Instance().device;
	auto& renderProcess = Context::Instance().renderProcess;
	auto& swapchain = Context::Instance().swapchain;
	auto& cmd = cmdBufs_[curFrame_];

	// 0. �ȴ�֮ǰ������ִ�����
	if (device.waitForFences(fences_[curFrame_], true, std::numeric_limits<uint64_t>::max()) != vk::Result::eSuccess)
		throw std::runtime_error("wait for fence failed");
	device.resetFences(fences_[curFrame_]);


	// 1. ��ȡ��һ��ͼƬ
	auto result = device.acquireNextImageKHR(
		Context::Instance().swapchain->swapchain,	// ������
		std::numeric_limits<uint64_t>::max(),		// ���ȴ�ʱ��
		imageAvaliableSems_[curFrame_],				// �ź���: ͼƬ�Ƿ����
		nullptr
	);
	if (result.result != vk::Result::eSuccess)
		throw std::runtime_error("wait for image in swapchain failed");
	imageIndex_ = result.value;

	// 2. ����command buffer
	cmd.reset();

	// 3. ��������
	// 3.0 ��ʼ����
	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit); // ���ύһ������
	cmd.begin(beginInfo);
	
	// 3.1 ������Ⱦ����
	vk::ClearValue clearValue;
	clearValue.color = vk::ClearColorValue(std::array<float, 4>{0.1f, 0.1f, 0.1f, 1.0f});
	vk::RenderPassBeginInfo renderPassBeginInfo;
	vk::Rect2D area({ 0,0 }, swapchain->GetExtent());
	renderPassBeginInfo.setRenderPass(renderProcess->renderPass)// ��Ⱦ����, �˴�ֻ��������������
		.setFramebuffer(swapchain->framebuffers[imageIndex_])	// ��Ⱦ���ĸ�framebuffer
		.setClearValues(clearValue)								// ��ʲô��ɫ����
		.setRenderArea(area);									// ��Ⱦ����

	// 3.2 ��ʼ��Ⱦ����
	cmd.beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);
}

void Renderer::EndRender() {
	auto& swapchain = Context::Instance().swapchain;
	auto& cmd = cmdBufs_[curFrame_];

	// 3.3 ������Ⱦ����
	cmd.endRenderPass();
	// 3.4 ��������
	cmd.end();

	// 4. �ύ����
	vk::PipelineStageFlags flags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	vk::SubmitInfo submitInfo;
	submitInfo.setCommandBuffers(cmd)		// �������
		.setWaitSemaphores(imageAvaliableSems_[curFrame_])	// �ȴ�ͼƬ����
		.setWaitDstStageMask(flags)							// �ȴ���ɫ���
		.setSignalSemaphores(renderFinishSems_[curFrame_]);	// ֪ͨͼƬ�������
	Context::Instance().graphicsQueue.submit(submitInfo, fences_[curFrame_]);

	// 5. ������ʾ
	vk::PresentInfoKHR presentInfo;
	presentInfo.setWaitSemaphores(renderFinishSems_[curFrame_])	// �ȴ�ͼƬ�������
		.setSwapchains(swapchain->swapchain)					// ������
		.setImageIndices(imageIndex_);							// Ҫ��ʾ��ͼƬ
	if (Context::Instance().presentQueue.presentKHR(presentInfo) != vk::Result::eSuccess)
		throw std::runtime_error("image present failed");

	// 6. �л�����һ֡
	curFrame_ = (curFrame_ + 1) % maxFlightCount_;
}

void Renderer::createFence() {
	fences_.resize(maxFlightCount_, nullptr);

	for (auto& fence : fences_) {
		vk::FenceCreateInfo createInfo;
		createInfo.setFlags(vk::FenceCreateFlagBits::eSignaled); // ��ʼ״̬Ϊsignaled
		fence = Context::Instance().device.createFence(createInfo);
	}
}

void Renderer::createSemaphore() {
	imageAvaliableSems_.resize(maxFlightCount_);
	renderFinishSems_.resize(maxFlightCount_);

	vk::SemaphoreCreateInfo createInfo;
	for (auto& semaphore : imageAvaliableSems_) 
		semaphore = Context::Instance().device.createSemaphore(createInfo);
	for (auto& semaphore : renderFinishSems_) 
		semaphore = Context::Instance().device.createSemaphore(createInfo);
}

void Renderer::createCmdBuffers() {
	cmdBufs_.resize(maxFlightCount_);
	for (auto& cmdBuf : cmdBufs_)
		cmdBuf = Context::Instance().commandManager->CreateOneCommandBuffer();
}

void Renderer::createBuffers() {
	rectVerticesBuffer_.reset(new Buffer(
		vk::BufferUsageFlagBits::eVertexBuffer,
		sizeof(Vertex) * 4,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent // CPU�ɼ�, CPU��GPU����
	));
	rectIndicesBuffer_.reset(new Buffer(
		vk::BufferUsageFlagBits::eIndexBuffer,
		sizeof(uint32_t) * 6,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent // CPU�ɼ�, CPU��GPU����
	));
	lineVerticesBuffer_.reset(new Buffer(
		vk::BufferUsageFlagBits::eVertexBuffer,
		sizeof(Vertex) * 2,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent // CPU�ɼ�, CPU��GPU����
	));
}

void Renderer::createUniformBuffer(int flightCount) {
	// 2��mat4��view�����projection����
	size_t size = sizeof(Mat4) * 2;
	uniformBuffers_.resize(flightCount);
	for (auto& buffer : uniformBuffers_) {
		buffer.reset(new Buffer(
			vk::BufferUsageFlagBits::eTransferSrc,
			size,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent // CPU�ɼ�, CPU��GPU����
		));
	}
	deviceUniformBuffers_.resize(flightCount);
	for (auto& buffer : deviceUniformBuffers_) {
		buffer.reset(new Buffer(
			vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformBuffer,
			size,
			vk::MemoryPropertyFlagBits::eDeviceLocal // GPUר��
		));
	}
}

void Renderer::transformBuffer2Device(Buffer& src, Buffer& dst, size_t srcOffset, size_t dstOffset, size_t size) {
	Context::Instance().commandManager->ExecuteCommand(Context::Instance().graphicsQueue, 
		[&](vk::CommandBuffer cmdBuf) {
			vk::BufferCopy region;
			region.setSize(size)
				.setSrcOffset(srcOffset)
				.setDstOffset(dstOffset);
			cmdBuf.copyBuffer(src.buffer, dst.buffer, region);
		});
}

std::uint32_t Renderer::queryBufferMemTypeIndex(std::uint32_t type, vk::MemoryPropertyFlags flag) {
	auto property = Context::Instance().phyDevice.getMemoryProperties();

	for (std::uint32_t i = 0; i < property.memoryTypeCount; i++) {
		if ((1 << i) & type &&	// ��������ڴ�����
			property.memoryTypes[i].propertyFlags & flag) { // �����������
			return i;
		}
	}

	return 0;
}

void Renderer::bufferRectData() {
	bufferRectVertexData();
	bufferRectIndicesData();
}

void Renderer::bufferRectVertexData() {
	Vertex vertices[] = {
		{Vec{-0.5, -0.5}, Vec{0, 0}},
		{Vec{0.5, -0.5} , Vec{1, 0}},
		{Vec{0.5, 0.5}  , Vec{1, 1}},
		{Vec{-0.5, 0.5} , Vec{0, 1}},
	};
	memcpy(rectVerticesBuffer_->map, vertices, sizeof(vertices));
}

void Renderer::bufferRectIndicesData() {
	std::uint32_t indices[] = {
		0, 1, 3,
		1, 2, 3,
	};
	memcpy(rectIndicesBuffer_->map, indices, sizeof(indices));
}

void Renderer::bufferLineData(const Vec& p1, const Vec& p2) {
	Vertex vertices[] = {
		{p1, Vec{0, 0}},
		{p2, Vec{0, 0}}
	};
	memcpy(lineVerticesBuffer_->map, vertices, sizeof(vertices));
}

void Renderer::bufferMVPData() {
	struct Matrices {
		Mat4 project;
		Mat4 view;
	} matrices;
	
	for (int i = 0; i < uniformBuffers_.size(); i++) {
		auto& buffer = uniformBuffers_[i];
		memcpy(buffer->map, (void*)&projectMat_, sizeof(Mat4));
		memcpy((float*)buffer->map + 16, (void*)&viewMat_, sizeof(Mat4));
		transformBuffer2Device(*buffer, *deviceUniformBuffers_[i], 0, 0, buffer->size);
	}
}

void Renderer::SetDrawColor(const Color& color){
	drawColor_ = color;
}

void Renderer::initMats() {
	viewMat_ = Mat4::CreateIdentity();
	projectMat_ = Mat4::CreateIdentity();
}

void Renderer::SetProject(int right, int left, int bottom, int top, int far, int near) {
	projectMat_ = Mat4::CreateOrtho(left, right, top, bottom, near, far);
	bufferMVPData();
}

void Renderer::updateDescriptorSets() {
	for (int i = 0; i < descriptorSets_.size(); i++) {
		// �� MVP uniform buffer
		vk::DescriptorBufferInfo bufferInfo1;
		bufferInfo1.setBuffer(deviceUniformBuffers_[i]->buffer)
			.setOffset(0)
			.setRange(sizeof(Mat4) * 2);

		std::vector<vk::WriteDescriptorSet> writeInfos(1);
		writeInfos[0].setBufferInfo(bufferInfo1)// ������buffer
			.setDstBinding(0)					// ���ĸ�binding����
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(1)				// �������Ӹ���
			.setDstArrayElement(0)				// ���������еĵڼ�����������
			.setDstSet(descriptorSets_[i].set);	// ���ĸ�������������

		Context::Instance().device.updateDescriptorSets(writeInfos, {});
	}
}

void Renderer::createWhiteTexture() {
	unsigned char data[] = { 0xFF, 0xFF, 0xFF, 0xFF };
	whiteTexture = TextureManager::Instance().Create((void*)data, 1, 1);
}

}
#include "renderer.h"
#include "context.h"

namespace toy2d {

const std::array<Vertex, 3> vertices = {
		Vertex{0.0, -0.5},
		Vertex{0.5, 0.5},
		Vertex{-0.5, 0.5}
};

const Uniform uniform{ Color{1, 1, 0} };

Renderer::Renderer(int maxFlightCount){
	this->maxFlightCount_ = maxFlightCount;
	this->curFrame_ = 0;
	createFence();			// ����ͬ������
	createSemaphore();		// �����ź���
	createCmdBuffers();		// �����������
	createVertexBuffer();	// �������㻺����
	bufferVertexData();		// ���������ݿ��������㻺����
	createUniformBuffer();	// ����Uniform������
	bufferUniformData();	// ��Uniform���ݿ�����Uniform������
	createDescriptorPool();	// ������������
	allocateSets();			// ������������
	updateSets();			// ������������, ��Uniform�������󶨵���������
}

Renderer::~Renderer(){
	Context::GetInstance().device.destroyDescriptorPool(descriptorPool_);

	hostUniformBuffer_.clear();
	deviceUniformBuffer_.clear();
	hostVertexBuffer_.reset();
	deviceVertexBuffer_.reset();

	for (auto& semaphore : imageAvaliableSems_)
		Context::GetInstance().device.destroySemaphore(semaphore);
	for (auto& semaphore : renderFinishSems_)
		Context::GetInstance().device.destroySemaphore(semaphore);
	for (auto& fence : fences_)
		Context::GetInstance().device.destroyFence(fence);
}

void Renderer::DrawTriangle() {
	auto& device = Context::GetInstance().device;
	auto& renderProcess = Context::GetInstance().renderProcess;
	auto& swapchain = Context::GetInstance().swapchain;
	auto& cmdMgr = Context::GetInstance().commandManager;
	
	// 0. �ȴ�֮ǰ������ִ�����
	if (device.waitForFences(fences_[curFrame_], true, std::numeric_limits<uint64_t>::max()) != vk::Result::eSuccess) 
		throw std::runtime_error("wait for fence failed");
	device.resetFences(fences_[curFrame_]);

	
	// 1. ��ȡ��һ��ͼƬ
	auto result = device.acquireNextImageKHR(
		Context::GetInstance().swapchain->swapchain,	// ������
		std::numeric_limits<uint64_t>::max(),			// ���ȴ�ʱ��
		imageAvaliableSems_[curFrame_],					// �ź���: ͼƬ�Ƿ����
		nullptr
	);
	if (result.result != vk::Result::eSuccess) 
		throw std::runtime_error("wait for image in swapchain failed");
	auto imageIndex = result.value;

	// 2. ����command buffer
	cmdBufs_[curFrame_].reset();

	// 3. ��������
	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit); // ���ύһ������
	cmdBufs_[curFrame_].begin(beginInfo);
	{
		// 3.1 ������Ⱦ����
		vk::RenderPassBeginInfo renderPassBeginInfo;
		vk::Rect2D area({ 0,0 }, swapchain->GetExtent());
		vk::ClearValue clearValue;
		clearValue.color = vk::ClearColorValue(std::array<float, 4>{0.1f, 0.1f, 0.1f, 1.0f});
		renderPassBeginInfo.setRenderPass(renderProcess->renderPass)// ��Ⱦ����, �˴�ֻ��������������
			.setFramebuffer(swapchain->framebuffers[imageIndex])	// ��Ⱦ���ĸ�framebuffer
			.setClearValues(clearValue)								// ��ʲô��ɫ����
			.setRenderArea(area);									// ��Ⱦ����
		
		// 3.2 ��ʼ��Ⱦ����
		cmdBufs_[curFrame_].beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);
		{
			// 3.2.1 �� Pipeline
			cmdBufs_[curFrame_].bindPipeline(vk::PipelineBindPoint::eGraphics, renderProcess->graphicsPipeline);
			// 3.2.2 ����������
			cmdBufs_[curFrame_].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, Context::GetInstance().renderProcess->layout, 0, sets_[curFrame_], {});
			// 3.2.3 �󶨶��㻺����
			vk::DeviceSize offset = 0;
			cmdBufs_[curFrame_].bindVertexBuffers(0, deviceVertexBuffer_->buffer, offset);
			// 3.2.4 ����������, ��������:
			// �������, ʵ������, ��һ�������index, ��һ��ʵ����index
			cmdBufs_[curFrame_].draw(3, 1, 0, 0);
		}
		cmdBufs_[curFrame_].endRenderPass();
	}
	cmdBufs_[curFrame_].end();

	// 4. �ύ����
	vk::PipelineStageFlags flags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	vk::SubmitInfo submitInfo;
	submitInfo.setCommandBuffers(cmdBufs_[curFrame_])		// �������
		.setWaitSemaphores(imageAvaliableSems_[curFrame_])	// �ȴ�ͼƬ����
		.setWaitDstStageMask(flags)							// �ȴ���ɫ���
		.setSignalSemaphores(renderFinishSems_[curFrame_]);	// ֪ͨͼƬ�������
	Context::GetInstance().graphicsQueue.submit(submitInfo, fences_[curFrame_]);

	// 5. ������ʾ
	vk::PresentInfoKHR presentInfo;
	presentInfo.setImageIndices(imageIndex)					// Ҫ��ʾ��ͼƬ
		.setSwapchains(swapchain->swapchain)				// ������
		.setWaitSemaphores(renderFinishSems_[curFrame_]);	// �ȴ�ͼƬ�������
	if (Context::GetInstance().presentQueue.presentKHR(presentInfo) != vk::Result::eSuccess)
		throw std::runtime_error("image present failed");

	curFrame_ = (curFrame_ + 1) % maxFlightCount_;
}

void Renderer::createFence() {
	fences_.resize(maxFlightCount_, nullptr);

	for (auto& fence : fences_) {
		vk::FenceCreateInfo createInfo;
		createInfo.setFlags(vk::FenceCreateFlagBits::eSignaled); // ��ʼ״̬Ϊsignaled
		fence = Context::GetInstance().device.createFence(createInfo);
	}
}

void Renderer::createSemaphore() {
	imageAvaliableSems_.resize(maxFlightCount_);
	renderFinishSems_.resize(maxFlightCount_);

	vk::SemaphoreCreateInfo createInfo;
	for (auto& semaphore : imageAvaliableSems_) 
		semaphore = Context::GetInstance().device.createSemaphore(createInfo);
	for (auto& semaphore : renderFinishSems_) 
		semaphore = Context::GetInstance().device.createSemaphore(createInfo);
}

void Renderer::createCmdBuffers() {
	cmdBufs_.resize(maxFlightCount_);
	for (auto& cmdBuf : cmdBufs_)
		cmdBuf = Context::GetInstance().commandManager->CreateOneCommandBuffer();
}

void Renderer::createVertexBuffer() {
	hostVertexBuffer_.reset(new Buffer(
		sizeof(vertices),
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent // CPU�ɼ�, CPU��GPU����
	));
	deviceVertexBuffer_.reset(new Buffer(
		sizeof(vertices),
		vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
		vk::MemoryPropertyFlagBits::eDeviceLocal // ��GPU�ɼ�
	));
}

void Renderer::bufferVertexData() {
	/* �����ݴ��䵽�����ڴ��� */
	// �����㻺����ӳ�䵽�ڴ���
	void* ptr = Context::GetInstance().device.mapMemory(hostVertexBuffer_->memory, 0, hostVertexBuffer_->size);
	// ��������
	memcpy(ptr, vertices.data(), sizeof(vertices));
	// ���ӳ��
	Context::GetInstance().device.unmapMemory(hostVertexBuffer_->memory);

	/* �����ݴӹ����ڴ�ŵ�GPUר���ڴ��� */
	copyBuffer(hostVertexBuffer_->buffer, deviceVertexBuffer_->buffer,
		hostVertexBuffer_->size, 0, 0);
}

void Renderer::createDescriptorPool() {
	// ��������: ���������������
	// ��������: ��Ӧshader�е�uniform����
	vk::DescriptorPoolSize poolSize;
	poolSize.setType(vk::DescriptorType::eUniformBuffer)
		.setDescriptorCount(maxFlightCount_); // �ܹ��������ٸ���������
	
	vk::DescriptorPoolCreateInfo createInfo;
	createInfo.setMaxSets(maxFlightCount_) // ���������������, ����֡����Ҫ�������ٸ���������
		.setPoolSizes(poolSize);

	descriptorPool_ = Context::GetInstance().device.createDescriptorPool(createInfo);
}

void Renderer::allocateSets() {
	std::vector<vk::DescriptorSetLayout> layouts(maxFlightCount_, Context::GetInstance().renderProcess->setLayout);
	
	vk::DescriptorSetAllocateInfo allocInfo;
	allocInfo.setDescriptorPool(descriptorPool_)	// ����������
		.setDescriptorSetCount(maxFlightCount_)		// ������������
		.setSetLayouts(layouts);					// ������������: �ж��ٸ���������, ����Ҫ�ж��ٸ�layout

	sets_ = Context::GetInstance().device.allocateDescriptorSets(allocInfo);
}

void Renderer::updateSets() {
	for (int i = 0; i < sets_.size(); i++) {
		auto& set = sets_[i];
		vk::DescriptorBufferInfo bufferInfo;
		bufferInfo.setBuffer(deviceUniformBuffer_[i]->buffer)
			.setOffset(0)
			.setRange(deviceUniformBuffer_[i]->size);

		vk::WriteDescriptorSet writer;
		writer.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setBufferInfo(bufferInfo)	// ������buffer
			.setDstBinding(0)			// ���ĸ�binding����
			.setDstSet(set)				// ���ĸ�������������
			.setDstArrayElement(0)		// ���������еĵڼ�����������
			.setDescriptorCount(1);		// �������Ӹ���
		Context::GetInstance().device.updateDescriptorSets(writer, {});
	}
}

void Renderer::createUniformBuffer() {
	hostUniformBuffer_.resize(maxFlightCount_);
	deviceUniformBuffer_.resize(maxFlightCount_);

	for (auto& buffer : hostUniformBuffer_) {
		buffer.reset(new Buffer(sizeof(uniform),
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible));
	}

	for (auto& buffer : deviceUniformBuffer_) {
		buffer.reset(new Buffer(sizeof(uniform),
			vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eDeviceLocal));
	}
}

void Renderer::bufferUniformData() {
	for (int i = 0; i < hostUniformBuffer_.size(); i++) {
		auto& buffer = hostUniformBuffer_[i];
		// �����ݴ��䵽�����ڴ���
		void* ptr = Context::GetInstance().device.mapMemory(buffer->memory, 0, buffer->size);
		memcpy(ptr, &uniform, sizeof(uniform));
		Context::GetInstance().device.unmapMemory(buffer->memory);
		
		// �����ݴӹ����ڴ�ŵ�GPUר���ڴ���
		copyBuffer(buffer->buffer, deviceUniformBuffer_[i]->buffer, buffer->size, 0, 0);
	}
}

void Renderer::copyBuffer(vk::Buffer& src, vk::Buffer& dst, size_t size, size_t srcOffset, size_t dstOffset) {
	vk::CommandBufferBeginInfo begin;
	begin.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	// ʹ����������������ݴ���
	auto cmdBuf = Context::GetInstance().commandManager->CreateOneCommandBuffer();
	cmdBuf.begin(begin);
	{
		vk::BufferCopy region;
		region.setSize(size)
			.setSrcOffset(srcOffset)
			.setDstOffset(dstOffset);
		cmdBuf.copyBuffer(src, dst, region);
	}
	cmdBuf.end();
	// �ύ����
	vk::SubmitInfo submit;
	submit.setCommandBuffers(cmdBuf);
	Context::GetInstance().graphicsQueue.submit(submit);
	// �ȴ�����ִ�����
	Context::GetInstance().device.waitIdle();
	// �ͷ��������
	Context::GetInstance().commandManager->FreeCommand(cmdBuf);
}
}
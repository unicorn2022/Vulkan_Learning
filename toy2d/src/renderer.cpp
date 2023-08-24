#include "renderer.h"
#include "math.h"
#include "context.h"

namespace toy2d {

Renderer::Renderer(int maxFlightCount){
	this->maxFlightCount_ = maxFlightCount;
	this->curFrame_ = 0;

	createFence();							// 创建同步对象
	createSemaphore();						// 创建信号量
	createCmdBuffers();						// 创建命令缓冲区
	
	
	createBuffers();						// 创建缓冲区
	createUniformBuffer(maxFlightCount);	// 创建Uniform缓冲区
	bufferRectData();							// 将数据拷贝到缓冲区
	
	descriptorSets_ = DescriptorSetManager::Instance().AllocBufferSets(maxFlightCount);
	updateDescriptorSets();					// 更新描述符集, 将Uniform缓冲区绑定到描述符集

	
	initMats();							// 初始化MVP矩阵
	createWhiteTexture();				// 创建白色纹理
	SetDrawColor(Color({ 1, 1, 1 }));	// 设置绘制颜色
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

	// 3.2.1 绑定 Pipeline
	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, ctx.renderProcess->graphicsPipelineWithTriangleTopology);

	// 3.2.2 绑定vertex&index缓冲区
	vk::DeviceSize offset = 0;
	cmd.bindVertexBuffers(0, rectVerticesBuffer_->buffer, offset);
	cmd.bindIndexBuffer(rectIndicesBuffer_->buffer, 0, vk::IndexType::eUint32);
	
	// 3.2.3 绑定描述符集(uniform)
	auto& layout = Context::Instance().renderProcess->layout;
	cmd.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics, 
		layout, 
		0,	// 描述符集的偏移量
		{ descriptorSets_[curFrame_].set, texture.set.set }, // uniform变量的set, 纹理的set
		{}
	);

	// 3.2.4 传输push constanst
	auto model = Mat4::CreateTranslate(rect.position).Mul(Mat4::CreateScale(rect.size));
	cmd.pushConstants(layout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(Mat4), model.GetData());
	cmd.pushConstants(layout, vk::ShaderStageFlagBits::eFragment, sizeof(Mat4), sizeof(Color), &drawColor_);
	
	// 3.2.4 绘制三角形
	cmd.drawIndexed(6, 1, 0, 0, 0);
}

void Renderer::DrawLine(const Vec& p1, const Vec& p2) {
	auto& ctx = Context::Instance();
	auto& device = Context::Instance().device;
	auto& cmd = cmdBufs_[curFrame_];

	bufferLineData(p1, p2);

	// 3.2.1 绑定 Pipeline
	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, ctx.renderProcess->graphicsPipelineWithLineTopology);

	// 3.2.2 绑定vertex缓冲区
	vk::DeviceSize offset = 0;
	cmd.bindVertexBuffers(0, lineVerticesBuffer_->buffer, offset);
	
	// 3.2.3 绑定描述符集(uniform)
	auto& layout = Context::Instance().renderProcess->layout;
	cmd.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics,
		layout,
		0,	// 描述符集的偏移量
		{ descriptorSets_[curFrame_].set, whiteTexture->set.set }, // uniform变量的set, 纹理的set
		{}
	);

	// 3.2.4 传输push constanst
	auto model = Mat4::CreateIdentity();
	cmd.pushConstants(layout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(Mat4), model.GetData());
	cmd.pushConstants(layout, vk::ShaderStageFlagBits::eFragment, sizeof(Mat4), sizeof(Color), &drawColor_);

	// 3.2.4 绘制直线
	cmd.draw(2, 1, 0, 0);
}

void Renderer::StartRender() {
	auto& device = Context::Instance().device;
	auto& renderProcess = Context::Instance().renderProcess;
	auto& swapchain = Context::Instance().swapchain;
	auto& cmd = cmdBufs_[curFrame_];

	// 0. 等待之前的命令执行完毕
	if (device.waitForFences(fences_[curFrame_], true, std::numeric_limits<uint64_t>::max()) != vk::Result::eSuccess)
		throw std::runtime_error("wait for fence failed");
	device.resetFences(fences_[curFrame_]);


	// 1. 获取下一张图片
	auto result = device.acquireNextImageKHR(
		Context::Instance().swapchain->swapchain,	// 交换链
		std::numeric_limits<uint64_t>::max(),		// 最大等待时间
		imageAvaliableSems_[curFrame_],				// 信号量: 图片是否可用
		nullptr
	);
	if (result.result != vk::Result::eSuccess)
		throw std::runtime_error("wait for image in swapchain failed");
	imageIndex_ = result.value;

	// 2. 重置command buffer
	cmd.reset();

	// 3. 传输命令
	// 3.0 开始命令
	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit); // 仅提交一次命令
	cmd.begin(beginInfo);
	
	// 3.1 配置渲染流程
	vk::ClearValue clearValue;
	clearValue.color = vk::ClearColorValue(std::array<float, 4>{0.1f, 0.1f, 0.1f, 1.0f});
	vk::RenderPassBeginInfo renderPassBeginInfo;
	vk::Rect2D area({ 0,0 }, swapchain->GetExtent());
	renderPassBeginInfo.setRenderPass(renderProcess->renderPass)// 渲染流程, 此处只是用了它的配置
		.setFramebuffer(swapchain->framebuffers[imageIndex_])	// 渲染到哪个framebuffer
		.setClearValues(clearValue)								// 用什么颜色清屏
		.setRenderArea(area);									// 渲染区域

	// 3.2 开始渲染流程
	cmd.beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);
}

void Renderer::EndRender() {
	auto& swapchain = Context::Instance().swapchain;
	auto& cmd = cmdBufs_[curFrame_];

	// 3.3 结束渲染流程
	cmd.endRenderPass();
	// 3.4 结束命令
	cmd.end();

	// 4. 提交命令
	vk::PipelineStageFlags flags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	vk::SubmitInfo submitInfo;
	submitInfo.setCommandBuffers(cmd)		// 命令缓冲区
		.setWaitSemaphores(imageAvaliableSems_[curFrame_])	// 等待图片可用
		.setWaitDstStageMask(flags)							// 等待颜色输出
		.setSignalSemaphores(renderFinishSems_[curFrame_]);	// 通知图片绘制完成
	Context::Instance().graphicsQueue.submit(submitInfo, fences_[curFrame_]);

	// 5. 进行显示
	vk::PresentInfoKHR presentInfo;
	presentInfo.setWaitSemaphores(renderFinishSems_[curFrame_])	// 等待图片绘制完成
		.setSwapchains(swapchain->swapchain)					// 交换链
		.setImageIndices(imageIndex_);							// 要显示的图片
	if (Context::Instance().presentQueue.presentKHR(presentInfo) != vk::Result::eSuccess)
		throw std::runtime_error("image present failed");

	// 6. 切换到下一帧
	curFrame_ = (curFrame_ + 1) % maxFlightCount_;
}

void Renderer::createFence() {
	fences_.resize(maxFlightCount_, nullptr);

	for (auto& fence : fences_) {
		vk::FenceCreateInfo createInfo;
		createInfo.setFlags(vk::FenceCreateFlagBits::eSignaled); // 初始状态为signaled
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
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent // CPU可见, CPU与GPU共享
	));
	rectIndicesBuffer_.reset(new Buffer(
		vk::BufferUsageFlagBits::eIndexBuffer,
		sizeof(uint32_t) * 6,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent // CPU可见, CPU与GPU共享
	));
	lineVerticesBuffer_.reset(new Buffer(
		vk::BufferUsageFlagBits::eVertexBuffer,
		sizeof(Vertex) * 2,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent // CPU可见, CPU与GPU共享
	));
}

void Renderer::createUniformBuffer(int flightCount) {
	// 2个mat4：view矩阵和projection矩阵
	size_t size = sizeof(Mat4) * 2;
	uniformBuffers_.resize(flightCount);
	for (auto& buffer : uniformBuffers_) {
		buffer.reset(new Buffer(
			vk::BufferUsageFlagBits::eTransferSrc,
			size,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent // CPU可见, CPU与GPU共享
		));
	}
	deviceUniformBuffers_.resize(flightCount);
	for (auto& buffer : deviceUniformBuffers_) {
		buffer.reset(new Buffer(
			vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformBuffer,
			size,
			vk::MemoryPropertyFlagBits::eDeviceLocal // GPU专用
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
		if ((1 << i) & type &&	// 是所需的内存类型
			property.memoryTypes[i].propertyFlags & flag) { // 有所需的属性
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
		// 绑定 MVP uniform buffer
		vk::DescriptorBufferInfo bufferInfo1;
		bufferInfo1.setBuffer(deviceUniformBuffers_[i]->buffer)
			.setOffset(0)
			.setRange(sizeof(Mat4) * 2);

		std::vector<vk::WriteDescriptorSet> writeInfos(1);
		writeInfos[0].setBufferInfo(bufferInfo1)// 关联的buffer
			.setDstBinding(0)					// 和哪个binding关联
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(1)				// 描述符子个数
			.setDstArrayElement(0)				// 描述符集中的第几个描述符子
			.setDstSet(descriptorSets_[i].set);	// 和哪个描述符集关联

		Context::Instance().device.updateDescriptorSets(writeInfos, {});
	}
}

void Renderer::createWhiteTexture() {
	unsigned char data[] = { 0xFF, 0xFF, 0xFF, 0xFF };
	whiteTexture = TextureManager::Instance().Create((void*)data, 1, 1);
}

}
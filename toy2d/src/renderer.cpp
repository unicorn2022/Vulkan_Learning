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
	createFence();			// 创建同步对象
	createSemaphore();		// 创建信号量
	createCmdBuffers();		// 创建命令缓冲区
	createVertexBuffer();	// 创建顶点缓冲区
	bufferVertexData();		// 将顶点数据拷贝到顶点缓冲区
	createUniformBuffer();	// 创建Uniform缓冲区
	bufferUniformData();	// 将Uniform数据拷贝到Uniform缓冲区
	createDescriptorPool();	// 创建描述符池
	allocateSets();			// 分配描述符集
	updateSets();			// 更新描述符集, 将Uniform缓冲区绑定到描述符集
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
	
	// 0. 等待之前的命令执行完毕
	if (device.waitForFences(fences_[curFrame_], true, std::numeric_limits<uint64_t>::max()) != vk::Result::eSuccess) 
		throw std::runtime_error("wait for fence failed");
	device.resetFences(fences_[curFrame_]);

	
	// 1. 获取下一张图片
	auto result = device.acquireNextImageKHR(
		Context::GetInstance().swapchain->swapchain,	// 交换链
		std::numeric_limits<uint64_t>::max(),			// 最大等待时间
		imageAvaliableSems_[curFrame_],					// 信号量: 图片是否可用
		nullptr
	);
	if (result.result != vk::Result::eSuccess) 
		throw std::runtime_error("wait for image in swapchain failed");
	auto imageIndex = result.value;

	// 2. 重置command buffer
	cmdBufs_[curFrame_].reset();

	// 3. 传输命令
	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit); // 仅提交一次命令
	cmdBufs_[curFrame_].begin(beginInfo);
	{
		// 3.1 配置渲染流程
		vk::RenderPassBeginInfo renderPassBeginInfo;
		vk::Rect2D area({ 0,0 }, swapchain->GetExtent());
		vk::ClearValue clearValue;
		clearValue.color = vk::ClearColorValue(std::array<float, 4>{0.1f, 0.1f, 0.1f, 1.0f});
		renderPassBeginInfo.setRenderPass(renderProcess->renderPass)// 渲染流程, 此处只是用了它的配置
			.setFramebuffer(swapchain->framebuffers[imageIndex])	// 渲染到哪个framebuffer
			.setClearValues(clearValue)								// 用什么颜色清屏
			.setRenderArea(area);									// 渲染区域
		
		// 3.2 开始渲染流程
		cmdBufs_[curFrame_].beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);
		{
			// 3.2.1 绑定 Pipeline
			cmdBufs_[curFrame_].bindPipeline(vk::PipelineBindPoint::eGraphics, renderProcess->graphicsPipeline);
			// 3.2.2 绑定描述符集
			cmdBufs_[curFrame_].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, Context::GetInstance().renderProcess->layout, 0, sets_[curFrame_], {});
			// 3.2.3 绑定顶点缓冲区
			vk::DeviceSize offset = 0;
			cmdBufs_[curFrame_].bindVertexBuffers(0, deviceVertexBuffer_->buffer, offset);
			// 3.2.4 绘制三角形, 参数如下:
			// 顶点个数, 实例个数, 第一个顶点的index, 第一个实例的index
			cmdBufs_[curFrame_].draw(3, 1, 0, 0);
		}
		cmdBufs_[curFrame_].endRenderPass();
	}
	cmdBufs_[curFrame_].end();

	// 4. 提交命令
	vk::PipelineStageFlags flags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	vk::SubmitInfo submitInfo;
	submitInfo.setCommandBuffers(cmdBufs_[curFrame_])		// 命令缓冲区
		.setWaitSemaphores(imageAvaliableSems_[curFrame_])	// 等待图片可用
		.setWaitDstStageMask(flags)							// 等待颜色输出
		.setSignalSemaphores(renderFinishSems_[curFrame_]);	// 通知图片绘制完成
	Context::GetInstance().graphicsQueue.submit(submitInfo, fences_[curFrame_]);

	// 5. 进行显示
	vk::PresentInfoKHR presentInfo;
	presentInfo.setImageIndices(imageIndex)					// 要显示的图片
		.setSwapchains(swapchain->swapchain)				// 交换链
		.setWaitSemaphores(renderFinishSems_[curFrame_]);	// 等待图片绘制完成
	if (Context::GetInstance().presentQueue.presentKHR(presentInfo) != vk::Result::eSuccess)
		throw std::runtime_error("image present failed");

	curFrame_ = (curFrame_ + 1) % maxFlightCount_;
}

void Renderer::createFence() {
	fences_.resize(maxFlightCount_, nullptr);

	for (auto& fence : fences_) {
		vk::FenceCreateInfo createInfo;
		createInfo.setFlags(vk::FenceCreateFlagBits::eSignaled); // 初始状态为signaled
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
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent // CPU可见, CPU与GPU共享
	));
	deviceVertexBuffer_.reset(new Buffer(
		sizeof(vertices),
		vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
		vk::MemoryPropertyFlagBits::eDeviceLocal // 仅GPU可见
	));
}

void Renderer::bufferVertexData() {
	/* 将数据传输到共享内存中 */
	// 将顶点缓冲区映射到内存中
	void* ptr = Context::GetInstance().device.mapMemory(hostVertexBuffer_->memory, 0, hostVertexBuffer_->size);
	// 传输数据
	memcpy(ptr, vertices.data(), sizeof(vertices));
	// 解除映射
	Context::GetInstance().device.unmapMemory(hostVertexBuffer_->memory);

	/* 将数据从共享内存放到GPU专属内存中 */
	copyBuffer(hostVertexBuffer_->buffer, deviceVertexBuffer_->buffer,
		hostVertexBuffer_->size, 0, 0);
}

void Renderer::createDescriptorPool() {
	// 描述符集: 包含多个描述符子
	// 描述符子: 对应shader中的uniform变量
	vk::DescriptorPoolSize poolSize;
	poolSize.setType(vk::DescriptorType::eUniformBuffer)
		.setDescriptorCount(maxFlightCount_); // 总共创建多少个描述符子
	
	vk::DescriptorPoolCreateInfo createInfo;
	createInfo.setMaxSets(maxFlightCount_) // 最大描述符集个数, 多少帧就需要创建多少个描述符集
		.setPoolSizes(poolSize);

	descriptorPool_ = Context::GetInstance().device.createDescriptorPool(createInfo);
}

void Renderer::allocateSets() {
	std::vector<vk::DescriptorSetLayout> layouts(maxFlightCount_, Context::GetInstance().renderProcess->setLayout);
	
	vk::DescriptorSetAllocateInfo allocInfo;
	allocInfo.setDescriptorPool(descriptorPool_)	// 描述符集池
		.setDescriptorSetCount(maxFlightCount_)		// 描述符集个数
		.setSetLayouts(layouts);					// 描述符集布局: 有多少个描述符集, 就需要有多少个layout

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
			.setBufferInfo(bufferInfo)	// 关联的buffer
			.setDstBinding(0)			// 和哪个binding关联
			.setDstSet(set)				// 和哪个描述符集关联
			.setDstArrayElement(0)		// 描述符集中的第几个描述符子
			.setDescriptorCount(1);		// 描述符子个数
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
		// 将数据传输到共享内存中
		void* ptr = Context::GetInstance().device.mapMemory(buffer->memory, 0, buffer->size);
		memcpy(ptr, &uniform, sizeof(uniform));
		Context::GetInstance().device.unmapMemory(buffer->memory);
		
		// 将数据从共享内存放到GPU专属内存中
		copyBuffer(buffer->buffer, deviceUniformBuffer_[i]->buffer, buffer->size, 0, 0);
	}
}

void Renderer::copyBuffer(vk::Buffer& src, vk::Buffer& dst, size_t size, size_t srcOffset, size_t dstOffset) {
	vk::CommandBufferBeginInfo begin;
	begin.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	// 使用命令缓冲区进行数据传输
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
	// 提交命令
	vk::SubmitInfo submit;
	submit.setCommandBuffers(cmdBuf);
	Context::GetInstance().graphicsQueue.submit(submit);
	// 等待命令执行完毕
	Context::GetInstance().device.waitIdle();
	// 释放命令缓冲区
	Context::GetInstance().commandManager->FreeCommand(cmdBuf);
}
}
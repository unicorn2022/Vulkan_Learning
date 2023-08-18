#include "renderer.h"
#include "context.h"

namespace toy2d {

Renderer::Renderer(int maxFlightCount){
	this->maxFlightCount = maxFlightCount;
	this->curFrame = 0;
	initCommandPool();
	allocCommandBuffer();
	createSemaphore();
	createFence();
}

Renderer::~Renderer(){
	auto& device = Context::GetInstance().device;
	device.freeCommandBuffers(commandPool, commandBuffer);
	device.destroyCommandPool(commandPool);
	
	for (auto& semaphore : imageAvaliableSemaphore) 
		device.destroySemaphore(semaphore);
	for (auto& semaphore : imageDrawFinishSemaphore)
		device.destroySemaphore(semaphore);
	for (auto& fence : fences)
		device.destroyFence(fence);
}

void Renderer::DrawTriangle() {
	auto& device = Context::GetInstance().device;
	auto& renderProcess = Context::GetInstance().renderProcess;
	auto& swapchain = Context::GetInstance().swapchain;
	
	// 0. 等待之前的命令执行完毕
	if (device.waitForFences(fences[curFrame], true, std::numeric_limits<uint64_t>::max()) != vk::Result::eSuccess) 
		throw std::runtime_error("wait for fence failed");
	device.resetFences(fences[curFrame]);

	
	// 1. 获取下一张图片
	auto result = device.acquireNextImageKHR(
		Context::GetInstance().swapchain->swapchain,	// 交换链
		std::numeric_limits<uint64_t>::max(),			// 最大等待时间
		imageAvaliableSemaphore[curFrame]				// 信号量: 图片是否可用
	);
	if (result.result != vk::Result::eSuccess) 
		throw std::runtime_error("acuqire next image failed");
	auto imageIndex = result.value;

	// 2. 重置command buffer
	commandBuffer[curFrame].reset();

	// 3. 传输命令
	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit); // 仅提交一次命令
	commandBuffer[curFrame].begin(beginInfo);
	{
		// 3.1 配置渲染流程
		vk::RenderPassBeginInfo renderPassBeginInfo;
		vk::Rect2D area;
		area.setOffset({ 0,0 })
			.setExtent(swapchain->info.imageExtent);
		vk::ClearValue clearValue;
		clearValue.color = vk::ClearColorValue(std::array<float, 4>{0.1f, 0.1f, 0.1f, 1.0f});
		renderPassBeginInfo.setRenderPass(renderProcess->renderPass)// 渲染流程, 此处只是用了它的配置
			.setRenderArea(area)									// 渲染区域
			.setFramebuffer(swapchain->framebuffers[imageIndex])	// 渲染到哪个framebuffer
			.setClearValues(clearValue);							// 用什么颜色清屏
		
		// 3.2 开始渲染流程
		commandBuffer[curFrame].beginRenderPass(renderPassBeginInfo, {});
		{
			// 3.2.1 绑定 Pipeline
			commandBuffer[curFrame].bindPipeline(vk::PipelineBindPoint::eGraphics, renderProcess->pipeline);
			// 3.2.2 绘制三角形, 参数如下:
			// 顶点个数, 实例个数, 第一个顶点的index, 第一个实例的index
			commandBuffer[curFrame].draw(3, 1, 0, 0);
		}
		commandBuffer[curFrame].endRenderPass();
	}
	commandBuffer[curFrame].end();

	// 4. 提交命令
	vk::PipelineStageFlags flags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	vk::SubmitInfo submitInfo;
	submitInfo.setCommandBuffers(commandBuffer[curFrame])			// 命令缓冲区
		.setWaitSemaphores(imageAvaliableSemaphore[curFrame])		// 等待图片可用
		.setWaitDstStageMask(flags)									// 等待颜色输出
		.setSignalSemaphores(imageDrawFinishSemaphore[curFrame]);	// 通知图片绘制完成
	Context::GetInstance().graphicsQueue.submit(submitInfo, fences[curFrame]);

	// 5. 进行显示
	vk::PresentInfoKHR presentInfo;
	presentInfo.setImageIndices(imageIndex)						// 要显示的图片
		.setSwapchains(swapchain->swapchain)					// 交换链
		.setWaitSemaphores(imageDrawFinishSemaphore[curFrame]);	// 等待图片绘制完成
	if (Context::GetInstance().presentQueue.presentKHR(presentInfo) != vk::Result::eSuccess)
		throw std::runtime_error("image present failed");
}

void Renderer::initCommandPool(){
	vk::CommandPoolCreateInfo createInfo;
	createInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer); // 从当前command pool中分配的command buffer可以单独重置

	commandPool = Context::GetInstance().device.createCommandPool(createInfo);
}

void Renderer::allocCommandBuffer() {
	commandBuffer.resize(maxFlightCount);

	vk::CommandBufferAllocateInfo allocInfo;
	allocInfo.setCommandPool(commandPool)
		.setCommandBufferCount(maxFlightCount)
		.setLevel(vk::CommandBufferLevel::ePrimary); // 设置命令级别: ePrimary可以主动执行

	commandBuffer = Context::GetInstance().device.allocateCommandBuffers(allocInfo);
}

void Renderer::createSemaphore() {
	imageAvaliableSemaphore.resize(maxFlightCount);
	imageDrawFinishSemaphore.resize(maxFlightCount);

	for (auto& semaphore : imageAvaliableSemaphore) {
		vk::SemaphoreCreateInfo createInfo;
		semaphore = Context::GetInstance().device.createSemaphore(createInfo);
	}

	for (auto& semaphore : imageDrawFinishSemaphore) {
		vk::SemaphoreCreateInfo createInfo;
		semaphore = Context::GetInstance().device.createSemaphore(createInfo);
	}
}
void Renderer::createFence() {
	fences.resize(maxFlightCount, nullptr);

	for (auto& fence : fences) {
		vk::FenceCreateInfo createInfo;
		createInfo.setFlags(vk::FenceCreateFlagBits::eSignaled); // 初始状态为signaled
		fence = Context::GetInstance().device.createFence(createInfo);
	}
}

}
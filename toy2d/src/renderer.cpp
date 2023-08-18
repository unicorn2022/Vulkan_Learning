#include "renderer.h"
#include "context.h"

namespace toy2d {
Renderer::Renderer(){
	initCommandPool();
	allocCommandBuffer();
	createSemaphore();
	createFence();
}
Renderer::~Renderer(){
	auto& device = Context::GetInstance().device;
	device.freeCommandBuffers(commandPool, commandBuffer);
	device.destroyCommandPool(commandPool);
	device.destroySemaphore(imageAvaliable);
	device.destroySemaphore(imageDrawFinish);
	device.destroyFence(commandAvailableFence);
}

void Renderer::Render() {
	auto& device = Context::GetInstance().device;
	auto& renderProcess = Context::GetInstance().renderProcess;
	auto& swapchain = Context::GetInstance().swapchain;
	// 1. 获取下一张图片
	auto result = device.acquireNextImageKHR(
		Context::GetInstance().swapchain->swapchain,	// 交换链
		std::numeric_limits<uint64_t>::max(),			// 最大等待时间
		imageAvaliable									// 信号量: 图片是否可用
	);
	if (result.result != vk::Result::eSuccess) {
		std::cout << "acuqire next image failed" << std::endl;
	}
	auto imageIndex = result.value;

	// 2. 重置command buffer
	commandBuffer.reset();

	// 3. 传输命令
	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit); // 仅提交一次命令
	commandBuffer.begin(beginInfo);
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
		commandBuffer.beginRenderPass(renderPassBeginInfo, {});
		{
			// 3.2.1 绑定 Pipeline
			commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, renderProcess->pipeline);
			// 3.2.2 绘制三角形, 参数如下:
			// 顶点个数, 实例个数, 第一个顶点的index, 第一个实例的index
			commandBuffer.draw(3, 1, 0, 0);
		}
		commandBuffer.endRenderPass();
	}
	commandBuffer.end();

	// 4. 提交命令
	vk::SubmitInfo submitInfo;
	submitInfo.setCommandBuffers(commandBuffer)	// 命令缓冲区
		.setWaitSemaphores(imageAvaliable)		// 等待图片可用
		.setSignalSemaphores(imageDrawFinish);	// 通知图片绘制完成
	Context::GetInstance().graphicsQueue.submit(submitInfo, commandAvailableFence);

	// 5. 进行显示
	vk::PresentInfoKHR presentInfo;
	presentInfo.setImageIndices(imageIndex)		// 要显示的图片
		.setSwapchains(swapchain->swapchain)	// 交换链
		.setWaitSemaphores(imageDrawFinish);	// 等待图片绘制完成
	if (Context::GetInstance().presentQueue.presentKHR(presentInfo) != vk::Result::eSuccess) {
		std::cout << "image present failed" << std::endl;
	}

	// 6. 等待命令执行完毕
	if (device.waitForFences(commandAvailableFence, true, std::numeric_limits<uint64_t>::max()) != vk::Result::eSuccess) {
		std::cout << "wait for fence failed" << std::endl;
	}
	device.resetFences(commandAvailableFence);
}

void Renderer::initCommandPool(){
	vk::CommandPoolCreateInfo createInfo;
	createInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer); // 从当前command pool中分配的command buffer可以单独重置

	commandPool = Context::GetInstance().device.createCommandPool(createInfo);
}

void Renderer::allocCommandBuffer() {
	vk::CommandBufferAllocateInfo allocInfo;
	allocInfo.setCommandPool(commandPool)
		.setCommandBufferCount(1)
		.setLevel(vk::CommandBufferLevel::ePrimary); // 设置命令级别: ePrimary可以主动执行

	commandBuffer = Context::GetInstance().device.allocateCommandBuffers(allocInfo)[0];
}

void Renderer::createSemaphore() {
	vk::SemaphoreCreateInfo createInfo;

	imageAvaliable = Context::GetInstance().device.createSemaphore(createInfo);
	imageDrawFinish = Context::GetInstance().device.createSemaphore(createInfo);
}
void Renderer::createFence() {
	vk::FenceCreateInfo createInfo;
	commandAvailableFence = Context::GetInstance().device.createFence(createInfo);
}
}
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
	// 1. ��ȡ��һ��ͼƬ
	auto result = device.acquireNextImageKHR(
		Context::GetInstance().swapchain->swapchain,	// ������
		std::numeric_limits<uint64_t>::max(),			// ���ȴ�ʱ��
		imageAvaliable									// �ź���: ͼƬ�Ƿ����
	);
	if (result.result != vk::Result::eSuccess) {
		std::cout << "acuqire next image failed" << std::endl;
	}
	auto imageIndex = result.value;

	// 2. ����command buffer
	commandBuffer.reset();

	// 3. ��������
	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit); // ���ύһ������
	commandBuffer.begin(beginInfo);
	{
		// 3.1 ������Ⱦ����
		vk::RenderPassBeginInfo renderPassBeginInfo;
		vk::Rect2D area;
		area.setOffset({ 0,0 })
			.setExtent(swapchain->info.imageExtent);
		vk::ClearValue clearValue;
		clearValue.color = vk::ClearColorValue(std::array<float, 4>{0.1f, 0.1f, 0.1f, 1.0f});
		renderPassBeginInfo.setRenderPass(renderProcess->renderPass)// ��Ⱦ����, �˴�ֻ��������������
			.setRenderArea(area)									// ��Ⱦ����
			.setFramebuffer(swapchain->framebuffers[imageIndex])	// ��Ⱦ���ĸ�framebuffer
			.setClearValues(clearValue);							// ��ʲô��ɫ����
		
		// 3.2 ��ʼ��Ⱦ����
		commandBuffer.beginRenderPass(renderPassBeginInfo, {});
		{
			// 3.2.1 �� Pipeline
			commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, renderProcess->pipeline);
			// 3.2.2 ����������, ��������:
			// �������, ʵ������, ��һ�������index, ��һ��ʵ����index
			commandBuffer.draw(3, 1, 0, 0);
		}
		commandBuffer.endRenderPass();
	}
	commandBuffer.end();

	// 4. �ύ����
	vk::SubmitInfo submitInfo;
	submitInfo.setCommandBuffers(commandBuffer)	// �������
		.setWaitSemaphores(imageAvaliable)		// �ȴ�ͼƬ����
		.setSignalSemaphores(imageDrawFinish);	// ֪ͨͼƬ�������
	Context::GetInstance().graphicsQueue.submit(submitInfo, commandAvailableFence);

	// 5. ������ʾ
	vk::PresentInfoKHR presentInfo;
	presentInfo.setImageIndices(imageIndex)		// Ҫ��ʾ��ͼƬ
		.setSwapchains(swapchain->swapchain)	// ������
		.setWaitSemaphores(imageDrawFinish);	// �ȴ�ͼƬ�������
	if (Context::GetInstance().presentQueue.presentKHR(presentInfo) != vk::Result::eSuccess) {
		std::cout << "image present failed" << std::endl;
	}

	// 6. �ȴ�����ִ�����
	if (device.waitForFences(commandAvailableFence, true, std::numeric_limits<uint64_t>::max()) != vk::Result::eSuccess) {
		std::cout << "wait for fence failed" << std::endl;
	}
	device.resetFences(commandAvailableFence);
}

void Renderer::initCommandPool(){
	vk::CommandPoolCreateInfo createInfo;
	createInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer); // �ӵ�ǰcommand pool�з����command buffer���Ե�������

	commandPool = Context::GetInstance().device.createCommandPool(createInfo);
}

void Renderer::allocCommandBuffer() {
	vk::CommandBufferAllocateInfo allocInfo;
	allocInfo.setCommandPool(commandPool)
		.setCommandBufferCount(1)
		.setLevel(vk::CommandBufferLevel::ePrimary); // ���������: ePrimary��������ִ��

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
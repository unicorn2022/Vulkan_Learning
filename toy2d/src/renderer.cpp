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
	
	// 0. �ȴ�֮ǰ������ִ�����
	if (device.waitForFences(fences[curFrame], true, std::numeric_limits<uint64_t>::max()) != vk::Result::eSuccess) 
		throw std::runtime_error("wait for fence failed");
	device.resetFences(fences[curFrame]);

	
	// 1. ��ȡ��һ��ͼƬ
	auto result = device.acquireNextImageKHR(
		Context::GetInstance().swapchain->swapchain,	// ������
		std::numeric_limits<uint64_t>::max(),			// ���ȴ�ʱ��
		imageAvaliableSemaphore[curFrame]				// �ź���: ͼƬ�Ƿ����
	);
	if (result.result != vk::Result::eSuccess) 
		throw std::runtime_error("acuqire next image failed");
	auto imageIndex = result.value;

	// 2. ����command buffer
	commandBuffer[curFrame].reset();

	// 3. ��������
	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit); // ���ύһ������
	commandBuffer[curFrame].begin(beginInfo);
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
		commandBuffer[curFrame].beginRenderPass(renderPassBeginInfo, {});
		{
			// 3.2.1 �� Pipeline
			commandBuffer[curFrame].bindPipeline(vk::PipelineBindPoint::eGraphics, renderProcess->pipeline);
			// 3.2.2 ����������, ��������:
			// �������, ʵ������, ��һ�������index, ��һ��ʵ����index
			commandBuffer[curFrame].draw(3, 1, 0, 0);
		}
		commandBuffer[curFrame].endRenderPass();
	}
	commandBuffer[curFrame].end();

	// 4. �ύ����
	vk::PipelineStageFlags flags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	vk::SubmitInfo submitInfo;
	submitInfo.setCommandBuffers(commandBuffer[curFrame])			// �������
		.setWaitSemaphores(imageAvaliableSemaphore[curFrame])		// �ȴ�ͼƬ����
		.setWaitDstStageMask(flags)									// �ȴ���ɫ���
		.setSignalSemaphores(imageDrawFinishSemaphore[curFrame]);	// ֪ͨͼƬ�������
	Context::GetInstance().graphicsQueue.submit(submitInfo, fences[curFrame]);

	// 5. ������ʾ
	vk::PresentInfoKHR presentInfo;
	presentInfo.setImageIndices(imageIndex)						// Ҫ��ʾ��ͼƬ
		.setSwapchains(swapchain->swapchain)					// ������
		.setWaitSemaphores(imageDrawFinishSemaphore[curFrame]);	// �ȴ�ͼƬ�������
	if (Context::GetInstance().presentQueue.presentKHR(presentInfo) != vk::Result::eSuccess)
		throw std::runtime_error("image present failed");
}

void Renderer::initCommandPool(){
	vk::CommandPoolCreateInfo createInfo;
	createInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer); // �ӵ�ǰcommand pool�з����command buffer���Ե�������

	commandPool = Context::GetInstance().device.createCommandPool(createInfo);
}

void Renderer::allocCommandBuffer() {
	commandBuffer.resize(maxFlightCount);

	vk::CommandBufferAllocateInfo allocInfo;
	allocInfo.setCommandPool(commandPool)
		.setCommandBufferCount(maxFlightCount)
		.setLevel(vk::CommandBufferLevel::ePrimary); // ���������: ePrimary��������ִ��

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
		createInfo.setFlags(vk::FenceCreateFlagBits::eSignaled); // ��ʼ״̬Ϊsignaled
		fence = Context::GetInstance().device.createFence(createInfo);
	}
}

}
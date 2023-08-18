#pragma once

#include "vulkan/vulkan.hpp"

namespace toy2d {

class Renderer final {
public:
	Renderer();
	~Renderer();

	// ����ÿһ֡����Ⱦ
	void Render();


private:
	void initCommandPool();
	void allocCommandBuffer();
	void createSemaphore();
	void createFence();

private:
	vk::CommandPool commandPool;		// �����
	vk::CommandBuffer commandBuffer;	// �������

	vk::Semaphore imageAvaliable;		// ͼƬ�Ƿ���õ��ź���
	vk::Semaphore imageDrawFinish;		// ͼƬ�Ƿ������ɵ��ź���
	vk::Fence commandAvailableFence;	// ��������Ƿ���õ��ź���


};
}
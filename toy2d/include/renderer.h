#pragma once

#include "vulkan/vulkan.hpp"

namespace toy2d {

class Renderer final {
public:
	Renderer(int maxFlightCount = 2);
	~Renderer();

	// ����ÿһ֡����Ⱦ
	void DrawTriangle();


private:
	void initCommandPool();
	void allocCommandBuffer();
	void createSemaphore();
	void createFence();

private:
	int maxFlightCount;
	int curFrame;

	vk::CommandPool commandPool;	// �����
	std::vector<vk::Fence> fences;	// ��������Ƿ���õ��ź���

	std::vector<vk::Semaphore> imageAvaliableSemaphore;	// ͼƬ�Ƿ���õ��ź���
	std::vector<vk::Semaphore> imageDrawFinishSemaphore;// ͼƬ�Ƿ������ɵ��ź���
	std::vector<vk::CommandBuffer> commandBuffer;	// �������


};
}
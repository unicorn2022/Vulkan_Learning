#pragma once

#include "vulkan/vulkan.hpp"

namespace toy2d {

class Renderer final {
public:
	Renderer(int maxFlightCount = 2);
	~Renderer();

	// 进行每一帧的渲染
	void DrawTriangle();


private:
	void initCommandPool();
	void allocCommandBuffer();
	void createSemaphore();
	void createFence();

private:
	int maxFlightCount;
	int curFrame;

	vk::CommandPool commandPool;	// 命令池
	std::vector<vk::Fence> fences;	// 命令缓冲区是否可用的信号量

	std::vector<vk::Semaphore> imageAvaliableSemaphore;	// 图片是否可用的信号量
	std::vector<vk::Semaphore> imageDrawFinishSemaphore;// 图片是否绘制完成的信号量
	std::vector<vk::CommandBuffer> commandBuffer;	// 命令缓冲区


};
}
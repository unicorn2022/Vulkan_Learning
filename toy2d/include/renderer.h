#pragma once

#include "vulkan/vulkan.hpp"

namespace toy2d {

class Renderer final {
public:
	Renderer();
	~Renderer();

	// 进行每一帧的渲染
	void Render();


private:
	void initCommandPool();
	void allocCommandBuffer();
	void createSemaphore();
	void createFence();

private:
	vk::CommandPool commandPool;		// 命令池
	vk::CommandBuffer commandBuffer;	// 命令缓冲区

	vk::Semaphore imageAvaliable;		// 图片是否可用的信号量
	vk::Semaphore imageDrawFinish;		// 图片是否绘制完成的信号量
	vk::Fence commandAvailableFence;	// 命令缓冲区是否可用的信号量


};
}
#pragma once

#include "vulkan/vulkan.hpp"
#include "context.h"
#include "command_manager.h"
#include "swapchain.h"
#include "vertex.h"
#include "buffer.h"
#include <limits>

namespace toy2d {

class Renderer final {
public:
	Renderer(int maxFlightCount = 2);
	~Renderer();

	// 进行每一帧的渲染
	void DrawTriangle();


private:
	void createFence();			// 创建命令缓冲区是否可用的信号量
	void createSemaphore();		// 创建信号量
	void createCmdBuffers();	// 创建命令缓冲区
	void createVertexBuffer();	// 创建顶点缓冲区
	void bufferVertexData();	// 将数据传输到顶点缓冲区

private:
	int maxFlightCount_;	// 最大同时渲染帧数
	int curFrame_;			// 当前帧

	std::vector<vk::Fence> fences_;	// 命令缓冲区是否可用的信号量

	std::vector<vk::Semaphore> imageAvaliableSems_;	// 图片是否可用的信号量
	std::vector<vk::Semaphore> renderFinishSems_;	// 图片是否绘制完成的信号量
	std::vector<vk::CommandBuffer> cmdBufs_;		// 命令缓冲区

	std::unique_ptr<Buffer> hostVertexBuffer_;		// CPU顶点缓冲区
	std::unique_ptr<Buffer> deviceVertexBuffer_;	// GPU顶点缓冲区
};
}
#pragma once

#include "vulkan/vulkan.hpp"
#include "context.h"
#include "command_manager.h"
#include "swapchain.h"
#include "vertex.h"
#include "buffer.h"
#include "uniform.h"
#include <limits>

namespace toy2d {

class Renderer final {
public:
	Renderer(int maxFlightCount = 2);
	~Renderer();

	// 进行每一帧的渲染
	void DrawTriangle();


private:
	/* 命令 */
	// 创建命令缓冲区是否可用的信号量
	void createFence();
	// 创建信号量
	void createSemaphore();
	// 创建命令缓冲区
	void createCmdBuffers();
	
	/* 顶点数据 */
	// 创建顶点缓冲区
	void createVertexBuffer();	
	// 将数据传输到顶点缓冲区
	void bufferVertexData();	
	
	/* uniform数据 */
	// 创建描述符池
	void createDescriptorPool();
	// 申请描述符集
	void allocateSets();
	// 更新描述符集: 将uniform缓冲区绑定到描述符集
	void updateSets();
	// 创建uniform缓冲区
	void createUniformBuffer();	
	// 将数据传输到uniform缓冲区
	void bufferUniformData();

	/* 工具函数 */
	// 将数据从srcBuffer拷贝到dstBuffer
	void copyBuffer(vk::Buffer& src, vk::Buffer& dst, size_t size, size_t srcOffset, size_t dstOffset);

private:
	int maxFlightCount_;	// 最大同时渲染帧数
	int curFrame_;			// 当前帧

	std::vector<vk::Fence> fences_;	// 命令缓冲区是否可用的信号量

	std::vector<vk::Semaphore> imageAvaliableSems_;	// 图片是否可用的信号量
	std::vector<vk::Semaphore> renderFinishSems_;	// 图片是否绘制完成的信号量
	std::vector<vk::CommandBuffer> cmdBufs_;		// 命令缓冲区

	/* 顶点数据 */
	std::unique_ptr<Buffer> hostVertexBuffer_;		// CPU顶点缓冲区
	std::unique_ptr<Buffer> deviceVertexBuffer_;	// GPU顶点缓冲区
	
	/* uniform数据 */
	vk::DescriptorPool descriptorPool_;
	std::vector<vk::DescriptorSet> sets_;
	std::vector<std::unique_ptr<Buffer>> hostUniformBuffer_;	// CPU uniform缓冲区
	std::vector<std::unique_ptr<Buffer>> deviceUniformBuffer_;	// GPU uniform缓冲区
};
}
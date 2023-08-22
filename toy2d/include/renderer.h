#pragma once

#include "vulkan/vulkan.hpp"
#include "context.h"
#include "command_manager.h"
#include "swapchain.h"
#include "mymath.h"
#include "buffer.h"
#include "texture.h"
#include <limits>

namespace toy2d {

class Renderer final {
public:
	Renderer(int maxFlightCount = 2);
	~Renderer();

	void SetProject(int right, int left, int bottom, int top, int far, int near);
	void DrawRect(const Rect&);
	void SetDrawColor(const Color&);


private:
	// fence: CPU 和 GPU 之间的同步
	// semaphore: GPU 的 Command Queue 之间的同步
	// event: CPU 和 GPU 之间发送信号进行同步
	// barrier: 用于隔离命令的执行
	
	/* 命令 */
	// 创建命令缓冲区是否可用的信号量
	void createFence();
	// 创建信号量
	void createSemaphore();
	// 创建命令缓冲区
	void createCmdBuffers();
	
	/* 创建缓冲区 */
	// 创建vertex&index缓冲区
	void createBuffers();
	// 创建uniform缓冲区
	void createUniformBuffer(int flightCount);

	/* 将数据传送给GPU */
	// 初始化MVP矩阵
	void initMats();
	// 传输数据
	void bufferData();
	// 传输vertex数据
	void bufferVertexData();
	// 传输index数据
	void bufferIndicesData();
	// 传输MVP矩阵
	void bufferMVPData();

	/* 创建描述符集 */
	// 创建描述符池
	void createDescriptorPool(int flightCount);
	// 申请描述符集
	std::vector<vk::DescriptorSet> allocDescriptorSet(int flightCount);
	// 申请描述符集
	void allocDescriptorSets(int flightCount);
	// 更新描述符集: 将uniform缓冲区绑定到描述符集
	void updateDescriptorSets();
	
	/* 创建纹理 */
	// 创建纹理采样器
	void createSampler();
	// 创建纹理
	void createTexture();

	/* 工具函数 */
	// 将数据从srcBuffer拷贝到dstBuffer
	void transformBuffer2Device(Buffer& src, Buffer& dst, size_t srcOffset, size_t dstOffset, size_t size);
	// 查询内存类型索引
	std::uint32_t queryBufferMemTypeIndex(std::uint32_t, vk::MemoryPropertyFlags);

private:
	int maxFlightCount_;	// 最大同时渲染帧数
	int curFrame_;			// 当前帧

	std::vector<vk::Fence> fences_;	// 命令缓冲区是否可用的信号量

	std::vector<vk::Semaphore> imageAvaliableSems_;	// 图片是否可用的信号量
	std::vector<vk::Semaphore> renderFinishSems_;	// 图片是否绘制完成的信号量
	std::vector<vk::CommandBuffer> cmdBufs_;		// 命令缓冲区

	/* 顶点数据 */
	std::unique_ptr<Buffer> verticesBuffer_;	// 顶点缓冲区
	std::unique_ptr<Buffer> indicesBuffer_;		// 索引缓冲区

	/* 变换矩阵 */
	Mat4 projectMat_;
	Mat4 viewMat_;
	
	/* uniform数据 */
	vk::DescriptorPool descriptorPool_;
	std::vector<vk::DescriptorSet> descriptorSets_;
	std::vector<std::unique_ptr<Buffer>> uniformBuffers_;		// uniform缓冲区
	std::vector<std::unique_ptr<Buffer>> colorBuffers_;			// 颜色缓冲区
	std::vector<std::unique_ptr<Buffer>> deviceUniformBuffers_;	// (GPU)uniform缓冲区
	std::vector<std::unique_ptr<Buffer>> deviceColorBuffers_;	// (GPU)颜色缓冲区

	/* 纹理 */
	std::unique_ptr<Texture> texture;
	vk::Sampler sampler;
};
}
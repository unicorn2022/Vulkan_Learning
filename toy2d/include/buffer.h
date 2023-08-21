#pragma once

#include "vulkan/vulkan.hpp"

namespace toy2d {

class Buffer final {
public:
	Buffer(size_t size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags property);
	~Buffer();

public:
	struct MemoryInfo final {
		size_t size;
		uint32_t index;
	};
	vk::Buffer buffer;		// 缓冲
	vk::DeviceMemory memory;// 缓冲内存
	size_t size;			// 缓冲大小s

private:
	// 创建缓冲
	void createBuffer(size_t size, vk::BufferUsageFlags usage);
	// 申请内存
	void allocateMemory(MemoryInfo info);
	// 绑定内存到缓冲
	void bindingMemory2Buffer();
	// 查询内存信息
	MemoryInfo queryMemoryInfo(vk::MemoryPropertyFlags property);
};
}
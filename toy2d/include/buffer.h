#pragma once

#include "context.h"

namespace toy2d {

struct Buffer {
	vk::Buffer buffer;		// 缓冲区
	vk::DeviceMemory memory;// 缓冲区内存
	void* map;				// 缓冲区映射地址	
	size_t size;			// 缓冲区大小
	size_t requireSize;		// 缓冲区需要的大小

	Buffer(vk::BufferUsageFlags usage, size_t size, vk::MemoryPropertyFlags memProperty);
	~Buffer();

	Buffer(const Buffer&) = delete;
	Buffer& operator=(const Buffer&) = delete;

};

// 查询缓冲区内存类型索引
std::uint32_t QueryBufferMemTypeIndex(std::uint32_t requirementBit, vk::MemoryPropertyFlags);
}
#pragma once

#include "context.h"

namespace toy2d {

struct Buffer {
	vk::Buffer buffer;		// ������
	vk::DeviceMemory memory;// �������ڴ�
	void* map;				// ������ӳ���ַ	
	size_t size;			// ��������С
	size_t requireSize;		// ��������Ҫ�Ĵ�С

	Buffer(vk::BufferUsageFlags usage, size_t size, vk::MemoryPropertyFlags memProperty);
	~Buffer();

	Buffer(const Buffer&) = delete;
	Buffer& operator=(const Buffer&) = delete;

};

// ��ѯ�������ڴ���������
std::uint32_t QueryBufferMemTypeIndex(std::uint32_t requirementBit, vk::MemoryPropertyFlags);
}
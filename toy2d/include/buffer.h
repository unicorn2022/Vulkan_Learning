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
	vk::Buffer buffer;		// ����
	vk::DeviceMemory memory;// �����ڴ�
	size_t size;			// �����Сs

private:
	// ��������
	void createBuffer(size_t size, vk::BufferUsageFlags usage);
	// �����ڴ�
	void allocateMemory(MemoryInfo info);
	// ���ڴ浽����
	void bindingMemory2Buffer();
	// ��ѯ�ڴ���Ϣ
	MemoryInfo queryMemoryInfo(vk::MemoryPropertyFlags property);
};
}
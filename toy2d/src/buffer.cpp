#include "buffer.h"
#include "context.h"

namespace toy2d {

Buffer::Buffer(size_t size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags property) {
	this->size = size;
	createBuffer(size, usage);
	auto info = queryMemoryInfo(property);
	allocateMemory(info);
	bindingMemory2Buffer();
}

Buffer::~Buffer() {
	Context::GetInstance().device.freeMemory(memory);
	Context::GetInstance().device.destroyBuffer(buffer);
}

void Buffer::createBuffer(size_t size, vk::BufferUsageFlags usage) {
	vk::BufferCreateInfo createInfo;
	createInfo.setSize(size)
		.setUsage(usage)
		.setSharingMode(vk::SharingMode::eExclusive);
	buffer = Context::GetInstance().device.createBuffer(createInfo);
}

void Buffer::allocateMemory(MemoryInfo info) {
	vk::MemoryAllocateInfo allocInfo;
	allocInfo.setAllocationSize(info.size)
		.setMemoryTypeIndex(info.index);
	memory = Context::GetInstance().device.allocateMemory(allocInfo);
}

Buffer::MemoryInfo Buffer::queryMemoryInfo(vk::MemoryPropertyFlags property){
	MemoryInfo info;
	
	// 1. 查询缓冲内存大小
	auto requirements = Context::GetInstance().device.getBufferMemoryRequirements(buffer);
	info.size = requirements.size;

	// 2. 查询缓冲内存类型
	auto properties = Context::GetInstance().phyDevice.getMemoryProperties();
	for (int i = 0; i < properties.memoryTypeCount; i++) {
		if ((1 << i) & requirements.memoryTypeBits &&			// 该内存类型可用
			properties.memoryTypes[i].propertyFlags & property	// 该内存类型支持该属性
		) {
			info.index = i;
			break;
		}
	}

	return info;
}

void Buffer::bindingMemory2Buffer() {
	Context::GetInstance().device.bindBufferMemory(buffer, memory, 0);
}

}
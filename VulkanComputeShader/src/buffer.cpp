#include "buffer.h"

Buffer::Buffer(vk::PhysicalDevice phyDevice, vk::Device device, vk::BufferUsageFlags usage, size_t size, vk::MemoryPropertyFlags memProperty) {
	phyDevice = phyDevice;
	device_ = device;
	this->size = size;

	// ����buffer
	vk::BufferCreateInfo createInfo;
	createInfo.setUsage(usage)
		.setSize(size)
		.setSharingMode(vk::SharingMode::eExclusive);
	buffer = device.createBuffer(createInfo);

	// ��ȡbuffer���ڴ�����
	auto requirements = device.getBufferMemoryRequirements(buffer);
	requireSize = requirements.size;
	auto index = queryBufferMemTypeIndex(requirements.memoryTypeBits, memProperty);

	// �����ڴ�
	vk::MemoryAllocateInfo allocInfo;
	allocInfo.setMemoryTypeIndex(index)
		.setAllocationSize(requireSize);
	memory = device.allocateMemory(allocInfo);

	// ���ڴ�
	device.bindBufferMemory(buffer, memory, 0);

	// ӳ���ڴ�
	if (memProperty & vk::MemoryPropertyFlagBits::eHostVisible) {
		map = device.mapMemory(memory, 0, size);
	}
	else {
		map = nullptr;
	}
}

Buffer::~Buffer() {
	if (map) device_.unmapMemory(memory);
	device_.freeMemory(memory);
	device_.destroyBuffer(buffer);
}

std::uint32_t Buffer::queryBufferMemTypeIndex(std::uint32_t type, vk::MemoryPropertyFlags flag) {
	auto property = phyDevice_.getMemoryProperties();

	for (std::uint32_t i = 0; i < property.memoryTypeCount; i++) {
		if ((1 << i) & type &&
			property.memoryTypes[i].propertyFlags & flag) {
			return i;
		}
	}
	return 0;
}

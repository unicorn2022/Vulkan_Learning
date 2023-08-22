#include "buffer.h"

namespace toy2d {

Buffer::Buffer(vk::BufferUsageFlags usage, size_t size, vk::MemoryPropertyFlags memProperty) {
    auto& device = Context::Instance().device;

    this->size = size;

    /* ����buffer */
    vk::BufferCreateInfo createInfo;
    createInfo.setUsage(usage)  // buffer��;
        .setSize(size)          // buffer��С
        .setSharingMode(vk::SharingMode::eExclusive);   // buffer����ģʽ
    buffer = device.createBuffer(createInfo);

    /* ����buffer�ڴ� */
    // 1. ��ѯ�����ڴ��С
    auto requirements = device.getBufferMemoryRequirements(buffer);
    requireSize = requirements.size;
    // 2. ��ѯ�����ڴ�����
    auto index = queryBufferMemTypeIndex(requirements.memoryTypeBits, memProperty);
    // 3. ���仺���ڴ�
    vk::MemoryAllocateInfo allocInfo;
    allocInfo.setMemoryTypeIndex(index)         // �ڴ�����
        .setAllocationSize(requirements.size);  // �ڴ��С
    memory = device.allocateMemory(allocInfo);

    /* ��buffer���ڴ� */
    device.bindBufferMemory(buffer, memory, 0);

    /* ���CPU�ɼ�, ��ӳ�䵽CPU */ 
    if (memProperty & vk::MemoryPropertyFlagBits::eHostVisible) 
        map = device.mapMemory(memory, 0, size);
    else 
        map = nullptr;
    
}

Buffer::~Buffer() {
    auto& device = Context::Instance().device;
    if (map) device.unmapMemory(memory);
    device.freeMemory(memory);
    device.destroyBuffer(buffer);
}

std::uint32_t Buffer::queryBufferMemTypeIndex(std::uint32_t type, vk::MemoryPropertyFlags flag) {
    auto property = Context::Instance().phyDevice.getMemoryProperties();

    for (std::uint32_t i = 0; i < property.memoryTypeCount; i++) {
        if ((1 << i) & type &&                              // ���ڴ����Ϳ���
            property.memoryTypes[i].propertyFlags & flag	// ���ڴ�����֧�ָ�����
        ) {
            return i;
        }
    }

    return 0;
}

}
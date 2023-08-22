#include "buffer.h"

namespace toy2d {

Buffer::Buffer(vk::BufferUsageFlags usage, size_t size, vk::MemoryPropertyFlags memProperty) {
    auto& device = Context::Instance().device;

    this->size = size;

    /* 创建buffer */
    vk::BufferCreateInfo createInfo;
    createInfo.setUsage(usage)  // buffer用途
        .setSize(size)          // buffer大小
        .setSharingMode(vk::SharingMode::eExclusive);   // buffer共享模式
    buffer = device.createBuffer(createInfo);

    /* 分配buffer内存 */
    // 1. 查询缓冲内存大小
    auto requirements = device.getBufferMemoryRequirements(buffer);
    requireSize = requirements.size;
    // 2. 查询缓冲内存类型
    auto index = queryBufferMemTypeIndex(requirements.memoryTypeBits, memProperty);
    // 3. 分配缓冲内存
    vk::MemoryAllocateInfo allocInfo;
    allocInfo.setMemoryTypeIndex(index)         // 内存类型
        .setAllocationSize(requirements.size);  // 内存大小
    memory = device.allocateMemory(allocInfo);

    /* 绑定buffer和内存 */
    device.bindBufferMemory(buffer, memory, 0);

    /* 如果CPU可见, 就映射到CPU */ 
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
        if ((1 << i) & type &&                              // 该内存类型可用
            property.memoryTypes[i].propertyFlags & flag	// 该内存类型支持该属性
        ) {
            return i;
        }
    }

    return 0;
}

}
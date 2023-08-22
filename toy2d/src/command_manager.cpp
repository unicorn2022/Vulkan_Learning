#include "command_manager.h"
#include "context.h"

namespace toy2d {
CommandManager::CommandManager() {
	pool_ = createCommandPool();
}

CommandManager::~CommandManager() {
	Context::Instance().device.destroyCommandPool(pool_);
}

void CommandManager::ResetCommands() {
	Context::Instance().device.resetCommandPool(pool_);
}

vk::CommandPool CommandManager::createCommandPool() {
	vk::CommandPoolCreateInfo createInfo;

	createInfo.setQueueFamilyIndex(Context::Instance().queueInfo.graphicsIndex.value())	// 设置命令池所属队列
		.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);					// 设置命令池的标志
	
	return Context::Instance().device.createCommandPool(createInfo);
}

std::vector<vk::CommandBuffer> CommandManager::CreateCommandBuffers(uint32_t count) {
	vk::CommandBufferAllocateInfo allocInfo;

	allocInfo.setCommandPool(pool_)	// 设置所属命令池
		.setCommandBufferCount(1)	// 设置命令缓冲区数量
		.setLevel(vk::CommandBufferLevel::ePrimary);	// 设置命令级别: ePrimary可以主动执行

	return Context::Instance().device.allocateCommandBuffers(allocInfo);
}

vk::CommandBuffer CommandManager::CreateOneCommandBuffer() {
	return CreateCommandBuffers(1)[0];
}

void CommandManager::FreeCommand(vk::CommandBuffer buf) {
	Context::Instance().device.freeCommandBuffers(pool_, buf);
}


}
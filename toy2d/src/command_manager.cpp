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

	createInfo.setQueueFamilyIndex(Context::Instance().queueInfo.graphicsIndex.value())	// �����������������
		.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);					// ��������صı�־
	
	return Context::Instance().device.createCommandPool(createInfo);
}

std::vector<vk::CommandBuffer> CommandManager::CreateCommandBuffers(uint32_t count) {
	vk::CommandBufferAllocateInfo allocInfo;

	allocInfo.setCommandPool(pool_)	// �������������
		.setCommandBufferCount(1)	// ���������������
		.setLevel(vk::CommandBufferLevel::ePrimary);	// ���������: ePrimary��������ִ��

	return Context::Instance().device.allocateCommandBuffers(allocInfo);
}

vk::CommandBuffer CommandManager::CreateOneCommandBuffer() {
	return CreateCommandBuffers(1)[0];
}

void CommandManager::FreeCommand(vk::CommandBuffer buf) {
	Context::Instance().device.freeCommandBuffers(pool_, buf);
}


}
#pragma once

#include "vulkan/vulkan.hpp"

namespace toy2d {

class CommandManager final {
public: 
	CommandManager();
	~CommandManager();

	// ���������
	vk::CommandBuffer CreateOneCommandBuffer();
	// ���������
	std::vector<vk::CommandBuffer> CreateCommandBuffers(uint32_t count);
	// ���������
	void ResetCommands();
	// �ͷ������
	void FreeCommand(vk::CommandBuffer buf);

private:
	vk::CommandPool pool_;
	// ���������
	vk::CommandPool createCommandPool();
};
}
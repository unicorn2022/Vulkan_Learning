#pragma once

#include "vulkan/vulkan.hpp"

namespace toy2d {

class CommandManager final {
public: 
	CommandManager();
	~CommandManager();

	// ´´½¨ÃüÁî»º³å
	vk::CommandBuffer CreateOneCommandBuffer();
	// ´´½¨ÃüÁî»º³å
	std::vector<vk::CommandBuffer> CreateCommandBuffers(uint32_t count);
	// ÖØÖÃÃüÁî»º³å
	void ResetCommands();
	// ÊÍ·ÅÃüÁî»º³å
	void FreeCommand(vk::CommandBuffer buf);

private:
	vk::CommandPool pool_;
	// ´´½¨ÃüÁî³Ø
	vk::CommandPool createCommandPool();
};
}
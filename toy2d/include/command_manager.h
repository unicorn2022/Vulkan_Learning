#pragma once

#include "vulkan/vulkan.hpp"
#include <functional>

namespace toy2d {

class CommandManager final {
public: 
	CommandManager();
	~CommandManager();

	// 创建命令缓冲
	vk::CommandBuffer CreateOneCommandBuffer();
	// 创建命令缓冲
	std::vector<vk::CommandBuffer> CreateCommandBuffers(uint32_t count);
	// 重置命令缓冲
	void ResetCommands();
	// 释放命令缓冲
	void FreeCommand(vk::CommandBuffer buf);

	using RecordCmdFunc = std::function<void(vk::CommandBuffer&)>;
	// func将命令写入buffer, 该函数将命令提交到queue中
	void ExecuteCommand(vk::Queue queue, RecordCmdFunc func);

private:
	vk::CommandPool pool_;

private:
	// 创建命令池
	vk::CommandPool createCommandPool();
};
}
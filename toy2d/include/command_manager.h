#pragma once

#include "vulkan/vulkan.hpp"
#include <functional>

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

	using RecordCmdFunc = std::function<void(vk::CommandBuffer&)>;
	// func������д��buffer, �ú����������ύ��queue��
	void ExecuteCommand(vk::Queue queue, RecordCmdFunc func);

private:
	vk::CommandPool pool_;

private:
	// ���������
	vk::CommandPool createCommandPool();
};
}
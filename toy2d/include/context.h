#pragma once

#include <memory>
#include <iostream>
#include <optional>
#include <functional>
#include <vector>
#include <array>

#include "vulkan/vulkan.hpp"
#include "swapchain.h"
#include "render_process.h"
#include "tool.h"
#include "command_manager.h"

namespace toy2d {

class Context final {
public:
	using GetSurfaceCallback = std::function<VkSurfaceKHR(VkInstance)>;
	friend void Init(std::vector<const char*>&, GetSurfaceCallback, int, int);

	static void Init(const std::vector<const char*> extensions, GetSurfaceCallback cb);
	static void Quit();
	static Context& GetInstance();

	// 初始化渲染流程
	void initRenderProcess();
	// 初始化交换链
	void initSwapchain(int w, int h);
	// 初始化图形管线
	void initGraphicsPipeline();
	// 初始化命令池
	void initCommandPool();

public:
	struct QueueInfo {
		std::optional<uint32_t> graphicsIndex;	// 图像处理命令队列
		std::optional<uint32_t> presentIndex;	// 图像显示命令队列

		operator bool() const {
			return graphicsIndex.has_value() && presentIndex.has_value();
		}

		bool isSameQueue() {
			return graphicsIndex.value() == presentIndex.value();
		}
	}queueInfo; // 物理设备中, 命令队列的index

	vk::Instance instance;			// vulkan实例
	vk::PhysicalDevice phyDevice;	// 物理设备, 只能选择, 无法销毁, 无法直接交互
	vk::Device device;				// 逻辑设备
	vk::Queue graphicsQueue;		// 图像处理命令 队列
	vk::Queue presentQueue;			// 图像显示命令队列

	std::unique_ptr<Swapchain> swapchain;			// 交换链
	std::unique_ptr<RenderProcess> renderProcess;	// 渲染管线
	std::unique_ptr<CommandManager> commandManager;	// 命令管理器

private:
	Context(const std::vector<const char*> extensions, GetSurfaceCallback cb);
	~Context();

	// 创建 vulkan 实例
	vk::Instance createInstance(const std::vector<const char*> extensions);
	// 选择物理设备
	vk::PhysicalDevice pickupPhysicalDevice();
	// 基于物理设备, 创建逻辑设备
	vk::Device  createDevice(vk::SurfaceKHR surface);
	// 查询物理设备支持的命令队列
	void queryQueueInfo(vk::SurfaceKHR surface);

private:
	static Context* instance_;
	vk::SurfaceKHR surface_;					// 用于显示的窗口
	GetSurfaceCallback getSurfaceCb_ = nullptr;	// 获取窗口的回调函数
};

}
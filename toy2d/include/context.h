#pragma once

#include "vulkan/vulkan.hpp"
#include <memory>
#include <cassert>
#include <iostream>
#include <optional>
#include "tool.h"
#include "swapchain.h"
#include "render_process.h"
#include "renderer.h"

namespace toy2d {

class Context final {
public:
	static void Init(const std::vector<const char*> extensions, CreateSurfaceFunc func);
	static void Quit();
	static Context& GetInstance() {
		assert(m_instance);
		return *m_instance;
	}

	void InitSwapchain(int w, int h) { swapchain.reset(new Swapchain(w, h)); }
	void DestroySwapchain(){ swapchain.reset(); }
	void InitRenderer() { renderer.reset(new Renderer()); }

	~Context();

public:
	struct QueueFamliIndices final {
		std::optional<uint32_t> graphicsQueue;	// 图像处理命令队列
		std::optional<uint32_t> presentQueue;	// 图像显示命令队列

		operator bool() const {
			return graphicsQueue.has_value() && presentQueue.has_value();
		}

		bool isSameQueue() {
			return graphicsQueue.value() == presentQueue.value();
		}
	};
	QueueFamliIndices queueFamilyIndices; // 物理设备中, 命令队列的index

	vk::Instance instance;			// vulkan实例
	vk::PhysicalDevice phyDevice;	// 物理设备, 只能选择, 无法销毁, 无法直接交互
	vk::Device device;				// 逻辑设备
	vk::Queue graphicsQueue;		// 图像处理命令 队列
	vk::Queue presentQueue;			// 图像显示命令队列
	vk::SurfaceKHR surface;			// surface扩展, 用于GPU绘制图像

	std::unique_ptr<Swapchain> swapchain;			// 交换链
	std::unique_ptr<RenderProcess> renderProcess;	// 渲染管线
	std::unique_ptr<Renderer> renderer;				// 渲染器

private:
	Context(const std::vector<const char*> extensions, CreateSurfaceFunc func);
	// 创建 vulkan 实例
	void createInstance(const std::vector<const char*> extensions);
	// 选择物理设备
	void pickupPhysicalDevice();
	// 查询物理设备支持的命令队列
	void queryQueueFamilyIndices();
	// 基于物理设备, 创建逻辑设备
	void createDevice();
	// 获取命令队列
	void getQueues();

private:
	static std::unique_ptr<Context> m_instance;
};

}
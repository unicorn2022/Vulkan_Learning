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

	// ��ʼ����Ⱦ����
	void initRenderProcess();
	// ��ʼ��������
	void initSwapchain(int w, int h);
	// ��ʼ��ͼ�ι���
	void initGraphicsPipeline();
	// ��ʼ�������
	void initCommandPool();

public:
	struct QueueInfo {
		std::optional<uint32_t> graphicsIndex;	// ͼ�����������
		std::optional<uint32_t> presentIndex;	// ͼ����ʾ�������

		operator bool() const {
			return graphicsIndex.has_value() && presentIndex.has_value();
		}

		bool isSameQueue() {
			return graphicsIndex.value() == presentIndex.value();
		}
	}queueInfo; // �����豸��, ������е�index

	vk::Instance instance;			// vulkanʵ��
	vk::PhysicalDevice phyDevice;	// �����豸, ֻ��ѡ��, �޷�����, �޷�ֱ�ӽ���
	vk::Device device;				// �߼��豸
	vk::Queue graphicsQueue;		// ͼ�������� ����
	vk::Queue presentQueue;			// ͼ����ʾ�������

	std::unique_ptr<Swapchain> swapchain;			// ������
	std::unique_ptr<RenderProcess> renderProcess;	// ��Ⱦ����
	std::unique_ptr<CommandManager> commandManager;	// ���������

private:
	Context(const std::vector<const char*> extensions, GetSurfaceCallback cb);
	~Context();

	// ���� vulkan ʵ��
	vk::Instance createInstance(const std::vector<const char*> extensions);
	// ѡ�������豸
	vk::PhysicalDevice pickupPhysicalDevice();
	// ���������豸, �����߼��豸
	vk::Device  createDevice(vk::SurfaceKHR surface);
	// ��ѯ�����豸֧�ֵ��������
	void queryQueueInfo(vk::SurfaceKHR surface);

private:
	static Context* instance_;
	vk::SurfaceKHR surface_;					// ������ʾ�Ĵ���
	GetSurfaceCallback getSurfaceCb_ = nullptr;	// ��ȡ���ڵĻص�����
};

}
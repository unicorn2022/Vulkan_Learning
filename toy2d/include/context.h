#pragma once

#include "vulkan/vulkan.hpp"
#include <memory>
#include <cassert>
#include <iostream>
#include <optional>
#include "tool.h"
#include "swapchain.h"

namespace toy2d {

class Context final {
public:
	static void Init(const std::vector<const char*> extensions, CreateSurfaceFunc func);
	void InitSwapchain(int w, int h);

	void DestroySwapchain();
	static void Quit();

	static Context& GetInstance() {
		assert(m_instance);
		return *m_instance;
	}

	~Context();

public:
	struct QueueFamliIndices final {
		std::optional<uint32_t> graphicsQueue;	// ͼ�����������
		std::optional<uint32_t> presentQueue;	// ͼ����ʾ�������

		operator bool() const {
			return graphicsQueue.has_value() && presentQueue.has_value();
		}

		bool isSameQueue() {
			return graphicsQueue.value() == presentQueue.value();
		}
	};
	QueueFamliIndices queueFamilyIndices; // �����豸��, ������е�index

	vk::Instance instance;			// vulkanʵ��
	vk::PhysicalDevice phyDevice;	// �����豸, ֻ��ѡ��, �޷�����, �޷�ֱ�ӽ���
	vk::Device device;				// �߼��豸
	vk::Queue graphicsQueue;		// ͼ�����������
	vk::Queue presentQueue;			// ͼ����ʾ�������
	vk::SurfaceKHR surface;			// surface��չ, ����GPU����ͼ��

	std::unique_ptr<Swapchain> swapchain; // ������

private:
	Context(const std::vector<const char*> extensions, CreateSurfaceFunc func);
	// ���� vulkan ʵ��
	void createInstance(const std::vector<const char*> extensions);
	// ѡ�������豸
	void pickupPhysicalDevice();
	// ��ѯ�����豸֧�ֵ��������
	void queryQueueFamilyIndices();
	// ���������豸, �����߼��豸
	void createDevice();
	// ��ȡ�������
	void getQueues();

private:
	static std::unique_ptr<Context> m_instance;
};

}
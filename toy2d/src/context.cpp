#include "context.h"
#include <iostream>

namespace toy2d {

std::unique_ptr<Context> Context::m_instance = nullptr;

void Context::Init(const std::vector<const char*> extensions, CreateSurfaceFunc func) {
	m_instance.reset(new Context(extensions, func));
}

void Context::InitSwapchain(int w, int h) {
	// ����������
	swapchain.reset(new Swapchain(w, h));
}
void Context::DestroySwapchain() {
	// ���ٽ�����
	swapchain.reset();
}
void Context::Quit() {
	m_instance.reset();
}

Context::Context(const std::vector<const char*> extensions, CreateSurfaceFunc func) {
	// ����vulkanʵ��
	createInstance(extensions);
	// ��ȡ�����豸
	pickupPhysicalDevice();
	// ����surface��
	surface = func(instance);
	// ��ѯ�����豸֧�ֵ��������
	queryQueueFamilyIndices();
	// ���������豸, �����߼��豸
	createDevice();
	// ��ȡ�������
	getQueues();
}

Context::~Context() {
	// ���� surface
	instance.destroySurfaceKHR(surface);
	// �����߼��豸
	device.destroy();
	// ���� vk ʵ��
	instance.destroy();
}

void Context::createInstance(const std::vector<const char*> extensions) {
	// Ӧ�ó�������
	vk::ApplicationInfo appInfo;
	appInfo.setApiVersion(VK_API_VERSION_1_3)
		.setPApplicationName("toy2D");

	// ʵ������, ������֤��
	const std::vector<const char*> layers = { "VK_LAYER_KHRONOS_validation" };
	vk::InstanceCreateInfo createInfo;
	createInfo.setPApplicationInfo(&appInfo)
		.setPEnabledLayerNames(layers)
		.setPEnabledExtensionNames(extensions);

	// ���� vk ʵ��
	instance = vk::createInstance(createInfo);
}

void Context::pickupPhysicalDevice() {
	// ��ȡ�����豸
	auto devices = instance.enumeratePhysicalDevices();

	// ѡ��һ�������豸, ����feature�ж�ѡ���ĸ��Կ�
	for (auto& device : devices) {
		auto feature = device.getFeatures();

		// ֻ��֧��geometryShader���Կ�, ���ܽ���ͼ�μ���
		if (feature.geometryShader) {
			phyDevice = device;
			break;
		}
	}

	std::cout << "ѡ��������豸: " << phyDevice.getProperties().deviceName << "\n";

}

void Context::createDevice() {
	/* ����������� */ 
	float priorities = 1.0;
	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
	// ͬһ������, ֻ��Ҫ����һ�����м���
	if (queueFamilyIndices.isSameQueue()) {
		vk::DeviceQueueCreateInfo queueCreateInfo;
		queueCreateInfo.setPQueuePriorities(&priorities) // ���ȼ�, �����1, �����0
			.setQueueCount(1) // ���д�С
			.setQueueFamilyIndex(queueFamilyIndices.graphicsQueue.value()); // ��������������豸�е�index
		queueCreateInfos.push_back(queueCreateInfo);
	}
	// �ǲ�ͬ�Ķ���, ��Ҫ������������
	else {
		vk::DeviceQueueCreateInfo queueCreateInfo;
		queueCreateInfo.setPQueuePriorities(&priorities)
			.setQueueCount(1)
			.setQueueFamilyIndex(queueFamilyIndices.graphicsQueue.value());
		queueCreateInfos.push_back(queueCreateInfo);
		
		queueCreateInfo.setPQueuePriorities(&priorities)
			.setQueueCount(1)
			.setQueueFamilyIndex(queueFamilyIndices.presentQueue.value());
		queueCreateInfos.push_back(queueCreateInfo);
	}

	/* �߼��豸���� */
	std::array extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	vk::DeviceCreateInfo createInfo;
	createInfo.setQueueCreateInfos(queueCreateInfos)
		.setPEnabledExtensionNames(extensions);		// ������չ


	/* �����߼��豸 */
	device = phyDevice.createDevice(createInfo);
}

void Context::queryQueueFamilyIndices() {
	// ��ȡ�����豸֧�ֵ�����������е�����
	auto properties = phyDevice.getQueueFamilyProperties();
	
	for (int i = 0; i < properties.size(); i++) {
		const auto& property = properties[i];
		// ͨ��λ���㣬�жϵ�ǰ��������Ƿ�֧��ĳһ����
		if (property.queueFlags | vk::QueueFlagBits::eGraphics) {
			queueFamilyIndices.graphicsQueue = i;
		}
		// �жϵ�ǰ�����豸�ĵ�i���������, �Ƿ�֧�ֵ�ǰsurface��չ
		if (phyDevice.getSurfaceSupportKHR(i, surface)) {
			queueFamilyIndices.presentQueue = i;
		}
		// �ҵ��������������, �˳�ѭ��
		if (queueFamilyIndices) break;
	}
}

void Context::getQueues() {
	graphicsQueue = device.getQueue(queueFamilyIndices.graphicsQueue.value(), 0);
	presentQueue = device.getQueue(queueFamilyIndices.presentQueue.value(), 0);
}

}
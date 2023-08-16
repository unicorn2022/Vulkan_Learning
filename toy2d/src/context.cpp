#include "context.h"
#include <iostream>

namespace toy2d {

std::unique_ptr<Context> Context::m_instance = nullptr;

void Context::Init(const std::vector<const char*> extensions, CreateSurfaceFunc func) {
	m_instance.reset(new Context(extensions, func));
}

void Context::InitSwapchain(int w, int h) {
	// 创建交换链
	swapchain.reset(new Swapchain(w, h));
}
void Context::DestroySwapchain() {
	// 销毁交换链
	swapchain.reset();
}
void Context::Quit() {
	m_instance.reset();
}

Context::Context(const std::vector<const char*> extensions, CreateSurfaceFunc func) {
	// 创建vulkan实例
	createInstance(extensions);
	// 获取物理设备
	pickupPhysicalDevice();
	// 创建surface层
	surface = func(instance);
	// 查询物理设备支持的命令队列
	queryQueueFamilyIndices();
	// 基于物理设备, 创建逻辑设备
	createDevice();
	// 获取命令队列
	getQueues();
}

Context::~Context() {
	// 销毁 surface
	instance.destroySurfaceKHR(surface);
	// 销毁逻辑设备
	device.destroy();
	// 销毁 vk 实例
	instance.destroy();
}

void Context::createInstance(const std::vector<const char*> extensions) {
	// 应用程序配置
	vk::ApplicationInfo appInfo;
	appInfo.setApiVersion(VK_API_VERSION_1_3)
		.setPApplicationName("toy2D");

	// 实例配置, 开启验证层
	const std::vector<const char*> layers = { "VK_LAYER_KHRONOS_validation" };
	vk::InstanceCreateInfo createInfo;
	createInfo.setPApplicationInfo(&appInfo)
		.setPEnabledLayerNames(layers)
		.setPEnabledExtensionNames(extensions);

	// 创建 vk 实例
	instance = vk::createInstance(createInfo);
}

void Context::pickupPhysicalDevice() {
	// 获取所有设备
	auto devices = instance.enumeratePhysicalDevices();

	// 选择一个物理设备, 根据feature判断选择哪个显卡
	for (auto& device : devices) {
		auto feature = device.getFeatures();

		// 只有支持geometryShader的显卡, 才能进行图形计算
		if (feature.geometryShader) {
			phyDevice = device;
			break;
		}
	}

	std::cout << "选择的物理设备: " << phyDevice.getProperties().deviceName << "\n";

}

void Context::createDevice() {
	/* 命令队列配置 */ 
	float priorities = 1.0;
	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
	// 同一个队列, 只需要创建一个队列即可
	if (queueFamilyIndices.isSameQueue()) {
		vk::DeviceQueueCreateInfo queueCreateInfo;
		queueCreateInfo.setPQueuePriorities(&priorities) // 优先级, 最高是1, 最低是0
			.setQueueCount(1) // 队列大小
			.setQueueFamilyIndex(queueFamilyIndices.graphicsQueue.value()); // 命令队列在物理设备中的index
		queueCreateInfos.push_back(queueCreateInfo);
	}
	// 是不同的队列, 需要创建两个队列
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

	/* 逻辑设备配置 */
	std::array extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	vk::DeviceCreateInfo createInfo;
	createInfo.setQueueCreateInfos(queueCreateInfos)
		.setPEnabledExtensionNames(extensions);		// 启用扩展


	/* 创建逻辑设备 */
	device = phyDevice.createDevice(createInfo);
}

void Context::queryQueueFamilyIndices() {
	// 获取物理设备支持的所有命令队列的属性
	auto properties = phyDevice.getQueueFamilyProperties();
	
	for (int i = 0; i < properties.size(); i++) {
		const auto& property = properties[i];
		// 通过位运算，判断当前命令队列是否支持某一功能
		if (property.queueFlags | vk::QueueFlagBits::eGraphics) {
			queueFamilyIndices.graphicsQueue = i;
		}
		// 判断当前物理设备的第i个命令队列, 是否支持当前surface扩展
		if (phyDevice.getSurfaceSupportKHR(i, surface)) {
			queueFamilyIndices.presentQueue = i;
		}
		// 找到了两种命令队列, 退出循环
		if (queueFamilyIndices) break;
	}
}

void Context::getQueues() {
	graphicsQueue = device.getQueue(queueFamilyIndices.graphicsQueue.value(), 0);
	presentQueue = device.getQueue(queueFamilyIndices.presentQueue.value(), 0);
}

}
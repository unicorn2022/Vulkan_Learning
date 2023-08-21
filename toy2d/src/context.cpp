#include "context.h"

namespace toy2d {

Context* Context::instance_ = nullptr;

void Context::Init(const std::vector<const char*> extensions, GetSurfaceCallback cb) {
	instance_ = new Context(extensions, cb);
}

void Context::Quit() {
	delete instance_;
}

Context& Context::GetInstance() {
	return *instance_;
}

Context::Context(const std::vector<const char*> extensions, GetSurfaceCallback cb) {
	getSurfaceCb_ = cb;
	
	// 创建vulkan实例
	instance = createInstance(extensions);
	if (!instance) {
		std::cout << "instance create failed" << std::endl;
		exit(1);
	}
	// 获取物理设备
	phyDevice = pickupPhysicalDevice();
	if(!phyDevice) {
		std::cout << "pickup physical device failed" << std::endl;
		exit(1);
	}
	// 创建surface层
	surface_ = getSurfaceCb_(instance);
	if (!surface_) {
		std::cout << "create surface failed" << std::endl;
		exit(1);
	}
	// 基于物理设备, 创建逻辑设备
	device = createDevice(surface_);
	if (!device) {
		std::cout << "create device failed" << std::endl;
		exit(1);
	}
	// 获取命令队列
	graphicsQueue = device.getQueue(queueInfo.graphicsIndex.value(), 0);
	presentQueue = device.getQueue(queueInfo.presentIndex.value(), 0);
}

Context::~Context() {
	commandManager.reset();
	renderProcess.reset();
	swapchain.reset();
	device.destroy();
	instance.destroy();
}

vk::Instance Context::createInstance(const std::vector<const char*> extensions) {
	// 应用程序配置
	vk::ApplicationInfo appInfo;
	appInfo.setApiVersion(VK_API_VERSION_1_3);

	// 实例配置
	const std::vector<const char*> layers = { "VK_LAYER_KHRONOS_validation" };
	vk::InstanceCreateInfo createInfo;
	createInfo.setPApplicationInfo(&appInfo)	// 应用程序配置
		.setPEnabledExtensionNames(extensions)	// 启用扩展
		.setPEnabledLayerNames(layers);			// 开启验证层

	// 创建 vk 实例
	return vk::createInstance(createInfo);
}

vk::PhysicalDevice Context::pickupPhysicalDevice() {
	// 获取所有设备
	auto devices = instance.enumeratePhysicalDevices();
	if (devices.size() == 0) {
		std::cout << "you don't have suitable device to support vulkan" << std::endl;
		exit(1);
	}

	// 选择一个物理设备, 根据feature判断选择哪个显卡
	for (auto& device : devices) {
		auto feature = device.getFeatures();

		// 只有支持geometryShader的显卡, 才能进行图形计算
		if (feature.geometryShader) {
			std::cout << "选择的物理设备: " << device.getProperties().deviceName << "\n";
			return device;
		}
	}

	return devices[0];
}

vk::Device Context::createDevice(vk::SurfaceKHR surface) {
	queryQueueInfo(surface);
	
	/* 命令队列配置 */
	float priority = 1;
	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
	// 同一个队列, 只需要创建一个队列即可
	if (queueInfo.isSameQueue()) {
		vk::DeviceQueueCreateInfo queueCreateInfo;
		queueCreateInfo.setPQueuePriorities(&priority) // 优先级, 最高是1, 最低是0
			.setQueueCount(1) // 队列大小
			.setQueueFamilyIndex(queueInfo.graphicsIndex.value()); // 命令队列在物理设备中的index
		queueCreateInfos.push_back(queueCreateInfo);
	}
	// 是不同的队列, 需要创建两个队列
	else {
		vk::DeviceQueueCreateInfo queueCreateInfo;
		queueCreateInfo.setPQueuePriorities(&priority)
			.setQueueCount(1)
			.setQueueFamilyIndex(queueInfo.graphicsIndex.value());
		queueCreateInfos.push_back(queueCreateInfo);

		queueCreateInfo.setQueueFamilyIndex(queueInfo.presentIndex.value());
		queueCreateInfos.push_back(queueCreateInfo);
	}

	/* 逻辑设备配置 */
	std::array extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	vk::DeviceCreateInfo deivceCreateInfo;
	deivceCreateInfo.setQueueCreateInfos(queueCreateInfos)
		.setPEnabledExtensionNames(extensions);		// 启用扩展

	/* 创建逻辑设备 */
	return phyDevice.createDevice(deivceCreateInfo);
}

void Context::queryQueueInfo(vk::SurfaceKHR surface) {
	// 获取物理设备支持的所有命令队列的属性
	auto properties = phyDevice.getQueueFamilyProperties();

	for (int i = 0; i < properties.size(); i++) {
		const auto& property = properties[i];
		// 通过位运算，判断当前命令队列是否支持某一功能
		if (property.queueFlags & vk::QueueFlagBits::eGraphics) {
			queueInfo.graphicsIndex = i;
		}
		// 判断当前物理设备的第i个命令队列, 是否支持当前surface扩展
		if (phyDevice.getSurfaceSupportKHR(i, surface)) {
			queueInfo.presentIndex = i;
		}
		// 找到了两种命令队列, 退出循环
		if (queueInfo) break;
	}
}

void Context::initSwapchain(int w, int h) {
	swapchain = std::make_unique<Swapchain>(surface_, w, h);
}

void Context::initRenderProcess() {
	renderProcess = std::make_unique<RenderProcess>();
}

void Context::initGraphicsPipeline() {
	auto vertexSource = ReadWholeFile("./shader/vert.spv");
	auto fragSource = ReadWholeFile("./shader/frag.spv");
	renderProcess->RecreateGraphicsPipeline(vertexSource, fragSource);
}

void Context::initCommandPool() {
	commandManager = std::make_unique<CommandManager>();
}

}
#include "context.h"

namespace toy2d {

Context* Context::instance_ = nullptr;

void Context::Init(const std::vector<const char*> extensions, GetSurfaceCallback cb) {
	instance_ = new Context(extensions, cb);
}

void Context::Quit() {
	delete instance_;
}

Context& Context::Instance() {
	return *instance_;
}

Context::Context(const std::vector<const char*> extensions, GetSurfaceCallback cb) {
	getSurfaceCb_ = cb;
	
	// ����vulkanʵ��
	instance = createInstance(extensions);
	if (!instance) {
		std::cout << "instance create failed" << std::endl;
		exit(1);
	}
	// ��ȡ�����豸
	phyDevice = pickupPhysicalDevice();
	if(!phyDevice) {
		std::cout << "pickup physical device failed" << std::endl;
		exit(1);
	}
	// ��ȡ����
	getSurface();
	// ���������豸, �����߼��豸
	device = createDevice(surface_);
	if (!device) {
		std::cout << "create device failed" << std::endl;
		exit(1);
	}
	// ��ȡ�������
	graphicsQueue = device.getQueue(queueInfo.graphicsIndex.value(), 0);
	presentQueue = device.getQueue(queueInfo.presentIndex.value(), 0);
}

void Context::getSurface() {
	surface_ = getSurfaceCb_(instance);
	if (!surface_) {
		std::cout << "create surface failed" << std::endl;
		exit(1);
	}
}

Context::~Context() {
	shader.reset();
	device.destroySampler(sampler);
	commandManager.reset();
	renderProcess.reset();
	swapchain.reset();
	device.destroy();
	instance.destroy();
}

vk::Instance Context::createInstance(const std::vector<const char*> extensions) {
	// Ӧ�ó�������
	vk::ApplicationInfo appInfo;
	vk::InstanceCreateInfo createInfo;
	appInfo.setApiVersion(VK_API_VERSION_1_3);
	createInfo.setPApplicationInfo(&appInfo)	// Ӧ�ó�������
		.setPEnabledExtensionNames(extensions);	// ������չ

	// ������֤��
	std::vector<const char*> layers = { "VK_LAYER_KHRONOS_validation" };
	createInfo.setPEnabledLayerNames(layers);

	// ���� vk ʵ��
	return vk::createInstance(createInfo);
}

vk::PhysicalDevice Context::pickupPhysicalDevice() {
	// ��ȡ�����豸
	auto devices = instance.enumeratePhysicalDevices();
	if (devices.size() == 0) {
		std::cout << "you don't have suitable device to support vulkan" << std::endl;
		exit(1);
	}

	// ѡ��һ�������豸, ����feature�ж�ѡ���ĸ��Կ�
	for (auto& device : devices) {
		auto feature = device.getFeatures();

		// ֻ��֧��geometryShader���Կ�, ���ܽ���ͼ�μ���
		if (feature.geometryShader) {
			std::cout << "ѡ��������豸: " << device.getProperties().deviceName << "\n";
			return device;
		}
	}

	return devices[0];
}

vk::Device Context::createDevice(vk::SurfaceKHR surface) {
	queryQueueInfo(surface);

	/* ������չ: ������ */
	vk::DeviceCreateInfo deviceCreateInfo;
	std::array extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	deviceCreateInfo.setPEnabledExtensionNames(extensions);

	
	/* ����������� */
	std::vector<vk::DeviceQueueCreateInfo> queueInfos;
	float priority = 1;
	// ͬһ������, ֻ��Ҫ����һ�����м���
	if (queueInfo.isSameQueue()) {
		vk::DeviceQueueCreateInfo queueCreateInfo;
		queueCreateInfo.setPQueuePriorities(&priority) // ���ȼ�, �����1, �����0
			.setQueueCount(1) // ���д�С
			.setQueueFamilyIndex(queueInfo.graphicsIndex.value()); // ��������������豸�е�index
		queueInfos.push_back(queueCreateInfo);
	}
	// �ǲ�ͬ�Ķ���, ��Ҫ������������
	else {
		vk::DeviceQueueCreateInfo queueCreateInfo;
		queueCreateInfo.setPQueuePriorities(&priority)
			.setQueueCount(1)
			.setQueueFamilyIndex(queueInfo.graphicsIndex.value());
		queueInfos.push_back(queueCreateInfo);

		queueCreateInfo.setQueueFamilyIndex(queueInfo.presentIndex.value());
		queueInfos.push_back(queueCreateInfo);
	}
	deviceCreateInfo.setQueueCreateInfos(queueInfos);

	/* �����߼��豸 */
	return phyDevice.createDevice(deviceCreateInfo);
}

void Context::queryQueueInfo(vk::SurfaceKHR surface) {
	// ��ȡ�����豸֧�ֵ�����������е�����
	auto properties = phyDevice.getQueueFamilyProperties();

	for (int i = 0; i < properties.size(); i++) {
		const auto& property = properties[i];
		// ͨ��λ���㣬�жϵ�ǰ��������Ƿ�֧��ĳһ����
		if (property.queueFlags & vk::QueueFlagBits::eGraphics) {
			queueInfo.graphicsIndex = i;
		}
		// �жϵ�ǰ�����豸�ĵ�i���������, �Ƿ�֧�ֵ�ǰsurface��չ
		if (phyDevice.getSurfaceSupportKHR(i, surface)) {
			queueInfo.presentIndex = i;
		}
		// �ҵ��������������, �˳�ѭ��
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
	renderProcess->CreateGraphicsPipeline(*shader);
}

void Context::initCommandPool() {
	commandManager = std::make_unique<CommandManager>();
}

void Context::initShaderModules() {
	auto vertexSource = ReadWholeFile("./shader/vert.spv");
	auto fragSource = ReadWholeFile("./shader/frag.spv");
	shader = std::make_unique<Shader>(vertexSource, fragSource);
}

void Context::initSampler() {
	vk::SamplerCreateInfo createInfo;
	createInfo.setMagFilter(vk::Filter::eLinear)			// �Ŵ������: ���Բ�ֵ
		.setMinFilter(vk::Filter::eLinear)					// ��С������: ���Բ�ֵ
		.setAddressModeU(vk::SamplerAddressMode::eRepeat)	// U�����: �ظ�
		.setAddressModeV(vk::SamplerAddressMode::eRepeat)	// V�����: �ظ�
		.setAddressModeW(vk::SamplerAddressMode::eRepeat)	// W�����: �ظ�
		.setAnisotropyEnable(false)							// ���ø������Թ���
		.setBorderColor(vk::BorderColor::eIntOpaqueBlack)	// �߽���ɫ
		.setUnnormalizedCoordinates(false)					// �Ƿ�ʹ�ù�һ������
		.setCompareEnable(false)							// �Ƿ����ñȽ�
		.setMipmapMode(vk::SamplerMipmapMode::eLinear);		// Mipmap����ģʽ
	sampler = Context::Instance().device.createSampler(createInfo);
}

}
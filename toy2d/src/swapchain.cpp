#include "swapchain.h"
#include "context.h"

namespace toy2d {
Swapchain::Swapchain(vk::SurfaceKHR surface, int w, int h) {
	this->surface = surface;
	querySurfaceInfo(w, h);
	swapchain = createSwapchain();
	createImageAndViews();
}

Swapchain::~Swapchain() {
	auto& device = Context::GetInstance().device;

	for (auto& img : images) 
		device.destroyImageView(img.view);
	for (auto& framebuffer : framebuffers)
		device.destroyFramebuffer(framebuffer);

	device.destroySwapchainKHR(swapchain);
	Context::GetInstance().instance.destroySurfaceKHR(surface);
}

void Swapchain::InitFramebuffers() {
	createFramebuffers();
}

void Swapchain::querySurfaceInfo(int w, int h) {
	surfaceInfo_.format = querySurfaceFormat();

	// 查询surface支持的图像个数
	auto capability = Context::GetInstance().phyDevice.getSurfaceCapabilitiesKHR(surface);
	surfaceInfo_.count = std::clamp<uint32_t>(capability.minImageCount + 1, capability.minImageCount, capability.maxImageCount);
	// 查询surface将图像贴到屏幕上之前, 对图像做的变换
	surfaceInfo_.transform = capability.currentTransform;
	// 查询surface支持的图像大小
	surfaceInfo_.extent = querySurfaceExtent(capability, w, h);
	// 查询surface的显示模式
	auto presents = Context::GetInstance().phyDevice.getSurfacePresentModesKHR(surface);
	surfaceInfo_.present = vk::PresentModeKHR::eFifo;
	for (const auto& present : presents) {
		// vk::PresentModeKHR::eFifo		-- 先进先出(默认模式)
		// vk::PresentModeKHR::eFifoRelaxed	-- 如果当前图像没有绘制完, 则舍弃当前图像的绘制, 绘制新图片
		// vk::PresentModeKHR::eImmediate	-- 立刻进行绘制
		// vk::PresentModeKHR::eMailb		-- 每次从信箱里面取一个图像, 新图像放在信箱中, 信箱中仅会保留最新的图像
		if (present == vk::PresentModeKHR::eMailbox) {
			surfaceInfo_.present = present;
			break;
		}
	}
}

vk::SurfaceFormatKHR Swapchain::querySurfaceFormat() {
	auto formats = Context::GetInstance().phyDevice.getSurfaceFormatsKHR(surface);
	for (const auto& format : formats) {
		if (format.format == vk::Format::eR8G8B8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
			return format;
		}
	}
	return formats[0];
}

vk::Extent2D Swapchain::querySurfaceExtent(const vk::SurfaceCapabilitiesKHR& capability, int w, int h) {
	if (capability.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capability.currentExtent;
	}
	else {
		auto extent = vk::Extent2D{
			static_cast<uint32_t>(w),
			static_cast<uint32_t>(h)
		};
		extent.width = std::clamp(extent.width, capability.minImageExtent.width, capability.maxImageExtent.width);
		extent.height = std::clamp(extent.height, capability.minImageExtent.height, capability.maxImageExtent.height);
		return extent;
	}
}

vk::SwapchainKHR Swapchain::createSwapchain() {
	/* 交换链配置 */
	vk::SwapchainCreateInfoKHR createInfo;
	createInfo.setClipped(true)						// GPU上的图像如果大于屏幕, 则进行裁剪
		.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)  // 将图像绘制到窗口上时, 怎样与原有图像进行混合: 不混合
		.setImageExtent(surfaceInfo_.extent)
		.setImageColorSpace(surfaceInfo_.format.colorSpace)
		.setImageFormat(surfaceInfo_.format.format)
		.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)	// 图像用途: 颜色附件, 允许GPU在其上面绘制像素点
		.setMinImageCount(surfaceInfo_.count)
		.setImageArrayLayers(1)						// 图像的layer数量, 每一个layer都是一个image数组
		.setPresentMode(surfaceInfo_.present)		// 显示模式
		.setPreTransform(surfaceInfo_.transform)	// 图像变换
		.setSurface(surface);						// 设置对应的surface

	auto queueInfo = Context::GetInstance().queueInfo;
	if (queueInfo.isSameQueue()) {
		createInfo.setQueueFamilyIndices(queueInfo.graphicsIndex.value())
			.setImageSharingMode(vk::SharingMode::eExclusive); // 同时只能被一个队列使用
	}
	else {
		std::array indices = {
			queueInfo.graphicsIndex.value(),
			queueInfo.presentIndex.value()
		};
		createInfo.setQueueFamilyIndices(indices)
			.setImageSharingMode(vk::SharingMode::eConcurrent);	// 可以并行使用
	}

	/* 创建交换链 */
	return Context::GetInstance().device.createSwapchainKHR(createInfo);
}

void Swapchain::createImageAndViews() {
	auto images = Context::GetInstance().device.getSwapchainImagesKHR(swapchain);
	for (auto& image : images) {
		Image img;
		img.image = image;

		/* 图像 => 视图 的通道映射*/
		vk::ComponentMapping mapping; // 默认不改变映射关系

		/* 多级渐变纹理 */
		vk::ImageSubresourceRange range;
		range.setBaseMipLevel(0)	// 基础级别
			.setBaseArrayLayer(0)	// 基础图形所在层级
			.setLevelCount(1)		// 级别数目
			.setLayerCount(1)		// 层级数量
			.setAspectMask(vk::ImageAspectFlagBits::eColor); // 图像的用途

		/* 视图配置 */
		vk::ImageViewCreateInfo viewCreateInfo;
		viewCreateInfo.setImage(image)				// 视图对应的图像
			.setFormat(surfaceInfo_.format.format)	// 视图格式
			.setViewType(vk::ImageViewType::e2D)	// 视图格式: 2D图像
			.setComponents(mapping)					// 映射关系
			.setSubresourceRange(range);			// 多级渐变纹理设置

		/* 创建视图 */
		img.view = Context::GetInstance().device.createImageView(viewCreateInfo);
		this->images.push_back(img);
	}
}

void Swapchain::createFramebuffers() {
	for (auto& img : images) {
		auto& view = img.view;

		vk::FramebufferCreateInfo createInfo;
		createInfo.setAttachments(view) // 附件: 第i个image
			.setLayers(1)
			.setWidth(GetExtent().width)
			.setHeight(GetExtent().height)
			.setRenderPass(Context::GetInstance().renderProcess->renderPass); // 绘制流程: 在renderpass中确定framebuffer的作用

		framebuffers.push_back(Context::GetInstance().device.createFramebuffer(createInfo));
	}
}


}
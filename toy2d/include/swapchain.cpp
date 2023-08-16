#include "swapchain.h"
#include "context.h"

namespace toy2d {
Swapchain::Swapchain(int w, int h) {
	queryInfo(w, h);

	/* 交换链配置 */
	vk::SwapchainCreateInfoKHR createInfo;
	createInfo.setClipped(true)		// GPU上的图像如果大于屏幕, 则进行裁剪
		.setImageArrayLayers(1)		// 图像的layer数量, 每一个layer都是一个image数组
		.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)	// 图像用途: 颜色附件, 允许GPU在其上面绘制像素点
		.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)  // 将图像绘制到窗口上时, 怎样与原有图像进行混合: 不混合
		.setSurface(Context::GetInstance().surface)					// 设置对应的surface
		.setImageColorSpace(info.format.colorSpace)
		.setImageFormat(info.format.format)
		.setImageExtent(info.imageExtent)
		.setMinImageCount(info.imageCount)
		.setPresentMode(info.present);

	auto queueIndicecs = Context::GetInstance().queueFamilyIndices;
	if (queueIndicecs.isSameQueue()) {
		createInfo.setQueueFamilyIndices(queueIndicecs.graphicsQueue.value())
			.setImageSharingMode(vk::SharingMode::eExclusive); // 同时只能被一个队列使用
	}
	else {
		std::array indices = {
			queueIndicecs.graphicsQueue.value(),
			queueIndicecs.presentQueue.value()
		};
		createInfo.setQueueFamilyIndices(indices)
			.setImageSharingMode(vk::SharingMode::eConcurrent);	// 可以并行使用
	}

	/* 创建交换链 */
	swapchain = Context::GetInstance().device.createSwapchainKHR(createInfo);

	/* 创建图像视图 */
	getImages();
	createImageViews();
}

Swapchain::~Swapchain() {
	// 销毁图像视图
	for (auto& view : imageViews) 
		Context::GetInstance().device.destroyImageView(view);

	// 销毁交换链
	Context::GetInstance().device.destroySwapchainKHR(swapchain);
}

void Swapchain::queryInfo(int w, int h) {
	auto& phyDevice = Context::GetInstance().phyDevice;
	auto& surface = Context::GetInstance().surface;

	// 查询surface支持的颜色格式
	auto formats = phyDevice.getSurfaceFormatsKHR(surface);
	info.format = formats[0];
	for (const auto& format : formats) {
		if (format.format == vk::Format::eR8G8B8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
			info.format = format;
			break;
		}
	}

	// 查询surface支持的图像个数
	auto capabilities = phyDevice.getSurfaceCapabilitiesKHR(surface);
	info.imageCount = std::clamp<uint32_t>(2, capabilities.minImageCount, capabilities.maxImageCount);

	// 查询surface支持的图像大小
	info.imageExtent.width = std::clamp<uint32_t>(w, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	info.imageExtent.height = std::clamp<uint32_t>(h, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

	// 查询surface将图像贴到屏幕上之前, 对图像做的变换
	info.transform = capabilities.currentTransform;

	// 查询surface的显示模式
	auto presents = phyDevice.getSurfacePresentModesKHR(surface);
	info.present = vk::PresentModeKHR::eFifo;
	for (const auto& present : presents) {
		// vk::PresentModeKHR::eFifo		-- 先进先出(默认模式)
		// vk::PresentModeKHR::eFifoRelaxed	-- 如果当前图像没有绘制完, 则舍弃当前图像的绘制, 绘制新图片
		// vk::PresentModeKHR::eImmediate	-- 立刻进行绘制
		// vk::PresentModeKHR::eMailb		-- 每次从信箱里面取一个图像, 新图像放在信箱中, 信箱中仅会保留最新的图像
		if (present == vk::PresentModeKHR::eMailbox) {
			info.present = present;
			break;
		}
	}
}
void Swapchain::getImages() {
	images = Context::GetInstance().device.getSwapchainImagesKHR(swapchain);
}
void Swapchain::createImageViews() {
	imageViews.resize(images.size());

	for (int i = 0; i < images.size(); i++) {
		/* 图像 => 视图 的通道映射*/
		vk::ComponentMapping mapping; // 默认不改变映射关系

		/* 多级渐变纹理 */
		vk::ImageSubresourceRange range;
		range.setBaseMipLevel(0)	// 基础级别
			.setLevelCount(1)		// 级别数目
			.setBaseArrayLayer(0)	// 基础图形所在层级
			.setLayerCount(1)		// 层级数量
			.setAspectMask(vk::ImageAspectFlagBits::eColor); // 图像的用途

		/* 视图配置 */
		vk::ImageViewCreateInfo createInfo;
		createInfo.setImage(images[i])			// 视图对应的图像
			.setViewType(vk::ImageViewType::e2D)// 视图格式: 2D图像
			.setComponents(mapping)				// 映射关系
			.setFormat(info.format.format)		// 视图格式
			.setSubresourceRange(range);		// 多级渐变纹理设置

		/* 创建视图 */
		imageViews[i] = Context::GetInstance().device.createImageView(createInfo);
	}
}
}
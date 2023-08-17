#include "swapchain.h"
#include "context.h"

namespace toy2d {
Swapchain::Swapchain(int w, int h) {
	queryInfo(w, h);

	/* ���������� */
	vk::SwapchainCreateInfoKHR createInfo;
	createInfo.setClipped(true)		// GPU�ϵ�ͼ�����������Ļ, ����вü�
		.setImageArrayLayers(1)		// ͼ���layer����, ÿһ��layer����һ��image����
		.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)	// ͼ����;: ��ɫ����, ����GPU��������������ص�
		.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)  // ��ͼ����Ƶ�������ʱ, ������ԭ��ͼ����л��: �����
		.setSurface(Context::GetInstance().surface)					// ���ö�Ӧ��surface
		.setImageColorSpace(info.format.colorSpace)
		.setImageFormat(info.format.format)
		.setImageExtent(info.imageExtent)
		.setMinImageCount(info.imageCount)
		.setPresentMode(info.present);

	auto queueIndicecs = Context::GetInstance().queueFamilyIndices;
	if (queueIndicecs.isSameQueue()) {
		createInfo.setQueueFamilyIndices(queueIndicecs.graphicsQueue.value())
			.setImageSharingMode(vk::SharingMode::eExclusive); // ͬʱֻ�ܱ�һ������ʹ��
	}
	else {
		std::array indices = {
			queueIndicecs.graphicsQueue.value(),
			queueIndicecs.presentQueue.value()
		};
		createInfo.setQueueFamilyIndices(indices)
			.setImageSharingMode(vk::SharingMode::eConcurrent);	// ���Բ���ʹ��
	}

	/* ���������� */
	swapchain = Context::GetInstance().device.createSwapchainKHR(createInfo);

	/* ����ͼ����ͼ */
	getImages();
	createImageViews();
}

Swapchain::~Swapchain() {
	// ����ͼ����ͼ
	for (auto& view : imageViews) 
		Context::GetInstance().device.destroyImageView(view);

	// ���ٽ�����
	Context::GetInstance().device.destroySwapchainKHR(swapchain);
}

void Swapchain::queryInfo(int w, int h) {
	auto& phyDevice = Context::GetInstance().phyDevice;
	auto& surface = Context::GetInstance().surface;

	// ��ѯsurface֧�ֵ���ɫ��ʽ
	auto formats = phyDevice.getSurfaceFormatsKHR(surface);
	info.format = formats[0];
	for (const auto& format : formats) {
		if (format.format == vk::Format::eR8G8B8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
			info.format = format;
			break;
		}
	}

	// ��ѯsurface֧�ֵ�ͼ�����
	auto capabilities = phyDevice.getSurfaceCapabilitiesKHR(surface);
	info.imageCount = std::clamp<uint32_t>(2, capabilities.minImageCount, capabilities.maxImageCount);

	// ��ѯsurface֧�ֵ�ͼ���С
	info.imageExtent.width = std::clamp<uint32_t>(w, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	info.imageExtent.height = std::clamp<uint32_t>(h, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

	// ��ѯsurface��ͼ��������Ļ��֮ǰ, ��ͼ�����ı任
	info.transform = capabilities.currentTransform;

	// ��ѯsurface����ʾģʽ
	auto presents = phyDevice.getSurfacePresentModesKHR(surface);
	info.present = vk::PresentModeKHR::eFifo;
	for (const auto& present : presents) {
		// vk::PresentModeKHR::eFifo		-- �Ƚ��ȳ�(Ĭ��ģʽ)
		// vk::PresentModeKHR::eFifoRelaxed	-- �����ǰͼ��û�л�����, ��������ǰͼ��Ļ���, ������ͼƬ
		// vk::PresentModeKHR::eImmediate	-- ���̽��л���
		// vk::PresentModeKHR::eMailb		-- ÿ�δ���������ȡһ��ͼ��, ��ͼ�����������, �����н��ᱣ�����µ�ͼ��
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
		/* ͼ�� => ��ͼ ��ͨ��ӳ��*/
		vk::ComponentMapping mapping; // Ĭ�ϲ��ı�ӳ���ϵ

		/* �༶�������� */
		vk::ImageSubresourceRange range;
		range.setBaseMipLevel(0)	// ��������
			.setLevelCount(1)		// ������Ŀ
			.setBaseArrayLayer(0)	// ����ͼ�����ڲ㼶
			.setLayerCount(1)		// �㼶����
			.setAspectMask(vk::ImageAspectFlagBits::eColor); // ͼ�����;

		/* ��ͼ���� */
		vk::ImageViewCreateInfo createInfo;
		createInfo.setImage(images[i])			// ��ͼ��Ӧ��ͼ��
			.setViewType(vk::ImageViewType::e2D)// ��ͼ��ʽ: 2Dͼ��
			.setComponents(mapping)				// ӳ���ϵ
			.setFormat(info.format.format)		// ��ͼ��ʽ
			.setSubresourceRange(range);		// �༶������������

		/* ������ͼ */
		imageViews[i] = Context::GetInstance().device.createImageView(createInfo);
	}
}
}
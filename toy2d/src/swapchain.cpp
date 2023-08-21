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

	// ��ѯsurface֧�ֵ�ͼ�����
	auto capability = Context::GetInstance().phyDevice.getSurfaceCapabilitiesKHR(surface);
	surfaceInfo_.count = std::clamp<uint32_t>(capability.minImageCount + 1, capability.minImageCount, capability.maxImageCount);
	// ��ѯsurface��ͼ��������Ļ��֮ǰ, ��ͼ�����ı任
	surfaceInfo_.transform = capability.currentTransform;
	// ��ѯsurface֧�ֵ�ͼ���С
	surfaceInfo_.extent = querySurfaceExtent(capability, w, h);
	// ��ѯsurface����ʾģʽ
	auto presents = Context::GetInstance().phyDevice.getSurfacePresentModesKHR(surface);
	surfaceInfo_.present = vk::PresentModeKHR::eFifo;
	for (const auto& present : presents) {
		// vk::PresentModeKHR::eFifo		-- �Ƚ��ȳ�(Ĭ��ģʽ)
		// vk::PresentModeKHR::eFifoRelaxed	-- �����ǰͼ��û�л�����, ��������ǰͼ��Ļ���, ������ͼƬ
		// vk::PresentModeKHR::eImmediate	-- ���̽��л���
		// vk::PresentModeKHR::eMailb		-- ÿ�δ���������ȡһ��ͼ��, ��ͼ�����������, �����н��ᱣ�����µ�ͼ��
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
	/* ���������� */
	vk::SwapchainCreateInfoKHR createInfo;
	createInfo.setClipped(true)						// GPU�ϵ�ͼ�����������Ļ, ����вü�
		.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)  // ��ͼ����Ƶ�������ʱ, ������ԭ��ͼ����л��: �����
		.setImageExtent(surfaceInfo_.extent)
		.setImageColorSpace(surfaceInfo_.format.colorSpace)
		.setImageFormat(surfaceInfo_.format.format)
		.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)	// ͼ����;: ��ɫ����, ����GPU��������������ص�
		.setMinImageCount(surfaceInfo_.count)
		.setImageArrayLayers(1)						// ͼ���layer����, ÿһ��layer����һ��image����
		.setPresentMode(surfaceInfo_.present)		// ��ʾģʽ
		.setPreTransform(surfaceInfo_.transform)	// ͼ��任
		.setSurface(surface);						// ���ö�Ӧ��surface

	auto queueInfo = Context::GetInstance().queueInfo;
	if (queueInfo.isSameQueue()) {
		createInfo.setQueueFamilyIndices(queueInfo.graphicsIndex.value())
			.setImageSharingMode(vk::SharingMode::eExclusive); // ͬʱֻ�ܱ�һ������ʹ��
	}
	else {
		std::array indices = {
			queueInfo.graphicsIndex.value(),
			queueInfo.presentIndex.value()
		};
		createInfo.setQueueFamilyIndices(indices)
			.setImageSharingMode(vk::SharingMode::eConcurrent);	// ���Բ���ʹ��
	}

	/* ���������� */
	return Context::GetInstance().device.createSwapchainKHR(createInfo);
}

void Swapchain::createImageAndViews() {
	auto images = Context::GetInstance().device.getSwapchainImagesKHR(swapchain);
	for (auto& image : images) {
		Image img;
		img.image = image;

		/* ͼ�� => ��ͼ ��ͨ��ӳ��*/
		vk::ComponentMapping mapping; // Ĭ�ϲ��ı�ӳ���ϵ

		/* �༶�������� */
		vk::ImageSubresourceRange range;
		range.setBaseMipLevel(0)	// ��������
			.setBaseArrayLayer(0)	// ����ͼ�����ڲ㼶
			.setLevelCount(1)		// ������Ŀ
			.setLayerCount(1)		// �㼶����
			.setAspectMask(vk::ImageAspectFlagBits::eColor); // ͼ�����;

		/* ��ͼ���� */
		vk::ImageViewCreateInfo viewCreateInfo;
		viewCreateInfo.setImage(image)				// ��ͼ��Ӧ��ͼ��
			.setFormat(surfaceInfo_.format.format)	// ��ͼ��ʽ
			.setViewType(vk::ImageViewType::e2D)	// ��ͼ��ʽ: 2Dͼ��
			.setComponents(mapping)					// ӳ���ϵ
			.setSubresourceRange(range);			// �༶������������

		/* ������ͼ */
		img.view = Context::GetInstance().device.createImageView(viewCreateInfo);
		this->images.push_back(img);
	}
}

void Swapchain::createFramebuffers() {
	for (auto& img : images) {
		auto& view = img.view;

		vk::FramebufferCreateInfo createInfo;
		createInfo.setAttachments(view) // ����: ��i��image
			.setLayers(1)
			.setWidth(GetExtent().width)
			.setHeight(GetExtent().height)
			.setRenderPass(Context::GetInstance().renderProcess->renderPass); // ��������: ��renderpass��ȷ��framebuffer������

		framebuffers.push_back(Context::GetInstance().device.createFramebuffer(createInfo));
	}
}


}
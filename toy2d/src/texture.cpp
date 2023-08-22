#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace toy2d {

Texture::Texture(std::string_view filename) {
	// ʹ��stb_image����ͼƬ, ��ת��ΪRGBA��ʽ
	int w, h, channel;
	stbi_uc* pixels = stbi_load(filename.data(), &w, &h, &channel, STBI_rgb_alpha);
	if (!pixels)
		throw std::runtime_error("image load failed");

	// ��ͼ�����ݷŵ�buffer��
	size_t size = w * h * 4;
	std::unique_ptr<Buffer> buffer(new Buffer(
		vk::BufferUsageFlagBits::eTransferSrc,
		size,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	));
	memcpy(buffer->map, pixels, size);

	// ����Image
	createImage(w, h);
	// �����ڴ�
	allocateMemory();
	// ���ڴ�
	Context::Instance().device.bindImageMemory(image, memory, 0);

	// ��Image�Ĳ��ִ�Undefinedת��ΪTransferDstOptimal
	transitionImageLayoutFromUndifine2Dst();
	// CPU���� => GPU Image
	transformData2Image(*buffer, w, h);
	// ��Image�Ĳ��ִ�TransferDstOptimalת��ΪShaderReadOnlyOptimal
	transitionImageLayoutFromDst2Optimal();

	// ����ImageView
	createImageView();

	// �ͷ�ͼ������
	stbi_image_free(pixels);
}

Texture::~Texture() {
	auto& device = Context::Instance().device;
	device.destroyImageView(view);
	device.freeMemory(memory);
	device.destroyImage(image);
}

void Texture::createImage(uint32_t w, uint32_t h) {
	vk::ImageCreateInfo createInfo;
	createInfo.setImageType(vk::ImageType::e2D)			// ͼ������
		.setArrayLayers(1)								// ͼ������㼶(������Ԫ�ظ���)
		.setMipLevels(1)								// mipmap�㼶
		.setExtent(vk::Extent3D(w, h, 1))				// ͼ���С
		.setFormat(vk::Format::eR8G8B8A8Srgb)			// ͼ���ʽ
		.setTiling(vk::ImageTiling::eOptimal)			// ͼ��ƽ�̷�ʽ(���洢��ʽ)
		.setInitialLayout(vk::ImageLayout::eUndefined)	// ͼ���ʼ����
		.setUsage(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled) // ͼ����;
		.setSamples(vk::SampleCountFlagBits::e1);		// ������
	image = Context::Instance().device.createImage(createInfo);
}

void Texture::allocateMemory() {
	auto& device = Context::Instance().device;
	vk::MemoryAllocateInfo allocInfo;

	// ��ȡ������image���ڴ�����
	auto requirements = device.getImageMemoryRequirements(image);
	allocInfo.setAllocationSize(requirements.size);

	// ��ȡ�ڴ���������
	auto index = QueryBufferMemTypeIndex(requirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
	allocInfo.setMemoryTypeIndex(index);

	memory = device.allocateMemory(allocInfo);
}

void Texture::transitionImageLayoutFromUndifine2Dst() {
	Context::Instance().commandManager->ExecuteCommand(Context::Instance().graphicsQueue,
		[&](vk::CommandBuffer cmdBuf) {
			vk::ImageSubresourceRange range;
			range.setLayerCount(1)
				.setBaseArrayLayer(0)
				.setLevelCount(1)
				.setBaseMipLevel(0)
				.setAspectMask(vk::ImageAspectFlagBits::eColor);
			
			vk::ImageMemoryBarrier barrier;
			barrier.setImage(image)										// �ȴ��ĸ�image
				.setOldLayout(vk::ImageLayout::eUndefined)				// imageԭ���Ĳ���
				.setNewLayout(vk::ImageLayout::eTransferDstOptimal)		// image�µĲ���
				.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)		// Դ����
				.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)		// Ŀ�����
				.setDstAccessMask(vk::AccessFlagBits::eTransferWrite)	// Ŀ�����Ȩ��: ����д��
				.setSubresourceRange(range);
			
			cmdBuf.pipelineBarrier(
				vk::PipelineStageFlagBits::eTopOfPipe,	// ��pipeline��ʼʱ���г���ת��
				vk::PipelineStageFlagBits::eTransfer,	// ת��Ϊ���ݴ���
				{}, {}, nullptr, barrier);
		});
}

void Texture::transformData2Image(Buffer& buffer, uint32_t w, uint32_t h) {
	Context::Instance().commandManager->ExecuteCommand(Context::Instance().graphicsQueue,
		[&](vk::CommandBuffer cmdBuf) {
			vk::ImageSubresourceLayers subsource;
			subsource.setAspectMask(vk::ImageAspectFlagBits::eColor)
				.setBaseArrayLayer(0)
				.setMipLevel(0)
				.setLayerCount(1);
			
			vk::BufferImageCopy region;
			region.setBufferImageHeight(0)	// ͼ��߶�, 0��ʾͼ��Ϊ��������, RGBA��ʽ������padding
				.setBufferOffset(0)			// image�����buffer��ƫ��
				.setImageOffset(0)			// imageƫ��
				.setImageExtent({ w, h, 1 })// image��С
				.setBufferRowLength(0)		// buffer�г���, 0��ʾͼ��Ϊ��������
				.setImageSubresource(subsource);

			cmdBuf.copyBufferToImage(buffer.buffer, image, vk::ImageLayout::eTransferDstOptimal, region);
		});
}

void Texture::transitionImageLayoutFromDst2Optimal() {
	Context::Instance().commandManager->ExecuteCommand(Context::Instance().graphicsQueue,
		[&](vk::CommandBuffer cmdBuf) {
			vk::ImageSubresourceRange range;
			range.setLayerCount(1)
				.setBaseArrayLayer(0)
				.setLevelCount(1)
				.setBaseMipLevel(0)
				.setAspectMask(vk::ImageAspectFlagBits::eColor);

			vk::ImageMemoryBarrier barrier;
			barrier.setImage(image)										// �ȴ��ĸ�image
				.setOldLayout(vk::ImageLayout::eTransferDstOptimal)		// imageԭ���Ĳ���
				.setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)	// image�µĲ���
				.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)		// Դ����
				.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)		// Ŀ�����
				.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)	// Դ����Ȩ��: ����д��
				.setDstAccessMask(vk::AccessFlagBits::eShaderRead)		// Ŀ�����Ȩ��: ���Զ�ȡ
				.setSubresourceRange(range);

			cmdBuf.pipelineBarrier(
				vk::PipelineStageFlagBits::eTransfer,	
				vk::PipelineStageFlagBits::eFragmentShader,	
				{}, {}, nullptr, barrier);
		});
}


void Texture::createImageView() {
	vk::ImageViewCreateInfo createInfo;
	vk::ComponentMapping mapping;
	vk::ImageSubresourceRange range;
	range.setAspectMask(vk::ImageAspectFlagBits::eColor)
		.setBaseArrayLayer(0)
		.setLayerCount(1)
		.setLevelCount(1)
		.setBaseMipLevel(0);
	createInfo.setImage(image)
		.setViewType(vk::ImageViewType::e2D)
		.setComponents(mapping)
		.setFormat(vk::Format::eR8G8B8A8Srgb)
		.setSubresourceRange(range);
	view = Context::Instance().device.createImageView(createInfo);
}

}
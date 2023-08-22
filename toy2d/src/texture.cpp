#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace toy2d {

Texture::Texture(std::string_view filename) {
	// 使用stb_image加载图片, 并转换为RGBA格式
	int w, h, channel;
	stbi_uc* pixels = stbi_load(filename.data(), &w, &h, &channel, STBI_rgb_alpha);
	if (!pixels)
		throw std::runtime_error("image load failed");

	// 将图形数据放到buffer中
	size_t size = w * h * 4;
	std::unique_ptr<Buffer> buffer(new Buffer(
		vk::BufferUsageFlagBits::eTransferSrc,
		size,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	));
	memcpy(buffer->map, pixels, size);

	// 创建Image
	createImage(w, h);
	// 分配内存
	allocateMemory();
	// 绑定内存
	Context::Instance().device.bindImageMemory(image, memory, 0);

	// 将Image的布局从Undefined转换为TransferDstOptimal
	transitionImageLayoutFromUndifine2Dst();
	// CPU数据 => GPU Image
	transformData2Image(*buffer, w, h);
	// 将Image的布局从TransferDstOptimal转换为ShaderReadOnlyOptimal
	transitionImageLayoutFromDst2Optimal();

	// 创建ImageView
	createImageView();

	// 释放图形数据
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
	createInfo.setImageType(vk::ImageType::e2D)			// 图像类型
		.setArrayLayers(1)								// 图像数组层级(即数组元素个数)
		.setMipLevels(1)								// mipmap层级
		.setExtent(vk::Extent3D(w, h, 1))				// 图像大小
		.setFormat(vk::Format::eR8G8B8A8Srgb)			// 图像格式
		.setTiling(vk::ImageTiling::eOptimal)			// 图像平铺方式(即存储方式)
		.setInitialLayout(vk::ImageLayout::eUndefined)	// 图像初始布局
		.setUsage(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled) // 图像用途
		.setSamples(vk::SampleCountFlagBits::e1);		// 采样数
	image = Context::Instance().device.createImage(createInfo);
}

void Texture::allocateMemory() {
	auto& device = Context::Instance().device;
	vk::MemoryAllocateInfo allocInfo;

	// 获取创建该image的内存需求
	auto requirements = device.getImageMemoryRequirements(image);
	allocInfo.setAllocationSize(requirements.size);

	// 获取内存类型索引
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
			barrier.setImage(image)										// 等待哪个image
				.setOldLayout(vk::ImageLayout::eUndefined)				// image原来的布局
				.setNewLayout(vk::ImageLayout::eTransferDstOptimal)		// image新的布局
				.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)		// 源队列
				.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)		// 目标队列
				.setDstAccessMask(vk::AccessFlagBits::eTransferWrite)	// 目标访问权限: 可以写入
				.setSubresourceRange(range);
			
			cmdBuf.pipelineBarrier(
				vk::PipelineStageFlagBits::eTopOfPipe,	// 在pipeline开始时进行场景转换
				vk::PipelineStageFlagBits::eTransfer,	// 转换为数据传输
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
			region.setBufferImageHeight(0)	// 图像高度, 0表示图形为紧密排列, RGBA格式不会有padding
				.setBufferOffset(0)			// image相对于buffer的偏移
				.setImageOffset(0)			// image偏移
				.setImageExtent({ w, h, 1 })// image大小
				.setBufferRowLength(0)		// buffer行长度, 0表示图形为紧密排列
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
			barrier.setImage(image)										// 等待哪个image
				.setOldLayout(vk::ImageLayout::eTransferDstOptimal)		// image原来的布局
				.setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)	// image新的布局
				.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)		// 源队列
				.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)		// 目标队列
				.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)	// 源访问权限: 可以写入
				.setDstAccessMask(vk::AccessFlagBits::eShaderRead)		// 目标访问权限: 可以读取
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
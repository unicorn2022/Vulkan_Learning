#include "render_process.h"
#include "context.h"
#include "swapchain.h"
#include "vertex.h"

namespace toy2d {

RenderProcess::RenderProcess() {
	layout = createLayout();
	renderPass = createRenderPass();
	graphicsPipeline = nullptr;
}

RenderProcess::~RenderProcess() {
	auto& device = Context::GetInstance().device;
	device.destroyRenderPass(renderPass);
	device.destroyPipelineLayout(layout);
	device.destroyPipeline(graphicsPipeline);
}

void RenderProcess::RecreateGraphicsPipeline(const std::vector<char>& vertexSource, const std::vector<char>& fragSource) {
	if (graphicsPipeline) {
		Context::GetInstance().device.destroyPipeline(graphicsPipeline);
	}
	graphicsPipeline = createGraphicsPipeline(vertexSource, fragSource);
}

void RenderProcess::RecreateRenderPass() {
	if (renderPass) {
		Context::GetInstance().device.destroyRenderPass(renderPass);
	}
	renderPass = createRenderPass();
}

vk::PipelineLayout RenderProcess::createLayout() {
	vk::PipelineLayoutCreateInfo createInfo;

	createInfo.setPushConstantRangeCount(0)
		.setSetLayoutCount(0);

	return Context::GetInstance().device.createPipelineLayout(createInfo);
}

vk::Pipeline RenderProcess::createGraphicsPipeline(const std::vector<char>& vertexSource, const std::vector<char>& fragSource) {
	/* 流水线配置 */
	vk::GraphicsPipelineCreateInfo createInfo;

	// 0. 创建着色器
	vk::ShaderModuleCreateInfo vertexModuleCreateInfo, fragModuleCreateInfo;
	vertexModuleCreateInfo.setCodeSize(vertexSource.size())
		.setPCode((std::uint32_t*)vertexSource.data());
	fragModuleCreateInfo.setCodeSize(fragSource.size())
		.setPCode((std::uint32_t*)fragSource.data());
	auto vertexModule = Context::GetInstance().device.createShaderModule(vertexModuleCreateInfo);
	auto fragModule = Context::GetInstance().device.createShaderModule(fragModuleCreateInfo);

	std::array<vk::PipelineShaderStageCreateInfo, 2> stageCreateInfos;
	stageCreateInfos[0].setModule(vertexModule)
		.setPName("main")
		.setStage(vk::ShaderStageFlagBits::eVertex);
	stageCreateInfos[1].setModule(fragModule)
		.setPName("main")
		.setStage(vk::ShaderStageFlagBits::eFragment);
	createInfo.setStages(stageCreateInfos);

	// 1. 顶点输入
	vk::PipelineVertexInputStateCreateInfo vertexInputCreateInfo;
	auto attribute = Vertex::GetAttribute();	// 顶点属性
	auto binding = Vertex::GetBinding();		// 顶点绑定
	vertexInputCreateInfo.setVertexAttributeDescriptions(attribute)
		.setVertexBindingDescriptions(binding);
	createInfo.setPVertexInputState(&vertexInputCreateInfo);

	// 2. 顶点聚集
	vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
	inputAssembly.setPrimitiveRestartEnable(false)
		.setTopology(vk::PrimitiveTopology::eTriangleList); // 顶点的聚集方式: 123, 456, 789
	createInfo.setPInputAssemblyState(&inputAssembly);

	// 3. 视口变换 & 裁剪
	// 3.1 视口 (起点, 宽高, 近远平面)
	vk::Viewport viewport(0, 0, Context::GetInstance().swapchain->GetExtent().width, Context::GetInstance().swapchain->GetExtent().height, 0, 1);
	vk::PipelineViewportStateCreateInfo viewportInfo;
	viewportInfo.setViewports(viewport);
	// 3.2 画中画
	vk::Rect2D scissor({ 0, 0 }, Context::GetInstance().swapchain->GetExtent());
	viewportInfo.setScissors(scissor);
	createInfo.setPViewportState(&viewportInfo);

	// 4. 光栅化
	vk::PipelineRasterizationStateCreateInfo rastInfo;
	rastInfo.setRasterizerDiscardEnable(false)			// 是否忽略光栅化结果
		.setCullMode(vk::CullModeFlagBits::eFront)		// 剔除背面
		.setFrontFace(vk::FrontFace::eCounterClockwise)	// 设置正面方向: 顺时针
		.setDepthClampEnable(false)						// 是否启用深度截取
		.setLineWidth(1)								// 线宽
		.setPolygonMode(vk::PolygonMode::eFill);		// 多边形填充模式: 填充
	createInfo.setPRasterizationState(&rastInfo);

	// 5. 多重采样
	vk::PipelineMultisampleStateCreateInfo multiSample;
	multiSample.setSampleShadingEnable(false) // 是否启用多重采样
		.setRasterizationSamples(vk::SampleCountFlagBits::e1); // 采样数量
	createInfo.setPMultisampleState(&multiSample);

	// 6. 测试: 深度测试, 模板测试

	// 7. 颜色混合
	vk::PipelineColorBlendAttachmentState attachs;
	attachs.setBlendEnable(false)	// 是否启用混合附件
		.setColorWriteMask(
			vk::ColorComponentFlagBits::eA |
			vk::ColorComponentFlagBits::eB |
			vk::ColorComponentFlagBits::eG |
			vk::ColorComponentFlagBits::eR); // 颜色写入掩码
	vk::PipelineColorBlendStateCreateInfo blendInfo;
	blendInfo.setLogicOpEnable(false)	// 是否启用逻辑运算
		.setAttachments(attachs);		// 颜色混合附件
	createInfo.setPColorBlendState(&blendInfo);

	// 8. render pass & layout
	createInfo.setRenderPass(renderPass)
		.setLayout(layout);


	/* 创建 pipeline */
	auto result = Context::GetInstance().device.createGraphicsPipeline(nullptr, createInfo);
	if (result.result != vk::Result::eSuccess) {
		std::cout << "create graphics pipeline failed: " << result.result << std::endl;
	}

	/* 释放着色器模块 */
	Context::GetInstance().device.destroyShaderModule(vertexModule);
	Context::GetInstance().device.destroyShaderModule(fragModule);

	return result.value;
}

vk::RenderPass RenderProcess::createRenderPass() {
	vk::RenderPassCreateInfo createInfo;
	/* 附件描述 */
	vk::AttachmentDescription attachDesc;
	attachDesc.setFormat(Context::GetInstance().swapchain->GetFormat().format)
		.setSamples(vk::SampleCountFlagBits::e1)	// 采样数量
		.setLoadOp(vk::AttachmentLoadOp::eClear)	// 颜色缓冲: 加载时清空
		.setStoreOp(vk::AttachmentStoreOp::eStore)	// 颜色缓冲: 存储时保留	
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)	// 模板缓冲
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)// 模板缓冲
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);
	createInfo.setAttachments(attachDesc);

	/* 子通道描述 */
	vk::AttachmentReference reference;
	reference.setLayout(vk::ImageLayout::eColorAttachmentOptimal) // 图像布局
		.setAttachment(0); // 附件索引
	vk::SubpassDescription subpassDesc;
	subpassDesc.setColorAttachments(reference)					 // 颜色附件
		.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics); // 绑定在哪一类流水线上
		
	createInfo.setSubpasses(subpassDesc);
	
	/* 子通道依赖 */
	vk::SubpassDependency dependency;
	dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL)	// 先执行的子通道: 外部通道
		.setDstSubpass(0)							// 后执行的子通道: 0号子通道
		.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput) // 子通道计算后的图像的应用场景
		.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);		// 子通道的访问权限
	createInfo.setDependencies(dependency);

	/* 创建渲染流程 */
	return Context::GetInstance().device.createRenderPass(createInfo);
}
}
#include "render_process.h"
#include "shader.h"
#include "context.h"
#include "swapchain.h"

namespace toy2d {

void RenderProcess::InitLayout() {
	vk::PipelineLayoutCreateInfo createInfo;
	layout = Context::GetInstance().device.createPipelineLayout(createInfo);
}

void RenderProcess::InitRenderPass() {
	vk::RenderPassCreateInfo createInfo;
	/* 附件描述 */
	vk::AttachmentDescription attachDesc;
	attachDesc.setFormat(Context::GetInstance().swapchain->info.format.format)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::ePresentSrcKHR)
		.setLoadOp(vk::AttachmentLoadOp::eClear)			// 颜色缓冲: 加载时清空
		.setStoreOp(vk::AttachmentStoreOp::eStore)			// 颜色缓冲: 存储时保留	
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)	// 模板缓冲
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)// 模板缓冲
		.setSamples(vk::SampleCountFlagBits::e1);			// 采样数量
	createInfo.setAttachments(attachDesc);

	/* 子通道描述 */
	vk::AttachmentReference reference;
	reference.setLayout(vk::ImageLayout::eColorAttachmentOptimal) // 图像布局
		.setAttachment(0); // 附件索引

	vk::SubpassDescription subpass;
	subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics) // 绑定在哪一类流水线上
		.setColorAttachments(reference); // 颜色附件
	createInfo.setSubpasses(subpass);
	
	/* 子通道依赖 */
	vk::SubpassDependency dependency;
	dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL)	// 先执行的子通道: 外部通道
		.setDstSubpass(0)							// 后执行的子通道: 0号子通道
		.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite) // 子通道的访问权限
		.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)		
		.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput); // 子通道计算后的图像的应用场景
	createInfo.setDependencies(dependency);

	/* 创建渲染流程 */
	renderPass = Context::GetInstance().device.createRenderPass(createInfo);
}

void RenderProcess::InitPipeline(int width, int height) {
	/* 流水线配置 */
	vk::GraphicsPipelineCreateInfo createInfo;

	// 1. 顶点输入
	vk::PipelineVertexInputStateCreateInfo inputState;
	createInfo.setPVertexInputState(&inputState);

	// 2. 顶点聚集
	vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
	inputAssembly.setPrimitiveRestartEnable(false)
		.setTopology(vk::PrimitiveTopology::eTriangleList); // 顶点的聚集方式: 123, 456, 789
	createInfo.setPInputAssemblyState(&inputAssembly);

	// 3. Shader
	auto stages = Shader::GetInstance().GetStage();
	createInfo.setStages(stages);

	// 4. 视口变换
	// 4.1 视口
	vk::Viewport viewport(0, 0, width, height, 0, 1);
	vk::PipelineViewportStateCreateInfo viewportState;
	viewportState.setViewports(viewport);
	// 4.2 画中画
	vk::Rect2D rect({ 0, 0 }, {static_cast<uint32_t>(width), static_cast<uint32_t>(height)});
	viewportState.setScissors(rect);
	createInfo.setPViewportState(&viewportState);

	// 5. 光栅化
	vk::PipelineRasterizationStateCreateInfo rastInfo;
	rastInfo.setRasterizerDiscardEnable(false)		// 是否忽略光栅化结果
		.setCullMode(vk::CullModeFlagBits::eBack)	// 剔除背面
		.setFrontFace(vk::FrontFace::eClockwise)	// 设置正面方向: 顺时针
		.setPolygonMode(vk::PolygonMode::eFill)		// 多边形填充模式: 填充
		.setLineWidth(1.0f);						// 线宽
	createInfo.setPRasterizationState(&rastInfo);

	// 6. 多重采样
	vk::PipelineMultisampleStateCreateInfo multiSample;
	multiSample.setSampleShadingEnable(false) // 是否启用多重采样
		.setRasterizationSamples(vk::SampleCountFlagBits::e1); // 采样数量
	createInfo.setPMultisampleState(&multiSample);

	// 7. 测试: 深度测试, 模板测试

	// 8. 颜色混合
	vk::PipelineColorBlendAttachmentState attachs;
	attachs.setBlendEnable(false)	// 是否启用混合附件
		.setColorWriteMask(
			vk::ColorComponentFlagBits::eA |
			vk::ColorComponentFlagBits::eB |
			vk::ColorComponentFlagBits::eG |
			vk::ColorComponentFlagBits::eR); // 颜色写入掩码
	vk::PipelineColorBlendStateCreateInfo blend;
	blend.setLogicOpEnable(false)	// 是否启用逻辑运算
		.setAttachments(attachs);	// 颜色混合附件
	createInfo.setPColorBlendState(&blend);

	// 9. render pass & layout
	createInfo.setRenderPass(renderPass)
		.setLayout(layout);


	/* 创建 pipeline */
	auto result = Context::GetInstance().device.createGraphicsPipeline(nullptr, createInfo);
	if (result.result != vk::Result::eSuccess) {
		throw std::runtime_error("create graphics pipeline failed");
	}
	pipeline = result.value;
}

RenderProcess::~RenderProcess() {
	auto& device = Context::GetInstance().device;
	device.destroyRenderPass(renderPass);
	device.destroyPipelineLayout(layout);
	device.destroyPipeline(pipeline);
}
}
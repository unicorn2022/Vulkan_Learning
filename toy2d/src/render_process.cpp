#include "render_process.h"
#include "shader.h"
#include "context.h"

namespace toy2d {
void RenderProcess::InitPipeline(int width, int height) {
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
	rastInfo.setRasterizerDiscardEnable(false)			// 是否忽略光栅化结果
		.setCullMode(vk::CullModeFlagBits::eBack)		// 剔除背面
		.setFrontFace(vk::FrontFace::eCounterClockwise) // 设置正面方向: 逆时针
		.setPolygonMode(vk::PolygonMode::eFill)			// 多边形填充模式: 填充
		.setLineWidth(1.0f)								// 线宽
		.setDepthClampEnable(false);					// 深度剔除
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
		.setPAttachments(&attachs); // 颜色混合附件
	createInfo.setPColorBlendState(&blend);

	auto result = Context::GetInstance().device.createGraphicsPipeline(nullptr, createInfo);
	if (result.result != vk::Result::eSuccess) {
		throw std::runtime_error("create graphics pipeline failed");
	}
	pipeline = result.value;
}

void RenderProcess::DestoryPipline() {
	Context::GetInstance().device.destroyPipeline(pipeline);
}
}
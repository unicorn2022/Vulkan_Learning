#include "render_process.h"
#include "context.h"
#include "swapchain.h"
#include "mymath.h"

namespace toy2d {

RenderProcess::RenderProcess() {
	layout = createLayout();
	renderPass = createRenderPass();
	graphicsPipeline = nullptr;
}

RenderProcess::~RenderProcess() {
	auto& device = Context::Instance().device;
	device.destroyRenderPass(renderPass);
	device.destroyPipelineLayout(layout);
	device.destroyPipeline(graphicsPipeline);
}

void RenderProcess::RecreateGraphicsPipeline(const Shader& shader) {
	if (graphicsPipeline) 
		Context::Instance().device.destroyPipeline(graphicsPipeline);
	
	graphicsPipeline = createGraphicsPipeline(shader);
}

void RenderProcess::RecreateRenderPass() {
	if (renderPass) 
		Context::Instance().device.destroyRenderPass(renderPass);
	
	renderPass = createRenderPass();
}

vk::PipelineLayout RenderProcess::createLayout() {
	vk::PipelineLayoutCreateInfo createInfo;

	auto range = Context::Instance().shader->GetPushConstantRange();
	createInfo.setSetLayouts(Context::Instance().shader->GetDescriptorSetLayouts())	// ����������
		.setPushConstantRanges(range);	// PushConstant����

	return Context::Instance().device.createPipelineLayout(createInfo);
}

vk::Pipeline RenderProcess::createGraphicsPipeline(const Shader& shader) {
	/* ��ˮ������ */
	vk::GraphicsPipelineCreateInfo createInfo;

	// 0. ��ɫ��
	std::array<vk::PipelineShaderStageCreateInfo, 2> stageCreateInfos;
	stageCreateInfos[0].setModule(shader.GetVertexModule())
		.setPName("main")
		.setStage(vk::ShaderStageFlagBits::eVertex);
	stageCreateInfos[1].setModule(shader.GetFragModule())
		.setPName("main")
		.setStage(vk::ShaderStageFlagBits::eFragment);
	createInfo.setStages(stageCreateInfos);

	// 1. ��������
	vk::PipelineVertexInputStateCreateInfo vertexInputCreateInfo;
	auto attributeDesc = Vec::GetAttributeDescription();	// ��������
	auto bindingDesc = Vec::GetBindingDescription();		// �����
	vertexInputCreateInfo.setVertexAttributeDescriptions(attributeDesc)
		.setVertexBindingDescriptions(bindingDesc);
	createInfo.setPVertexInputState(&vertexInputCreateInfo);

	// 2. ����ۼ�
	vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
	inputAssembly.setPrimitiveRestartEnable(false)
		.setTopology(vk::PrimitiveTopology::eTriangleList); // ����ľۼ���ʽ: 123, 456, 789
	createInfo.setPInputAssemblyState(&inputAssembly);

	// 3. �ӿڱ任 & �ü�
	// 3.1 �ӿ� (���, ���, ��Զƽ��)
	vk::Viewport viewport(0, 0, Context::Instance().swapchain->GetExtent().width, Context::Instance().swapchain->GetExtent().height, 0, 1);
	vk::PipelineViewportStateCreateInfo viewportInfo;
	viewportInfo.setViewports(viewport);
	// 3.2 ���л�
	vk::Rect2D scissor({ 0, 0 }, Context::Instance().swapchain->GetExtent());
	viewportInfo.setScissors(scissor);
	createInfo.setPViewportState(&viewportInfo);

	// 4. ��դ��
	vk::PipelineRasterizationStateCreateInfo rastInfo;
	rastInfo.setCullMode(vk::CullModeFlagBits::eFront)	// �޳�����
		.setFrontFace(vk::FrontFace::eCounterClockwise)	// �������淽��: ˳ʱ��
		.setDepthClampEnable(false)						// �Ƿ�������Ƚ�ȡ
		.setLineWidth(1)								// �߿�
		.setPolygonMode(vk::PolygonMode::eFill)		// ��������ģʽ: ���
		.setRasterizerDiscardEnable(false);			// �Ƿ���Թ�դ�����
	createInfo.setPRasterizationState(&rastInfo);

	// 5. ���ز���
	vk::PipelineMultisampleStateCreateInfo multiSample;
	multiSample.setSampleShadingEnable(false) // �Ƿ����ö��ز���
		.setRasterizationSamples(vk::SampleCountFlagBits::e1); // ��������
	createInfo.setPMultisampleState(&multiSample);

	// 6. ����: ��Ȳ���, ģ�����

	// 7. ��ɫ��� (��ʽ��) 
	// newRGB = (srcFactor * srcRGB) op (dstFactor * dstRGB)
	// newA = (srcFactor * srcA) op (dstFactor * dstA)
	// ����͸���Ȼ����˵: newRGB = 1 * srcRGB + (1 - scrA) * dstRGB, newA = srcA
	vk::PipelineColorBlendAttachmentState attachs;
	attachs.setBlendEnable(true)	// �Ƿ����û�ϸ���
		.setColorWriteMask(
			vk::ColorComponentFlagBits::eA |
			vk::ColorComponentFlagBits::eB |
			vk::ColorComponentFlagBits::eG |
			vk::ColorComponentFlagBits::eR) // ��ɫд������
		.setSrcColorBlendFactor(vk::BlendFactor::eOne)
		.setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
		.setColorBlendOp(vk::BlendOp::eAdd)
		.setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
		.setDstAlphaBlendFactor(vk::BlendFactor::eZero)
		.setAlphaBlendOp(vk::BlendOp::eAdd);

	vk::PipelineColorBlendStateCreateInfo blendInfo;
	blendInfo.setLogicOpEnable(false)	// �Ƿ������߼�����
		.setAttachments(attachs);		// ��ɫ��ϸ���
	createInfo.setPColorBlendState(&blendInfo);

	// 8. render pass & layout
	createInfo.setRenderPass(renderPass)
		.setLayout(layout);


	/* ���� pipeline */
	auto result = Context::Instance().device.createGraphicsPipeline(nullptr, createInfo);
	if (result.result != vk::Result::eSuccess) {
		std::cout << "create graphics pipeline failed: " << result.result << std::endl;
	}

	return result.value;
}

vk::RenderPass RenderProcess::createRenderPass() {
	vk::RenderPassCreateInfo createInfo;
	/* �������� */
	vk::AttachmentDescription attachDesc;
	attachDesc.setFormat(Context::Instance().swapchain->GetFormat().format)
		.setSamples(vk::SampleCountFlagBits::e1)	// ��������
		.setLoadOp(vk::AttachmentLoadOp::eClear)	// ��ɫ����: ����ʱ���
		.setStoreOp(vk::AttachmentStoreOp::eStore)	// ��ɫ����: �洢ʱ����	
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)	// ģ�建��
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)// ģ�建��
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);
	createInfo.setAttachments(attachDesc);

	/* ��ͨ������ */
	vk::AttachmentReference reference;
	reference.setLayout(vk::ImageLayout::eColorAttachmentOptimal) // ͼ�񲼾�
		.setAttachment(0); // ��������
	vk::SubpassDescription subpassDesc;
	subpassDesc.setColorAttachments(reference)					 // ��ɫ����
		.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics); // ������һ����ˮ����
	createInfo.setSubpasses(subpassDesc);
	
	/* ��ͨ������ */
	vk::SubpassDependency dependency;
	dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL)	// ��ִ�е���ͨ��: �ⲿͨ��
		.setDstSubpass(0)							// ��ִ�е���ͨ��: 0����ͨ��
		.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput) // ��ͨ��������ͼ���Ӧ�ó���
		.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);		// ��ͨ���ķ���Ȩ��
	createInfo.setDependencies(dependency);

	/* ������Ⱦ���� */
	return Context::Instance().device.createRenderPass(createInfo);
}

}
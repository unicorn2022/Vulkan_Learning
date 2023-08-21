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
	/* ��ˮ������ */
	vk::GraphicsPipelineCreateInfo createInfo;

	// 0. ������ɫ��
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

	// 1. ��������
	vk::PipelineVertexInputStateCreateInfo vertexInputCreateInfo;
	auto attribute = Vertex::GetAttribute();	// ��������
	auto binding = Vertex::GetBinding();		// �����
	vertexInputCreateInfo.setVertexAttributeDescriptions(attribute)
		.setVertexBindingDescriptions(binding);
	createInfo.setPVertexInputState(&vertexInputCreateInfo);

	// 2. ����ۼ�
	vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
	inputAssembly.setPrimitiveRestartEnable(false)
		.setTopology(vk::PrimitiveTopology::eTriangleList); // ����ľۼ���ʽ: 123, 456, 789
	createInfo.setPInputAssemblyState(&inputAssembly);

	// 3. �ӿڱ任 & �ü�
	// 3.1 �ӿ� (���, ���, ��Զƽ��)
	vk::Viewport viewport(0, 0, Context::GetInstance().swapchain->GetExtent().width, Context::GetInstance().swapchain->GetExtent().height, 0, 1);
	vk::PipelineViewportStateCreateInfo viewportInfo;
	viewportInfo.setViewports(viewport);
	// 3.2 ���л�
	vk::Rect2D scissor({ 0, 0 }, Context::GetInstance().swapchain->GetExtent());
	viewportInfo.setScissors(scissor);
	createInfo.setPViewportState(&viewportInfo);

	// 4. ��դ��
	vk::PipelineRasterizationStateCreateInfo rastInfo;
	rastInfo.setRasterizerDiscardEnable(false)			// �Ƿ���Թ�դ�����
		.setCullMode(vk::CullModeFlagBits::eFront)		// �޳�����
		.setFrontFace(vk::FrontFace::eCounterClockwise)	// �������淽��: ˳ʱ��
		.setDepthClampEnable(false)						// �Ƿ�������Ƚ�ȡ
		.setLineWidth(1)								// �߿�
		.setPolygonMode(vk::PolygonMode::eFill);		// ��������ģʽ: ���
	createInfo.setPRasterizationState(&rastInfo);

	// 5. ���ز���
	vk::PipelineMultisampleStateCreateInfo multiSample;
	multiSample.setSampleShadingEnable(false) // �Ƿ����ö��ز���
		.setRasterizationSamples(vk::SampleCountFlagBits::e1); // ��������
	createInfo.setPMultisampleState(&multiSample);

	// 6. ����: ��Ȳ���, ģ�����

	// 7. ��ɫ���
	vk::PipelineColorBlendAttachmentState attachs;
	attachs.setBlendEnable(false)	// �Ƿ����û�ϸ���
		.setColorWriteMask(
			vk::ColorComponentFlagBits::eA |
			vk::ColorComponentFlagBits::eB |
			vk::ColorComponentFlagBits::eG |
			vk::ColorComponentFlagBits::eR); // ��ɫд������
	vk::PipelineColorBlendStateCreateInfo blendInfo;
	blendInfo.setLogicOpEnable(false)	// �Ƿ������߼�����
		.setAttachments(attachs);		// ��ɫ��ϸ���
	createInfo.setPColorBlendState(&blendInfo);

	// 8. render pass & layout
	createInfo.setRenderPass(renderPass)
		.setLayout(layout);


	/* ���� pipeline */
	auto result = Context::GetInstance().device.createGraphicsPipeline(nullptr, createInfo);
	if (result.result != vk::Result::eSuccess) {
		std::cout << "create graphics pipeline failed: " << result.result << std::endl;
	}

	/* �ͷ���ɫ��ģ�� */
	Context::GetInstance().device.destroyShaderModule(vertexModule);
	Context::GetInstance().device.destroyShaderModule(fragModule);

	return result.value;
}

vk::RenderPass RenderProcess::createRenderPass() {
	vk::RenderPassCreateInfo createInfo;
	/* �������� */
	vk::AttachmentDescription attachDesc;
	attachDesc.setFormat(Context::GetInstance().swapchain->GetFormat().format)
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
	return Context::GetInstance().device.createRenderPass(createInfo);
}
}
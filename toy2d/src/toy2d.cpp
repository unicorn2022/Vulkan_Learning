#include "toy2d.h"

namespace toy2d {

std::unique_ptr<Renderer> renderer_;

void Init(const std::vector<const char*> extensions, Context::GetSurfaceCallback func, int w, int h) {
	Context::Init(extensions, func);
	auto& ctx = Context::Instance();
	ctx.initSwapchain(w, h);
	ctx.initShaderModules();
	ctx.initRenderProcess();
	ctx.initGraphicsPipeline();
	ctx.swapchain->InitFramebuffers();
	ctx.initCommandPool();
	ctx.initSampler();

	int maxFlightCount = 2;
	DescriptorSetManager::Init(maxFlightCount);
	renderer_  = std::make_unique<Renderer>(maxFlightCount);
	renderer_->SetProject(w, 0, 0, h, -1, 1);
}

void Quit() {
	Context::Instance().device.waitIdle(); // 等待GPU执行完所有的命令
	renderer_.reset();
	TextureManager::Instance().Clear();
	DescriptorSetManager::Quit();
	Context::Quit();
}

Texture* LoadTexture(const std::string& filename) {
	return TextureManager::Instance().Load(filename);
}

void DestroyTexture(Texture* texture) {
	TextureManager::Instance().Destroy(texture);
}

Renderer* GetRenderer() {
	return renderer_.get();
}

}


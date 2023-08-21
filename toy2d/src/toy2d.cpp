#include "toy2d.h"

namespace toy2d {

	std::unique_ptr<Renderer> renderer_;

	void Init(const std::vector<const char*> extensions, Context::GetSurfaceCallback func, int w, int h) {
		Context::Init(extensions, func);
		auto& ctx = Context::GetInstance();
		ctx.initSwapchain(w, h);
		ctx.initRenderProcess();
		ctx.initGraphicsPipeline();
		ctx.swapchain->InitFramebuffers();
		ctx.initCommandPool();

		renderer_  = std::make_unique<Renderer>();
	}

	void Quit() {
		Context::GetInstance().device.waitIdle(); // �ȴ�GPUִ�������е�����
		renderer_.reset();
		Context::Quit();
	}

	Renderer* GetRenderer() {
		return renderer_.get();
	}
}


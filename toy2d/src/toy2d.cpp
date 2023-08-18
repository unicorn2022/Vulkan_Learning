#include "toy2d.h"

namespace toy2d {
	void Init(const std::vector<const char*> extensions, CreateSurfaceFunc func, int w, int h) {
		Context::Init(extensions, func);
		Context::GetInstance().InitSwapchain(w, h);
		Shader::Init(ReadWholeFile("./shader/vert.spv"), ReadWholeFile("./shader/frag.spv"));
		Context::GetInstance().renderProcess->InitRenderPass();
		Context::GetInstance().renderProcess->InitLayout();
		Context::GetInstance().swapchain->createFramebuffers(w, h);
		Context::GetInstance().renderProcess->InitPipeline(w, h);
	}

	void Quit() {
		Context::GetInstance().renderProcess.reset();
		Context::GetInstance().DestroySwapchain();
		Shader::Quit();
		Context::Quit();
	}
}


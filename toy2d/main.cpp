#include "toy2d.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_vulkan.h"
#include <SDL2/SDL_video.h>

constexpr uint32_t WindowWidth = 1024;
constexpr uint32_t WindowHeight = 720;

struct MyContext {
	bool shouldClose = false;
	float x, y; 
	toy2d::Renderer* renderer;
	toy2d::Texture* texture1;
	toy2d::Texture* texture2;
};
void MainLoop();
void HandleInput(MyContext& context);

int main(int argc, char* argv[]) {
	// SDL初始化
	SDL_Init(SDL_INIT_EVERYTHING);

	// 创建SDL窗口
	SDL_Window* window = SDL_CreateWindow("test",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		WindowWidth, WindowHeight,
		SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

	if (!window) {
		SDL_Log("create window failed");
		exit(2);
	}

	// 获取当前系统支持的窗口扩展
	unsigned int count;
	SDL_Vulkan_GetInstanceExtensions(window, &count, nullptr);
	std::vector<const char*> extensions(count);
	SDL_Vulkan_GetInstanceExtensions(window, &count, extensions.data());

	// 初始化toy2d
	toy2d::Init(extensions, 
		[&](vk::Instance instance) {
			VkSurfaceKHR surface;
			if (!SDL_Vulkan_CreateSurface(window, instance, &surface)) {
				throw std::runtime_error("cannot create surface");
			}
			return surface;
		}, 1024, 720);

	// 开启渲染循环
	MainLoop();

	// 销毁SDL窗口, 并推出SDL应用
	toy2d::Quit();
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}

void MainLoop() {
	MyContext context;
	context.shouldClose = false;
	context.x = 100;
	context.y = 100;
	context.renderer = toy2d::GetRenderer();
	context.texture1 = toy2d::LoadTexture("./img/avatar.png");
	context.texture2 = toy2d::LoadTexture("./img/role.png");

    while (!context.shouldClose) {
		// 处理SDL事件
		HandleInput(context);
       
		// 绘制矩形
		context.renderer->StartRender();
		context.renderer->SetDrawColor(toy2d::Color{ 1, 0, 0 });
		context.renderer->DrawTexture(toy2d::Rect{
			toy2d::Vec{context.x, context.y},
			toy2d::Size{200, 300} 
		}, *context.texture1);
		context.renderer->SetDrawColor(toy2d::Color{ 0, 1, 0 });
		context.renderer->DrawTexture(toy2d::Rect{
			toy2d::Vec{500, 100},
			toy2d::Size{200, 300}
			}, *context.texture2);
		context.renderer->SetDrawColor(toy2d::Color{ 0, 0, 1 });
		context.renderer->DrawLine(
			toy2d::Vec{ 0,0 },
			toy2d::Vec{ WindowWidth, WindowHeight }
		);
		context.renderer->EndRender();
    }

	toy2d::DestroyTexture(context.texture1);
	toy2d::DestroyTexture(context.texture2);
}

void HandleInput(MyContext& context) {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		// 处理退出事件
		if (event.type == SDL_QUIT) {
			context.shouldClose = true;
		}

		// 处理窗口大小改变事件
		if (event.type == SDL_WINDOWEVENT) {
			if(event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
				toy2d::ResizeSwapchainImage(event.window.data1, event.window.data2);
		}

		// 处理输入事件
		if (event.type == SDL_KEYDOWN) {
			if (event.key.keysym.sym == SDLK_a) {
				context.x -= 10;
			}
			if (event.key.keysym.sym == SDLK_d) {
				context.x += 10;
			}
			if (event.key.keysym.sym == SDLK_w) {
				context.y -= 10;
			}
			if (event.key.keysym.sym == SDLK_s) {
				context.y += 10;
			}
			if (event.key.keysym.sym == SDLK_ESCAPE) {
				context.shouldClose = true;
			}
		}
	}
}
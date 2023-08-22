#include "toy2d.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_vulkan.h"
#include <SDL2/SDL_video.h>

void MainLoop();

int main(int argc, char* argv[]) {
	// SDL初始化
	SDL_Init(SDL_INIT_EVERYTHING);

	// 创建SDL窗口
	SDL_Window* window = SDL_CreateWindow("test",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		1024, 720,
		SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN);

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
	auto renderer = toy2d::GetRenderer();

    bool shouldClose = false;
    SDL_Event event;

	float x = 100, y = 100;
	renderer->SetDrawColor(toy2d::Color{ 0, 1, 0 });

    while (!shouldClose) {
		// 处理SDL事件
        while (SDL_PollEvent(&event)) {
			// 处理退出事件
            if (event.type == SDL_QUIT) {
                shouldClose = true;
            }

			// 处理输入事件
			if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.sym == SDLK_a) {
					x -= 10;
				}
				if (event.key.keysym.sym == SDLK_d) {
					x += 10;
				}
				if (event.key.keysym.sym == SDLK_w) {
					y -= 10;
				}
				if (event.key.keysym.sym == SDLK_s) {
					y += 10;
				}
				if (event.key.keysym.sym == SDLK_0) {
					renderer->SetDrawColor(toy2d::Color{ 1, 0, 0 });
				}
				if (event.key.keysym.sym == SDLK_1) {
					renderer->SetDrawColor(toy2d::Color{ 0, 1, 0 });
				}
				if (event.key.keysym.sym == SDLK_2) {
					renderer->SetDrawColor(toy2d::Color{ 0, 0, 1 });
				}
			}
        }
		// 绘制矩形
		renderer->DrawRect(toy2d::Rect{ 
			toy2d::Vec{x, y},
			toy2d::Size{200, 300} 
		});
    }
}
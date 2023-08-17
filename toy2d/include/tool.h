#pragma once

#include <algorithm>
#include <vector>
#include <functional>
#include <iostream>
#include <fstream>
#include "vulkan/vulkan.hpp"

using CreateSurfaceFunc = std::function<vk::SurfaceKHR(vk::Instance)>;

namespace toy2d {
	std::string ReadWholeFile(const std::string& filename);
}

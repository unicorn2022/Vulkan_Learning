#pragma once

#include <algorithm>
#include <vector>
#include <functional>
#include <iostream>
#include <fstream>
#include "vulkan/vulkan.hpp"

using CreateSurfaceFunc = std::function<vk::SurfaceKHR(vk::Instance)>;

namespace toy2d {

    template <typename T, typename U>
    void RemoveNosupportedElems(std::vector<T>& elems, const std::vector<U>& supportedElems,
        std::function<bool(const T&, const U&)> eq) {
        int i = 0;
        while (i < elems.size()) {
            auto it = std::find_if(supportedElems.begin(), supportedElems.end(), [&](const U& e) {return eq(elems[i], e);});

            if (it == supportedElems.end()) {
                elems.erase(elems.begin() + i);
            }
            else {
                i++;
            }
        }
    }

	std::string ReadWholeFile(const std::string& filename);
}

#include "tool.h"

namespace toy2d {
	std::string ReadWholeFile(const std::string& filename) {
		std::ifstream file(filename, std::ios::binary | std::ios::ate);

		if (!file.is_open()) {
			std::cout << "read " << filename << " failed\n";
			return std::string{};
		}

		auto size = file.tellg();
		std::string content;
		content.resize(size);
		file.seekg(0);

		file.read(content.data(), content.size());
		file.close();
		return content;
	}
}
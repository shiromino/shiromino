#pragma once
#include <filesystem>

namespace fs = std::filesystem;

namespace Shiro::OS {
    const std::filesystem::path& getBasePath();
}
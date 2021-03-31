#pragma once
#include <filesystem>
namespace fs = std::filesystem;
namespace Shiro::OS {
    std::filesystem::path getExecutablePath();
}
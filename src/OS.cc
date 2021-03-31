#include "OS.h"
#ifdef _WIN32
#include <windows.h>
#elif __APPLE__
#include <mach-o/dyld.h>
#include <sys/syslimits.h>
#endif
#include <filesystem>
namespace fs = std::filesystem;
fs::path Shiro::OS::getExecutablePath() {
#ifdef _WIN32
    char path[MAX_PATH];
    GetModuleFileName(nullptr, path, MAX_PATH);
    return fs::canonical(std::string(path));
#elif __APPLE__
    char path[PATH_MAX];
    uint32_t size = PATH_MAX;
    _NSGetExecutablePath(path, &size);
    return fs::canonical(std::string(path));
#else
    return fs::canonical("/proc/self/exe");
#endif
}
#pragma once

#include <vector>
#include <string>

namespace Shiro {
    class Path {
    public:
        Path();
        Path(std::string root);

        Path& operator<<(const std::string name);
        Path& operator<<(const Path& path);
        operator std::string();

    private:
        std::string root;
        std::vector<std::string> names;
    };
}

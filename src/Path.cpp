#include <regex>

#include "Path.hpp"
#include "stringtools.hpp"

using namespace Shiro;
using namespace std;

Path::Path() {}

Path::Path(std::string root) : root(root) {
}

Path& Path::operator<<(const string name) {
    names.push_back(name);
    return *this;
}

Path& Path::operator<<(const Path& path) {
    for (auto& name : path.names) {
        names.push_back(name);
    }
    return *this;
}

Path::operator std::string() {
    string pathStr = root;

    if (names.size()) {
        if (root == "") {
            pathStr = names[0];
        }
        else {
            pathStr += "/" + names[0];
        }
        for (size_t i = 1; i < names.size(); i++) {
            pathStr += "/" + names[i];
        }
    }

    return pathStr;
}

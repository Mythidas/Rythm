#pragma once

#include <string>

namespace rm {
    struct ShaderSpec {
        const std::string path;
    };

    class Shader {
    public:
        virtual ~Shader() = default;
    };
}

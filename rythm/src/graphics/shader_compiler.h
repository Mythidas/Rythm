#pragma once

#include <memory>
#include <vector>
#include <string>

namespace rm {
    struct ShaderBytecode {
        std::vector<unsigned int> code;

        unsigned long long Size() const {
            return code.size() * sizeof(unsigned int);
        }

        const unsigned int* Data() const {
            return code.data();
        }
    };

    class ShaderCompiler {
    public:
        ShaderCompiler();

        ShaderBytecode Compile(const std::string& path);
    };
}

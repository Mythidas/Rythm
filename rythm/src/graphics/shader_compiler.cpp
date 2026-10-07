#include "rmpch.h"
#include "shader_compiler.h"

#include <slang/slang-com-ptr.h>
#include <slang/slang.h>

namespace rm {
    struct Impl {
        Slang::ComPtr<slang::IGlobalSession> globalSession;
        Slang::ComPtr<slang::ISession> session;
        slang::SessionDesc sessionDesc;
    } impl;

    ShaderCompiler::ShaderCompiler() {
        slang::createGlobalSession(impl.globalSession.writeRef());

        auto slangTargets{
            std::to_array<slang::TargetDesc>({ { .format{ SLANG_SPIRV }, .profile{ impl.globalSession->findProfile("spirv_1_4") } } })
        };
        auto slangOptions{
            std::to_array<slang::CompilerOptionEntry>({ { slang::CompilerOptionName::EmitSpirvDirectly, { slang::CompilerOptionValueKind::Int, 1 } } })
        };
        impl.sessionDesc = {
            .targets{ slangTargets.data() },
            .targetCount{ SlangInt(slangTargets.size()) },
            .defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR,
            .compilerOptionEntries{ slangOptions.data() },
            .compilerOptionEntryCount{ uint32_t(slangOptions.size()) }
        };

        impl.globalSession->createSession(impl.sessionDesc, impl.session.writeRef());
    }

    ShaderBytecode ShaderCompiler::Compile(const std::string& path) {

        Slang::ComPtr<slang::IModule> slangModule{ impl.session->loadModuleFromSource("triangle", path.c_str(), nullptr, nullptr) };
        Slang::ComPtr<ISlangBlob> spirv;
        slangModule->getTargetCode(0, spirv.writeRef());

        const size_t byteSize = spirv->getBufferSize();
        const size_t wordCount = byteSize / sizeof(uint32_t);

        const auto* data = static_cast<const uint32_t*>(spirv->getBufferPointer());

        ShaderBytecode bytecode;
        bytecode.code.assign(data, data + wordCount);
        return bytecode;
    }
}

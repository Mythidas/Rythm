project "Rythm"
    location "src"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++latest"
    staticruntime "off"

    targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
    objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

    pchheader "rmpch.h"
    pchsource "src/rmpch.cpp"
    enablepch("Off")

    local vulkanSDK = os.getenv("VULKAN_SDK")

    files
    {
        "src/**.h",
        "src/**.cpp",
        "../vendor/volk/volk.c"
    }

    includedirs
    {
        "src",
        "../vendor/sdl/include",
        "../vendor/glm",
        "../vendor/vma/include",
        "../vendor/volk",
        "../vendor/vulkan-headers/include",
        "../vendor/tinyobj",
        "../vendor/ktx/lib/include",
        "../vendor/ktx/external/dfdutils",
        vulkanSDK .. "/include",
        "../vendor/spdlog/include"
    }

    libdirs
    {
        "../vendor/sdl/build",
        "../vendor/ktx/lib/build",
        vulkanSDK .. "/lib",
        "../vendor/spdlog/build",
    }

    links
    {
        "SDL3",
        "ktx",
        "slang-compiler",
        "spdlog"
    }

    defines
    {
        "VK_NO_PROTOTYPES",
        "KHRONOS_STATIC"
    }

    filter "system:windows"
        systemversion "latest"

        defines
        {
            "RM_PLAT_WINDOWS",
        }

    filter "configurations:Debug"
        defines "RM_DEBUG"
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines "RM_RELEASE"
        optimize "On"
        runtime "Release"

    filter "configurations:Dist"
        defines "RM_DIST"
        optimize "On"
        runtime "Release"

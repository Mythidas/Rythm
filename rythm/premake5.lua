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

    files
    {
        "src/**.h",
        "src/**.cpp",
    }

    includedirs
    {
    }

    links
    {
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

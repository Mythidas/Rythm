workspace "Rythm"
    architecture "x64"
    startproject "rythm"

    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Core"
    include "rythm"
group ""

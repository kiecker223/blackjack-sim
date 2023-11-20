workspace "BlackjackSim"
    configurations { "Debug", "Release" }
    platforms { "x64" }

    -- This is a bug down here,
    -- it thinks it needs a string but the docs
    -- says this is valid input
    -- system { "Windows", "Unix" }


    warnings "Default"

    filter "platforms:x64"
      architecture "x86_64"
    filter {}
   
    filter "configurations:Debug"
      defines { "DEBUG" }

    filter "configurations:Release"
      defines { "RELEASE", "NDEBUG" }
      flags {
        "LinkTimeOptimization",
      }
      optimize "On"

    filter {"platforms:x64", "configurations:Release"}
      targetdir "Build/Release"
    filter {"platforms:x64", "configurations:Debug"}
      targetdir "Build/Debug"
    filter {}

    objdir "%{cfg.targetdir}/obj"

    symbols "FastLink"

    filter {"configurations:Release"}
      symbols "Full"
    filter {}

    flags {
      "MultiProcessorCompile"
    }

    staticruntime "On"

    cppdialect "C++17"
    language "C++"
    characterset "ASCII"
    startproject "Game"

    parentdir = os.getcwd()

project "BjSimulator"
    kind "ConsoleApp"
    files { "src/**.h", "src/**.cpp" }

    filter "system:Windows"
      links { "user32", "kernel32", "d3dcompiler" }
    filter {}

    includedirs { "%{parentdir}" }

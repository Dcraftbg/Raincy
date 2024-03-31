workspace "Raincy"
        configurations {"Debug", "Release", "Dist"}
        architecture "x64"
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
project "Raincy"
        location "Raincy"
        kind "ConsoleApp"
        language "C"
        targetdir ("out/bin/" .. outputdir .. "/%{prj.name}")
        objdir    ("out/int/" .. outputdir .. "/%{prj.name}")
        includedirs {
           "%{prj.name}/vendor/raylib/include"
        }
        libdirs {
           "%{prj.name}/vendor/raylib/lib"
        }
        links {
           "raylib",
        }
        files {
           "%{prj.name}/src/**.h",
           "%{prj.name}/src/**.c",
        }
        filter "configurations:Debug"
            defines {
                "RAIN_TARGET_DEBUG"
            }
            symbols "On"
            optimize "Off"
        filter "configurations:Release"
            defines {
                "RAIN_TARGET_RELEASE"
            }
            symbols "On"
            optimize "On"
        filter "configurations:Dist"
            defines {
                "RAIN_TARGET_DIST"
            }
            symbols "Off"
            optimize "On"
        filter "system:Windows"
            defines {
                "TARGET_PLATFORM_WINDOWS"
            }
            links {
                "winmm"
            }
        filter "system:Linux"
            defines {
                "TARGET_PLATFORM_LINUX"
            }
        
        filter "system:Android"
            defines {
                "TARGET_PLATFORM_ANDROID"
            }

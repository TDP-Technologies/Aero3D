project "Engine"
	kind "SharedLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/Core/**.h",
		"src/Core/**.cpp",
		"src/Event/**.h",
		"src/Event/**.cpp",
		"src/Graphics/**.h",
		"src/Graphics/**.cpp",
		"src/Platform/Vulkan/**.h",
		"src/Platform/Vulkan/**.cpp",
		"src/IO/**.h",
		"src/IO/**.cpp",
		"src/Scene/**.h",
		"src/Scene/**.cpp",
		"src/Utils/**.h",
		"src/Utils/**.cpp",
		"src/Vendor/**.h",
		"src/Vendor/**.cpp",
	}

	includedirs
	{
		"src",
		"src/Vendor",
		"%{IncludeDir.sdl}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.vulkan}",
	}

	filter "system:windows"
		systemversion "latest"

		libdirs { WindowsLibraryDir }

		files
		{
			"src/Platform/Windows/**.h",
			"src/Platform/Windows/**.cpp",
		}

		links
		{
			"SDL3/SDL3.lib",
			"Vulkan/vulkan-1.lib",
			"Vulkan/shaderc.lib",
			"Vulkan/shaderc_combined.lib",
			"Vulkan/shaderc_util.lib",
			"Vulkan/glslang.lib",
			"Vulkan/SPIRV.lib",
			"Vulkan/SPIRV-Tools.lib",
			"Vulkan/SPIRV-Tools-opt.lib"
		}

		defines
		{
			"A3D_PLATFORM_WINDOWS",
			"A3D_BUILD_DLL"
		}

        postbuildcommands
		{
			("{COPY} %{cfg.buildtarget.relpath} \"%{wks.location}/bin/" .. outputdir .. "/Sandbox/\"")
		}

	filter "configurations:Debug"
		defines "A3D_DEBUG"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		defines "A3D_RELEASE"
		runtime "Release"
		optimize "On"	

	filter "configurations:Dist"
		defines "A3D_DIST"
		runtime "Release"
		optimize "On"
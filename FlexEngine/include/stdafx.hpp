#pragma once

#define COMPILE_OPEN_GL 1
#define COMPILE_VULKAN 0

#define ENABLE_PROFILING 1

#define NOMINMAX

#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS 1

//#pragma warning(disable : 4201) // nonstandard extension used: nanmeless struct/union
//#pragma warning(disable : 4820) // bytes' bytes padding added after construct 'member_name'
//#pragma warning(disable : 4868) // compiler may not enforce left-to-right evaluation order in braced initializer list
//#pragma warning(disable : 4710) // function not inlined

#include "Types.hpp"
#include "Logger.hpp"

#pragma warning(push, 0)
#include <ft2build.h>
#include FT_FREETYPE_H
#pragma warning(pop)

#if COMPILE_VULKAN
#pragma warning(push, 0)
	#include <glad/glad.h>
	#include <vulkan/vulkan.hpp>
	#include <GLFW/glfw3.h>
	#include <GLFW/glfw3native.h>
#pragma warning(pop)

	#include "Graphics/Vulkan/VulkanRenderer.hpp"
	#include "Window/Vulkan/VulkanWindowWrapper.hpp"
#endif // COMPILE_VULKAN

#if COMPILE_OPEN_GL
#pragma warning(push, 0)
// TODO: Does this line need to be included above earlier include in Vulkan section?
#define GLFW_EXPOSE_NATIVE_WIN32
	#include <glad/glad.h>
	#include <GLFW/glfw3.h>
	#include <GLFW/glfw3native.h>
#pragma warning(pop)

	#include "Graphics/GL/GLRenderer.hpp"
	#include "Window/GL/GLWindowWrapper.hpp"

#endif // COMPILE_OPEN_GL

#include "Physics/PhysicsTypeConversions.hpp"

template<class T>
inline void SafeDelete(T &pObjectToDelete)
{
	if (pObjectToDelete != nullptr)
	{
		delete(pObjectToDelete);
		pObjectToDelete = nullptr;
	}
}

#ifndef btAssert
#define btAssert(e) assert(e)
#endif

#pragma warning(push, 0)
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED

// For matrix_decompose
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#pragma warning(pop)

#define PI (glm::pi<real>())
#define TWO_PI (glm::two_pi<real>())
#define PI_DIV_TWO (glm::half_pi<real>())
#define PI_DIV_FOUR (glm::quarter_pi<real>())
#define THREE_OVER_TWO_PI (glm::three_over_two_pi<real>())

#if ENABLE_PROFILING
#define PROFILE_BEGIN(blockName) Profiler::Begin(blockName);
#define PROFILE_END(blockName) Profiler::End(blockName);
#else
#define PROFILE_BEGIN(blockName)
#define PROFILE_END(blockName)
#endif

namespace flex
{
	static const std::string RESOURCE_LOCATION = "../../../FlexEngine/resources/";

	// These fields are defined and initialized in FlexEngine.cpp
	extern class Window* g_Window;
	extern class CameraManager* g_CameraManager;
	extern class InputManager* g_InputManager;
	extern class Renderer* g_Renderer;
	extern class FlexEngine* g_EngineInstance;
	extern class SceneManager* g_SceneManager;
	extern struct Monitor* g_Monitor;
	extern class PhysicsManager* g_PhysicsManager;

	extern sec g_SecElapsedSinceProgramStart;
	extern sec g_DeltaTime;
}

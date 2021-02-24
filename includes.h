#pragma once

#include <Windows.h>
#include <iostream>
#include <random>

// Detours Hooking
#include "detours/detours.h"

// ImGui
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"

#include "imgui/FontAwesome/FontAwesome.h"
#include "imgui/FontAwesome/FontAwesome.cpp"

#include "imgui/TextEditor/TextEditor.h"

// DirectX
#include <d3d11.h>

// Boost
#include <boost/function.hpp>
#include <boost/filesystem.hpp>

// FiveM SDK
#include "sdk/ResourceCache.h"
#include "sdk/ResourceManager.h"
#include "sdk/ResourceMetaDataComponent.h"

// Namespace
using namespace std;
using namespace fx;
#include "includes.h"

// Variables
static int selected_tab = 1;
static int selected_resource = NULL;
static const char* current_resource = NULL;
TextEditor editor;
auto resource_manager = Instance<ResourceManager>::Get();
	
// DirectX Exportation
typedef HRESULT(__stdcall* D3D11PresentHook) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
typedef HRESULT(__stdcall* D3D11ResizeBuffersHook) (IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
typedef void(__stdcall* D3D11PSSetShaderResourcesHook) (ID3D11DeviceContext* pContext, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const* ppShaderResourceViews);
typedef void(__stdcall* D3D11DrawHook) (ID3D11DeviceContext* pContext, UINT VertexCount, UINT StartVertexLocation);
typedef void(__stdcall* D3D11DrawIndexedHook) (ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation);
typedef void(__stdcall* D3D11DrawIndexedInstancedHook) (ID3D11DeviceContext* pContext, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation);
typedef void(__stdcall* D3D11CreateQueryHook) (ID3D11Device* pDevice, const D3D11_QUERY_DESC* pQueryDesc, ID3D11Query** ppQuery);

// ImGui Exportation
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DXGIMsgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) { return DefWindowProc(hwnd, uMsg, wParam, lParam); }

// Other namespace
namespace Cheats
{
	string RandomString(int lenght)
	{
		string str("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

		random_device rd;
		mt19937 generator(rd());

		shuffle(str.begin(), str.end(), generator);

		return str.substr(0, lenght);
	}

	void ExecuteNewResource(string code)
	{
		if (!code.c_str())
			return;

		boost::filesystem::create_directory("C:\\Supreme");
		boost::filesystem::create_directory("C:\\Supreme\\Exec");

		string metadataPath = "C:\\Supreme\\Exec\\__resource.lua";
		string file = RandomString(16);
		string buffer;
		ofstream metadata;
		ofstream script;

		metadata.open(metadataPath.c_str());
		metadata << "resource_manifest_version '44febabe-d386-4d18-afbe-5e627f4af937'\n";
		metadata << "client_scripts {\n";
		metadata << "   '";
		metadata << file;
		metadata << ".lua'\n";
		metadata << "}";
		metadata.close();

		string path;
		path += "C:\\Supreme\\Exec\\";
		path += file;
		path += ".lua";

		buffer += "Citizen.CreateThread(function()";
		buffer += code.c_str();
		buffer += "end)";

		script.open(path);
		script << buffer;
		script.close();

		auto resource = resource_manager->CreateResource(file.c_str(), NULL);
		resource->SetComponent(new ResourceCacheEntryList{});
		resource->LoadFrom("C:\\Supreme\\Exec");
		resource->Start();

		remove(metadataPath.c_str());
		remove(path.c_str());
	}

	void LoadNewResourceJS(string code)
	{
		// To do
	}

	void ExecuteNewResourceJS(string code)
	{
		// To do
	}

	void StartResource(string name)
	{
		if (name.c_str() != 0 || name.c_str() != "_cfx_internal")
			resource_manager->GetResource(name.c_str())->Start();
	}

	void StopResource(string name)
	{
		if (name.c_str() != 0 || name.c_str() != "_cfx_internal")
			resource_manager->GetResource(name.c_str())->Stop();
	}

	string ResourceState(string name)
	{
		if (resource_manager->GetResource(name.c_str())->GetState() == fx::ResourceState::Started)
			return "Started";
		else if (resource_manager->GetResource(name.c_str())->GetState() == fx::ResourceState::Stopped)
			return "Stopped";
		else if (resource_manager->GetResource(name.c_str())->GetState() == fx::ResourceState::Starting)
			return "Starting";
		else if (resource_manager->GetResource(name.c_str())->GetState() == fx::ResourceState::Stopping)
			return "Stopping";
		else if (resource_manager->GetResource(name.c_str())->GetState() == fx::ResourceState::Uninitialized)
			return "Uninitialized";
	}

	vector<Resource*> GetAllResources()
	{
		vector<Resource*> localresources;

		resource_manager->ForAllResources([&](fwRefContainer<Resource> resource)
			{
				localresources.push_back(resource.GetRef());
			});

		return localresources;
	}
}

namespace GUI
{
	void CALLBACK Style()
	{
		auto& style = ImGui::GetStyle();
		style.WindowMinSize = ImVec2(770, 453);

		style.FrameBorderSize = 0;
		style.WindowRounding = 0;
		style.TabRounding = 0;
		style.ScrollbarRounding = 0;
		style.FramePadding = ImVec2(8, 6);
		style.WindowTitleAlign = ImVec2(0.0f, 0.5f);

		style.Colors[ImGuiCol_TitleBg] = ImColor(227, 0, 40, 255);
		style.Colors[ImGuiCol_TitleBgActive] = ImColor(227, 0, 40, 255);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImColor(15, 15, 15, 50);

		style.Colors[ImGuiCol_WindowBg] = ImColor(12, 12, 12, 255);

		style.Colors[ImGuiCol_Button] = ImColor(24, 25, 24, 255);
		style.Colors[ImGuiCol_ButtonActive] = ImColor(54, 53, 55);
		style.Colors[ImGuiCol_ButtonHovered] = ImColor(54, 53, 55);

		style.Colors[ImGuiCol_CheckMark] = ImColor(255, 255, 255, 255);

		style.Colors[ImGuiCol_FrameBg] = ImColor(36, 37, 36, 255);
		style.Colors[ImGuiCol_FrameBgActive] = ImColor(36, 37, 36, 255);
		style.Colors[ImGuiCol_FrameBgHovered] = ImColor(36, 37, 36, 255);

		style.Colors[ImGuiCol_Header] = ImColor(24, 24, 24, 255);
		style.Colors[ImGuiCol_HeaderActive] = ImColor(54, 53, 55);
		style.Colors[ImGuiCol_HeaderHovered] = ImColor(24, 24, 24, 100);

		style.Colors[ImGuiCol_ResizeGrip] = ImColor(51, 49, 50, 255);
		style.Colors[ImGuiCol_ResizeGripActive] = ImColor(54, 53, 55);
		style.Colors[ImGuiCol_ResizeGripHovered] = ImColor(51, 49, 50, 255);

		style.Colors[ImGuiCol_SliderGrab] = ImColor(249, 79, 49, 255);
		style.Colors[ImGuiCol_SliderGrabActive] = ImColor(249, 79, 49, 255);

		style.Colors[ImGuiCol_Border] = ImColor(54, 54, 54);
		style.Colors[ImGuiCol_Separator] = ImColor(54, 54, 54);
		style.Colors[ImGuiCol_SeparatorActive] = ImColor(54, 54, 54);
		style.Colors[ImGuiCol_SeparatorHovered] = ImColor(54, 54, 54);
	}

	void CALLBACK InitHook()
	{
		Style();

		ImGui::Begin("Supreme Cheats");

		if (ImGui::Button(ICON_FA_CODE " Lua Executor", ImVec2(120, 35))) { selected_tab = 1; }
		ImGui::SameLine(0.0f, 10.0f);
		if (ImGui::Button(ICON_FA_COFFEE " JS Executor", ImVec2(125, 35))) { selected_tab = 2; }
		ImGui::SameLine(0.0f, 10.0f);
		if (ImGui::Button(ICON_FA_DOWNLOAD " Dumper", ImVec2(100, 35))) { selected_tab = 3; }
		ImGui::SameLine(0.0f, 10.0f);
		if (ImGui::Button(ICON_FA_SEARCH_PLUS " Triggers", ImVec2(100, 35))) { selected_tab = 4; }
		ImGui::SameLine(0.0f, 10.0f);
		if (ImGui::Button(ICON_FA_FOLDER_OPEN " Stopper", ImVec2(100, 35))) { selected_tab = 5; }
		ImGui::SameLine(0.0f, 10.0f);
		if (ImGui::Button(ICON_FA_BOOK " Menus", ImVec2(100, 35))) { selected_tab = 6; }
		ImGui::SameLine(0.0f, 10.0f);
		if (ImGui::Button(ICON_FA_COGS " Config", ImVec2(100, 35))) { selected_tab = 7; }

		if (selected_tab == 1)
		{
			auto size = ImGui::GetWindowSize();
			bool selected = (current_resource == "_cfx_internal");

			editor.Render("TextEditor", ImVec2(size.x - 15, size.y - 120), true); ImGui::Spacing();
			editor.SetShowWhitespaces(false);
			editor.SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());
			editor.SetPalette(TextEditor::GetDarkPalette());

			if (ImGui::Button(ICON_FA_CODE " Execute")) { Cheats::ExecuteNewResource(editor.GetText()); }
			ImGui::SameLine(0.0f, 10.0f);
			if (ImGui::Button(ICON_FA_FILE " Load from File")) { MessageBoxA(NULL, "Option not available", "Supreme", NULL); }
			ImGui::SameLine(0.0f, 10.0f);
			if (ImGui::BeginCombo("##resourceslist", current_resource)) { for (fx::Resource* resource : Cheats::GetAllResources()) { if (resource->GetName() != "_cfx_internal") { if (resource->GetState() == fx::ResourceState::Started) { bool selected = (current_resource == resource->GetName().c_str()); if (ImGui::Selectable(resource->GetName().c_str(), selected)) { current_resource = resource->GetName().c_str(); if (selected) { ImGui::SetItemDefaultFocus(); } } } } } }
		}
		else if (selected_tab == 2)
		{
			auto size = ImGui::GetWindowSize();
			bool selected = (current_resource == "_cfx_internal");

			editor.Render("TextEditor", ImVec2(size.x - 15, size.y - 120), true); ImGui::Spacing();
			editor.SetShowWhitespaces(false);
			editor.SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());
			editor.SetPalette(TextEditor::GetDarkPalette());

			if (ImGui::Button(ICON_FA_COFFEE " Execute")) { /*Cheats::ExecuteNewResourceJS(editor.GetText());*/ }
			ImGui::SameLine(0.0f, 10.0f);
			if (ImGui::Button(ICON_FA_FILE " Load from File")) { MessageBoxA(NULL, "Option not available", "Supreme", NULL); }
			ImGui::SameLine(0.0f, 10.0f);
			if (ImGui::BeginCombo("##resourceslist", current_resource)) { for (fx::Resource* resource : Cheats::GetAllResources()) { if (resource->GetState() == fx::ResourceState::Started) { bool selected = (current_resource == resource->GetName().c_str()); if (ImGui::Selectable(resource->GetName().c_str(), selected)) { current_resource = resource->GetName().c_str(); if (selected) { ImGui::SetItemDefaultFocus(); } } } } }
		}
		else if (selected_tab == 3)
		{
			for (fx::Resource* resource : Cheats::GetAllResources())
			{
				if (resource->GetState() == fx::ResourceState::Started)
				{
					if (ImGui::CollapsingHeader(resource->GetName().c_str()))
					{

					}

				}
			}
		}
		else if (selected_tab == 5)
		{
			auto size = ImGui::GetWindowSize();
			if(ImGui::BeginListBox("##resourcestopper", ImVec2(size.x - 15, size.y - 120)))
			{
				for (fx::Resource* resource : Cheats::GetAllResources())
				{
					current_resource = resource->GetName().c_str();
					string name = resource->GetName().c_str() + (string)" [" + Cheats::ResourceState(current_resource) + (string)"]";
					ImGui::Selectable(name.c_str(), selected_resource);
				}
			}
			ImGui::ListBoxFooter();
		}

		ImGui::End();
	}
}

namespace D3DHook
{
	// D3D11 Classes
	D3D11PresentHook phookD3D11Present = NULL;
	D3D11ResizeBuffersHook phookD3D11ResizeBuffers = NULL;
	ID3D11RenderTargetView* RenderTargetView = NULL;
	D3D11DrawIndexedHook phookD3D11DrawIndexed = NULL;
	D3D11DrawIndexedInstancedHook phookD3D11DrawIndexedInstanced = NULL;
	D3D11PSSetShaderResourcesHook phookD3D11PSSetShaderResources = NULL;
	D3D11DrawHook phookD3D11Draw = NULL;
	D3D11CreateQueryHook phookD3D11CreateQuery = NULL;

	ID3D11Device* pDevice = NULL;
	ID3D11DeviceContext* pContext = NULL;

	// D3D11 Variables
	ID3D11Buffer* vertexBuffer; // Vertex ->
	D3D11_BUFFER_DESC vertexDesc;
	UINT vertexStride;
	UINT vertexOffset; // <-

	ID3D11Buffer* indexBuffer; // Index ->
	D3D11_BUFFER_DESC indexDesc;
	DXGI_FORMAT indexFormat;
	UINT indexOffset; // <-

	ID3D11Buffer* psgetBuffer; // PSGet ->
	D3D11_BUFFER_DESC psgetDesc;
	UINT psgetOffset; // <-

	ID3D11Buffer* vsgetBuffer; // VSGet ->
	D3D11_BUFFER_DESC vsgetDesc;
	UINT vsgetOffset; // <-

	// D3D11 Visuals
	ID3D11DepthStencilState* DepthStencilState = NULL;
	ID3D11RasterizerState* DepthbiassStateTrue;
	ID3D11RasterizerState* DepthbiassStateFalse;
	#define CalculateDepth(d) (d/(1/pow(2,23)))

	// D3D11 Viewport
	HRESULT swapchainResult;
	D3D11_VIEWPORT viewportBuffer;
	UINT viewportOffset;
	float screenCenterX;
	float screenCenterY;

	// D3D11 Hook Class
	DWORD_PTR* pSwapChainVtable = NULL;
	DWORD_PTR* pContextVTable = NULL;
	DWORD_PTR* pDeviceVTable = NULL;

	// Menu Variables
	WNDPROC handler = nullptr;
	HWND window = nullptr;
	BOOL once = true;
	BOOL menu = false;

	// Functions

	LRESULT CALLBACK HookHWND(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		ImGuiIO& io = ImGui::GetIO();
		POINT position;

		GetCursorPos(&position);
		ScreenToClient(window, &position);
		ImGui::GetIO().MousePos.x = (float)position.x;
		ImGui::GetIO().MousePos.y = (float)position.y;

		if (menu)
		{
			ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
			return true;
		}

		return CallWindowProc(handler, hWnd, uMsg, wParam, lParam);
	}
	
	HRESULT CALLBACK HookResizeBuffers(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
	{
		ImGui_ImplDX11_InvalidateDeviceObjects();
		if (nullptr != RenderTargetView) { RenderTargetView->Release(); RenderTargetView = nullptr; }
		HRESULT toReturn = phookD3D11ResizeBuffers(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
		ImGui_ImplDX11_CreateDeviceObjects();
		return toReturn;

	}

	HRESULT CALLBACK HookPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
	{
		if (once)
		{
			once = false;

			if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice)))
			{
				pSwapChain->GetDevice(__uuidof(pDevice), (void**)&pDevice);
				pDevice->GetImmediateContext(&pContext);
			}

			DXGI_SWAP_CHAIN_DESC swpchainDesc;
			pSwapChain->GetDesc(&swpchainDesc);
			ImGui::CreateContext();
			ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantTextInput || ImGui::GetIO().WantCaptureKeyboard;
			
			static const ImWchar icons_ranges[] = { 0xf000, 0xf3ff, 0 };
			ImFontConfig icons_config;
			icons_config.MergeMode = true;
			icons_config.PixelSnapH = true;
			icons_config.OversampleH = (int)2.5;
			icons_config.OversampleV = (int)2.5;

			ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\verdana.ttf", 16);
			ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(font_awesome_data, font_awesome_size, 16.0f, &icons_config, icons_ranges);
			
			window = swpchainDesc.OutputWindow;

			handler = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)HookHWND);

			ImGui_ImplWin32_Init(window);
			ImGui_ImplDX11_Init(pDevice, pContext);
			ImGui::GetIO().ImeWindowHandle = window;

			D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
			depthStencilDesc.DepthEnable = TRUE;
			depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			depthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
			depthStencilDesc.StencilEnable = FALSE;
			depthStencilDesc.StencilReadMask = 0xFF;
			depthStencilDesc.StencilWriteMask = 0xFF;
			depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
			depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
			depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
			depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
			depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
			depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

			pDevice->CreateDepthStencilState(&depthStencilDesc, &DepthStencilState);

			D3D11_RASTERIZER_DESC rasterizer_desc;
			ZeroMemory(&rasterizer_desc, sizeof(rasterizer_desc));
			rasterizer_desc.FillMode = D3D11_FILL_SOLID;
			rasterizer_desc.CullMode = D3D11_CULL_NONE;
			rasterizer_desc.FrontCounterClockwise = false;
			float bias = 1000.0f;
			float bias_float = static_cast<float>(-bias);
			bias_float /= 10000.0f;
			rasterizer_desc.DepthBias = (INT)CalculateDepth(*(DWORD*)&bias_float);
			rasterizer_desc.SlopeScaledDepthBias = 0.0f;
			rasterizer_desc.DepthBiasClamp = 0.0f;
			rasterizer_desc.DepthClipEnable = true;
			rasterizer_desc.ScissorEnable = false;
			rasterizer_desc.MultisampleEnable = false;
			rasterizer_desc.AntialiasedLineEnable = false;

			pDevice->CreateRasterizerState(&rasterizer_desc, &DepthbiassStateFalse);

			D3D11_RASTERIZER_DESC nrasterizer_desc;
			ZeroMemory(&nrasterizer_desc, sizeof(nrasterizer_desc));
			nrasterizer_desc.FillMode = D3D11_FILL_SOLID;
			nrasterizer_desc.CullMode = D3D11_CULL_NONE;
			nrasterizer_desc.FrontCounterClockwise = false;
			nrasterizer_desc.DepthBias = (INT)0.0f;
			nrasterizer_desc.SlopeScaledDepthBias = 0.0f;
			nrasterizer_desc.DepthBiasClamp = 0.0f;
			nrasterizer_desc.DepthClipEnable = true;
			nrasterizer_desc.ScissorEnable = false;
			nrasterizer_desc.MultisampleEnable = false;
			nrasterizer_desc.AntialiasedLineEnable = false;
			pDevice->CreateRasterizerState(&nrasterizer_desc, &DepthbiassStateTrue);
		}

		if (RenderTargetView == NULL)
		{
			pContext->RSGetViewports(&viewportOffset, &viewportBuffer);
			screenCenterX = viewportBuffer.Width / 2.0f;
			screenCenterY = viewportBuffer.Height / 2.0f;

			ID3D11Texture2D* backbuffer = NULL;
			swapchainResult = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backbuffer);
			if (FAILED(swapchainResult))
			{
				return swapchainResult;
			}

			swapchainResult = pDevice->CreateRenderTargetView(backbuffer, NULL, &RenderTargetView);
			backbuffer->Release();
			if (FAILED(swapchainResult))
			{
				return swapchainResult;
			}
		}
		else
		{
			pContext->OMSetRenderTargets(1, &RenderTargetView, NULL);
		}

		ImGui_ImplWin32_NewFrame();
		ImGui_ImplDX11_NewFrame();
		ImGui::NewFrame();

		// GUI Here

		if (menu)
		{
			ImGui::GetIO().MouseDrawCursor = true;
			GUI::InitHook();
		}
		else
			ImGui::GetIO().MouseDrawCursor = false;

		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		return phookD3D11Present(pSwapChain, SyncInterval, Flags);
	}

	void CALLBACK HookDrawIndexed(ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation)
	{
		pContext->IAGetVertexBuffers(0, 1, &vertexBuffer, &vertexStride, &vertexOffset);
		if (vertexBuffer != NULL)
			vertexBuffer->GetDesc(&vertexDesc);
		if (vertexBuffer != NULL) { vertexBuffer->Release(); vertexBuffer = NULL; }

		pContext->IAGetIndexBuffer(&indexBuffer, &indexFormat, &indexOffset);
		if (indexBuffer != NULL)
			indexBuffer->GetDesc(&indexDesc);
		if (indexBuffer != NULL) { indexBuffer->Release(); indexBuffer = NULL; }

		pContext->PSGetConstantBuffers(psgetOffset, 1, &psgetBuffer);
		if (psgetBuffer != NULL)
			psgetBuffer->GetDesc(&psgetDesc);
		if (psgetBuffer != NULL) { psgetBuffer->Release(); psgetBuffer = NULL; }

		pContext->VSGetConstantBuffers(vsgetOffset, 1, &vsgetBuffer);
		if (vsgetBuffer != NULL)
			vsgetBuffer->GetDesc(&vsgetDesc);
		if (vsgetBuffer != NULL) { vsgetBuffer->Release(); vsgetBuffer = NULL; }

		return phookD3D11DrawIndexed(pContext, IndexCount, StartIndexLocation, BaseVertexLocation);
	}

	void CALLBACK HookDrawIndexedInstanced(ID3D11DeviceContext* pContext, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation)
	{
		pContext->IAGetVertexBuffers(0, 1, &vertexBuffer, &vertexStride, &vertexOffset);
		if (vertexBuffer != NULL)
			vertexBuffer->GetDesc(&vertexDesc);
		if (vertexBuffer != NULL) { vertexBuffer->Release(); vertexBuffer = NULL; }

		pContext->IAGetIndexBuffer(&indexBuffer, &indexFormat, &indexOffset);
		if (indexBuffer != NULL)
			indexBuffer->GetDesc(&indexDesc);
		if (indexBuffer != NULL) { indexBuffer->Release(); indexBuffer = NULL; }

		pContext->PSGetConstantBuffers(psgetOffset, 1, &psgetBuffer);
		if (psgetBuffer != NULL)
			psgetBuffer->GetDesc(&psgetDesc);
		if (psgetBuffer != NULL) { psgetBuffer->Release(); psgetBuffer = NULL; }

		pContext->VSGetConstantBuffers(vsgetOffset, 1, &vsgetBuffer);
		if (vsgetBuffer != NULL)
			vsgetBuffer->GetDesc(&vsgetDesc);
		if (vsgetBuffer != NULL) { vsgetBuffer->Release(); vsgetBuffer = NULL; }

		return phookD3D11DrawIndexedInstanced(pContext, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
	}

	void CALLBACK HookShaders(ID3D11DeviceContext* pContext, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const* ppShaderResourceViews)
	{
		if (GetAsyncKeyState(VK_INSERT) & true)
			menu = !menu;

		return phookD3D11PSSetShaderResources(pContext, StartSlot, NumViews, ppShaderResourceViews);
	}

	void CALLBACK HookDraw(ID3D11DeviceContext* pContext, UINT VertexCount, UINT StartVertexLocation)
	{
		return phookD3D11Draw(pContext, VertexCount, StartVertexLocation);
	}

	void CALLBACK CreateQuery(ID3D11Device* pDevice, const D3D11_QUERY_DESC* pQueryDesc, ID3D11Query** ppQuery)
	{
		return phookD3D11CreateQuery(pDevice, pQueryDesc, ppQuery);
	}

	DWORD CALLBACK InitHook(LPVOID)
	{
		IDXGISwapChain* pSwapChain;
		WNDCLASSEXA wc = { sizeof(WNDCLASSEX), CS_CLASSDC, DXGIMsgProc, 0L, 0L, GetModuleHandleA(NULL), NULL, NULL, NULL, NULL, "Valve001", NULL };
		RegisterClassExA(&wc);
		HWND hWnd = CreateWindowA("Valve001", NULL, WS_OVERLAPPEDWINDOW, 100, 100, 300, 300, NULL, NULL, wc.hInstance, NULL);

		D3D_FEATURE_LEVEL requestedLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1 };
		D3D_FEATURE_LEVEL obtainedLevel;
		ID3D11Device* d3dDevice = nullptr;
		ID3D11DeviceContext* d3dContext = nullptr;

		DXGI_SWAP_CHAIN_DESC scd;
		ZeroMemory(&scd, sizeof(scd));
		scd.BufferCount = 1;
		scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

		scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		scd.OutputWindow = hWnd;
		scd.SampleDesc.Count = true;
		scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		scd.Windowed = ((GetWindowLongPtr(hWnd, GWL_STYLE) & WS_POPUP) != 0) ? false : true;

		scd.BufferDesc.Width = 1;
		scd.BufferDesc.Height = 1;
		scd.BufferDesc.RefreshRate.Numerator = 0;
		scd.BufferDesc.RefreshRate.Denominator = 1;

		UINT createFlags = 0;
		IDXGISwapChain* d3dSwapChain = 0;

		
		if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createFlags, requestedLevels, sizeof(requestedLevels) / sizeof(D3D_FEATURE_LEVEL), D3D11_SDK_VERSION, &scd, &pSwapChain, &pDevice, &obtainedLevel, &pContext)))
		{
			MessageBox(hWnd, "Failed to create directX device and swapchain!", "Error", MB_ICONERROR);
			return NULL;
		}

		pSwapChainVtable = (DWORD_PTR*)pSwapChain;
		pSwapChainVtable = (DWORD_PTR*)pSwapChainVtable[0];

		pContextVTable = (DWORD_PTR*)pContext;
		pContextVTable = (DWORD_PTR*)pContextVTable[0];

		pDeviceVTable = (DWORD_PTR*)pDevice;
		pDeviceVTable = (DWORD_PTR*)pDeviceVTable[0];

		phookD3D11Present = (D3D11PresentHook)(DWORD_PTR*)pSwapChainVtable[8];
		phookD3D11ResizeBuffers = (D3D11ResizeBuffersHook)(DWORD_PTR*)pSwapChainVtable[13];
		phookD3D11PSSetShaderResources = (D3D11PSSetShaderResourcesHook)(DWORD_PTR*)pContextVTable[8];
		phookD3D11Draw = (D3D11DrawHook)(DWORD_PTR*)pContextVTable[13];
		phookD3D11DrawIndexed = (D3D11DrawIndexedHook)(DWORD_PTR*)pContextVTable[12];
		phookD3D11DrawIndexedInstanced = (D3D11DrawIndexedInstancedHook)(DWORD_PTR*)pContextVTable[20];

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());

		DetourAttach(&(LPVOID&)phookD3D11Present, (PBYTE)HookPresent);
		DetourAttach(&(LPVOID&)phookD3D11ResizeBuffers, (PBYTE)HookResizeBuffers);
		DetourAttach(&(LPVOID&)phookD3D11PSSetShaderResources, (PBYTE)HookShaders);
		DetourAttach(&(LPVOID&)phookD3D11Draw, (PBYTE)HookDraw);
		DetourAttach(&(LPVOID&)phookD3D11DrawIndexed, (PBYTE)HookDrawIndexed);
		DetourAttach(&(LPVOID&)phookD3D11DrawIndexedInstanced, (PBYTE)HookDrawIndexedInstanced);
		DetourTransactionCommit();

		DWORD dwOld;
		VirtualProtect(phookD3D11Present, 2, PAGE_EXECUTE_READWRITE, &dwOld);

		while (true)
		{
			Sleep(10);
		}

		pDevice->Release();
		pContext->Release();
		pSwapChain->Release();

		return NULL;
	}
}

BOOL WINAPI DllMain(_In_ HINSTANCE hinstDLL, _In_ DWORD fdwReason, _In_ LPVOID lpvReserved)
{
	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH:
		{
			CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)D3DHook::InitHook, NULL, NULL, NULL);
			break;
		}
	}

	return true;
}

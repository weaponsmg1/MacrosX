#include <Windows.h>
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx9.h"
#include "ImGui/imgui_impl_win32.h"
#include <d3d9.h>
#include <tchar.h>
#include <string>
#include <fstream>
#include <vector>
#include <filesystem>
#include "json/json.hpp"
#include "json/json_fwd.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;

int speed = 0;
int speedx = 0;
bool enable = false;
bool isBindingKey = false;
int bindKey = 0;
bool sound = false;
static bool useStaticBackground = false;
static ImVec4 staticBackgroundColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
static char configName[128] = "config1";
static char visualConfigName[128] = "visual1";
static std::vector<std::string> configFiles;
static std::vector<std::string> visualConfigFiles;
static int selectedConfig = -1;
static int selectedVisualConfig = -1;

void MoveMouse();

std::string GetExeDirectory() {
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    std::string exePath = path;
    return exePath.substr(0, exePath.find_last_of("\\/")) + "\\";
}

void PlaySoundFromExeFolder(const char* filename) {
    std::string soundPath = GetExeDirectory() + filename;
    PlaySoundA(soundPath.c_str(), NULL, SND_FILENAME | SND_ASYNC);
}

void RefreshConfigLists() {
    configFiles.clear();
    visualConfigFiles.clear();

    std::string exeDir = GetExeDirectory();

    for (const auto& entry : fs::directory_iterator(exeDir)) {
        if (entry.path().extension() == ".json") {
            std::string filename = entry.path().stem().string();
            if (filename.find("visual_") == 0) {
                visualConfigFiles.push_back(filename.substr(7));
            }
            else {
                configFiles.push_back(filename);
            }
        }
    }
}

void SaveVisualConfig(const std::string& filename) {
    json config;

    ImGuiStyle& style = ImGui::GetStyle();

    for (int i = 0; i < ImGuiCol_COUNT; i++) {
        const char* name = ImGui::GetStyleColorName(i);
        config["colors"][name] = {
            style.Colors[i].x,
            style.Colors[i].y,
            style.Colors[i].z,
            style.Colors[i].w
        };
    }

    config["style"]["WindowRounding"] = style.WindowRounding;
    config["style"]["FrameRounding"] = style.FrameRounding;
    config["style"]["GrabRounding"] = style.GrabRounding;
    config["style"]["TabRounding"] = style.TabRounding;

    std::ofstream file(GetExeDirectory() + "visual_" + filename + ".json");
    if (file.is_open()) {
        file << config.dump(4);
        file.close();
    }

    RefreshConfigLists();
}

bool LoadVisualConfig(const std::string& filename) {
    std::ifstream file(GetExeDirectory() + "visual_" + filename + ".json");
    if (!file.is_open()) {
        return false;
    }

    try {
        json config;
        file >> config;

        ImGuiStyle& style = ImGui::GetStyle();

        if (config.contains("colors")) {
            for (int i = 0; i < ImGuiCol_COUNT; i++) {
                const char* name = ImGui::GetStyleColorName(i);
                if (config["colors"].contains(name)) {
                    auto color = config["colors"][name];
                    style.Colors[i] = ImVec4(
                        color[0].get<float>(),
                        color[1].get<float>(),
                        color[2].get<float>(),
                        color[3].get<float>()
                    );
                }
            }
        }

        if (config.contains("style")) {
            style.WindowRounding = config["style"].value("WindowRounding", style.WindowRounding);
            style.FrameRounding = config["style"].value("FrameRounding", style.FrameRounding);
            style.GrabRounding = config["style"].value("GrabRounding", style.GrabRounding);
            style.TabRounding = config["style"].value("TabRounding", style.TabRounding);
        }

        file.close();
        return true;
    }
    catch (...) {
        return false;
    }
}

void SaveConfig(const std::string& filename) {
    json config;

    config["speed"] = speed;
    config["speedx"] = speedx;
    config["rcsEnabled"] = enable;
    config["bindKey"] = bindKey;
    config["sound"] = sound;
    config["useStaticBackground"] = useStaticBackground;
    config["staticBackgroundColor"] = {
        staticBackgroundColor.x,
        staticBackgroundColor.y,
        staticBackgroundColor.z,
        staticBackgroundColor.w
    };

    std::ofstream file(GetExeDirectory() + filename + ".json");
    if (file.is_open()) {
        file << config.dump(4);
        file.close();
    }

    RefreshConfigLists();
}

bool LoadConfig(const std::string& filename) {
    std::ifstream file(GetExeDirectory() + filename + ".json");
    if (!file.is_open()) {
        return false;
    }

    try {
        json config;
        file >> config;

        speed = config.value("speed", 4);
        speedx = config.value("speedx", -2);
        enable = config.value("rcsEnabled", false);
        bindKey = config.value("bindKey", 0);
        sound = config.value("sound", false);
        useStaticBackground = config.value("useStaticBackground", false);

        if (config.contains("staticBackgroundColor")) {
            auto color = config["staticBackgroundColor"];
            staticBackgroundColor = ImVec4(
                color[0].get<float>(),
                color[1].get<float>(),
                color[2].get<float>(),
                color[3].get<float>()
            );
        }

        file.close();
        return true;
    }
    catch (...) {
        return false;
    }
}

void MoveMouse() {
    if (enable && (GetAsyncKeyState(VK_LBUTTON) & 0x8000)) {
        POINT p;
        GetCursorPos(&p);

        int deltaY = static_cast<int>(speed);
        int deltaX = static_cast<int>(speedx);

        mouse_event(MOUSEEVENTF_MOVE, deltaX, deltaY, 0, 0);
        Sleep(10);
    }

    static bool keyWasPressed = false;
    if (GetAsyncKeyState(bindKey) & 0x8000) {
        if (!keyWasPressed) {
            enable = !enable;
            keyWasPressed = true;

            if (sound) {
                if (enable) {
                    PlaySoundFromExeFolder("on.wav");
                }
                else {
                    PlaySoundFromExeFolder("off.wav");
                }
            }
        }
    }
    else {
        keyWasPressed = false;
    }
}

static LPDIRECT3D9              g_pD3D = nullptr;
static LPDIRECT3DDEVICE9        g_pd3dDevice = nullptr;
static bool                     g_DeviceLost = false;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ResetDevice();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    ImGui_ImplWin32_EnableDpiAwareness();
    float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));

    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"MacrosX", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"MacrosX", WS_OVERLAPPEDWINDOW, 100, 100, (int)(1280 * main_scale), (int)(800 * main_scale), nullptr, nullptr, wc.hInstance, nullptr);

    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);

    RefreshConfigLists();

    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    bool done = false;
    while (!done)
    {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        MoveMouse();

        if (g_DeviceLost)
        {
            HRESULT hr = g_pd3dDevice->TestCooperativeLevel();
            if (hr == D3DERR_DEVICELOST)
            {
                ::Sleep(10);
                continue;
            }
            if (hr == D3DERR_DEVICENOTRESET)
                ResetDevice();
            g_DeviceLost = false;
        }

        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            g_d3dpp.BackBufferWidth = g_ResizeWidth;
            g_d3dpp.BackBufferHeight = g_ResizeHeight;
            g_ResizeWidth = g_ResizeHeight = 0;
            ResetDevice();
        }

        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        {
            ImGui::SetNextWindowSize(ImVec2(320, 320), ImGuiCond_FirstUseEver);

            ImGui::Begin("MacrosX", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

            if (ImGui::BeginTabBar("MainTabs"))
            {
                if (ImGui::BeginTabItem("Macros"))
                {
                    ImGui::Checkbox("Enable", &enable);
                    ImGui::Checkbox("Toggle Sound", &sound);
                    ImGui::SliderInt("SpeedX", &speedx, -10, 10);
                    ImGui::SliderInt("SpeedY", &speed, 0, 10);

                    if (ImGui::Button(isBindingKey ? "Press any key..." : ("Bind Key: " + std::string(bindKey == -1 ? "None" : (char*)&bindKey)).c_str())) {
                        isBindingKey = true;
                    }

                    if (isBindingKey) {
                        for (int i = 0; i < 256; i++) {
                            if (GetAsyncKeyState(i) & 0x8000) {
                                if (i != VK_ESCAPE) {
                                    bindKey = i;
                                }
                                isBindingKey = false;
                                break;
                            }
                        }
                    }

                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Window"))
                {
                    ImGui::Checkbox("Static background", &useStaticBackground);
                    if (useStaticBackground) {
                        ImGui::ColorEdit4("Background color", (float*)&staticBackgroundColor);
                    }

                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Visual"))
                {
                    const char* styles[] = { "Dark", "Light" };
                    static int current_style = 0;
                    if (ImGui::Combo("Theme", &current_style, styles, IM_ARRAYSIZE(styles)))
                    {
                        switch (current_style)
                        {
                        case 0: ImGui::StyleColorsDark(); break;
                        case 1: ImGui::StyleColorsLight(); break;
                        }
                    }

                    ImGui::SliderFloat("Window Rounding", &ImGui::GetStyle().WindowRounding, 0.0f, 12.0f);
                    ImGui::SliderFloat("Frame Rounding", &ImGui::GetStyle().FrameRounding, 0.0f, 12.0f);

                    if (ImGui::TreeNode("Colors"))
                    {
                        ImGuiColorEditFlags flags = ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreview;
                        ImGui::ColorEdit4("Text", (float*)&ImGui::GetStyle().Colors[ImGuiCol_Text], flags);
                        ImGui::ColorEdit4("Window BG", (float*)&ImGui::GetStyle().Colors[ImGuiCol_WindowBg], flags);
                        ImGui::ColorEdit4("Frame BG", (float*)&ImGui::GetStyle().Colors[ImGuiCol_FrameBg], flags);
                        ImGui::ColorEdit4("Button", (float*)&ImGui::GetStyle().Colors[ImGuiCol_Button], flags);
                        ImGui::ColorEdit4("Header", (float*)&ImGui::GetStyle().Colors[ImGuiCol_Header], flags);
                        ImGui::TreePop();
                    }

                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Config"))
                {
                    if (ImGui::BeginTabBar("ConfigTabs"))
                    {
                        if (ImGui::BeginTabItem("Macros"))
                        {
                            ImGui::InputText("Name", configName, IM_ARRAYSIZE(configName));

                            if (ImGui::Button("Save")) {
                                SaveConfig(configName);
                            }

                            ImGui::Separator();

                            ImGui::ListBox("##ConfigList", &selectedConfig, [](void* data, int idx, const char** out_text) {
                                *out_text = configFiles[idx].c_str();
                                return true;
                                }, nullptr, configFiles.size(), 5);

                            if (selectedConfig >= 0 && selectedConfig < configFiles.size()) {
                                if (ImGui::Button("Load")) {
                                    LoadConfig(configFiles[selectedConfig]);
                                }

                                ImGui::SameLine();

                                if (ImGui::Button("Delete")) {
                                    std::string path = GetExeDirectory() + configFiles[selectedConfig] + ".json";
                                    fs::remove(path);
                                    RefreshConfigLists();
                                    selectedConfig = -1;
                                }
                            }

                            ImGui::EndTabItem();
                        }

                        if (ImGui::BeginTabItem("Visual"))
                        {
                            ImGui::InputText("Name", visualConfigName, IM_ARRAYSIZE(visualConfigName));

                            if (ImGui::Button("Save")) {
                                SaveVisualConfig(visualConfigName);
                            }

                            ImGui::Separator();

                            ImGui::ListBox("##VisualConfigList", &selectedVisualConfig, [](void* data, int idx, const char** out_text) {
                                *out_text = visualConfigFiles[idx].c_str();
                                return true;
                                }, nullptr, visualConfigFiles.size(), 5);

                            if (selectedVisualConfig >= 0 && selectedVisualConfig < visualConfigFiles.size()) {
                                if (ImGui::Button("Load")) {
                                    LoadVisualConfig(visualConfigFiles[selectedVisualConfig]);
                                }

                                ImGui::SameLine();

                                if (ImGui::Button("Delete")) {
                                    std::string path = GetExeDirectory() + "visual_" + visualConfigFiles[selectedVisualConfig] + ".json";
                                    fs::remove(path);
                                    RefreshConfigLists();
                                    selectedVisualConfig = -1;
                                }
                            }

                            ImGui::EndTabItem();
                        }

                        ImGui::EndTabBar();
                    }

                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }

            ImGui::End();
        }

        ImGui::EndFrame();
        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

        if (useStaticBackground) {
            clear_color = staticBackgroundColor;
        }
        else {
            float time = GetTickCount() / 1000.0f;
            clear_color.x = (sinf(time * 0.8f) + 1.0f) * 0.5f;
            clear_color.y = (sinf(time * 1.0f + 2.0f) + 1.0f) * 0.5f;
            clear_color.z = (sinf(time * 1.3f + 4.0f) + 1.0f) * 0.5f;
            clear_color.w = 1.0f;
        }

        D3DCOLOR clear_col_dx = D3DCOLOR_RGBA(
            (int)(clear_color.x * 255.0f),
            (int)(clear_color.y * 255.0f),
            (int)(clear_color.z * 255.0f),
            (int)(clear_color.w * 255.0f)
        );
        g_pd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
        if (g_pd3dDevice->BeginScene() >= 0)
        {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }
        HRESULT result = g_pd3dDevice->Present(nullptr, nullptr, nullptr, nullptr);
        if (result == D3DERR_DEVICELOST)
            g_DeviceLost = true;
    }

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

bool CreateDeviceD3D(HWND hWnd)
{
    if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr)
        return false;

    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
    if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
        return false;

    return true;
}

void CleanupDeviceD3D()
{
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
    if (g_pD3D) { g_pD3D->Release(); g_pD3D = nullptr; }
}

void ResetDevice()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
    if (hr == D3DERR_INVALIDCALL)
        IM_ASSERT(0);
    ImGui_ImplDX9_CreateDeviceObjects();
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam);
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
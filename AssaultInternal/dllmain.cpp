#include <windows.h>
#include <gl/GL.h>

#include "includes/imgui/imgui.h"
#include "includes/imgui/imgui_impl_opengl3.h"
#include "includes/imgui/imgui_impl_win32.h"
#include "includes/imgui/imgui_internal.h"

#include "includes/minhook/minhook.h"

#include "memory/mem.h"
#include <string>

typedef BOOL(__stdcall* twglSwapBuffers)(HDC hDc);

HWND g_hwnd = NULL;
HMODULE g_hmodule = NULL;
twglSwapBuffers wglSwapBuffersGateway;

WNDPROC oWndProc;
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam); 

bool health = false;
bool ammo = false;
bool ghost = false;


bool show = false;


LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (show)
    {
        if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
            return true;

        ImGuiIO& io = ImGui::GetIO();

        switch (uMsg)
        {
        case WM_LBUTTONDOWN:
            io.MouseDown[1] = !io.MouseDown[0];
            return 0;
        case WM_RBUTTONDOWN:
            io.MouseDown[1] = !io.MouseDown[1];
            return 0;
        case WM_MBUTTONDOWN:
            io.MouseDown[2] = !io.MouseDown[2];
            return 0;
        case WM_MOUSEWHEEL:
            return 0;
        case WM_MOUSEMOVE:
            io.MousePos.x = (signed short)(lParam);
            io.MousePos.y = (signed short)(lParam >> 16);
            return 0;
        }
    }
    return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

BOOL CALLBACK find_window(HWND hwnd, LPARAM lParam)
{
    DWORD lpdwProcessId;
    GetWindowThreadProcessId(hwnd, &lpdwProcessId);
    if (lpdwProcessId == lParam)
    {
        g_hwnd = hwnd;
        return FALSE;
    }
    return TRUE;
}

BOOL __stdcall update(HDC hDc)
{
    static bool init = false;

    if (!init)
    {
        EnumWindows(find_window, GetCurrentProcessId());

        // mouse input fix
        if (!IsWindowVisible(g_hwnd))
            return wglSwapBuffersGateway(hDc);

        oWndProc = (WNDPROC)SetWindowLongPtrA(g_hwnd, GWLP_WNDPROC, (LONG_PTR)WndProc);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = NULL;

        ImGui::StyleColorsDark();
        ImGui_ImplWin32_Init(g_hwnd);
        ImGui_ImplOpenGL3_Init();

        init = true;
        
    }

    if (GetAsyncKeyState(VK_INSERT) & 1)
    {
        show = !show;
    }

    if (GetAsyncKeyState(VK_F1) & 1)
    {
        health = !health;
    }

    if (GetAsyncKeyState(VK_F2) & 1)
    {
        ammo = !ammo;
    }

    if (GetAsyncKeyState(VK_F3) & 1)
    {
        ghost = !ghost;
    }

    if (show)
    {
        //ShowCursor(TRUE);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGuiIO& io = ImGui::GetIO();

        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Once);
        ImGui::Begin("Haha car go brrrr...");

        ImGui::Checkbox("Health", &health);
        ImGui::Checkbox("Ammo", &ammo);
        ImGui::Checkbox("Ghost", &ghost);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        ImGui::End();

        io.MouseDrawCursor = io.WantCaptureMouse;

        ImGui::EndFrame();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
    else
        ShowCursor(FALSE);
        ShowCursor(FALSE);

    return wglSwapBuffersGateway(hDc);
}

DWORD WINAPI hook_init(HMODULE hModule)
{
    g_hmodule = hModule;

    MH_STATUS status = MH_Initialize();
    if (status != MH_OK)
        return 1;

    HMODULE hMod = GetModuleHandleA("opengl32.dll");
    if (hMod == NULL)
        return 1;

    FARPROC swapbuffers_proc = GetProcAddress(hMod, "wglSwapBuffers");
    if (swapbuffers_proc == NULL)
        return 1;

    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    printf("Hook Initialised!");

    if (MH_CreateHook(swapbuffers_proc, &update, (LPVOID*)&wglSwapBuffersGateway) != MH_OK)
        return 1;

    if (MH_EnableHook(swapbuffers_proc) != MH_OK)
        return 1;

    return 0;
}

DWORD WINAPI hackThread(HMODULE hModule)
{
    uintptr_t moduleBase = (uintptr_t)GetModuleHandle(L"ac_client.exe");
    uintptr_t* localPlayerPtr = (uintptr_t*)(moduleBase + 0x10f4f4);

    if (localPlayerPtr == NULL)
    {
        uintptr_t* localPlayerPtr = (uintptr_t*)(moduleBase + 0x10f4f4);      
    }

    while (true)
    {
        if (health)
        {
            *(int*)(*localPlayerPtr + 0xf8) = 999;
        }

        if (ammo)
        {
            *(int*)mem::FindDMAAdy(moduleBase + 0x10f4f4, {0x374, 0x14, 0x0}) = 999;
            // 0x0150 -> rifle
            // 0x13c -> pistol
            // 0x158 -> grenade
        }

        if (ghost)
        {
            BYTE* ghostMode = (BYTE*)(*localPlayerPtr + 0x76);
            BYTE* flyMode = (BYTE*)(*localPlayerPtr + 0x318);
            *ghostMode = 6;
            *flyMode = 5;
        }

    }

    return 0;
}

DWORD WINAPI TriggerThread(HMODULE hModule)
{ 

    return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)hook_init, hModule, 0, nullptr));
            CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)TriggerThread, hModule, 0, nullptr));
            CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)hackThread, hModule, 0, nullptr));
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}


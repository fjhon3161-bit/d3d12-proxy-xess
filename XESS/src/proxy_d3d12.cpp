// proxy_d3d12.cpp
// DLL proxy for d3d12 with MinHook to intercept Present() and run XeSS.
// Educational skeleton — frame-level hook.

#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <string>
#include <xess_d3d12.h>   // Intel XeSS SDK
#include "MinHook.h"      // include MinHook headers

static HMODULE realD3D12 = nullptr;
static decltype(D3D12CreateDevice)* real_D3D12CreateDevice = nullptr;

static xess_context_handle_t g_xessContext = nullptr;

// Pointer to original Present
typedef HRESULT (STDMETHODCALLTYPE* Present_t)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
Present_t oPresent = nullptr;

// Hooked Present
HRESULT STDMETHODCALLTYPE hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
    if (g_xessContext)
    {
        // Example: execute XeSS here on the current backbuffer
        // NOTE: In real integration, you'd also need motion vectors, depth, etc.
        xess_d3d12_execute_params_t params = {};
        params.inputResource = nullptr; // TODO: bind actual low-res input
        params.outputResource = nullptr; // TODO: bind actual high-res output

        xessExecute(g_xessContext, &params);
        xessSynchronize(g_xessContext);
    }

    return oPresent(pSwapChain, SyncInterval, Flags);
}

// Forward D3D12CreateDevice
extern "C" HRESULT WINAPI D3D12CreateDevice(
    IUnknown* pAdapter, D3D_FEATURE_LEVEL MinimumFeatureLevel,
    REFIID riid, void** ppDevice)
{
    if (!realD3D12)
    {
        char sysPath[MAX_PATH];
        GetSystemDirectoryA(sysPath, MAX_PATH);
        std::string fullPath = std::string(sysPath) + "\\d3d12.dll";
        realD3D12 = LoadLibraryA(fullPath.c_str());
        real_D3D12CreateDevice =
            (decltype(D3D12CreateDevice)*)GetProcAddress(realD3D12, "D3D12CreateDevice");
    }

    HRESULT hr = real_D3D12CreateDevice(pAdapter, MinimumFeatureLevel, riid, ppDevice);

    if (SUCCEEDED(hr) && ppDevice && *ppDevice)
    {
        ID3D12Device* dev = (ID3D12Device*)(*ppDevice);

        // Init XeSS
        xess_d3d12_init_params_t initParams = {};
        initParams.device = dev;
        initParams.outputResolution.Width = 1920;
        initParams.outputResolution.Height = 1080;
        initParams.initFlags = 0;

        if (xessD3D12CreateContext(dev, &g_xessContext) == XESS_RESULT_SUCCESS)
        {
            xessD3D12Init(g_xessContext, &initParams);
            OutputDebugStringA("[XeSS Proxy] XeSS context initialized\n");
        }

        // Init MinHook
        if (MH_Initialize() == MH_OK)
        {
            IDXGIFactory4* factory;
            if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))))
            {
                IDXGIAdapter* adapter;
                factory->EnumAdapters(0, &adapter);

                DXGI_SWAP_CHAIN_DESC desc = {};
                desc.BufferCount = 2;
                desc.BufferDesc.Width = 640;
                desc.BufferDesc.Height = 480;
                desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
                desc.OutputWindow = GetConsoleWindow();
                desc.SampleDesc.Count = 1;
                desc.Windowed = TRUE;
                desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

                ID3D12CommandQueue* queue = nullptr;
                D3D12_COMMAND_QUEUE_DESC qdesc = {};
                qdesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
                dev->CreateCommandQueue(&qdesc, IID_PPV_ARGS(&queue));

                IDXGISwapChain* dummySwapChain = nullptr;
                factory->CreateSwapChain(queue, &desc, &dummySwapChain);

                void** vtbl = *(void***)dummySwapChain;
                void* pPresent = vtbl[8]; // Present is usually at index 8

                MH_CreateHook(pPresent, &hkPresent, reinterpret_cast<LPVOID*>(&oPresent));
                MH_EnableHook(pPresent);

                dummySwapChain->Release();
                queue->Release();
                adapter->Release();
                factory->Release();
            }
        }
    }
    return hr;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_DETACH)
    {
        if (g_xessContext)
        {
            xessDestroyContext(g_xessContext);
            g_xessContext = nullptr;
        }
        MH_Uninitialize();
        if (realD3D12)
        {
            FreeLibrary(realD3D12);
            realD3D12 = nullptr;
        }
    }
    return TRUE;
}

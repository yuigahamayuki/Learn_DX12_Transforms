#include "my_engine.h"

#include "d3dx12.h"
#include "win32_application.h"

MyEngine::MyEngine(UINT width, UINT height, std::wstring name) : DXSample(width, height, name),
  fence_values_{},
  width_(width), height_(height)
{
}

MyEngine::~MyEngine()
{
}

void MyEngine::OnInit()
{
  LoadPipeline();
  LoadAssets();
  LoadSizeDependentResources();
}

void MyEngine::OnUpdate()
{
  scene_->Update();
}

void MyEngine::OnRender()
{
  scene_->Render(command_queue_.Get());

  ThrowIfFailed(swap_chain_->Present(0, DXGI_PRESENT_ALLOW_TEARING));

  MoveToNextFrame();
}

void MyEngine::OnSizeChanged(UINT width, UINT height, bool minimized)
{
}

void MyEngine::OnDestroy()
{
}

void MyEngine::OnKeyDown(UINT8 key)
{
  scene_->KeyDown(key);
}

void MyEngine::OnKeyUp(UINT8 key)
{
  scene_->KeyUp(key);
}

void MyEngine::LoadPipeline()
{
  UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
  // Enable the debug layer (requires the Graphics Tools "optional feature").
  // NOTE: Enabling the debug layer after device creation will invalidate the active device.
  {
    ComPtr<ID3D12Debug> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
    {
      debugController->EnableDebugLayer();

      // Enable additional debug layers.
      dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
    }
  }
#endif

  ComPtr<IDXGIFactory6> dxgi_factory;
  ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&dxgi_factory)));

  ComPtr<IDXGIAdapter1> hardware_adapter;
  GetHardwareAdapter(dxgi_factory.Get(), &hardware_adapter, true);
  ThrowIfFailed(D3D12CreateDevice(hardware_adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device_)));

  D3D12_COMMAND_QUEUE_DESC command_queue_desc{};
  command_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  command_queue_desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
  command_queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  command_queue_desc.NodeMask = 0;  // single GPU
  ThrowIfFailed(device_->CreateCommandQueue(&command_queue_desc, IID_PPV_ARGS(&command_queue_)));

  DXGI_SWAP_CHAIN_DESC1 swap_chain_desc{};
  swap_chain_desc.Width = width_;
  swap_chain_desc.Height = height_;
  swap_chain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  swap_chain_desc.Stereo = false;
  swap_chain_desc.SampleDesc.Count = 1;
  swap_chain_desc.SampleDesc.Quality = 0;
  swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swap_chain_desc.BufferCount = kFrameCount;
  swap_chain_desc.Scaling = DXGI_SCALING_STRETCH;
  swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  swap_chain_desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
  swap_chain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
  bool prevIsFullscreen = Win32Application::IsFullscreen();
  if (prevIsFullscreen)
  {
    Win32Application::SetWindowZorderToTopMost(false);
  }
  ComPtr<IDXGISwapChain1> temp_swap_chain;
  ThrowIfFailed(dxgi_factory->CreateSwapChainForHwnd(command_queue_.Get(), Win32Application::GetHwnd(), &swap_chain_desc, nullptr, nullptr, &temp_swap_chain));
  if (prevIsFullscreen)
  {
    Win32Application::SetWindowZorderToTopMost(true);
  }
  ThrowIfFailed(dxgi_factory->MakeWindowAssociation(Win32Application::GetHwnd(), DXGI_MWA_NO_ALT_ENTER));
  ThrowIfFailed(temp_swap_chain.As(&swap_chain_));
  current_frame_index_ = swap_chain_->GetCurrentBackBufferIndex();

  ThrowIfFailed(device_->CreateFence(fence_values_[current_frame_index_], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_)));
  fence_values_[current_frame_index_]++;
  fence_event_ = CreateEvent(nullptr, false, false, nullptr);
  if (fence_event_ == nullptr) {
    ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
  }
}

void MyEngine::LoadAssets()
{
  if (!scene_) {
    scene_ = std::make_unique<Scene>(kFrameCount, width_, height_);
  }

  scene_->Initialize(device_.Get(), command_queue_.Get(), current_frame_index_);
  WaitForGPU();
}

void MyEngine::LoadSizeDependentResources()
{
  ComPtr<ID3D12Resource> render_targets[kFrameCount]{};
  for (UINT i = 0; i < kFrameCount; ++i) {
    ThrowIfFailed(swap_chain_->GetBuffer(i, IID_PPV_ARGS(&render_targets[i])));
  }

  scene_->LoadSizeDependentResources(device_.Get(), render_targets, width_, height_);
}

void MyEngine::WaitForGPU()
{
  command_queue_->Signal(fence_.Get(), fence_values_[current_frame_index_]);

  ThrowIfFailed(fence_->SetEventOnCompletion(fence_values_[current_frame_index_], fence_event_));
  WaitForSingleObjectEx(fence_event_, INFINITE, false);

  fence_values_[current_frame_index_]++;
}

void MyEngine::MoveToNextFrame()
{
  const UINT64 current_fence_value = fence_values_[current_frame_index_];
  ThrowIfFailed(command_queue_->Signal(fence_.Get(), current_fence_value));

  current_frame_index_ = swap_chain_->GetCurrentBackBufferIndex();

  if (fence_->GetCompletedValue() < fence_values_[current_frame_index_]) {
    ThrowIfFailed(fence_->SetEventOnCompletion(fence_values_[current_frame_index_], fence_event_));
    WaitForSingleObjectEx(fence_event_, INFINITE, false);
  }
  scene_->SetFrameIndex(current_frame_index_);

  fence_values_[current_frame_index_] = current_fence_value + 1;
}

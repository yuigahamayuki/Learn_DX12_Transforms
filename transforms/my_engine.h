#pragma once

#include "dx_sample.h"

#include "scene.h"

using Microsoft::WRL::ComPtr;

class MyEngine : public DXSample {
public:
  MyEngine(UINT width, UINT height, std::wstring name);
  ~MyEngine();

  static constexpr UINT kFrameCount = 3;

protected:
  void OnInit() override;
  void OnUpdate() override;
  void OnRender() override;
  void OnSizeChanged(UINT width, UINT height, bool minimized) override;
  void OnDestroy() override;

private:
  void LoadPipeline();
  void LoadAssets();
  void LoadSizeDependentResources();

  void WaitForGPU();
  void MoveToNextFrame();

  // D3D objects
  ComPtr<ID3D12Device> device_;
  ComPtr<IDXGISwapChain4> swap_chain_;
  ComPtr<ID3D12CommandQueue> command_queue_;
  ComPtr<ID3D12Fence> fence_;

  // Scene rendering resources.
  std::unique_ptr<Scene> scene_;

  // Frame synchronization objects
  UINT current_frame_index_ = 0;
  HANDLE fence_event_ = nullptr;
  UINT64 fence_values_[kFrameCount];

  UINT width_ = 0;
  UINT height_ = 0;
};
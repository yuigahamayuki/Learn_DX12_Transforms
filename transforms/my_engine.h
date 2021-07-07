#pragma once

#include "dx_sample.h"

using Microsoft::WRL::ComPtr;

class MyEngine : public DXSample {
public:
  MyEngine(UINT width, UINT height, std::wstring name);
  ~MyEngine();

  static const UINT kFrameCount = 3;

protected:
  void OnInit() override;
  void OnUpdate() override;
  void OnRender() override;
  void OnSizeChanged(UINT width, UINT height, bool minimized) override;
  void OnDestroy() override;

private:
  void LoadPipeline();
  void LoadAssets();

  ComPtr<ID3D12Device> device_;
  ComPtr<IDXGISwapChain4> swap_chain_;
  ComPtr<ID3D12CommandQueue> command_queue_;

  UINT current_frame_index_ = 0;

  UINT width_ = 0;
  UINT height_ = 0;
};
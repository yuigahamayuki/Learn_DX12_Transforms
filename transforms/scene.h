#pragma once

#include "common_headers.h"

using Microsoft::WRL::ComPtr;

class Scene {
public:
  Scene();
  ~Scene();

  void Render(ID3D12CommandQueue* command_queue);
private:
  void PopulateCommandLists();

  // D3D objects
  ComPtr<ID3D12PipelineState> pipeline_state_;
  ComPtr<ID3D12CommandAllocator> command_allocator_;
  ComPtr<ID3D12GraphicsCommandList> command_list_;
};
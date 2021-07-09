#pragma once

#include <vector>

#include "common_headers.h"
#include "d3dx12.h"

using Microsoft::WRL::ComPtr;

class Scene {
public:
  Scene(UINT frame_count, UINT width, UINT height);
  ~Scene();
  
  void Initialize(ID3D12Device* device, ID3D12CommandQueue* command_queue, UINT frame_index);
  void LoadSizeDependentResources(ID3D12Device* device, ComPtr<ID3D12Resource>* render_targets, UINT width, UINT height);
  void Render(ID3D12CommandQueue* command_queue);

  void SetFrameIndex(UINT frame_index) {
    current_frame_index_ = frame_index;
  }

private:
  void PopulateCommandLists();

  UINT frame_count_ = 0;
  UINT current_frame_index_ = 0;

  // D3D objects
  ComPtr<ID3D12PipelineState> pipeline_state_;
  std::vector<ComPtr<ID3D12CommandAllocator>> command_allocators_;
  ComPtr<ID3D12GraphicsCommandList> command_list_;
  std::vector<ComPtr<ID3D12Resource>> render_targets_;

  // Heap objects
  ComPtr<ID3D12DescriptorHeap> rtv_descriptor_heap_;
  UINT rtv_descriptor_increment_size_ = 0;

  CD3DX12_VIEWPORT view_port_;
  CD3DX12_RECT scissor_rect_;
};
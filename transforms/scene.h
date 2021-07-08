#pragma once

#include "common_headers.h"
#include "d3dx12.h"

using Microsoft::WRL::ComPtr;

class Scene {
public:
  Scene(UINT frame_count, UINT width, UINT height);
  ~Scene();
  
  void LoadSizeDependentResources(ID3D12Device* device, ComPtr<ID3D12Resource>* render_targets, UINT width, UINT height);
  void Render(ID3D12CommandQueue* command_queue);
private:
  void PopulateCommandLists();

  UINT frame_count_;

  // D3D objects
  ComPtr<ID3D12PipelineState> pipeline_state_;
  ComPtr<ID3D12CommandAllocator> command_allocator_;
  ComPtr<ID3D12GraphicsCommandList> command_list_;
  std::vector<ComPtr<ID3D12Resource>> render_targets_;

  // Heap objects
  ComPtr<ID3D12DescriptorHeap> rtv_descriptor_heap_;
  UINT rtv_descriptor_size_ = 0;

  CD3DX12_VIEWPORT view_port_;
  CD3DX12_RECT scissor_rect_;
};
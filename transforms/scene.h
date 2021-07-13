#pragma once

#include <vector>

#include "common_headers.h"
#include "d3dx12.h"
#include "camera.h"

using Microsoft::WRL::ComPtr;

struct InputState
{
  bool rightArrowPressed;
  bool leftArrowPressed;
  bool upArrowPressed;
  bool downArrowPressed;
};

struct SceneConstantBuffer {
  XMFLOAT4X4 model;
  XMFLOAT4X4 view;
  XMFLOAT4X4 proj;
};

class Scene {
public:
  Scene(UINT frame_count, UINT width, UINT height);
  ~Scene();
  
  void Initialize(ID3D12Device* device, ID3D12CommandQueue* command_queue, UINT frame_index);
  void LoadSizeDependentResources(ID3D12Device* device, ComPtr<ID3D12Resource>* render_targets, UINT width, UINT height);
  void Update();
  void Render(ID3D12CommandQueue* command_queue);
  void KeyDown(UINT8 key);
  void KeyUp(UINT8 key);

  void SetFrameIndex(UINT frame_index) {
    current_frame_index_ = frame_index;
  }

private:
  void CreateConstanfBuffer(ID3D12Device* device, UINT size, ID3D12Resource** ppResource, D3D12_RESOURCE_STATES initState);

  void CreateScenePipelineState(ID3D12Device* device);
  void CreateAndMapSceneConstantBuffer(ID3D12Device* device);
  void CreateAssets(ID3D12Device* device);
  void UpdateConstantBuffer();
  void CommitConstantBuffer();
  void SetCameras();
  void PopulateCommandLists();

  UINT frame_count_ = 0;
  UINT current_frame_index_ = 0;

  // D3D objects
  ComPtr<ID3D12RootSignature> root_signature_;
  ComPtr<ID3D12PipelineState> pipeline_state_;
  ComPtr<ID3D12Resource> scene_constant_buffer_view_;
  std::vector<ComPtr<ID3D12CommandAllocator>> command_allocators_;
  ComPtr<ID3D12GraphicsCommandList> command_list_;
  std::vector<ComPtr<ID3D12Resource>> render_targets_;
  ComPtr<ID3D12Resource> vertex_buffer_;
  ComPtr<ID3D12Resource> vertex_upload_heap_;
  ComPtr<ID3D12Resource> index_buffer_;
  ComPtr<ID3D12Resource> index_upload_heap_;
  D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view_{};
  D3D12_INDEX_BUFFER_VIEW index_buffer_view_{};

  // Heap objects
  ComPtr<ID3D12DescriptorHeap> rtv_descriptor_heap_;
  UINT rtv_descriptor_increment_size_ = 0;

  CD3DX12_VIEWPORT view_port_;
  CD3DX12_RECT scissor_rect_;

  InputState keyboard_input_;

  std::vector<Camera> cameras_;
  SceneConstantBuffer scene_constant_buffer_;
  void* scene_constant_buffer_pointer_ = nullptr;

  UINT camera_index_ = 0;
};
#include "scene.h"

#include "dx_sample_helper.h"

Scene::Scene(UINT frame_count, UINT width, UINT height) : frame_count_(frame_count),
  view_port_(0.0f, 0.0f, (float)width, (float)height),
  scissor_rect_(0, 0, width, height)
{

}

Scene::~Scene()
{
}

void Scene::Initialize(ID3D12Device* device, ID3D12CommandQueue* command_queue, UINT frame_index)
{
  if (device == nullptr || command_queue == nullptr) {
    return;
  }

  SetFrameIndex(frame_index);

  // TODO: create pso
  //D3D12_GRAPHICS_PIPELINE_STATE_DESC pipeline_state_desc{};
  //device->CreateGraphicsPipelineState()

  D3D12_COMMAND_QUEUE_DESC command_queue_desc{};
  command_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  command_queue_desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
  command_queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  command_queue_desc.NodeMask = 0;
  command_allocators_.resize(frame_count_);
  for (UINT i = 0; i < frame_count_; ++i) {
    device->CreateCommandQueue(&command_queue_desc, IID_PPV_ARGS(&command_allocators_[i]));
  }
  
  // TODO: pass pso pointer
  device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, command_allocators_[current_frame_index_].Get(), nullptr, IID_PPV_ARGS(&command_list_));
}

void Scene::LoadSizeDependentResources(ID3D12Device* device, ComPtr<ID3D12Resource>* render_targets, UINT width, UINT height)
{
  D3D12_DESCRIPTOR_HEAP_DESC rtv_descriptor_heap_desc{};
  rtv_descriptor_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
  rtv_descriptor_heap_desc.NumDescriptors = frame_count_;
  rtv_descriptor_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  rtv_descriptor_heap_desc.NodeMask = 0;
  ThrowIfFailed(device->CreateDescriptorHeap(&rtv_descriptor_heap_desc, IID_PPV_ARGS(&rtv_descriptor_heap_)));
  rtv_descriptor_increment_size_ = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

  CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_cpu_descriptor_handle(rtv_descriptor_heap_->GetCPUDescriptorHandleForHeapStart());
  for (UINT i = 0; i < frame_count_; ++i) {
    render_targets_.emplace_back(render_targets[i]);
    device->CreateRenderTargetView(render_targets[i].Get(), nullptr, rtv_cpu_descriptor_handle);
    rtv_cpu_descriptor_handle.Offset(rtv_descriptor_increment_size_);
  }
}

void Scene::Render(ID3D12CommandQueue* command_queue)
{
  PopulateCommandLists();

  ID3D12CommandList* command_lists[] = { command_list_.Get() };
  command_queue->ExecuteCommandLists(1, command_lists);
}

void Scene::PopulateCommandLists()
{
  command_allocators_[current_frame_index_]->Reset();
  command_list_->Reset(command_allocators_[current_frame_index_].Get(), pipeline_state_.Get());

  command_list_->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(render_targets_[current_frame_index_].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
  CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_cpu_descriptor_handle(rtv_descriptor_heap_->GetCPUDescriptorHandleForHeapStart(), current_frame_index_, rtv_descriptor_increment_size_);
  const FLOAT clear_color[] = { 0.0f, 0.6f, 0.0f, 1.0f };
  command_list_->ClearRenderTargetView(rtv_cpu_descriptor_handle, clear_color, 0, nullptr);

  // command_list_->IASetVertexBuffers()
  command_list_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  command_list_->RSSetViewports(1, &view_port_);
  command_list_->RSSetScissorRects(1, &scissor_rect_);

  command_list_->OMSetRenderTargets(1, &rtv_cpu_descriptor_handle, false, nullptr);

  //command_list_->DrawInstanced()
  command_list_->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(render_targets_[current_frame_index_].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

  ThrowIfFailed(command_list_->Close());
}

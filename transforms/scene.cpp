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

void Scene::LoadSizeDependentResources(ID3D12Device* device, ComPtr<ID3D12Resource>* render_targets, UINT width, UINT height)
{
  D3D12_DESCRIPTOR_HEAP_DESC rtv_descriptor_heap_desc{};
  rtv_descriptor_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
  rtv_descriptor_heap_desc.NumDescriptors = frame_count_;
  rtv_descriptor_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  rtv_descriptor_heap_desc.NodeMask = 0;
  ThrowIfFailed(device->CreateDescriptorHeap(&rtv_descriptor_heap_desc, IID_PPV_ARGS(&rtv_descriptor_heap_)));
  rtv_descriptor_size_ = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

  CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_cpu_descriptor_handle(rtv_descriptor_heap_->GetCPUDescriptorHandleForHeapStart());
  for (UINT i = 0; i < frame_count_; ++i) {
    render_targets_.emplace_back(render_targets[i]);
    device->CreateRenderTargetView(render_targets[i].Get(), nullptr, rtv_cpu_descriptor_handle);
    rtv_cpu_descriptor_handle.Offset(rtv_descriptor_size_);
  }
}

void Scene::Render(ID3D12CommandQueue* command_queue)
{
}

void Scene::PopulateCommandLists()
{
  command_allocator_->Reset();
  command_list_->Reset(command_allocator_.Get(), pipeline_state_.Get());

  // command_list_->IASetVertexBuffers()
  command_list_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  command_list_->RSSetViewports(1, &view_port_);
  command_list_->RSSetScissorRects(1, &scissor_rect_);

  //command_list_->OMSetRenderTargets()
}

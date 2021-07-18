#include "scene.h"

#include "dx_sample_helper.h"
#include "assets_manager.h"

Scene::Scene(UINT frame_count, UINT width, UINT height) : frame_count_(frame_count),
  view_port_(0.0f, 0.0f, (float)width, (float)height),
  scissor_rect_(0, 0, width, height)
{
  cameras_.resize(kTotalCameraCount_);
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

  SetCameras();

  CreateScenePipelineState(device);
  CreateAndMapSceneConstantBuffer(device);

  CreateCameraDrawPipelineState(device);

  command_allocators_.resize(frame_count_);
  for (UINT i = 0; i < frame_count_; ++i) {
    ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&command_allocators_[i])));
  }
 
  ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, command_allocators_[current_frame_index_].Get(), pipeline_state_.Get(), IID_PPV_ARGS(&command_list_)));

  CreateAssets(device);

  CreateCameraPoints(device);

  ThrowIfFailed(command_list_->Close());
  ID3D12CommandList* command_lists[] = { command_list_.Get() };
  command_queue->ExecuteCommandLists(1, command_lists);
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

void Scene::Update()
{
  const float angleChange = 0.002f;

  if (keyboard_input_.leftArrowPressed)
    cameras_[camera_index_].RotateAroundYAxis(-angleChange);
  if (keyboard_input_.rightArrowPressed)
    cameras_[camera_index_].RotateAroundYAxis(angleChange);
  if (keyboard_input_.upArrowPressed)
    cameras_[camera_index_].RotatePitch(-angleChange);
  if (keyboard_input_.downArrowPressed)
    cameras_[camera_index_].RotatePitch(angleChange);

  for (auto i = 0; i < cameras_.size(); ++i) {
    cameras_[i].UpdateDirections();
  }

  UpdateConstantBuffer();

  CommitConstantBuffer();
}

void Scene::Render(ID3D12CommandQueue* command_queue)
{
  PopulateCommandLists();

  ID3D12CommandList* command_lists[] = { command_list_.Get() };
  command_queue->ExecuteCommandLists(1, command_lists);
}

void Scene::KeyDown(UINT8 key)
{
  switch (key)
  {
  case VK_LEFT:
    keyboard_input_.leftArrowPressed = true;
    break;
  case VK_RIGHT:
    keyboard_input_.rightArrowPressed = true;
    break;
  case VK_UP:
    keyboard_input_.upArrowPressed = true;
    break;
  case VK_DOWN:
    keyboard_input_.downArrowPressed = true;
    break;
  case '1':
    camera_index_ = 0;
    break;
  case '2':
    camera_index_ = 1;
    break;
  case '3':
    camera_index_ = 2;
    break;
  case '4':
    camera_index_ = 3;
    break;
  default:
    break;
  }
}

void Scene::KeyUp(UINT8 key)
{
  switch (key)
  {
  case VK_LEFT:
    keyboard_input_.leftArrowPressed = false;
    break;
  case VK_RIGHT:
    keyboard_input_.rightArrowPressed = false;
    break;
  case VK_UP:
    keyboard_input_.upArrowPressed = false;
    break;
  case VK_DOWN:
    keyboard_input_.downArrowPressed = false;
    break;
  default:
    break;
  }
}

void Scene::CreateConstanfBuffer(ID3D12Device* device, UINT size, ID3D12Resource** ppResource, D3D12_RESOURCE_STATES initState)
{
  *ppResource = nullptr;

  const UINT alignedSize = CalculateConstantBufferByteSize(size);
  auto upload_heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
  auto constant_buffer_resource_desc = CD3DX12_RESOURCE_DESC::Buffer(alignedSize);
  ThrowIfFailed(device->CreateCommittedResource(
    &upload_heap_properties,
    D3D12_HEAP_FLAG_NONE,
    &constant_buffer_resource_desc,
    initState,
    nullptr,
    IID_PPV_ARGS(ppResource)));

}

void Scene::CreateScenePipelineState(ID3D12Device* device)
{
  D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

  // This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
  featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

  if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
  {
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
  }

  CD3DX12_ROOT_PARAMETER1 root_parameters[1];
  root_parameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX);

  CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC root_signature_desc;
  root_signature_desc.Init_1_1(1, root_parameters,
    0, nullptr,
    D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
    D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
    D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
    D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
    D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS);
  ComPtr<ID3DBlob> root_signature_blob;
  ComPtr<ID3DBlob> error;
  ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&root_signature_desc, featureData.HighestVersion, &root_signature_blob, &error));
  ThrowIfFailed(device->CreateRootSignature(0, root_signature_blob->GetBufferPointer(), root_signature_blob->GetBufferSize(), IID_PPV_ARGS(&root_signature_)));

  ComPtr<ID3DBlob> vertex_shader = CompileShader(L"vertex_shader.hlsl", nullptr, "main", "vs_5_0");
  ComPtr<ID3DBlob> pixel_shader = CompileShader(L"pixel_shader.hlsl", nullptr, "main", "ps_5_0");

  D3D12_INPUT_ELEMENT_DESC input_element_descs[] = {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
  };
  D3D12_INPUT_LAYOUT_DESC input_layout_desc{};
  input_layout_desc.pInputElementDescs = input_element_descs;
  input_layout_desc.NumElements = _countof(input_element_descs);

  D3D12_GRAPHICS_PIPELINE_STATE_DESC pipeline_state_desc{};
  pipeline_state_desc.pRootSignature = root_signature_.Get();
  pipeline_state_desc.VS = CD3DX12_SHADER_BYTECODE(vertex_shader.Get());
  pipeline_state_desc.PS = CD3DX12_SHADER_BYTECODE(pixel_shader.Get());
  pipeline_state_desc.BlendState = CD3DX12_BLEND_DESC(CD3DX12_DEFAULT());
  pipeline_state_desc.SampleMask = UINT_MAX;
  pipeline_state_desc.RasterizerState = CD3DX12_RASTERIZER_DESC(CD3DX12_DEFAULT());
  pipeline_state_desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(CD3DX12_DEFAULT());
  pipeline_state_desc.InputLayout = input_layout_desc;
  pipeline_state_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
  pipeline_state_desc.NumRenderTargets = 1;
  pipeline_state_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
  pipeline_state_desc.SampleDesc.Count = 1;
  pipeline_state_desc.NodeMask = 0;
  ThrowIfFailed(device->CreateGraphicsPipelineState(&pipeline_state_desc, IID_PPV_ARGS(&pipeline_state_)));
}

void Scene::CreateAndMapSceneConstantBuffer(ID3D12Device* device)
{
  CreateConstanfBuffer(device, sizeof(SceneConstantBuffer), &scene_constant_buffer_view_, D3D12_RESOURCE_STATE_GENERIC_READ);
  const CD3DX12_RANGE readRange(0, 0);
  scene_constant_buffer_view_->Map(0, &readRange, &scene_constant_buffer_pointer_);
}

void Scene::CreateCameraDrawPipelineState(ID3D12Device* device)
{
  D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

  // This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
  featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

  if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
  {
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
  }

  CD3DX12_ROOT_PARAMETER1 root_parameters[1]{};
  root_parameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_GEOMETRY);

  CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC root_signature_desc;
  root_signature_desc.Init_1_1(1, root_parameters,
    0, nullptr,
    D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
    D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS |
    D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
    D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
    D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS);
  ComPtr<ID3DBlob> root_signature_blob;
  ComPtr<ID3DBlob> error;
  ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&root_signature_desc, featureData.HighestVersion, &root_signature_blob, &error));
  ThrowIfFailed(device->CreateRootSignature(0, root_signature_blob->GetBufferPointer(), root_signature_blob->GetBufferSize(), IID_PPV_ARGS(&camera_draw_root_signature_)));

  ComPtr<ID3DBlob> vertex_shader = CompileShader(L"camera_draw_vertex_shader.hlsl", nullptr, "main", "vs_5_0");
  ComPtr<ID3DBlob> geometry_shader = CompileShader(L"camera_draw_geometry_shader.hlsl", nullptr, "main", "gs_5_1");
  ComPtr<ID3DBlob> pixel_shader = CompileShader(L"camera_draw_pixel_shader.hlsl", nullptr, "main", "ps_5_0");

  D3D12_INPUT_ELEMENT_DESC input_element_descs[] = {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"TEXCOORD", 1, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"TEXCOORD", 2, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
  };
  D3D12_INPUT_LAYOUT_DESC input_layout_desc{};
  input_layout_desc.NumElements = _countof(input_element_descs);
  input_layout_desc.pInputElementDescs = input_element_descs;

  D3D12_GRAPHICS_PIPELINE_STATE_DESC pipeline_state_desc{};
  pipeline_state_desc.pRootSignature = camera_draw_root_signature_.Get();
  pipeline_state_desc.VS = CD3DX12_SHADER_BYTECODE(vertex_shader.Get());
  pipeline_state_desc.GS = CD3DX12_SHADER_BYTECODE(geometry_shader.Get());
  pipeline_state_desc.PS = CD3DX12_SHADER_BYTECODE(pixel_shader.Get());
  pipeline_state_desc.BlendState = CD3DX12_BLEND_DESC(CD3DX12_DEFAULT());
  pipeline_state_desc.SampleMask = UINT_MAX;
  pipeline_state_desc.RasterizerState = CD3DX12_RASTERIZER_DESC(CD3DX12_DEFAULT());
  pipeline_state_desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(CD3DX12_DEFAULT());
  pipeline_state_desc.InputLayout = input_layout_desc;
  pipeline_state_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
  pipeline_state_desc.NumRenderTargets = 1;
  pipeline_state_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
  pipeline_state_desc.SampleDesc.Count = 1;
  pipeline_state_desc.NodeMask = 0;
  ThrowIfFailed(device->CreateGraphicsPipelineState(&pipeline_state_desc, IID_PPV_ARGS(&camera_draw_pipeline_state_)));
}

void Scene::CreateAssets(ID3D12Device* device)
{
  AssetsManager::Vertex* vertices_data;
  AssetsManager::GetSharedInstance().GetCubeVertexData(&vertices_data);
  size_t vertex_data_size = AssetsManager::GetSharedInstance().GetCubeVertexDataSize();

  CD3DX12_HEAP_PROPERTIES default_heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
  CD3DX12_RESOURCE_DESC vertex_buffer_resource_desc = CD3DX12_RESOURCE_DESC::Buffer(vertex_data_size);
  ThrowIfFailed(device->CreateCommittedResource(&default_heap_properties,
    D3D12_HEAP_FLAG_NONE,
    &vertex_buffer_resource_desc,
    D3D12_RESOURCE_STATE_COPY_DEST,
    nullptr,
    IID_PPV_ARGS(&vertex_buffer_)));

  CD3DX12_HEAP_PROPERTIES upload_heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
  ThrowIfFailed(device->CreateCommittedResource(&upload_heap_properties,
    D3D12_HEAP_FLAG_NONE,
    &vertex_buffer_resource_desc,
    D3D12_RESOURCE_STATE_GENERIC_READ,
    nullptr,
    IID_PPV_ARGS(&vertex_upload_heap_)));

  D3D12_SUBRESOURCE_DATA vertex_subresource_data{};
  vertex_subresource_data.pData = vertices_data;
  vertex_subresource_data.RowPitch = vertex_data_size;
  vertex_subresource_data.SlicePitch = vertex_data_size;
  UpdateSubresources(command_list_.Get(), vertex_buffer_.Get(), vertex_upload_heap_.Get(), 0, 0, 1, &vertex_subresource_data);

  vertex_buffer_view_.BufferLocation = vertex_buffer_->GetGPUVirtualAddress();
  vertex_buffer_view_.SizeInBytes = static_cast<UINT>(vertex_data_size);
  vertex_buffer_view_.StrideInBytes = static_cast<UINT>(AssetsManager::GetSharedInstance().GetVertexStride());

  DWORD* indices_data = nullptr;
  AssetsManager::GetSharedInstance().GetCubeIndexData(&indices_data);
  size_t index_data_size = AssetsManager::GetSharedInstance().GetCubeIndexDataSize();
  CD3DX12_RESOURCE_DESC index_buffer_resource_desc = CD3DX12_RESOURCE_DESC::Buffer(index_data_size);
  ThrowIfFailed(device->CreateCommittedResource(&default_heap_properties,
    D3D12_HEAP_FLAG_NONE,
    &index_buffer_resource_desc,
    D3D12_RESOURCE_STATE_COPY_DEST,
    nullptr,
    IID_PPV_ARGS(&index_buffer_)));

  ThrowIfFailed(device->CreateCommittedResource(&upload_heap_properties,
    D3D12_HEAP_FLAG_NONE,
    &index_buffer_resource_desc,
    D3D12_RESOURCE_STATE_GENERIC_READ,
    nullptr,
    IID_PPV_ARGS(&index_upload_heap_)));

  D3D12_SUBRESOURCE_DATA index_subresource_data{};
  index_subresource_data.pData = indices_data;
  index_subresource_data.RowPitch = index_data_size;
  index_subresource_data.SlicePitch = index_data_size;
  UpdateSubresources(command_list_.Get(), index_buffer_.Get(), index_upload_heap_.Get(), 0, 0, 1, &index_subresource_data);

  index_buffer_view_.BufferLocation = index_buffer_->GetGPUVirtualAddress();
  index_buffer_view_.SizeInBytes = static_cast<UINT>(index_data_size);
  index_buffer_view_.Format = DXGI_FORMAT_R32_UINT;

  delete[] vertices_data;
  delete[] indices_data;
}

void Scene::CreateCameraPoints(ID3D12Device* device)
{
  CD3DX12_HEAP_PROPERTIES default_heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
  auto buffer_size = sizeof(Camera::Vertex) * (kTotalCameraCount_ - 1);
  CD3DX12_RESOURCE_DESC camera_points_buffer_desc = CD3DX12_RESOURCE_DESC::Buffer(buffer_size);
  ThrowIfFailed(device->CreateCommittedResource(&default_heap_properties,
    D3D12_HEAP_FLAG_NONE,
    &camera_points_buffer_desc,
    D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
    nullptr,
    IID_PPV_ARGS(&camera_points_vertex_buffer_)));
  NAME_D3D12_OBJECT(camera_points_vertex_buffer_);

  CD3DX12_HEAP_PROPERTIES upload_heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
  ThrowIfFailed(device->CreateCommittedResource(&upload_heap_properties,
    D3D12_HEAP_FLAG_NONE,
    &camera_points_buffer_desc,
    D3D12_RESOURCE_STATE_GENERIC_READ,
    nullptr,
    IID_PPV_ARGS(&camera_points_vertex_upload_heap_)));

  // TODO: not updatesubresources

  camera_points_vertex_buffer_view_.BufferLocation = camera_points_vertex_buffer_->GetGPUVirtualAddress();
  camera_points_vertex_buffer_view_.SizeInBytes = static_cast<UINT>(buffer_size);
  camera_points_vertex_buffer_view_.StrideInBytes = static_cast<UINT>(sizeof(Camera::Vertex));
}

void Scene::UpdateConstantBuffer()
{
  XMStoreFloat4x4(&scene_constant_buffer_.model, XMMatrixIdentity());
  cameras_[camera_index_].Get3DViewProjMatrices(&scene_constant_buffer_.view, &scene_constant_buffer_.proj, 90.f, view_port_.Width, view_port_.Height);
}

void Scene::CommitConstantBuffer()
{
  memcpy(scene_constant_buffer_pointer_, &scene_constant_buffer_, sizeof(scene_constant_buffer_));
}

void Scene::SetCameras()
{
  XMVECTOR eye = XMVectorSet(0.0f, 1.0f, 2.0f, 0.0f);
  XMVECTOR at = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
  XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
  cameras_[1].Set(eye, at, up);

  eye = XMVectorSet(-2.0f, 1.0f, 0.0f, 0.0f);
  cameras_[2].Set(eye, at, up);

  eye = XMVectorSet(2.0f, 1.0f, 0.0f, 0.0f);
  cameras_[3].Set(eye, at, up);
}

void Scene::PopulateCommandLists()
{
  ThrowIfFailed(command_allocators_[current_frame_index_]->Reset());
  ThrowIfFailed(command_list_->Reset(command_allocators_[current_frame_index_].Get(), pipeline_state_.Get()));
  command_list_->SetGraphicsRootSignature(root_signature_.Get());
  command_list_->SetGraphicsRootConstantBufferView(0, scene_constant_buffer_view_->GetGPUVirtualAddress());

  CD3DX12_RESOURCE_BARRIER resource_barrier = CD3DX12_RESOURCE_BARRIER::Transition(render_targets_[current_frame_index_].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
  command_list_->ResourceBarrier(1, &resource_barrier);
  CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_cpu_descriptor_handle(rtv_descriptor_heap_->GetCPUDescriptorHandleForHeapStart(), current_frame_index_, rtv_descriptor_increment_size_);
  const FLOAT clear_color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
  command_list_->ClearRenderTargetView(rtv_cpu_descriptor_handle, clear_color, 0, nullptr);

  command_list_->IASetVertexBuffers(0, 1, &vertex_buffer_view_);
  command_list_->IASetIndexBuffer(&index_buffer_view_);
  command_list_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  command_list_->RSSetViewports(1, &view_port_);
  command_list_->RSSetScissorRects(1, &scissor_rect_);

  command_list_->OMSetRenderTargets(1, &rtv_cpu_descriptor_handle, false, nullptr);

  command_list_->DrawIndexedInstanced(36, 1, 0, 0, 0);

  DrawCameras();

  resource_barrier = CD3DX12_RESOURCE_BARRIER::Transition(render_targets_[current_frame_index_].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
  command_list_->ResourceBarrier(1, &resource_barrier);



  ThrowIfFailed(command_list_->Close());
}

void Scene::DrawCameras()
{
  command_list_->SetPipelineState(camera_draw_pipeline_state_.Get());
  command_list_->SetGraphicsRootSignature(camera_draw_root_signature_.Get());
  // TODO: need to call?
  //command_list_->SetGraphicsRootConstantBufferView(0, scene_constant_buffer_view_->GetGPUVirtualAddress());
  
  UpdateVerticesOfCameraPoints();
  
  // TODO: slot 0 ok ?
  command_list_->IASetVertexBuffers(0, 1, &camera_points_vertex_buffer_view_);
  command_list_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

  command_list_->DrawInstanced(kTotalCameraCount_ - 1, 1, 0, 0);
}

void Scene::UpdateVerticesOfCameraPoints()
{
  Camera::Vertex camera_draw_vertices[kTotalCameraCount_ - 1]{};
  int camera_point_index = 0;
  for (auto i = 0; i < kTotalCameraCount_; ++i) {
    if (i != camera_index_) {
      cameras_[i].GetCameraVertexData(&camera_draw_vertices[camera_point_index]);
      camera_point_index++;
    }
  }

  D3D12_SUBRESOURCE_DATA camera_points_subresource_data{};
  camera_points_subresource_data.pData = camera_draw_vertices;
  camera_points_subresource_data.RowPitch = sizeof(camera_draw_vertices);
  camera_points_subresource_data.SlicePitch = camera_points_subresource_data.RowPitch;

  CD3DX12_RESOURCE_BARRIER resource_barrier_1 = CD3DX12_RESOURCE_BARRIER::Transition(camera_points_vertex_buffer_.Get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
  command_list_->ResourceBarrier(1, &resource_barrier_1);
  UpdateSubresources(command_list_.Get(), camera_points_vertex_buffer_.Get(), camera_points_vertex_upload_heap_.Get(), 0, 0, 1, &camera_points_subresource_data);

  CD3DX12_RESOURCE_BARRIER resource_barrier_2 = CD3DX12_RESOURCE_BARRIER::Transition(camera_points_vertex_buffer_.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
  command_list_->ResourceBarrier(1, &resource_barrier_2);
}

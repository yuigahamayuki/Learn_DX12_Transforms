#include "scene.h"

Scene::Scene()
{
}

Scene::~Scene()
{
}

void Scene::Render(ID3D12CommandQueue* command_queue)
{
}

void Scene::PopulateCommandLists()
{
  command_allocator_->Reset();
  command_list_->Reset(command_allocator_.Get(), pipeline_state_.Get());
}

#include "assets_manager.h"

AssetsManager& AssetsManager::GetSharedInstance()
{
  static AssetsManager instance;
  return instance;
}

AssetsManager::AssetsManager()
{

}

AssetsManager::~AssetsManager()
{
}
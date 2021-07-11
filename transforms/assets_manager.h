#pragma once

#include "common_headers.h"

class AssetsManager {
public:
  struct Vertex {
    Vertex() = default;
    Vertex(float x, float y, float z, float r, float g, float b) {
      position_[0] = x;
      position_[1] = y;
      position_[2] = z;
      color_[0] = r;
      color_[1] = g;
      color_[2] = b;
    }

    float position_[3]{};
    float color_[3]{};
  };

  static AssetsManager& GetSharedInstance();

  ~AssetsManager();

  AssetsManager(const AssetsManager&) = delete;
  AssetsManager& operator=(const AssetsManager&) = delete;

  size_t GetVertexStride() const {
    return sizeof(Vertex);
  }

  void GetQuadVertexData(Vertex** vertices_data) const {
    (*vertices_data) = new Vertex[4];
    (*vertices_data)[0] = Vertex(-0.5f, -0.5f, 1.0f, 0.6f, 0.0f, 0.0f);
    (*vertices_data)[1] = Vertex(-0.5f, 0.5f, 1.0f, 0.6f, 0.0f, 0.0f);
    (*vertices_data)[2] = Vertex(0.5f, -0.5f, 1.0f, 0.6f, 0.0f, 0.0f);
    (*vertices_data)[3] = Vertex(0.5f, 0.5f, 1.0f, 0.6f, 0.0f, 0.0f);
  }

  size_t GetQuadVertexDataSize() const {
    return sizeof(Vertex) * 4;
  }

  void GetQuadIndexData(DWORD** indices_data) const {
    (*indices_data) = new DWORD[6];
    (*indices_data)[0] = 0;
    (*indices_data)[1] = 1;
    (*indices_data)[2] = 2;
    (*indices_data)[3] = 2;
    (*indices_data)[4] = 1;
    (*indices_data)[5] = 3;
  }

  size_t GetQuadIndexDataSize() const {
    return sizeof(DWORD) * 6;
  }

private:
  AssetsManager();
};
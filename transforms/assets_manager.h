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

  void GetCubeVertexData(Vertex** vertices_data) const {
    (*vertices_data) = new Vertex[8];
    (*vertices_data)[0] = Vertex(-0.5f, -0.5f, -0.5f, 0.6f, 0.0f, 0.0f);
    (*vertices_data)[1] = Vertex(-0.5f, 0.5f, -0.5f, 0.0f, 0.6f, 0.0f);
    (*vertices_data)[2] = Vertex(0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.6f);
    (*vertices_data)[3] = Vertex(0.5f, 0.5f, -0.5f, 0.6f, 0.0f, 0.0f);
    (*vertices_data)[4] = Vertex(-0.5f, -0.5f, 0.5f, 0.0f, 0.6f, 0.6f);
    (*vertices_data)[5] = Vertex(-0.5f, 0.5f, 0.5f, 0.6f, 0.0f, 0.6f);
    (*vertices_data)[6] = Vertex(0.5f, -0.5f, 0.5f, 0.6f, 0.6f, 0.6f);
    (*vertices_data)[7] = Vertex(0.5f, 0.5f, 0.5f, 0.0f, 0.6f, 0.6f);
  }

  size_t GetCubeVertexDataSize() const {
    return sizeof(Vertex) * 8;
  }

  void GetCubeIndexData(DWORD** indices_data) const {
    (*indices_data) = new DWORD[36];

    // front
    (*indices_data)[0] = 0;
    (*indices_data)[1] = 1;
    (*indices_data)[2] = 2;
    (*indices_data)[3] = 2;
    (*indices_data)[4] = 1;
    (*indices_data)[5] = 3;

    // back
    (*indices_data)[6] = 6;
    (*indices_data)[7] = 7;
    (*indices_data)[8] = 4;
    (*indices_data)[9] = 4;
    (*indices_data)[10] = 7;
    (*indices_data)[11] = 5;

    // left
    (*indices_data)[12] = 4;
    (*indices_data)[13] = 5;
    (*indices_data)[14] = 0;
    (*indices_data)[15] = 0;
    (*indices_data)[16] = 5;
    (*indices_data)[17] = 1;

    // right
    (*indices_data)[18] = 2;
    (*indices_data)[19] = 3;
    (*indices_data)[20] = 6;
    (*indices_data)[21] = 6;
    (*indices_data)[22] = 3;
    (*indices_data)[23] = 7;

    // top
    (*indices_data)[24] = 1;
    (*indices_data)[25] = 5;
    (*indices_data)[26] = 3;
    (*indices_data)[27] = 3;
    (*indices_data)[28] = 5;
    (*indices_data)[29] = 7;

    // bottom
    (*indices_data)[30] = 4;
    (*indices_data)[31] = 0;
    (*indices_data)[32] = 6;
    (*indices_data)[33] = 6;
    (*indices_data)[34] = 0;
    (*indices_data)[35] = 2;
  }

  size_t GetCubeIndexDataSize() const {
    return sizeof(DWORD) * 36;
  }

private:
  AssetsManager();
};
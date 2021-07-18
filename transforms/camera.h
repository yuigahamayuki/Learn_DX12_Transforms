//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#pragma once

#include "common_headers.h"

using namespace DirectX;

class Camera
{
public:
  struct Vertex {
    XMFLOAT3 position_;
    XMFLOAT3 look_direction_;
    XMFLOAT3 right_;
    XMFLOAT3 refined_up_;
  };

  Camera();
  ~Camera();


  void Get3DViewProjMatricesLH(XMFLOAT4X4* view, XMFLOAT4X4* proj, float fovInDegrees, float screenWidth, float screenHeight);
  void Get3DViewProjMatrices(XMFLOAT4X4* view, XMFLOAT4X4* proj, float fovInDegrees, float screenWidth, float screenHeight);
  void Reset();
  void Set(XMVECTOR eye, XMVECTOR at, XMVECTOR up);

  void RotateAroundYAxis(float angleRad);
  void RotateYaw(float angleRad);
  void RotatePitch(float angleRad);
  void GetOrthoProjMatrices(XMFLOAT4X4* view, XMFLOAT4X4* proj, float width, float height);

  void UpdateDirections();

  void GetCameraVertexData(Vertex* vertex) {
    XMStoreFloat3(&(vertex->position_), mEye);
    vertex->look_direction_ = XMFLOAT3(look_direction_.x, look_direction_.y, look_direction_.z);
    vertex->right_ = XMFLOAT3(right_.x, right_.y, right_.z);
    vertex->refined_up_ = XMFLOAT3(refined_up_.x, refined_up_.y, refined_up_.z);
  }

  XMVECTOR mEye; // Where the camera is in world space. Z increases into of the screen when using LH coord system (which we are and DX uses)
  XMVECTOR mAt; // What the camera is looking at (world origin)
  XMVECTOR mUp; // Which way is up

  XMFLOAT4 look_direction_;  // camera pos to target
  XMFLOAT4 right_;
  XMFLOAT4 refined_up_;
private:
  
};
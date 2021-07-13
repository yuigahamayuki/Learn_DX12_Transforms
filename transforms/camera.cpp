#include "camera.h"

Camera::Camera()
{
  Reset();
}

Camera::~Camera()
{
}

void Camera::Get3DViewProjMatricesLH(XMFLOAT4X4* view, XMFLOAT4X4* proj, float fovInDegrees, float screenWidth, float screenHeight)
{
  float aspectRatio = (float)screenWidth / (float)screenHeight;
  float fovAngleY = fovInDegrees * XM_PI / 180.0f;

  if (aspectRatio < 1.0f)
  {
    fovAngleY /= aspectRatio;
  }

  XMStoreFloat4x4(view, XMMatrixTranspose(XMMatrixLookAtLH(mEye, mAt, mUp)));
  XMStoreFloat4x4(proj, XMMatrixTranspose(XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, 0.01f, 10.0f)));
}

void Camera::Get3DViewProjMatrices(XMFLOAT4X4* view, XMFLOAT4X4* proj, float fovInDegrees, float screenWidth, float screenHeight)
{
  float aspectRatio = (float)screenWidth / (float)screenHeight;
  float fovAngleY = fovInDegrees * XM_PI / 180.0f;

  if (aspectRatio < 1.0f)
  {
    fovAngleY /= aspectRatio;
  }

  XMStoreFloat4x4(view, XMMatrixTranspose(XMMatrixLookAtRH(mEye, mAt, mUp)));
  XMStoreFloat4x4(proj, XMMatrixTranspose(XMMatrixPerspectiveFovRH(fovAngleY, aspectRatio, 0.01f, 10.0f)));
}

void Camera::Reset()
{
  mEye = XMVectorSet(0.0f, 1.0f, -2.0f, 0.0f);
  mAt = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
  mUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
}

void Camera::Set(XMVECTOR eye, XMVECTOR at, XMVECTOR up)
{
  mEye = eye;
  mAt = at;
  mUp = up;
}

void Camera::RotateAroundYAxis(float angleRad)
{
  XMMATRIX rotation = XMMatrixRotationY(angleRad);

  mEye = mAt + XMVector3TransformCoord(mEye - mAt, rotation);
  mUp = XMVector3TransformCoord(mUp, rotation);
}

void Camera::RotateYaw(float angleRad)
{
  XMMATRIX rotation = XMMatrixRotationAxis(mUp, angleRad);

  mEye = mAt + XMVector3TransformCoord(mEye - mAt, rotation);
}

void Camera::RotatePitch(float angleRad)
{
  XMVECTOR right = XMVector3Normalize(XMVector3Cross(mAt - mEye, mUp));
  XMMATRIX rotation = XMMatrixRotationAxis(right, angleRad);

  mEye = mAt + XMVector3TransformCoord(mEye - mAt, rotation);
  mUp = XMVector3TransformCoord(mUp, rotation);
}

void Camera::GetOrthoProjMatrices(XMFLOAT4X4* view, XMFLOAT4X4* proj, float width, float height)
{
}

void Camera::UpdateDirections()
{
  auto look_direction_vector = XMVector4Normalize(mAt - mEye);
  XMStoreFloat4(&look_direction_, look_direction_vector);

  auto right_vector = XMVector3Cross(mUp, look_direction_vector);
  XMStoreFloat4(&right_, right_vector);

  auto refined_up_vector = XMVector3Cross(look_direction_vector, right_vector);
  XMStoreFloat4(&refined_up_, refined_up_vector);
}

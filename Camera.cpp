#include "Camera.h"
#include "Input.h"
#include <memory>

Camera::Camera(float aspectRatio, DirectX::XMFLOAT3 initPos, DirectX::XMFLOAT3 startOrient, float fovAngle, float nearClip, float farClip, float moveSpeed, float mouseLookSpeed)
{
	this->aspectRatio = aspectRatio;
	this->initPos = initPos;
	this->startOrient = startOrient;
	this->fovAngle = fovAngle;
	this->nearClip = nearClip;
	this->farClip = farClip;
	this->moveSpeed = moveSpeed;
	this->mouseLookSpeed = mouseLookSpeed;

	transform = std::make_shared<Transform>();
	transform->SetPosition(initPos);

	UpdateViewMatrix();
	UpdateProjectionMatrix(aspectRatio);
}

DirectX::XMFLOAT4X4 Camera::GetViewMatrix()
{
	return viewMatrix;
}

DirectX::XMFLOAT4X4 Camera::GetProjectionMatrix()
{
	return projMatrix;
}

std::shared_ptr<Transform> Camera::GetTransform()
{
	return transform;
}

void Camera::UpdateProjectionMatrix(float aspectRatio)
{
	DirectX::XMStoreFloat4x4(&projMatrix, DirectX::XMMatrixPerspectiveFovLH(fovAngle, aspectRatio, nearClip, farClip));
}

void Camera::UpdateViewMatrix()
{
	DirectX::XMFLOAT3 posVec = transform->GetPosition();
	DirectX::XMFLOAT3 dirVec = transform->GetForward();
	DirectX::XMStoreFloat4x4(&viewMatrix, DirectX::XMMatrixLookToLH(
		DirectX::XMLoadFloat3(&posVec),
		DirectX::XMLoadFloat3(&dirVec),
		DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
	));
}

void Camera::Update(float dt)
{
	// back and forth
	if (Input::KeyDown('W')) { transform->MoveRelative(0.0f, 0.0f, 0.5f * dt); }
	if (Input::KeyDown('S')) { transform->MoveRelative(0.0f, 0.0f, -0.5f * dt); }

	// left and right
	if (Input::KeyDown('A')) { transform->MoveRelative(-0.5f * dt, 0.0f, 0.0f); }
	if (Input::KeyDown('D')) { transform->MoveRelative(0.5f * dt, 0.0f, 0.0f); }

	// up and down
	if (Input::KeyDown('E')) { transform->MoveRelative(0.0f, 0.5f * dt, 0.0f); }
	if (Input::KeyDown('Q')) { transform->MoveRelative(0.0f, -0.5f * dt, 0.0f); }

	// mouse rotation
	if (Input::MouseLeftDown())
	{
		int cursorMovementX = Input::GetMouseXDelta();
		int cursorMovementY = Input::GetMouseYDelta();

		transform->Rotate(cursorMovementY * dt, cursorMovementX * dt, 0.0f);

		// clamp x movement to prevent camera flippy floppy madness
		DirectX::XMFLOAT3 rotation = transform->GetPitchYawRoll();
		if (rotation.x > DirectX::XMConvertToRadians(45)) rotation.x = DirectX::XMConvertToRadians(45);
		if (rotation.x < DirectX::XMConvertToRadians(-45)) rotation.x = DirectX::XMConvertToRadians(-45);
		transform->SetRotation(rotation);
	}

	UpdateViewMatrix();
}

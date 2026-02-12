#pragma once
#include <DirectXMath.h>
#include <memory>
#include "Transform.h"
class Camera
{
public:
	Camera(float aspectRatio, DirectX::XMFLOAT3 initPos, DirectX::XMFLOAT3 startOrient, float fovAngle, float nearClip, float farClip, float moveSpeed, float mouseLookSpeed);

	DirectX::XMFLOAT4X4 GetViewMatrix();
	DirectX::XMFLOAT4X4 GetProjectionMatrix();
	std::shared_ptr<Transform> GetTransform();

	void UpdateProjectionMatrix(float aspectRatio);
	void UpdateViewMatrix();
	void Update(float dt);

private:
	std::shared_ptr<Transform> transform;
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projMatrix;

	float aspectRatio;
	float fovAngle;
	float nearClip;
	float farClip;
	float moveSpeed;
	float mouseLookSpeed;
	bool isPerspective;
	DirectX::XMFLOAT3 initPos;
	DirectX::XMFLOAT3 startOrient;
};


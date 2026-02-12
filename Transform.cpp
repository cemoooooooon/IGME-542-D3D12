#include "Transform.h"

Transform::Transform() :
	filthyMatrix(false),
	rancidVector(false),
	position(0.0f, 0.0f, 0.0f),
	rotation(0.0f, 0.0f, 0.0f),
	scale(1.0f, 1.0f, 1.0f),
	right(1.0f, 0.0f, 0.0f),
	up(0.0f, 1.0f, 0.0f),
	forward(0.0f, 0.0f, 1.0f)
{
	DirectX::XMStoreFloat4x4(&worldMatrix, DirectX::XMMatrixIdentity());
	DirectX::XMStoreFloat4x4(&worldInverseTranspose, DirectX::XMMatrixIdentity());
}

void Transform::SetPosition(float x, float y, float z)
{
	this->position.x = x;
	this->position.y = y;
	this->position.z = z;
	filthyMatrix = true;
}

void Transform::SetPosition(DirectX::XMFLOAT3 position)
{
	this->position = position;
	filthyMatrix = true;
}

void Transform::SetRotation(float pitch, float yaw, float roll)
{
	this->rotation.x = pitch;
	this->rotation.y = yaw;
	this->rotation.z = roll;
	filthyMatrix = true;
}

void Transform::SetRotation(DirectX::XMFLOAT3 rotation)
{
	this->rotation = rotation;
	filthyMatrix = true;
	rancidVector = true;
}

void Transform::SetScale(float x, float y, float z)
{
	this->scale.x = x;
	this->scale.y = y;
	this->scale.z = z;
	filthyMatrix = true;
}

void Transform::SetScale(DirectX::XMFLOAT3 scale)
{
	this->scale = scale;
	filthyMatrix = true;
}

DirectX::XMFLOAT3 Transform::GetPosition()
{
	CleanMatrix();
	return position;
}

DirectX::XMFLOAT3 Transform::GetPitchYawRoll()
{
	CleanMatrix();
	return rotation;
}

DirectX::XMFLOAT3 Transform::GetScale()
{
	CleanMatrix();
	return scale;
}

DirectX::XMFLOAT4X4 Transform::GetWorldMatrix()
{
	CleanMatrix();
	return worldMatrix;
}

DirectX::XMFLOAT4X4 Transform::GetWorldInverseTransposeMatrix()
{
	CleanMatrix();
	return worldInverseTranspose;
}

DirectX::XMFLOAT3 Transform::GetRight()
{
	CleanVector();
	return right;
}

DirectX::XMFLOAT3 Transform::GetUp()
{
	CleanVector();
	return up;
}

DirectX::XMFLOAT3 Transform::GetForward()
{
	CleanVector();
	return forward;
}

void Transform::MoveAbsolute(float x, float y, float z)
{
	position.x += x;
	position.y += y;
	position.z += z;
	filthyMatrix = true;
}

void Transform::MoveAbsolute(DirectX::XMFLOAT3 offset)
{
	MoveAbsolute(offset.x, offset.y, offset.z);
}

void Transform::Rotate(float pitch, float yaw, float roll)
{
	rotation.x += pitch;
	rotation.y += yaw;
	rotation.z += roll;
	filthyMatrix = true;
	rancidVector = true;
}

void Transform::Rotate(DirectX::XMFLOAT3 rotation)
{
	Rotate(rotation.x, rotation.y, rotation.z);
}

void Transform::Scale(float x, float y, float z)
{
	scale.x += x;
	scale.y += y;
	scale.z += z;
	filthyMatrix = true;
}

void Transform::Scale(DirectX::XMFLOAT3 scale)
{
	Scale(scale.x, scale.y, scale.z);
}

void Transform::MoveRelative(float x, float y, float z)
{
	DirectX::XMFLOAT3 offset(x, y, z);
	DirectX::XMVECTOR offsetVec = DirectX::XMLoadFloat3(&offset);
	DirectX::XMVECTOR rotQuat = DirectX::XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
	DirectX::XMVECTOR posOffsetVec = DirectX::XMVector3Rotate(offsetVec, rotQuat);

	//convert offest vec as a float3 to add back to position
	DirectX::XMFLOAT3 posOffset;
	DirectX::XMStoreFloat3(&posOffset, posOffsetVec);

	position.x += posOffset.x;
	position.y += posOffset.y;
	position.z += posOffset.z;

	filthyMatrix = true;
}

void Transform::MoveRelative(DirectX::XMFLOAT3 offset)
{
	MoveRelative(offset.x, offset.y, offset.z);
}

void Transform::CleanMatrix()
{
	//if clean, no need for cleaning
	if (!filthyMatrix)
		return;

	// making my transformation matrices
	DirectX::XMMATRIX translationMatrix = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
	DirectX::XMMATRIX scaleMatrix = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
	DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(rotation.z, rotation.x, rotation.y);

	DirectX::XMMATRIX world = scaleMatrix * rotationMatrix * translationMatrix;
	DirectX::XMStoreFloat4x4(&worldMatrix, world);
	DirectX::XMStoreFloat4x4(&worldInverseTranspose, XMMatrixInverse(0, XMMatrixTranspose(world)));

	filthyMatrix = false;
}

// reset the right, up and forward vectors to a nice and clean state
void Transform::CleanVector()
{
	if (!rancidVector)
		return;

	DirectX::XMVECTOR rotQuat = DirectX::XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);

	DirectX::XMStoreFloat3(&right, DirectX::XMVector3Rotate(DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), rotQuat));
	DirectX::XMStoreFloat3(&up, DirectX::XMVector3Rotate(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), rotQuat));
	DirectX::XMStoreFloat3(&forward, DirectX::XMVector3Rotate(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), rotQuat));

	rancidVector = false;
}

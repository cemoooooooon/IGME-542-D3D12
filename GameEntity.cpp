#include "GameEntity.h"

GameEntity::GameEntity(std::shared_ptr<Mesh> mesh)
	: mesh(mesh)
{
	transform = std::make_shared<Transform>();
}

std::shared_ptr<Mesh> GameEntity::GetMesh() { return mesh; }
std::shared_ptr<Transform> GameEntity::GetTransform() { return transform; }
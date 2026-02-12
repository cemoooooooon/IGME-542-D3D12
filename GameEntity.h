#pragma once
#include "Mesh.h"
#include "Transform.h"
#include <memory>

class GameEntity
{
public:
	GameEntity(std::shared_ptr<Mesh> mesh);

	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Transform> GetTransform();

private:
	std::shared_ptr<Transform> transform;
	std::shared_ptr<Mesh> mesh;
};


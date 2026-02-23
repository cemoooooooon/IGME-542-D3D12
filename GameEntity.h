#pragma once
#include "Mesh.h"
#include "Transform.h"
#include "Material.h"
#include <memory>

class GameEntity
{
public:
	GameEntity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material);

	// getters!
	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Transform> GetTransform();
	std::shared_ptr<Material> GetMaterial();

	// setters!
	void SetMesh(std::shared_ptr<Mesh> mesh);
	void SetMaterial(std::shared_ptr<Material> material);

private:
	std::shared_ptr<Transform> transform;
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Material> material;
};


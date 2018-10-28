#ifndef __QUADTREE_H__
#define __QUADTREE_H__

#include "GameObject.h"
#include "MathGeoLib/include/Geometry/AABB.h"

#include <vector>
#include <list>

#define BUCKET_SIZE 1
#define MAX_SUBDIVISIONS 10

class QuadtreeNode 
{
public:

	QuadtreeNode(const math::AABB& boundingBox);
	~QuadtreeNode();

	bool IsLeaf() const;

	void Insert(GameObject* gameObject);
	void Subdivide();
	void RedistributeChildren();

	template<typename Type>
	void CollectIntersections(std::vector<GameObject*>& gameObjects, Type& primitive);

public:

	math::AABB boundingBox;
	std::list<GameObject*> objects;

	QuadtreeNode* parent = nullptr;
	QuadtreeNode* children[4];

	uint subdivision = 0;
};

class Quadtree
{
public:

	Quadtree();
	~Quadtree();

	void SetBoundary(const math::AABB& limits);
	void Clear();

	void Insert(GameObject* gameObject);

	template<typename Type>
	void CollectIntersections(std::vector<GameObject*>& gameObjects, Type& primitive);

public:

	QuadtreeNode* root = nullptr;
};

#endif
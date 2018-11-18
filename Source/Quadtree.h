#ifndef __QUADTREE_H__
#define __QUADTREE_H__

#include "Globals.h"

#include "MathGeoLib\include\Geometry\AABB.h"

#include <vector>
#include <list>

#define BUCKET_SIZE 5
#define MAX_SUBDIVISIONS 15

class GameObject;

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

template<typename Type>
inline void QuadtreeNode::CollectIntersections(std::vector<GameObject*>& gameObjects, Type& primitive)
{
	if (primitive.Intersects(boundingBox))
	{
		for (std::list<GameObject*>::const_iterator it = objects.begin(); it != objects.end(); ++it)
		{
			if (primitive.Intersects((*it)->boundingBox))
			{
				if (std::find(gameObjects.begin(), gameObjects.end(), *it) == gameObjects.end())
					gameObjects.push_back(*it);
			}
		}

		if (!IsLeaf())
		{
			for (uint i = 0; i < 4; ++i)
				children[i]->CollectIntersections(gameObjects, primitive);
		}
	}
}

template<typename Type>
inline void Quadtree::CollectIntersections(std::vector<GameObject*>& gameObjects, Type& primitive)
{
	if (root != nullptr)
		root->CollectIntersections(gameObjects, primitive);
}

#endif
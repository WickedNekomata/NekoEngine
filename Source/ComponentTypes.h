#ifndef COMPONENT_TYPES_H
#define COMPONENT_TYPES_H

enum ComponentTypes
{ 
	NoComponentType,
	TransformComponent,
	MeshComponent,
	MaterialComponent,
	CameraComponent,

	// Physics
	RigidStaticComponent,
	RigidDynamicComponent,
	BoxColliderComponent,
	SphereColliderComponent,
	CapsuleColliderComponent,
	PlaneColliderComponent

};

#endif
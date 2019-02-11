#ifndef COMPONENT_TYPES_H
#define COMPONENT_TYPES_H

enum ComponentTypes
{
	NoComponentType,
	TransformComponent,
	MeshComponent,
	MaterialComponent,
	CameraComponent,
	NavAgentComponent,
	EmitterComponent,
	ScriptComponent,
	BoneComponent,
	AnimationComponent,

	// Physics
	RigidStaticComponent,
	RigidDynamicComponent,
	BoxColliderComponent,
	SphereColliderComponent,
	CapsuleColliderComponent,
	PlaneColliderComponent
};

#endif

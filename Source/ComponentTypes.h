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
	LightComponent,

	// Physics
	RigidStaticComponent,
	RigidDynamicComponent,
	BoxColliderComponent,
	SphereColliderComponent,
	CapsuleColliderComponent,
	PlaneColliderComponent,

	//Audio
	AudioListenerComponent
};

#endif

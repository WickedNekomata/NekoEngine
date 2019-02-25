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
	ProjectorComponent,

	// Physics
	/// Rigid Actors
	RigidStaticComponent,
	RigidDynamicComponent,

	/// Colliders
	BoxColliderComponent,
	SphereColliderComponent,
	CapsuleColliderComponent,
	PlaneColliderComponent,

	/// Joints
	FixedJointComponent,
	DistanceJointComponent,
	SphericalJointComponent,
	RevoluteJointComponent,
	PrismaticJointComponent,
	D6JointComponent
};

#endif

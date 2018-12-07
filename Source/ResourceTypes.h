#ifndef RESOURCE_TYPE_H
#define RESOURCE_TYPE_H

enum ResourceType 
{ 
	No_Type_Resource,

	Mesh_Resource,
	Texture_Resource,

	Vertex_Shader_Object_Resource, // Shader Object
	Fragment_Shader_Object_Resource, // Shader Object
	Shader_Program_Resource // Shader Program
};

#endif
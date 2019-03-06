#version 330 core

layout(triangles_adjacency) in;
layout(triangle_strip, max_vertices=255) out;

in VS_OUT
{
  vec3 gPosition;
  vec3 gNormal;
  vec4 gColor;
  vec2 gTexCoord;
} gs_in[];

out GS_OUT
{
  vec3 fPosition;
  vec3 fNormal;
  vec4 fColor;
  vec2 fTexCoord;
} gs_out;

flat out int fIsEdge; // which output primitives are silhouette edges

uniform float edgeWidth; // wdth of silhouette edge in clip
uniform float pctExtend; // percentage to extend quad

bool isFrontFacing(vec3 a, vec3 b, vec3 c) // is a triangle front facing?
{
    // Compute the triangle's z coordinate of the normal vector (cross product)
    return ((a.x * b.y - b.x * a.y) + (b.x * c.y - c.x * b.y) + (c.x * a.y - a.x * c.y)) > 0;
}

void emitEdgeQuad(vec3 e0, vec3 e1)
{
    vec2 ext = pctExtend * (e1.xy - e0.xy);
    vec2 v = normalize(e1.xy - e0.xy);
    vec2 n = vec2(-v.y, v.x) * edgeWidth;
    
    // Emit the quad
    fIsEdge = 1; // this is part of an edge
    
    gl_Position = vec4(e0.xy - ext, e0.z, 1.0);
    EmitVertex();
    gl_Position = vec4(e0.xy - n - ext, e0.z, 1.0);
    EmitVertex();
    gl_Position = vec4(e1.xy + ext, e1.z, 1.0);
    EmitVertex();
    gl_Position = vec4(e1.xy - n + ext, e1.z, 1.0);
    EmitVertex();
    
    EndPrimitive();
}

void main()
{
	vec3 p0 = gl_in[0].gl_Position.xyz / gl_in[0].gl_Position.w;
	vec3 p1 = gl_in[1].gl_Position.xyz / gl_in[1].gl_Position.w;
	vec3 p2 = gl_in[2].gl_Position.xyz / gl_in[2].gl_Position.w;
	vec3 p3 = gl_in[3].gl_Position.xyz / gl_in[3].gl_Position.w;
	vec3 p4 = gl_in[4].gl_Position.xyz / gl_in[4].gl_Position.w;
	vec3 p5 = gl_in[5].gl_Position.xyz / gl_in[5].gl_Position.w;
	
	if (isFrontFacing(p0, p2, p4))
	{
	    if (!isFrontFacing(p0, p1, p2))
	        emitEdgeQuad(p0, p2);
	    if (!isFrontFacing(p2, p3, p4))
	        emitEdgeQuad(p2, p4);
	    if (!isFrontFacing(p4, p5, p0))
	        emitEdgeQuad(p4, p0);
	}
	
	// Output the original triangle
	fIsEdge = 0; // this is not part of an edge
	
	gs_out.fPosition = gs_in[0].gPosition;
	gs_out.fNormal = gs_in[0].gNormal;
	gs_out.fColor = gs_in[0].gColor;
	gs_out.fTexCoord = gs_in[0].gTexCoord;
	gl_Position = gl_in[0].gl_Position;
	EmitVertex();
	
	gs_out.fPosition = gs_in[2].gPosition;
	gs_out.fNormal = gs_in[2].gNormal;
	gs_out.fColor = gs_in[2].gColor;
	gs_out.fTexCoord = gs_in[2].gTexCoord;
	gl_Position = gl_in[2].gl_Position;
	EmitVertex();
	
	gs_out.fPosition = gs_in[4].gPosition;
	gs_out.fNormal = gs_in[4].gNormal;
	gs_out.fColor = gs_in[4].gColor;
	gs_out.fTexCoord = gs_in[4].gTexCoord;
	gl_Position = gl_in[4].gl_Position;
	EmitVertex();
	
	EndPrimitive();
	

}
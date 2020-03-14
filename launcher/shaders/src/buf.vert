#version 450

#extension GL_ARB_separate_shader_objects: enable
#extension GL_ARB_shading_language_420pack: enable

layout (location = 0) in vec3 in_pos;
layout (location = 0) out vec2 frag_pos;

void main()
{
	gl_Position = vec4(in_pos.xyz, 1);
	frag_pos = gl_Position.xy;
}

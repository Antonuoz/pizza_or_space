#version 430 core

uniform samplerCube skybox;

in vec3 texCoords;

out vec4 out_color;

void main()
{
	out_color = texture(skybox,texCoords);
}
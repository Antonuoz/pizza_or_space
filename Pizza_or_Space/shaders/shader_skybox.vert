#version 430 core

layout(location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 view;
out vec3 texCoords;
const float skyboxScaleFactor = 500.0;
void main()
{
    vec3 scaledPos = aPos * skyboxScaleFactor;

    texCoords = normalize(scaledPos);
    gl_Position = projection * view * vec4(scaledPos, 1.0);
}
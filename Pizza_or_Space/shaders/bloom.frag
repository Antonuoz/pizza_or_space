#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D sceneTexture;

void main()
{
    // Apply bloom effect here
    // Example: just output the scene texture for now
    FragColor = texture(sceneTexture, TexCoords);
}
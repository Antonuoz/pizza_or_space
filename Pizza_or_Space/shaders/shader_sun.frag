#version 430 core

uniform sampler2D colorTexture; 

in vec2 vecTex; // Texture coordinates

out vec4 outColor; // Output color of the fragment

void main()
{
    vec3 textureColor = texture(colorTexture, vecTex).xyz; // Get the texture color
    outColor = vec4(textureColor, 1.0); // Set the output color
}

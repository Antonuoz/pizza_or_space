#version 430 core

uniform sampler2D colorTexture; // Texture of the sun
uniform bool isHighlighted;     // Check if the object is highlighted
uniform vec3 highlightColor;    // Color for highlighting

in vec2 vecTex; // Texture coordinates

out vec4 outColor; // Output color of the fragment

void main()
{
    vec3 textureColor = texture(colorTexture, vecTex).xyz; // Get the texture color

    // Check if the object is highlighted and mix the colors accordingly
    if(isHighlighted) {
        textureColor = mix(textureColor, highlightColor, 0.5);
    }

    outColor = vec4(textureColor, 1.0); // Set the output color
}

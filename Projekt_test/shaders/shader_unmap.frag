#version 430 core

uniform sampler2D colorTexture;
uniform sampler2D normalMap; // Normal map
uniform vec3 lightPos;
uniform vec3 cameraPos; // Pozycja kamery dla PBR
uniform bool isHighlighted;
uniform vec3 highlightColor;
uniform float brightnessFactor = 2; // Faktor jasnoœci dla PBR

uniform float metallic;
uniform float roughness;
uniform float ao; // Ambient occlusion factor, not used directly in this example

in vec3 FragPos;
in vec2 TexCoords;
in mat3 TBN; // TBN matrix for normal mapping

out vec4 outColor;

// Funkcje pomocnicze dla PBR, jak wczeœniej
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = 3.14159265359 * denom * denom;
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;
    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float ggx2 = GeometrySchlickGGX(max(dot(N, V), 0.0), roughness);
    float ggx1 = GeometrySchlickGGX(max(dot(N, L), 0.0), roughness);
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

void main() {
    // Teksturowanie z uwzglêdnieniem mapy normalnych
    vec3 albedo = texture(colorTexture, TexCoords).rgb * brightnessFactor;
    vec3 normal = texture(normalMap, TexCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0); // Do przestrzeni [-1, 1]
    vec3 N = normalize(TBN * normal); // Do przestrzeni œwiata

    vec3 V = normalize(cameraPos - FragPos);
    vec3 L = normalize(lightPos - FragPos);
    vec3 H = normalize(V + L);

    // Obliczenia PBR
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), vec3(0.04) * (1.0 - metallic) + albedo * metallic);
    vec3 nominator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
    vec3 specular = nominator / denominator;

    // Œwiat³o otoczenia i dyfuzyjne
    float NdotL = max(dot(N, L), 0.0);
    vec3 ambient = vec3(0.03) * albedo;
    vec3 lighting = (vec3(1.0) - F) * albedo / 3.14159265359 + specular;
    lighting *= NdotL;

    vec3 result = ambient + lighting;
    if(isHighlighted) {
        result = mix(result, highlightColor, 0.5);
    }

    outColor = vec4(result, 1.0);
}


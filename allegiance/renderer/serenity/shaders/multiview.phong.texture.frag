

#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_multiview : enable

layout(location = 0) out vec4 fragColor;

layout(location = 0) in vec3 worldNormal;
layout(location = 1) in vec3 worldPosition;
layout(location = 2) in vec2 texCoords;

const int MAX_LIGHTS = 8;
const int TYPE_POINT = 0;
const int TYPE_DIRECTIONAL = 1;
const int TYPE_SPOT = 2;

struct Light {
    vec4 color;

    vec3 worldDirection;
    float intensity;

    int type;
    float constantAttenuation;
    float linearAttenuation;
    float quadraticAttenuation;

    vec3 localDirection;
    float cutOffAngle;

    vec3 worldPosition;
    float padding;
};

struct CameraData {
    mat4 viewMatrix;
    mat4 inverseViewMatrix;
    mat4 projectionMatrix;
    mat4 inverseProjectionMatrix;
    mat4 viewProjectionMatrix;
    mat4 inverseViewProjectionMatrix;
    mat4 viewportMatrix;
};

layout(set = 1, binding = 0) uniform SerenityCamera
{
    CameraData data[2];
}
camera;

layout(std140, set = 2, binding = 0) uniform SerenityLights
{
    Light data[MAX_LIGHTS];
    int lightCount;
}
lights;

layout(std140, set = 3, binding = 0) uniform SerenityPhongMaterial
{
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float shininess;
    int hasDiffuseTexture;
}
material;

layout(set = 4, binding = 0) uniform sampler2D texSampler;

void adsModel(const in vec3 worldPos,
              const in vec3 worldNormal,
              const in vec3 worldView,
              const in float shininess,
              out vec3 ambientColor,
              out vec3 diffuseColor,
              out vec3 specularColor)
{
    ambientColor = vec3(0.0);
    diffuseColor = vec3(0.0);
    specularColor = vec3(0.0);

    // We perform all work in world space
    vec3 n = normalize(worldNormal);
    vec3 s = vec3(0.0);

    for (int i = 0; i < lights.lightCount; ++i) {
        float att = 1.0;
        float sDotN = 0.0;

        if (lights.data[i].type != TYPE_DIRECTIONAL) {
            // Point and Spot lights

            // Light position is already in world space
            vec3 sUnnormalized = lights.data[i].worldPosition - worldPos;
            s = normalize(sUnnormalized); // Light direction

            // Calculate the attenuation factor
            sDotN = dot(s, n);
            if (sDotN > 0.0) {
                if (lights.data[i].constantAttenuation != 0.0 || lights.data[i].linearAttenuation != 0.0 || lights.data[i].quadraticAttenuation != 0.0) {
                    float dist = length(sUnnormalized);
                    att = 1.0 / (lights.data[i].constantAttenuation + lights.data[i].linearAttenuation * dist + lights.data[i].quadraticAttenuation * dist * dist);
                }

                // The light direction is in world space already
                if (lights.data[i].type == TYPE_SPOT) {
                    // Check if fragment is inside or outside of the spot light cone
                    if (degrees(acos(dot(-s, lights.data[i].localDirection))) > lights.data[i].cutOffAngle)
                        sDotN = 0.0;
                }
            }
        } else {
            // Directional lights
            // The light direction is in world space already
            s = normalize(-lights.data[i].worldDirection);
            sDotN = dot(s, n);
        }

        // Calculate the diffuse factor
        float diffuse = max(sDotN, 0.0);

        // Calculate the specular factor
        float specular = 0.0;
        if (diffuse > 0.0 && shininess > 0.0) {
            float normFactor = (shininess + 2.0) / 2.0;
            vec3 r = reflect(-s, n); // Reflection direction in world space
            specular = normFactor * pow(max(dot(r, worldView), 0.0), shininess);
        }

        // Accumulate the diffuse and specular contributions
        ambientColor += att * lights.data[i].intensity * 1.0 * lights.data[i].color.rgb;
        diffuseColor += att * lights.data[i].intensity * diffuse * lights.data[i].color.rgb;
        specularColor += att * lights.data[i].intensity * specular * lights.data[i].color.rgb;
    }
}

void main()
{
    // Extract world eye pos from viewMatrix
    vec3 eyePos = camera.data[gl_ViewIndex].inverseViewMatrix[3].xyz;
    vec3 worldView = normalize(eyePos - worldPosition);

    // Calculate the lighting model, keeping the specular component separate
    vec3 ambientColor, diffuseColor, specularColor;
    adsModel(worldPosition, normalize(worldNormal), worldView,
             material.shininess, ambientColor, diffuseColor, specularColor);

    vec4 diffuseTex = material.diffuse;

    if (material.hasDiffuseTexture > 0) {
        diffuseTex = texture(texSampler, texCoords);
    }

    // Combine spec with ambient+diffuse for final fragment color
    vec3 color = ambientColor * material.ambient.rgb +
            diffuseColor * diffuseTex.rgb +
            specularColor * material.specular.rgb;

    fragColor = vec4(color, diffuseTex.a);
}

#include "LightingShader.h"
#include <cmath>
#include <raymath.h>

// Tuning. Colors are 0 to 1 per channel.
static const float LIGHT_HEIGHT = 1.8f;
static const float LIGHT_BASE_RADIUS = 11.0f;   // Tiles until the lantern fades to nothing
static const float LIGHT_INTENSITY = 1.5f;
static const float LIGHT_COLOR[3] = { 1.0f, 0.72f, 0.42f };
static const float AMBIENT_COLOR[3] = { 0.20f, 0.21f, 0.30f }; // Remembered, unlit areas stay readable but dim

static const char* VERTEX_SOURCE = R"GLSL(
#version 330
in vec3 vertexPosition;
in vec4 vertexColor;
in vec2 vertexTexCoord;
uniform mat4 mvp;
uniform mat4 lightModelMatrix;
out vec3 fragPosition;
out vec4 fragColor;
out vec2 fragTexCoord;
void main()
{
    fragPosition = vec3(lightModelMatrix * vec4(vertexPosition, 1.0));
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
)GLSL";

static const char* FRAGMENT_SOURCE = R"GLSL(
#version 330
in vec3 fragPosition;
in vec4 fragColor;
in vec2 fragTexCoord;
out vec4 finalColor;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float lightRadius;
uniform vec3 ambientColor;
uniform sampler2D texture0;
uniform vec4 colDiffuse;
void main()
{
    // Flat face normal from how world position changes across the screen
    vec3 faceNormal = cross(dFdx(fragPosition), dFdy(fragPosition));
    if (dot(faceNormal, faceNormal) < 1e-9)
    {
        faceNormal = vec3(0.0, 1.0, 0.0); // Lines and degenerate fragments
    }
    else
    {
        faceNormal = normalize(faceNormal);
    }

    vec3 toLight = lightPos - fragPosition;
    float distanceToLight = length(toLight);
    vec3 lightDir = toLight / max(distanceToLight, 0.0001);

    // Half-lambert: faces turned away from the lamp still catch some light, which keeps blocks readable
    float diffuse = dot(faceNormal, lightDir) * 0.5 + 0.5;
    diffuse = diffuse * diffuse;

    float falloff = clamp(1.0 - distanceToLight / lightRadius, 0.0, 1.0);
    falloff = falloff * falloff;

    vec3 lighting = ambientColor + lightColor * diffuse * falloff;
    vec4 baseColor = texture(texture0, fragTexCoord) * fragColor * colDiffuse;
    finalColor = vec4(baseColor.rgb * lighting, baseColor.a);
}
)GLSL";

void InitLighting(LightingState& lighting)
{
    lighting.shader = LoadShaderFromMemory(VERTEX_SOURCE, FRAGMENT_SOURCE);
    if (lighting.shader.id == 0)
    {
        lighting.ready = false;
        return;
    }

    lighting.lightPosLoc = GetShaderLocation(lighting.shader, "lightPos");
    lighting.lightColorLoc = GetShaderLocation(lighting.shader, "lightColor");
    lighting.lightRadiusLoc = GetShaderLocation(lighting.shader, "lightRadius");
    lighting.ambientLoc = GetShaderLocation(lighting.shader, "ambientColor");
    lighting.modelMatrixLoc = GetShaderLocation(lighting.shader, "lightModelMatrix");
    SetShaderValueMatrix(lighting.shader, lighting.modelMatrixLoc, MatrixIdentity());
    lighting.ready = true;
}

void UnloadLighting(LightingState& lighting)
{
    if (lighting.ready)
    {
        UnloadShader(lighting.shader);
        lighting.ready = false;
    }
}

void SetLightingModelMatrix(LightingState& lighting, Matrix matrix)
{
    if (!lighting.ready)
    {
        return;
    }
    SetShaderValueMatrix(lighting.shader, lighting.modelMatrixLoc, matrix);
}

void UpdateLighting(LightingState& lighting, float lightX, float lightZ)
{
    if (!lighting.ready)
    {
        return;
    }

    float time = (float)GetTime();
    float flicker = 1.0f + 0.05f * sinf(time * 9.0f) + 0.03f * sinf(time * 23.0f + 1.3f);

    float lightPos[3] = { lightX, LIGHT_HEIGHT, lightZ };
    float lightColor[3] = { LIGHT_COLOR[0] * LIGHT_INTENSITY, LIGHT_COLOR[1] * LIGHT_INTENSITY, LIGHT_COLOR[2] * LIGHT_INTENSITY };
    float lightRadius = LIGHT_BASE_RADIUS * flicker;

    SetShaderValue(lighting.shader, lighting.lightPosLoc, lightPos, SHADER_UNIFORM_VEC3);
    SetShaderValue(lighting.shader, lighting.lightColorLoc, lightColor, SHADER_UNIFORM_VEC3);
    SetShaderValue(lighting.shader, lighting.lightRadiusLoc, &lightRadius, SHADER_UNIFORM_FLOAT);
    SetShaderValue(lighting.shader, lighting.ambientLoc, AMBIENT_COLOR, SHADER_UNIFORM_VEC3);
}
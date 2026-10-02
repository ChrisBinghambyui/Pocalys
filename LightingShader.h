#pragma once
#include <raylib.h>

// One warm point light (the player's lantern) over a dark ambient. Faces are shaded flat, which suits cubes.
// Normals come from screen-space derivatives in the fragment shader, so nothing has to supply them.
struct LightingState
{
    Shader shader = { 0 };
    int lightPosLoc = -1;
    int lightColorLoc = -1;
    int lightRadiusLoc = -1;
    int ambientLoc = -1;
    int modelMatrixLoc = -1;
    bool ready = false; // False if the shader failed to compile. Callers skip BeginShaderMode and draw unlit.
};

// Call once after InitWindow (needs the GL context).
void InitLighting(LightingState& lighting);

// Call before CloseWindow.
void UnloadLighting(LightingState& lighting);

// Call once per frame, before BeginShaderMode. lightX and lightZ are tile-space centers (player.x + 0.5f).
void UpdateLighting(LightingState& lighting, float lightX, float lightZ);


// Tells the shader where the next model draw sits in the world. Pass MatrixIdentity() after the draw so cubes light correctly.
void SetLightingModelMatrix(LightingState& lighting, Matrix matrix);
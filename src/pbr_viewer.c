/***********************************************************************************
*
*   rPBR 1.0 - Physically based rendering viewer for raylib
*
*   FEATURES:
*       - Load OBJ models and texture images in real-time by drag and drop.
*       - Use right mouse button to rotate lighting.
*       - Use middle mouse button to rotate and pan camera.
*       - Use interface to adjust lighting, material and screen parameters (space - display/hide interface).
*       - Press F12 or use Screenshot button to capture a screenshot and save it as PNG file.
*
*   LICENSE: zlib/libpng
*
*   rPBR is licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software:
*
*   Copyright (c) 2017 Victor Fisac
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
***********************************************************************************/

//----------------------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------------------
#include "raylib.h"                         // Required for raylib framework
#include "raymath.h"                        // Required for matrix, vectors and other math functions
#include "pbrcore.h"                        // Required for lighting, environment and drawing functions

//----------------------------------------------------------------------------------
// Defines
//----------------------------------------------------------------------------------
#define         PATH_TEXTURES_HDR           "resources/textures/hdr/pinetree.hdr"

#define         PATH_MODEL                  "resources/models/robot.obj"
#define         PATH_TEXTURES_ALBEDO        "resources/textures/robot/robot_albedo.png"
#define         PATH_TEXTURES_NORMALS       "resources/textures/robot/robot_normals.png"
#define         PATH_TEXTURES_METALLIC      "resources/textures/robot/robot_metallic.png"
#define         PATH_TEXTURES_ROUGHNESS     "resources/textures/robot/robot_roughness.png"
#define         PATH_TEXTURES_AO            "resources/textures/robot/robot_ao.png"
#define         PATH_TEXTURES_EMISSION      "resources/textures/robot/robot_emission.png"
// #define      PATH_TEXTURES_HEIGHT        "resources/textures/robot/robot_height.png"

#define         PATH_SHADERS_POSTFX_VS      "resources/shaders/postfx.vs"
#define         PATH_SHADERS_POSTFX_FS      "resources/shaders/postfx.fs"

#define         MAX_LIGHTS                  4               // Max lights supported by shader
#define         MAX_ROWS                    1               // Rows to render models
#define         MAX_COLUMNS                 1               // Columns to render models
#define         MODEL_SCALE                 1.75f           // Model scale transformation for rendering
#define         MODEL_OFFSET                0.45f           // Distance between models for rendering
#define         ROTATION_SPEED              0.0f            // Models rotation speed
#define         LIGHT_SPEED                 0.1f            // Light rotation input speed
#define         LIGHT_DISTANCE              3.5f            // Light distance from center of world
#define         LIGHT_HEIGHT                1.0f            // Light height from center of world
#define         LIGHT_RADIUS                0.05f           // Light gizmo drawing radius
#define         LIGHT_OFFSET                0.03f           // Light gizmo drawing radius when mouse is over

#define         CUBEMAP_SIZE                1024            // Cubemap texture size
#define         IRRADIANCE_SIZE             32              // Irradiance map from cubemap texture size
#define         PREFILTERED_SIZE            256             // Prefiltered HDR environment map texture size
#define         BRDF_SIZE                   512             // BRDF LUT texture map size

//----------------------------------------------------------------------------------
// Structs and enums
//----------------------------------------------------------------------------------
typedef enum { DEFAULT, ALBEDO, NORMALS, METALLIC, ROUGHNESS, AMBIENT_OCCLUSION, EMISSION, LIGHTING, FRESNEL, IRRADIANCE, REFLECTION } RenderMode;

//----------------------------------------------------------------------------------
// Function Declarations
//----------------------------------------------------------------------------------
void DrawLight(Light light, bool over);                     // Draw a light gizmo based on light attributes

//----------------------------------------------------------------------------------
// Main program
//----------------------------------------------------------------------------------
int main()
{
    // Initialization
    //------------------------------------------------------------------------------
    int screenWidth = 1280;
    int screenHeight = 720;

    // Enable Multi Sampling Anti Aliasing 4x (if available)
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(screenWidth, screenHeight, "rPBR - Physically Based Rendering Viewer");

    // Define render settings states
    RenderMode mode = DEFAULT;
    BackgroundMode backMode = BACKGROUND_SKY;
    bool drawGrid = false;
    bool drawWires = true;
    bool drawLights = true;
    bool drawSkybox = true;

    // Define post-processing effects enabled states
    bool enabledFxaa = true;
    bool enabledBloom = true;
    bool enabledVignette = true;

    // Initialize lighting rotation
    int mousePosX = 0;
    int lastMousePosX = 0;
    float lightAngle = 0.0f;

    // Define the camera to look into our 3d world, its mode and model drawing position
    float rotationAngle = 0.0f;
    Vector3 rotationAxis = { 0.0f, 1.0f, 0.0f };
    Camera camera = {{ 3.5f, 3.0f, 3.5f }, { 0.0f, 0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 60.0f };
    SetCameraMode(camera, CAMERA_FREE);

    // Define environment attributes
    Environment environment = LoadEnvironment(PATH_TEXTURES_HDR, CUBEMAP_SIZE, IRRADIANCE_SIZE, PREFILTERED_SIZE, BRDF_SIZE);

    // Load external resources
    Model model = LoadModel(PATH_MODEL);
    MaterialPBR matPBR = SetupMaterialPBR(environment, (Color){ 255 }, 255, 255);
#if defined(PATH_TEXTURES_ALBEDO)
    SetMaterialTexturePBR(&matPBR, PBR_ALBEDO, LoadTexture(PATH_TEXTURES_ALBEDO));
    SetTextureFilter(matPBR.albedoTex, FILTER_BILINEAR);
#endif
#if defined(PATH_TEXTURES_NORMALS)
    SetMaterialTexturePBR(&matPBR, PBR_NORMALS, LoadTexture(PATH_TEXTURES_NORMALS));
    SetTextureFilter(matPBR.normalsTex, FILTER_BILINEAR);
#endif
#if defined(PATH_TEXTURES_METALLIC)
    SetMaterialTexturePBR(&matPBR, PBR_METALLIC, LoadTexture(PATH_TEXTURES_METALLIC));
    SetTextureFilter(matPBR.metallicTex, FILTER_BILINEAR);
#endif
#if defined(PATH_TEXTURES_ROUGHNESS)
    SetMaterialTexturePBR(&matPBR, PBR_ROUGHNESS, LoadTexture(PATH_TEXTURES_ROUGHNESS));
    SetTextureFilter(matPBR.roughnessTex, FILTER_BILINEAR);
#endif
#if defined(PATH_TEXTURES_AO)
    SetMaterialTexturePBR(&matPBR, PBR_AO, LoadTexture(PATH_TEXTURES_AO));
    SetTextureFilter(matPBR.aoTex, FILTER_BILINEAR);
#endif
#if defined(PATH_TEXTURES_EMISSION)
    SetMaterialTexturePBR(&matPBR, PBR_EMISSION, LoadTexture(PATH_TEXTURES_EMISSION));
    SetTextureFilter(matPBR.heightTex, FILTER_BILINEAR);
#endif
#if defined(PATH_TEXTURES_HEIGHT)
    SetMaterialTexturePBR(&matPBR, PBR_HEIGHT, LoadTexture(PATH_TEXTURES_HEIGHT));
    SetTextureFilter(matPBR.heightTex, FILTER_BILINEAR);
#endif

    // Set up materials and lighting
    Material material = { 0 };
    material.shader = matPBR.env.pbrShader;
    model.material = material;

    // Get PBR shader locations
    int shaderModeLoc = GetShaderLocation(model.material.shader, "renderMode");

    // Define lights attributes
    int lightsCount = 0;
    Light lights[MAX_LIGHTS] = { 0 };
    lights[lightsCount] = CreateLight(LIGHT_POINT, (Vector3){ LIGHT_DISTANCE, LIGHT_HEIGHT, 0.0f }, (Vector3){ 0.0f, 0.0f, 0.0f }, (Color){ 255, 0, 0, 255 }, model.material.shader, &lightsCount);
    lights[lightsCount] = CreateLight(LIGHT_POINT, (Vector3){ 0.0f, LIGHT_HEIGHT, LIGHT_DISTANCE }, (Vector3){ 0.0f, 0.0f, 0.0f }, (Color){ 0, 255, 0, 255 }, model.material.shader, &lightsCount);
    lights[lightsCount] = CreateLight(LIGHT_POINT, (Vector3){ -LIGHT_DISTANCE, LIGHT_HEIGHT, 0.0f }, (Vector3){ 0.0f, 0.0f, 0.0f }, (Color){ 0, 0, 255, 255 }, model.material.shader, &lightsCount);
    lights[lightsCount] = CreateLight(LIGHT_DIRECTIONAL, (Vector3){ 0, LIGHT_HEIGHT*2.0f, -LIGHT_DISTANCE }, (Vector3){ 0.0f, 0.0f, 0.0f }, (Color){ 255, 0, 255, 255 }, model.material.shader, &lightsCount);

    // Create a render texture for antialiasing post-processing effect and initialize Bloom shader
    RenderTexture2D fxTarget = LoadRenderTexture(screenWidth, screenHeight);
    Shader fxShader = LoadShader(PATH_SHADERS_POSTFX_VS, PATH_SHADERS_POSTFX_FS);

    // Get post-processing shader locations
    int enabledFxaaLoc = GetShaderLocation(fxShader, "enabledFxaa");
    int enabledBloomLoc = GetShaderLocation(fxShader, "enabledBloom");
    int enabledVignetteLoc = GetShaderLocation(fxShader, "enabledVignette");

    // Send resolution values to post-processing shader
    float resolution[2] = { (float)screenWidth, (float)screenHeight };
    SetShaderValue(fxShader, GetShaderLocation(fxShader, "resolution"), resolution, 2);
    SetShaderValue(environment.skyShader, GetShaderLocation(environment.skyShader, "resolution"), resolution, 2);

    // Set our game to run at 60 frames-per-second
    SetTargetFPS(60);
    //------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())
    {
        // Update
        //--------------------------------------------------------------------------
        // Update current rotation angle
        rotationAngle += ROTATION_SPEED;

        // Check if a file is dropped
        if (IsFileDropped())
        {
            int fileCount = 0;
            char **droppedFiles = GetDroppedFiles(&fileCount);

            // Check extensions
            if (IsFileExtension(droppedFiles[0], ".hdr"))
            {
                UnloadEnvironment(environment);
                environment = LoadEnvironment(droppedFiles[0], CUBEMAP_SIZE, IRRADIANCE_SIZE, PREFILTERED_SIZE, BRDF_SIZE);
                SetShaderValue(environment.skyShader, GetShaderLocation(environment.skyShader, "resolution"), resolution, 2);
                UnloadMaterialPBR(matPBR);
                matPBR = SetupMaterialPBR(environment, (Color){ 255 }, 255, 255);
            #if defined(PATH_TEXTURES_ALBEDO)
                SetMaterialTexturePBR(&matPBR, PBR_ALBEDO, LoadTexture(PATH_TEXTURES_ALBEDO));
                SetTextureFilter(matPBR.albedoTex, FILTER_BILINEAR);
            #endif
            #if defined(PATH_TEXTURES_NORMALS)
                SetMaterialTexturePBR(&matPBR, PBR_NORMALS, LoadTexture(PATH_TEXTURES_NORMALS));
                SetTextureFilter(matPBR.normalsTex, FILTER_BILINEAR);
            #endif
            #if defined(PATH_TEXTURES_METALLIC)
                SetMaterialTexturePBR(&matPBR, PBR_METALLIC, LoadTexture(PATH_TEXTURES_METALLIC));
                SetTextureFilter(matPBR.metallicTex, FILTER_BILINEAR);
            #endif
            #if defined(PATH_TEXTURES_ROUGHNESS)
                SetMaterialTexturePBR(&matPBR, PBR_ROUGHNESS, LoadTexture(PATH_TEXTURES_ROUGHNESS));
                SetTextureFilter(matPBR.roughnessTex, FILTER_BILINEAR);
            #endif
            #if defined(PATH_TEXTURES_AO)
                SetMaterialTexturePBR(&matPBR, PBR_AO, LoadTexture(PATH_TEXTURES_AO));
                SetTextureFilter(matPBR.aoTex, FILTER_BILINEAR);
            #endif
            #if defined(PATH_TEXTURES_EMISSION)
                SetMaterialTexturePBR(&matPBR, PBR_EMISSION, LoadTexture(PATH_TEXTURES_EMISSION));
                SetTextureFilter(matPBR.heightTex, FILTER_BILINEAR);
            #endif
            #if defined(PATH_TEXTURES_HEIGHT)
                SetMaterialTexturePBR(&matPBR, PBR_HEIGHT, LoadTexture(PATH_TEXTURES_HEIGHT));
                SetTextureFilter(matPBR.heightTex, FILTER_BILINEAR);
            #endif

                // Set up materials and lighting
                material = (Material){ 0 };
                material.shader = matPBR.env.pbrShader;
                model.material = material;
            }
            else if (IsFileExtension(droppedFiles[0], ".obj"))
            {
                UnloadModel(model);
                model = LoadModel(droppedFiles[0]);
                model.material = material;
            }

            ClearDroppedFiles();
        }

        // Check for capture screenshot input
        if (IsKeyPressed(KEY_P)) TakeScreenshot();

        // Check for scene camera reset input
        if (IsKeyPressed(KEY_R))
        {
            rotationAngle = 0.0f;
            camera.position = (Vector3){ 3.5f, 3.0f, 3.5f };
            camera.target = (Vector3){ 0.0f, 0.5f, 0.0f };
            camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
            camera.fovy = 45.0f;
            SetCameraMode(camera, CAMERA_FREE);

            lightAngle = 0.0f;

            for (int i = 0; i < lightsCount; i++)
            {
                float angle = lightAngle + 90*i;
                lights[i].position.x = LIGHT_DISTANCE*cosf(angle*DEG2RAD);
                lights[i].position.z = LIGHT_DISTANCE*sinf(angle*DEG2RAD);
            }
        }

        // Check for lights movement input
        if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON))
        {
            lastMousePosX = mousePosX;
            mousePosX = GetMouseX();
            lightAngle += (mousePosX - lastMousePosX)*LIGHT_SPEED;

            for (int i = 0; i < lightsCount; i++)
            {
                float angle = lightAngle + 90*i;
                lights[i].position.x = LIGHT_DISTANCE*cosf(angle*DEG2RAD);
                lights[i].position.z = LIGHT_DISTANCE*sinf(angle*DEG2RAD);
            }
        }
        else mousePosX = GetMouseX();

        // Check for render mode inputs
        if (IsKeyPressed(KEY_ONE)) mode = DEFAULT;
        else if (IsKeyPressed(KEY_TWO)) mode = ALBEDO;
        else if (IsKeyPressed(KEY_THREE)) mode = NORMALS;
        else if (IsKeyPressed(KEY_FOUR)) mode = METALLIC;
        else if (IsKeyPressed(KEY_FIVE)) mode = ROUGHNESS;
        else if (IsKeyPressed(KEY_SIX)) mode = AMBIENT_OCCLUSION;
        else if (IsKeyPressed(KEY_SEVEN)) mode = EMISSION;
        else if (IsKeyPressed(KEY_EIGHT)) mode = LIGHTING;
        else if (IsKeyPressed(KEY_NINE)) mode = FRESNEL;
        else if (IsKeyPressed(KEY_ZERO)) mode = IRRADIANCE;
        else if (IsKeyPressed(46)) mode = REFLECTION;     // KEY: .

        // Send current mode to PBR shader and enabled screen effects states to post-processing shader
        int shaderMode[1] = { mode };
        SetShaderValuei(model.material.shader, shaderModeLoc, shaderMode, 1);
        shaderMode[0] = enabledFxaa;
        SetShaderValuei(fxShader, enabledFxaaLoc, shaderMode, 1);
        shaderMode[0] = enabledBloom;
        SetShaderValuei(fxShader, enabledBloomLoc, shaderMode, 1);
        shaderMode[0] = enabledVignette;
        SetShaderValuei(fxShader, enabledVignetteLoc, shaderMode, 1);

        // Update current light position to PBR shader
        for (int i = 0; i < MAX_LIGHTS; i++)
        {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                Ray ray = GetMouseRay(GetMousePosition(), camera);
                if (CheckCollisionRaySphere(ray, lights[i].position, LIGHT_RADIUS)) lights[i].enabled = !lights[i].enabled;
            }

            UpdateLightValues(environment.pbrShader, lights[i]);
        }

        // Update camera values and send them to all required shaders
        UpdateCamera(&camera);
        float cameraPos[3] = { camera.position.x, camera.position.y, camera.position.z };
        SetShaderValue(environment.pbrShader, environment.pbrViewLoc, cameraPos, 3);
        //--------------------------------------------------------------------------

        // Draw
        //--------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(DARKGRAY);

            // Render to texture for antialiasing post-processing
            BeginTextureMode(fxTarget);

                Begin3dMode(camera);

                    // Draw ground grid
                    if (drawGrid) DrawGrid(10, 1.0f);

                    // Draw loaded model using physically based rendering
                    DrawModelPBR(model, matPBR, (Vector3){ 0, 0, 0 }, rotationAxis, rotationAngle, (Vector3){ MODEL_SCALE, MODEL_SCALE, MODEL_SCALE });
                    if (drawWires) DrawModelWires(model, (Vector3){ 0, 0, 0 }, MODEL_SCALE, DARKGRAY);

                    // Draw light gizmos
                    if (drawLights) for (unsigned int i = 0; (i < MAX_LIGHTS); i++)
                    {
                        Ray ray = GetMouseRay(GetMousePosition(), camera);
                        DrawLight(lights[i], CheckCollisionRaySphere(ray, lights[i].position, LIGHT_RADIUS));
                    }

                    // Render skybox (render as last to prevent overdraw)
                    if (drawSkybox) DrawSkybox(environment, backMode, camera);

                End3dMode();

            EndTextureMode();

            BeginShaderMode(fxShader);

                DrawTextureRec(fxTarget.texture, (Rectangle){ 0, 0, fxTarget.texture.width, -fxTarget.texture.height }, (Vector2){ 0, 0 }, WHITE);

            EndShaderMode();

            DrawFPS(10, 10);

        EndDrawing();
        //--------------------------------------------------------------------------
    }

    // De-Initialization
    //------------------------------------------------------------------------------
    // Clear internal buffers
    ClearDroppedFiles();

    // Unload loaded model mesh and binded textures
    UnloadModel(model);

    // Unload materialPBR assigned textures
    UnloadMaterialPBR(matPBR);

    // Unload environment loaded shaders and dynamic textures
    UnloadEnvironment(environment);

    // Unload other resources
    UnloadRenderTexture(fxTarget);
    UnloadShader(fxShader);

    // Close window and OpenGL context
    CloseWindow();
    //------------------------------------------------------------------------------

    return 0;
}

//----------------------------------------------------------------------------------
// Function Declarations
//----------------------------------------------------------------------------------
// Draw a light gizmo based on light attributes
void DrawLight(Light light, bool over)
{
    switch (light.type)
    {
        case LIGHT_DIRECTIONAL:
        {
            DrawSphere(light.position, (over ? (LIGHT_RADIUS + LIGHT_OFFSET) : LIGHT_RADIUS), (light.enabled ? light.color : GRAY));
            DrawLine3D(light.position, light.target, (light.enabled ? light.color : DARKGRAY));
        } break;
        case LIGHT_POINT: DrawSphere(light.position, (over ? (LIGHT_RADIUS + LIGHT_OFFSET) : LIGHT_RADIUS), (light.enabled ? light.color : GRAY));
        default: break;
    }
}
#include "raylib.h"
#include "raymath.h"
#include <cmath>

enum GameState { MENU, GAMEPLAY };

struct Vehicle {
    Vector3 position;
    float rotationAngle; // Ángulo en radianes
    float speed;
};

int main(void)
{
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    const int screenWidth = 1280;
    const int screenHeight = 720;
    InitWindow(screenWidth, screenHeight, "Carrera Sidecar 3D - Modelo OBJ");
    SetTargetFPS(60);

    GameState currentState = MENU;

    // 1. Cargar el modelo 3D
    Model sidecarModel = LoadModel("sidecarobj/sidecar.obj");

    // 2. Centrar los vértices del modelo en (0,0,0)
    BoundingBox box = GetModelBoundingBox(sidecarModel);
    Vector3 center = {
        (box.max.x + box.min.x) / 2.0f,
        (box.max.y + box.min.y) / 2.0f,
        box.min.z // Apoyar la base sobre el suelo
    };

    for (int i = 0; i < sidecarModel.meshCount; i++) {
        for (int v = 0; v < sidecarModel.meshes[i].vertexCount * 3; v += 3) {
            sidecarModel.meshes[i].vertices[v]     -= center.x;
            sidecarModel.meshes[i].vertices[v + 1] -= center.y;
            sidecarModel.meshes[i].vertices[v + 2] -= center.z;
        }
        UpdateMeshBuffer(sidecarModel.meshes[i], 0, sidecarModel.meshes[i].vertices, sidecarModel.meshes[i].vertexCount * 3 * sizeof(float), 0);
    }

    // 3. Recalcular escala ajustada
    box = GetModelBoundingBox(sidecarModel);
    float maxDimension = fmaxf(fmaxf(box.max.x - box.min.x, box.max.y - box.min.y), box.max.z - box.min.z);
    float modelScale = (maxDimension > 0.0f) ? (3.0f / maxDimension) : 1.0f;

    // Colores de material visible
    for (int i = 0; i < sidecarModel.materialCount; i++) {
        sidecarModel.materials[i].maps[MATERIAL_MAP_DIFFUSE].color = RED;
    }

    Vehicle vehicle = { { 0.0f, 0.0f, 0.0f }, 0.0f, 0.0f };

    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 4.0f, -8.0f };
    camera.target = vehicle.position;
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 50.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    while (!WindowShouldClose())
    {
        if (currentState == MENU) 
        {
            if (IsKeyPressed(KEY_ENTER)) currentState = GAMEPLAY;
        } 
        else if (currentState == GAMEPLAY) 
        {
            if (IsKeyDown(KEY_W)) vehicle.speed += 0.08f;
            if (IsKeyDown(KEY_S)) vehicle.speed -= 0.04f;

            if (IsKeyDown(KEY_A)) vehicle.rotationAngle += 0.035f;
            if (IsKeyDown(KEY_D)) vehicle.rotationAngle -= 0.035f;

            vehicle.speed *= 0.96f;

            vehicle.position.x += sinf(vehicle.rotationAngle) * vehicle.speed;
            vehicle.position.z += cosf(vehicle.rotationAngle) * vehicle.speed;

            // Seguimiento de cámara
            camera.target = (Vector3){ vehicle.position.x, vehicle.position.y + 0.8f, vehicle.position.z };

            float cameraDistance = 7.0f;
            float cameraHeight = 3.0f;

            camera.position.x = vehicle.position.x - sinf(vehicle.rotationAngle) * cameraDistance;
            camera.position.z = vehicle.position.z - cosf(vehicle.rotationAngle) * cameraDistance;
            camera.position.y = vehicle.position.y + cameraHeight;
        }

        BeginDrawing();
            ClearBackground(DARKGRAY);

            if (currentState == MENU) 
            {
                DrawText("SIMULADOR DE SIDECAR 3D", 380, 250, 35, RED);
                DrawText("PRESIONA ENTER PARA EMPEZAR", 410, 350, 20, RAYWHITE);
            } 
            else if (currentState == GAMEPLAY) 
            {
                BeginMode3D(camera);

                    DrawGrid(100, 2.0f);

                    if (sidecarModel.meshCount > 0) {
                        // MATRIZ DE TRANSFORMACIÓN CON REORIENTACIÓN (+90º)
                        Matrix transform = MatrixIdentity();
                        transform = MatrixMultiply(transform, MatrixScale(modelScale, modelScale, modelScale));
                        transform = MatrixMultiply(transform, MatrixRotateX(-90.0f * DEG2RAD)); // Levanta el modelo
                        transform = MatrixMultiply(transform, MatrixRotateY(90.0f * DEG2RAD));  // Alinea de frente
                        transform = MatrixMultiply(transform, MatrixRotateY(vehicle.rotationAngle)); // Giro de conducción
                        transform = MatrixMultiply(transform, MatrixTranslate(vehicle.position.x, vehicle.position.y, vehicle.position.z));

                        sidecarModel.transform = transform;
                        DrawModel(sidecarModel, (Vector3){ 0, 0, 0 }, 1.0f, WHITE);
                    } else {
                        DrawCube(vehicle.position, 1.5f, 0.8f, 2.5f, RED);
                    }

                EndMode3D();

                DrawText("Usa W, A, S, D para conducir", 10, 10, 20, WHITE);
                DrawFPS(10, 40);
            }

        EndDrawing();
    }

    UnloadModel(sidecarModel);
    CloseWindow();

    return 0;
}
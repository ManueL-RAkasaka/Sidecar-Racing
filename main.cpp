#include "raylib.h"
#include "raymath.h"
#include <cmath>
#include <vector>

enum GameState { MENU, GAMEPLAY };

struct Vehicle {
    Vector3 position;
    float rotationAngle;
    float speed;
};

// Puntos de control reescalados a 1.8x
const std::vector<Vector3> waypoints = {
    {    0.0f, 0.0f,     0.0f }, // Meta
    { 2160.0f, 0.0f,   270.0f }, // Curva 1
    { 2880.0f, 0.0f,  -630.0f }, // Curva 2
    { 4140.0f, 0.0f, -1260.0f }, // Curva 3
    { 4500.0f, 0.0f, -2700.0f }, // Curva 4
    { 3780.0f, 0.0f, -2070.0f }, // Curva 5
    { 3240.0f, 0.0f, -4140.0f }, // Curva 6
    {-1440.0f, 0.0f, -3960.0f }, // Curva 7 (Recta inferior)
    {-1980.0f, 0.0f, -3240.0f }, // Curva 8
    { 1440.0f, 0.0f, -2700.0f }, // Curva 9
    { 1980.0f, 0.0f,  -900.0f }, // Curva 10
    {-1800.0f, 0.0f, -1440.0f }, // Curva 11
    {-1710.0f, 0.0f,  -720.0f }, // Curva 12
    {-1620.0f, 0.0f,  -360.0f }, // Curva 13
    {-1980.0f, 0.0f,   540.0f }, // Curva 14
    { -180.0f, 0.0f,  1350.0f }, // Curva 15
    { -900.0f, 0.0f,   630.0f }, // Curva 16
    { -720.0f, 0.0f,   180.0f }  // Curva 17
};

Vector3 GetSplinePoint(const std::vector<Vector3>& points, float t) {
    int pSize = (int)points.size();
    float scaledT = t * pSize;
    int i = (int)scaledT;
    float localT = scaledT - i;

    Vector3 p0 = points[(i - 1 + pSize) % pSize];
    Vector3 p1 = points[i % pSize];
    Vector3 p2 = points[(i + 1) % pSize];
    Vector3 p3 = points[(i + 2) % pSize];

    float t2 = localT * localT;
    float t3 = t2 * localT;

    float q0 = -t3 + 2.0f*t2 - localT;
    float q1 = 3.0f*t3 - 5.0f*t2 + 2.0f;
    float q2 = -3.0f*t3 + 4.0f*t2 + localT;
    float q3 = t3 - t2;

    return (Vector3){
        0.5f * (p0.x * q0 + p1.x * q1 + p2.x * q2 + p3.x * q3),
        0.0f,
        0.5f * (p0.z * q0 + p1.z * q1 + p2.z * q2 + p3.z * q3)
    };
}

std::vector<Vector3> trackPoints;

void GenerateSmoothTrack(int segmentsPerSegment) {
    trackPoints.clear();
    int totalSegments = (int)waypoints.size() * segmentsPerSegment;
    for (int i = 0; i < totalSegments; i++) {
        float t = (float)i / (float)totalSegments;
        trackPoints.push_back(GetSplinePoint(waypoints, t));
    }
}

void DrawCircuit(float trackWidth) {
    size_t numPoints = trackPoints.size();
    float halfWidth = trackWidth / 2.0f;
    Color asphaltColor = (Color){ 30, 30, 35, 255 };

    for (size_t i = 0; i < numPoints; i++) {
        Vector3 p1 = trackPoints[i];
        Vector3 p2 = trackPoints[(i + 1) % numPoints];

        Vector3 dir = Vector3Normalize(Vector3Subtract(p2, p1));
        Vector3 normal = { -dir.z, 0.0f, dir.x };

        Vector3 left1  = Vector3Add(p1, Vector3Scale(normal, halfWidth));
        Vector3 right1 = Vector3Subtract(p1, Vector3Scale(normal, halfWidth));
        Vector3 left2  = Vector3Add(p2, Vector3Scale(normal, halfWidth));
        Vector3 right2 = Vector3Subtract(p2, Vector3Scale(normal, halfWidth));

        DrawTriangle3D(left1, left2, right1, asphaltColor);
        DrawTriangle3D(right1, left2, right2, asphaltColor);

        Vector3 pEdgeL1 = Vector3Add(left1, Vector3Scale(normal, 0.6f));
        Vector3 pEdgeL2 = Vector3Add(left2, Vector3Scale(normal, 0.6f));
        DrawTriangle3D(left1, pEdgeL2, pEdgeL1, RED);
        DrawTriangle3D(left1, left2, pEdgeL2, RED);

        Vector3 pEdgeR1 = Vector3Subtract(right1, Vector3Scale(normal, 0.6f));
        Vector3 pEdgeR2 = Vector3Subtract(right2, Vector3Scale(normal, 0.6f));
        DrawTriangle3D(right1, pEdgeR1, pEdgeR2, RED);
        DrawTriangle3D(right1, pEdgeR2, right2, RED);
    }

    DrawCube((Vector3){0.0f, 0.1f, 0.0f}, trackWidth, 0.1f, 4.0f, WHITE);
}

int main(void)
{
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    const int screenWidth = 1280;
    const int screenHeight = 720;
    InitWindow(screenWidth, screenHeight, "Carrera Sidecar TT 3D");
    SetTargetFPS(60);

    GenerateSmoothTrack(35);

    GameState currentState = MENU;

    Model sidecarModel = LoadModel("sidecarobj/sidecar.obj");

    BoundingBox box = GetModelBoundingBox(sidecarModel);
    Vector3 center = {
        (box.max.x + box.min.x) / 2.0f,
        (box.max.y + box.min.y) / 2.0f,
        box.min.z
    };

    for (int i = 0; i < sidecarModel.meshCount; i++) {
        for (int v = 0; v < sidecarModel.meshes[i].vertexCount * 3; v += 3) {
            sidecarModel.meshes[i].vertices[v]     -= center.x;
            sidecarModel.meshes[i].vertices[v + 1] -= center.y;
            sidecarModel.meshes[i].vertices[v + 2] -= center.z;
        }
        UpdateMeshBuffer(sidecarModel.meshes[i], 0, sidecarModel.meshes[i].vertices, sidecarModel.meshes[i].vertexCount * 3 * sizeof(float), 0);
    }

    box = GetModelBoundingBox(sidecarModel);
    float targetSidecarWidth = 3.2f; 
    float currentWidth = box.max.x - box.min.x;
    float modelScale = (currentWidth > 0.0f) ? (targetSidecarWidth / currentWidth) : 1.0f;

    for (int i = 0; i < sidecarModel.materialCount; i++) {
        sidecarModel.materials[i].maps[MATERIAL_MAP_DIFFUSE].color = RED;
    }

    Vehicle vehicle = { { 0.0f, 0.0f, 0.0f }, 1.45f, 0.0f };

    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 2.0f, -4.5f };
    camera.target = vehicle.position;
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    const float MAX_SPEED_KMH = 255.0f;
    const float SPEED_SCALE = 57.0f;
    const float MAX_INTERNAL_SPEED = MAX_SPEED_KMH / SPEED_SCALE;

    int gamepad = 0; 

    while (!WindowShouldClose())
    {
        float deltaTime = GetFrameTime();
        bool isGamepadConnected = IsGamepadAvailable(gamepad);

        if (currentState == MENU) 
        {
            if (IsKeyPressed(KEY_ENTER) || (isGamepadConnected && IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_MIDDLE_RIGHT))) {
                currentState = GAMEPLAY;
            }
        } 
        else if (currentState == GAMEPLAY) 
        {
            float currentKmh = vehicle.speed * SPEED_SCALE;

            // --- ACELERACIÓN Y FRENO ---
            float throttle = 0.0f;
            float brake = 0.0f;

            if (isGamepadConnected) {
                throttle = (GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_RIGHT_TRIGGER) + 1.0f) / 2.0f;
                brake = (GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_TRIGGER) + 1.0f) / 2.0f;

                if (IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) throttle = 1.0f;
                if (IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_LEFT)) brake = 1.0f;
            }

            if (IsKeyDown(KEY_W)) throttle = 1.0f;
            if (IsKeyDown(KEY_S)) brake = 1.0f;

            if (throttle > 0.1f) {
                float accelRate = 38.5f * (1.0f - (currentKmh / (MAX_SPEED_KMH + 15.0f)));
                vehicle.speed += (accelRate / SPEED_SCALE) * throttle * deltaTime;
            } else if (brake > 0.1f) {
                vehicle.speed -= (60.0f / SPEED_SCALE) * brake * deltaTime;
            } else {
                vehicle.speed -= (8.0f / SPEED_SCALE) * deltaTime;
            }

            // --- DIRECCIÓN (Sensibilidad reducida para mayor firmeza) ---
            float steerInput = 0.0f;

            if (isGamepadConnected) {
                steerInput = GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_X);
                if (std::abs(steerInput) < 0.1f) steerInput = 0.0f;
            }

            if (IsKeyDown(KEY_A)) steerInput = -1.0f;
            if (IsKeyDown(KEY_D)) steerInput = 1.0f;

            // Factor ajustado a 0.65f (antes 1.3f) para endurecer el giro
            vehicle.rotationAngle -= steerInput * 0.65f * deltaTime;

            // Límites de velocidad
            if (vehicle.speed > MAX_INTERNAL_SPEED) vehicle.speed = MAX_INTERNAL_SPEED;
            if (vehicle.speed < 0.0f) vehicle.speed = 0.0f;

            vehicle.position.x += sinf(vehicle.rotationAngle) * vehicle.speed;
            vehicle.position.z += cosf(vehicle.rotationAngle) * vehicle.speed;

            // --- CÁMARA CERCANA ---
            camera.target = (Vector3){ vehicle.position.x, vehicle.position.y + 0.8f, vehicle.position.z };

            float cameraDistance = 4.5f; // Distancia reducida
            float cameraHeight = 2.0f;   // Altura reducida

            camera.position.x = vehicle.position.x - sinf(vehicle.rotationAngle) * cameraDistance;
            camera.position.z = vehicle.position.z - cosf(vehicle.rotationAngle) * cameraDistance;
            camera.position.y = vehicle.position.y + cameraHeight;
        }

        BeginDrawing();
            ClearBackground((Color){ 34, 139, 34, 255 });

            if (currentState == MENU) 
            {
                DrawText("SIMULADOR DE SIDECAR TT 3D", 360, 250, 35, RED);
                DrawText("PRESIONA ENTER O START EN MANDO", 370, 350, 20, RAYWHITE);
            } 
            else if (currentState == GAMEPLAY) 
            {
                BeginMode3D(camera);

                    DrawCircuit(8.0f);

                    if (sidecarModel.meshCount > 0) {
                        Matrix transform = MatrixIdentity();
                        transform = MatrixMultiply(transform, MatrixScale(modelScale, modelScale, modelScale));
                        transform = MatrixMultiply(transform, MatrixRotateX(-90.0f * DEG2RAD)); 
                        transform = MatrixMultiply(transform, MatrixRotateY(90.0f * DEG2RAD));
                        transform = MatrixMultiply(transform, MatrixRotateY(vehicle.rotationAngle)); 
                        transform = MatrixMultiply(transform, MatrixTranslate(vehicle.position.x, vehicle.position.y, vehicle.position.z));

                        sidecarModel.transform = transform;
                        DrawModel(sidecarModel, (Vector3){ 0, 0, 0 }, 1.0f, WHITE);
                    } else {
                        DrawCube(vehicle.position, 3.2f, 1.2f, 4.5f, RED);
                    }

                EndMode3D();

                DrawText(isGamepadConnected ? "Mando detectado: Usa Stick Izq y Gatillos / A" : "Usa W, A, S, D para conducir", 10, 10, 20, WHITE);
                DrawText(TextFormat("Velocidad: %.0f km/h", vehicle.speed * SPEED_SCALE), 10, 35, 20, YELLOW);
                DrawFPS(10, 65);
            }

        EndDrawing();
    }

    UnloadModel(sidecarModel);
    CloseWindow();

    return 0;
}
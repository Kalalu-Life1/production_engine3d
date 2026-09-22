#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <SDL2/SDL.h>
#include <vector>
#include <array>
#include <chrono>
#include <string>

struct Vector3D {
    float x, y, z;
};

struct Vector2D {
    int x, y;
};

struct Triangle3D {
    std::array<size_t, 3> indices;
    SDL_Color color;
    float averageDepth;
};

struct Mesh3D {
    std::vector<Vector3D> vertices;
    std::vector<Triangle3D> triangles;
    Vector3D position;
    Vector3D rotation;
    Vector3D velocity;
    float boundingRadius;
};

struct RenderJob {
    Vector2D p0, p1, p2;
    SDL_Color color;
    float depth;
};

class GameEngine {
private:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    bool isRunning = false;
    
    std::vector<Mesh3D> sceneMeshes;
    float globalTimeClock = 0.0f;

    uint64_t totalFramesRendered = 0;
    double telemetryInputTimeMs = 0.0;
    double telemetryTransformTimeMs = 0.0;
    double telemetrySortTimeMs = 0.0;
    double telemetryRasterTimeMs = 0.0;

    void fillTriangle(Vector2D p0, Vector2D p1, Vector2D p2, SDL_Color color);
    bool loadMeshFromBinary(const std::string& filePath, Mesh3D& outMesh, SDL_Color meshColor);

    Vector3D calculateNormal(Vector3D p0, Vector3D p1, Vector3D p2);
    float dotProduct(Vector3D v0, Vector3D v1);
    Vector3D normalize(Vector3D v);

public:
    GameEngine() = default;
    ~GameEngine() = default;

    bool initialize();
    void run();
    void shutdown();
};

#endif // ENGINE_HPP

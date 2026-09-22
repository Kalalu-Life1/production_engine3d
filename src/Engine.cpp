#include "Engine.hpp"
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>

Vector3D GameEngine::calculateNormal(Vector3D p0, Vector3D p1, Vector3D p2) {
    Vector3D edge1 = { p1.x - p0.x, p1.y - p0.y, p1.z - p0.z };
    Vector3D edge2 = { p2.x - p0.x, p2.y - p0.y, p2.z - p0.z };
    return {
        edge1.y * edge2.z - edge1.z * edge2.y,
        edge1.z * edge2.x - edge1.x * edge2.z,
        edge1.x * edge2.y - edge1.y * edge2.x
    };
}

float GameEngine::dotProduct(Vector3D v0, Vector3D v1) {
    return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
}

Vector3D GameEngine::normalize(Vector3D v) {
    float length = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    if (length == 0.0f) return {0.0f, 0.0f, 0.0f};
    return { v.x / length, v.y / length, v.z / length };
}

bool GameEngine::loadMeshFromBinary(const std::string& filePath, Mesh3D& outMesh, SDL_Color meshColor) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Production Error: Binary asset missing! -> " << filePath << std::endl;
        return false;
    }

    uint32_t magicCheck = 0;
    file.read(reinterpret_cast<char*>(&magicCheck), sizeof(magicCheck));
    if (magicCheck != 0x3D474D45) {
        std::cerr << "Critical Error: Asset validation crash! Invalid signature format." << std::endl;
        return false;
    }

    uint64_t vertexCount = 0;
    uint64_t triangleCount = 0;
    file.read(reinterpret_cast<char*>(&vertexCount), sizeof(vertexCount));
    file.read(reinterpret_cast<char*>(&triangleCount), sizeof(triangleCount));

    outMesh.vertices.resize(vertexCount);
    file.read(reinterpret_cast<char*>(outMesh.vertices.data()), vertexCount * sizeof(Vector3D));

    struct PackedTriangle { uint32_t i0, i1, i2; };
    std::vector<PackedTriangle> tempTriangles(triangleCount);
    file.read(reinterpret_cast<char*>(tempTriangles.data()), triangleCount * sizeof(PackedTriangle));

    outMesh.triangles.resize(triangleCount);
    for (size_t i = 0; i < triangleCount; ++i) {
        outMesh.triangles[i].indices = { tempTriangles[i].i0, tempTriangles[i].i1, tempTriangles[i].i2 };
        outMesh.triangles[i].color = meshColor;
        outMesh.triangles[i].averageDepth = 0.0f;
    }

    file.close();
    std::cout << "[BINARY STREAM SUCCESS] Managed loading pipeline fetched file: " << filePath 
              << " [" << vertexCount << " Vertices, " << triangleCount << " Triangles]" << std::endl;
    return true;
}

bool GameEngine::initialize() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return false;

    window = SDL_CreateWindow(
        "Production Engine 3D - High-Speed Binary Streaming",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600, SDL_WINDOW_SHOWN
    );
    if (!window) return false;

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) return false;

    // Entity 1: Cube Structure Baseline
    Mesh3D cube;
    cube.position = {-1.6f, 0.4f, 5.0f};
    cube.rotation = {0.0f, 0.0f, 0.0f};
    cube.velocity = {0.025f, 0.015f, 0.0f};
    cube.boundingRadius = 1.4f;
    cube.vertices = {
        {-1.0f, -1.0f, -1.0f}, { 1.0f, -1.0f, -1.0f}, { 1.0f,  1.0f, -1.0f}, {-1.0f,  1.0f, -1.0f},
        {-1.0f, -1.0f,  1.0f}, { 1.0f, -1.0f,  1.0f}, { 1.0f,  1.0f,  1.0f}, {-1.0f,  1.0f,  1.0f}
    };
    cube.triangles = {
        {{0, 1, 2}, {240, 240, 240, 255}, 0.0f}, {{0, 2, 3}, {240, 240, 240, 255}, 0.0f},
        {{1, 5, 6}, {240, 240, 240, 255}, 0.0f}, {{1, 6, 2}, {240, 240, 240, 255}, 0.0f},
        {{4, 0, 3}, {240, 240, 240, 255}, 0.0f}, {{4, 3, 7}, {240, 240, 240, 255}, 0.0f},
        {{5, 4, 7}, {240, 240, 240, 255}, 0.0f}, {{5, 7, 6}, {240, 240, 240, 255}, 0.0f},
        {{3, 2, 6}, {240, 240, 240, 255}, 0.0f}, {{3, 6, 7}, {240, 240, 240, 255}, 0.0f},
        {{4, 5, 1}, {240, 240, 240, 255}, 0.0f}, {{4, 1, 0}, {240, 240, 240, 255}, 0.0f}
    };
    sceneMeshes.push_back(cube);

    // Entity 2: Loaded directly from our freshly baked binary file model
    Mesh3D dynamicDiamond;
    dynamicDiamond.position = {1.6f, -0.4f, 4.8f};
    dynamicDiamond.rotation = {0.0f, 0.0f, 0.0f};
    dynamicDiamond.velocity = {-0.02f, -0.012f, 0.0f};
    dynamicDiamond.boundingRadius = 1.3f;
    
    if (!loadMeshFromBinary("assets/diamond.mesh", dynamicDiamond, {0, 220, 255, 255})) return false;
    sceneMeshes.push_back(dynamicDiamond);

    return true;
}

void GameEngine::fillTriangle(Vector2D p0, Vector2D p1, Vector2D p2, SDL_Color color) {
    if (p0.y > p1.y) std::swap(p0, p1);
    if (p0.y > p2.y) std::swap(p0, p2);
    if (p1.y > p2.y) std::swap(p1, p2);
    if (p0.y == p2.y) return;

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (int y = p0.y; y <= p2.y; ++y) {
        bool isTopHalf = y < p1.y;
        float alpha = static_cast<float>(y - p0.y) / static_cast<float>(p2.y - p0.y);
        float beta  = isTopHalf ? 
            static_cast<float>(y - p0.y) / static_cast<float>(p1.y - p0.y) :
            static_cast<float>(y - p1.y) / static_cast<float>(p2.y - p1.y);

        int startX = static_cast<int>(p0.x + (p2.x - p0.x) * alpha);
        int endX   = isTopHalf ? 
            static_cast<int>(p0.x + (p1.x - p0.x) * beta) :
            static_cast<int>(p1.x + (p2.x - p1.x) * beta);

        if (startX > endX) std::swap(startX, endX);
        SDL_RenderDrawLine(renderer, startX, y, endX, y);
    }
}

void GameEngine::run() {
    isRunning = true;
    SDL_Event event;
    const int HALF_WIDTH = 400; const int HALF_HEIGHT = 300; const float FOV_SCALE = 450.0f;
    Vector3D lightSourceDir = normalize({0.5f, -1.0f, -0.8f});

    while (isRunning) {
        auto frameStartClock = std::chrono::high_resolution_clock::now();
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) isRunning = false;
        }
        auto inputEndClock = std::chrono::high_resolution_clock::now();
        telemetryInputTimeMs += std::chrono::duration<double, std::milli>(inputEndClock - frameStartClock).count();

        SDL_SetRenderDrawColor(renderer, 15, 18, 25, 255); SDL_RenderClear(renderer);
        globalTimeClock += 0.015f;

        // Physics Operations
        for (auto& mesh : sceneMeshes) {
            mesh.position.x += mesh.velocity.x; mesh.position.y += mesh.velocity.y;
            if (mesh.position.x > 3.2f || mesh.position.x < -3.2f) mesh.velocity.x *= -1.0f;
            if (mesh.position.y > 2.2f || mesh.position.y < -2.2f) mesh.velocity.y *= -1.0f;
            mesh.rotation.y += 0.02f; mesh.rotation.x += 0.01f;
        }
        
        float dx = sceneMeshes[0].position.x - sceneMeshes[1].position.x;
        float dy = sceneMeshes[0].position.y - sceneMeshes[1].position.y;
        if (std::sqrt(dx*dx + dy*dy) < (sceneMeshes[0].boundingRadius + sceneMeshes[1].boundingRadius)) {
            std::swap(sceneMeshes[0].velocity.x, sceneMeshes[1].velocity.x);
            std::swap(sceneMeshes[0].velocity.y, sceneMeshes[1].velocity.y);
            sceneMeshes[0].position.x += sceneMeshes[0].velocity.x;
            sceneMeshes[1].position.x += sceneMeshes[1].velocity.x;
        }

        std::vector<RenderJob> globalRenderQueue;
        auto transformStartClock = std::chrono::high_resolution_clock::now();

        for (const auto& mesh : sceneMeshes) {
            std::vector<Vector3D> transformedVertices(mesh.vertices.size());
            std::vector<Vector2D> projectedPoints(mesh.vertices.size());

            for (size_t i = 0; i < mesh.vertices.size(); ++i) {
                float rx = mesh.vertices[i].x; float ry = mesh.vertices[i].y; float rz = mesh.vertices[i].z;
                float rotX = rx, rotY = ry, rotZ = rz;
                if (mesh.rotation.y != 0.0f) {
                    float tx = rotX * cosf(mesh.rotation.y) - rotZ * sinf(mesh.rotation.y);
                    float tz = rotX * sinf(mesh.rotation.y) + rotZ * cosf(mesh.rotation.y);
                    rotX = tx; rotZ = tz;
                }
                if (mesh.rotation.x != 0.0f) {
                    float ty = rotY * cosf(mesh.rotation.x) - rotZ * sinf(mesh.rotation.x);
                    float tz = rotY * sinf(mesh.rotation.x) + rotZ * cosf(mesh.rotation.x);
                    rotY = ty; rotZ = tz;
                }
                transformedVertices[i] = {rotX + mesh.position.x, rotY + mesh.position.y, rotZ + mesh.position.z};
                projectedPoints[i].x = static_cast<int>(HALF_WIDTH + (transformedVertices[i].x * FOV_SCALE) / transformedVertices[i].z);
                projectedPoints[i].y = static_cast<int>(HALF_HEIGHT + (transformedVertices[i].y * FOV_SCALE) / transformedVertices[i].z);
            }

            for (const auto& tri : mesh.triangles) {
                Vector3D v0 = transformedVertices[tri.indices[0]];
                Vector3D v1 = transformedVertices[tri.indices[1]];
                Vector3D v2 = transformedVertices[tri.indices[2]];

                Vector3D normalVector = normalize(calculateNormal(v0, v1, v2));
                if (dotProduct(normalVector, normalize({v0.x, v0.y, v0.z})) > 0.0f) continue;

                float lightIntensity = dotProduct(normalVector, lightSourceDir);
                float finalLightingShade = 0.15f + ((lightIntensity < 0.0f ? 0.0f : lightIntensity) * 0.85f);

                SDL_Color litColor = {
                    static_cast<Uint8>(tri.color.r * finalLightingShade),
                    static_cast<Uint8>(tri.color.g * finalLightingShade),
                    static_cast<Uint8>(tri.color.b * finalLightingShade),
                    255
                };

                float avgDepth = (v0.z + v1.z + v2.z) / 3.0f;
                globalRenderQueue.push_back({projectedPoints[tri.indices[0]], projectedPoints[tri.indices[1]], projectedPoints[tri.indices[2]], litColor, avgDepth});
            }
        }
auto transformEndClock = std::chrono::high_resolution_clock::now();telemetryTransformTimeMs += std::chrono::duration<double, std::milli>(transformEndClock - transformStartClock).count();auto sortStartClock = std::chrono::high_resolution_clock::now();std::sort(globalRenderQueue.begin(), globalRenderQueue.end(), [](const RenderJob& a, const RenderJob& b) { return a.depth > b.depth; });auto sortEndClock = std::chrono::high_resolution_clock::now();telemetrySortTimeMs += std::chrono::duration<double, std::milli>(sortEndClock - sortStartClock).count();auto rasterStartClock = std::chrono::high_resolution_clock::now();for (const auto& job : globalRenderQueue) fillTriangle(job.p0, job.p1, job.p2, job.color);auto rasterEndClock = std::chrono::high_resolution_clock::now();telemetryRasterTimeMs += std::chrono::duration<double, std::milli>(rasterEndClock - rasterStartClock).count();SDL_RenderPresent(renderer); totalFramesRendered++;if (totalFramesRendered % 60 == 0) {std::cout << "\n=============================================" << std::endl;std::cout << "  3D ENGINE CORE PERFORMANCE METRICS OVERLAY  " << std::endl;std::cout << "=============================================" << std::endl;std::cout << " -> Vector Transform  : " << telemetryTransformTimeMs / 60.0 << " ms" << std::endl;std::cout << " -> Software Raster   : " << telemetryRasterTimeMs / 60.0 << " ms" << std::endl;std::cout << " -> File Streaming Layer: 100% Cooked Binary (.mesh Active)" << std::endl;telemetryInputTimeMs = telemetryTransformTimeMs = telemetrySortTimeMs = telemetryRasterTimeMs = 0.0;}SDL_Delay(16);}}void GameEngine::shutdown() {std::cout << "[SYSTEM] Releasing Global Engine Pipeline Context..." << std::endl;if (renderer) SDL_DestroyRenderer(renderer);if (window) SDL_DestroyWindow(window);SDL_Quit();}

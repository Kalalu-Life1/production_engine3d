#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cstdint>

struct Vector3D { float x, y, z; };
struct TriangleData { uint32_t i0, i1, i2; };

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: ./asset_cooker <input.obj> <output.mesh>\n";
        return 1;
    }

    std::string inputPath = argv[1];
    std::string outputPath = argv[2];

    std::ifstream file(inputPath);
    if (!file.is_open()) {
        std::cerr << "Error: Input file missing!\n";
        return 1;
    }

    std::vector<Vector3D> vertices;
    std::vector<TriangleData> triangles;

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string header;
        ss >> header;
        if (header == "v") {
            Vector3D v;
            ss >> v.x >> v.y >> v.z;
            vertices.push_back(v);
        } else if (header == "f") {
            uint32_t i0, i1, i2;
            ss >> i0 >> i1 >> i2;
            triangles.push_back({ i0 - 1, i1 - 1, i2 - 1 });
        }
    }
    file.close();

    // Open clean out stream to bake raw binary files
    std::ofstream outBinary(outputPath, std::ios::binary);
    if (!outBinary.is_open()) {
        std::cerr << "Error: Failed to open output write stream!\n";
        return 1;
    }

    // Write a unique Magic Header Signature (Verifies file validity at runtime)
    uint32_t magicHeader = 0x3D474D45; // "3DGM" (3D Game Mesh Signature)
    outBinary.write(reinterpret_cast<const char*>(&magicHeader), sizeof(magicHeader));

    // Write Array Counts sequentially
    uint64_t vertexCount = vertices.size();
    uint64_t triangleCount = triangles.size();
    outBinary.write(reinterpret_cast<const char*>(&vertexCount), sizeof(vertexCount));
    outBinary.write(reinterpret_cast<const char*>(&triangleCount), sizeof(triangleCount));

    // Stream out raw binary memory array blocks in one transaction pass
    outBinary.write(reinterpret_cast<const char*>(vertices.data()), vertices.size() * sizeof(Vector3D));
    outBinary.write(reinterpret_cast<const char*>(triangles.data()), triangles.size() * sizeof(TriangleData));
    outBinary.close();

    std::cout << "[ASSET COOKER SUCCESS] Baked text asset completely out into optimized binary format: " << outputPath << std::endl;
    return 0;
}

#include "GreedyMesher.h"
#include "Log.h"
#include "AssetManager.h"
#include <array>
#include <vector>
#include <cstring>
#include <stdexcept>




namespace {
    struct MaskCell {
        BlockId id;
        int normal; // +1 or -1
    };

    struct BlockSurface {
        Rectangle uv;
        Color tint;
    };

    BlockSurface GetBlockSurface(BlockId id) {
        switch (id) {
            case 1: return { { 0.0f, 0.0f, 0.5f, 0.5f }, { 255, 255, 255, 255 } };
            case 2: return { { 0.5f, 0.0f, 0.5f, 0.5f }, { 200, 200, 200, 255 } };
            case 3: return { { 0.0f, 0.5f, 0.5f, 0.5f }, { 220, 180, 160, 255 } };
            case 4: return { { 0.5f, 0.5f, 0.5f, 0.5f }, { 100, 150, 255, 128 } }; // Water: semi-transparent blue
            default: return { { 0.5f, 0.5f, 0.5f, 0.5f }, { 255, 255, 255, 255 } };
        }
    }

    inline BlockId SampleLocal(const Chunk& chunk, int x, int y, int z) {
        if (x < 0 || x >= Chunk::SIZE || y < 0 || y >= Chunk::SIZE || z < 0 || z >= Chunk::SIZE) {
            return 0;
        }
        return chunk.Get(x, y, z);
    }

    void AppendQuad(const std::array<float, 3> corners[4], float nx, float ny, float nz,
                    std::vector<float>& vertices, std::vector<float>& normals,
                    std::vector<float>& uvs, std::vector<unsigned char>& colors,
                    std::vector<unsigned short>& indices, std::vector<Quad>& quads,
                    int normalSign, const Rectangle& uvRect, Color tint,
                    int tileWidth, int tileHeight) {
        unsigned short base = static_cast<unsigned short>(vertices.size() / 3);
        for (int i = 0; i < 4; ++i) {
            vertices.push_back(corners[i][0]);
            vertices.push_back(corners[i][1]);
            vertices.push_back(corners[i][2]);

            normals.push_back(nx);
            normals.push_back(ny);
            normals.push_back(nz);

            // Tile the uvRect across the merged quad by scaling the uv extents
            float u0 = uvRect.x;
            float v0 = uvRect.y;
            float u1 = uvRect.x + uvRect.width * static_cast<float>(tileWidth);
            float v1 = uvRect.y + uvRect.height * static_cast<float>(tileHeight);

            // Map corners to tiled UVs. Note: corners are ordered so that
            // 0=(i,j), 1=(i+width,j), 2=(i+width,j+height), 3=(i,j+height)
            switch (i) {
                case 0: uvs.push_back(u0); uvs.push_back(v0); break;
                case 1: uvs.push_back(u1); uvs.push_back(v0); break;
                case 2: uvs.push_back(u1); uvs.push_back(v1); break;
                default: uvs.push_back(u0); uvs.push_back(v1); break;
            }

            colors.push_back(tint.r);
            colors.push_back(tint.g);
            colors.push_back(tint.b);
            colors.push_back(tint.a);
        }

        if (normalSign > 0) {
            indices.push_back(base + 0);
            indices.push_back(base + 1);
            indices.push_back(base + 2);
            indices.push_back(base + 0);
            indices.push_back(base + 2);
            indices.push_back(base + 3);
        } else {
            indices.push_back(base + 0);
            indices.push_back(base + 2);
            indices.push_back(base + 1);
            indices.push_back(base + 0);
            indices.push_back(base + 3);
            indices.push_back(base + 2);
        }

        Quad q;
        for (int i = 0; i < 4; ++i) {
            q.corners[i] = { corners[i][0], corners[i][1], corners[i][2] };
        }
        quads.push_back(q);
    }
} // namespace

bool GreedyMesher::MeshChunk(Chunk& chunk) {
    auto sampler = [&](int gx, int gy, int gz) -> BlockId {
        int ox = chunk.GetCoord().x * Chunk::SIZE;
        int oy = chunk.GetCoord().y * Chunk::SIZE;
        int oz = chunk.GetCoord().z * Chunk::SIZE;
        int lx = gx - ox;
        int ly = gy - oy;
        int lz = gz - oz;
        return SampleLocal(chunk, lx, ly, lz);
    };
    return MeshChunk(chunk, sampler);
}

bool GreedyMesher::MeshChunk(Chunk& chunk, std::function<BlockId(int,int,int)> neighborSampler) {
    const int S = Chunk::SIZE;
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> uvs;
    std::vector<unsigned char> colors;
    std::vector<unsigned short> indices;
    std::vector<Quad> quads;

    vertices.reserve(4096);
    normals.reserve(4096);
    uvs.reserve(4096);
    colors.reserve(4096);
    indices.reserve(8192);
    quads.reserve(256);

    const int dims[3] = { S, S, S };
    std::vector<MaskCell> mask(S * S);

    const int origin[3] = {
        chunk.GetCoord().x * S,
        chunk.GetCoord().y * S,
        chunk.GetCoord().z * S
    };

    auto sample = [&](int x, int y, int z) -> BlockId {
        if (x >= 0 && x < S && y >= 0 && y < S && z >= 0 && z < S) {
            return chunk.Get(x, y, z);
        }
        return neighborSampler(origin[0] + x, origin[1] + y, origin[2] + z);
    };

    for (int axis = 0; axis < 3; ++axis) {
        int u = (axis + 1) % 3;
        int v = (axis + 2) % 3;

        for (int d = -1; d < dims[axis]; ++d) {
            int n = 0;
            for (int j = 0; j < dims[v]; ++j) {
                for (int i = 0; i < dims[u]; ++i, ++n) {
                    std::array<int, 3> a = {0, 0, 0};
                    std::array<int, 3> b = {0, 0, 0};
                    a[axis] = d;
                    a[u] = i;
                    a[v] = j;
                    b = a;
                    b[axis] = d + 1;

                    BlockId voxelA = (d >= 0) ? sample(a[0], a[1], a[2]) : 0;
                    BlockId voxelB = (d + 1 < dims[axis]) ? sample(b[0], b[1], b[2]) : 0;

                    if ((voxelA != 0) == (voxelB != 0)) {
                        mask[n] = {0, 0};
                    } else if (voxelA != 0) {
                        mask[n] = {voxelA, +1};
                    } else {
                        mask[n] = {voxelB, -1};
                    }
                }
            }

            n = 0;
            for (int j = 0; j < dims[v]; ++j) {
                for (int i = 0; i < dims[u]; ++i, ++n) {
                    MaskCell cell = mask[n];
                    if (cell.id == 0) {
                        continue;
                    }

                    int width = 1;
                    while (i + width < dims[u]) {
                        MaskCell rhs = mask[n + width];
                        if (rhs.id != cell.id || rhs.normal != cell.normal) break;
                        ++width;
                    }

                    int height = 1;
                    bool blocked = false;
                    while (!blocked && j + height < dims[v]) {
                        for (int k = 0; k < width; ++k) {
                            MaskCell below = mask[n + k + height * dims[u]];
                            if (below.id != cell.id || below.normal != cell.normal) {
                                blocked = true;
                                break;
                            }
                        }
                        if (!blocked) ++height;
                    }

                    float plane = static_cast<float>(d + 1);
                    std::array<float, 3> corners[4];
                    for (int c = 0; c < 4; ++c) {
                        corners[c] = {0.0f, 0.0f, 0.0f};
                        corners[c][axis] = plane;
                    }

                    corners[0][u] = static_cast<float>(i);
                    corners[0][v] = static_cast<float>(j);

                    corners[1][u] = static_cast<float>(i + width);
                    corners[1][v] = static_cast<float>(j);

                    corners[2][u] = static_cast<float>(i + width);
                    corners[2][v] = static_cast<float>(j + height);

                    corners[3][u] = static_cast<float>(i);
                    corners[3][v] = static_cast<float>(j + height);

                    float nx = 0.0f, ny = 0.0f, nz = 0.0f;
                    if (axis == 0) nx = static_cast<float>(cell.normal);
                    if (axis == 1) ny = static_cast<float>(cell.normal);
                    if (axis == 2) nz = static_cast<float>(cell.normal);

                    BlockSurface surface = GetBlockSurface(cell.id);
                                    AppendQuad(corners, nx, ny, nz, vertices, normals, uvs, colors, indices, quads, cell.normal, surface.uv, surface.tint, width, height);

                    for (int dy = 0; dy < height; ++dy) {
                        for (int dx = 0; dx < width; ++dx) {
                            mask[n + dx + dy * dims[u]] = {0, 0};
                        }
                    }
                }
            }
        }
    }

    if (vertices.empty()) {
        if (chunk.hasModel) {
            UnloadModel(chunk.model);
            chunk.hasModel = false;
        }
        chunk.quads.clear();
        return true;
    }

    Mesh mesh{};
    mesh.vertexCount = static_cast<int>(vertices.size() / 3);
    mesh.triangleCount = static_cast<int>(indices.size() / 3);

    mesh.vertices = static_cast<float*>(RL_MALLOC(sizeof(float) * vertices.size()));
    mesh.normals = static_cast<float*>(RL_MALLOC(sizeof(float) * normals.size()));
    mesh.texcoords = static_cast<float*>(RL_MALLOC(sizeof(float) * uvs.size()));
    mesh.colors = static_cast<unsigned char*>(RL_MALLOC(sizeof(unsigned char) * colors.size()));
    mesh.indices = static_cast<unsigned short*>(RL_MALLOC(sizeof(unsigned short) * indices.size()));

    std::memcpy(mesh.vertices, vertices.data(), sizeof(float) * vertices.size());
    std::memcpy(mesh.normals, normals.data(), sizeof(float) * normals.size());
    std::memcpy(mesh.texcoords, uvs.data(), sizeof(float) * uvs.size());
    std::memcpy(mesh.colors, colors.data(), sizeof(unsigned char) * colors.size());
    std::memcpy(mesh.indices, indices.data(), sizeof(unsigned short) * indices.size());

    UploadMesh(&mesh, false);

    Model model = LoadModelFromMesh(mesh);
    if (chunk.hasModel) {
        UnloadModel(chunk.model);
    }

    Texture2D atlas = {};
    try {
        atlas = AssetManager::GetTexture("blocks");
    } catch (const std::out_of_range&) {
        // Fallback: try to load the atlas directly if AssetManager didn't
        Log::Warning("GreedyMesher: atlas not in AssetManager; attempting fallback load");
        std::vector<std::string> candidates = {
            "res/gravel_64x64_09.png",
            "../res/gravel_64x64_09.png",
            "build/res/gravel_64x64_09.png",
            "../../res/gravel_64x64_09.png"
        };
        for (const auto& path : candidates) {
            Image img = LoadImage(path.c_str());
            if (img.data) {
                atlas = LoadTextureFromImage(img);
                UnloadImage(img);
                if (atlas.id != 0) {
                    SetTextureFilter(atlas, TEXTURE_FILTER_POINT);
                    SetTextureWrap(atlas, TEXTURE_WRAP_REPEAT);
                    Log::Info(std::string("GreedyMesher: fallback loaded atlas from ") + path);
                    break;
                }
            }
        }
    }

    if (atlas.id != 0) {
        SetMaterialTexture(&model.materials[0], MATERIAL_MAP_DIFFUSE, atlas);
        model.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
        SetModelMeshMaterial(&model, 0, 0);
    }

    chunk.model = model;
    chunk.hasModel = true;
    chunk.quads = quads;

    Log::Info("GreedyMesher: quads=" + std::to_string(quads.size()) +
              " verts=" + std::to_string(mesh.vertexCount));

    return true;
}

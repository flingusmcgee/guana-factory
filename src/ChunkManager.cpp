#include "ChunkManager.h"
#include "Chunk.h"
#include "WorldGenerator.h"
#include "HeightmapGenerator.h"
#include "Log.h"
#include "GreedyMesher.h"
#include "Input.h"
#include "AssetManager.h"
#include <vector>
#include <memory>
#include <unordered_map>
#include <string>

namespace {
    std::unique_ptr<WorldGenerator> g_generator;
    std::unordered_map<std::string, std::unique_ptr<Chunk>> g_chunkMap;
    static inline std::string KeyFor(int x,int y,int z) { return std::to_string(x) + "," + std::to_string(y) + "," + std::to_string(z); }
}

namespace ChunkManager {
    static bool g_drawVoxelDebug = false; // default to meshed rendering

    void Init() {
        g_generator = std::make_unique<HeightmapGenerator>();
        const int R = 1;
        const int S = Chunk::SIZE;
        int totalFilled = 0;
        g_chunkMap.clear();
        for (int cx = -R; cx <= R; ++cx) {
            for (int cz = -R; cz <= R; ++cz) {
                int cy = 0;
                auto ch = std::make_unique<Chunk>();
                ch->Init(ChunkCoord{cx,cy,cz});
                int volume = S*S*S;
                std::vector<BlockId> blocks(volume);
                g_generator->GenerateChunk(ch->GetCoord(), blocks.data(), S);
                int filled = 0;
                for (int x = 0; x < S; ++x) for (int y = 0; y < S; ++y) for (int z = 0; z < S; ++z) {
                    int idx = x + z * S + y * S * S;
                    ch->Set(x,y,z, blocks[idx]);
                    if (blocks[idx] != 0) ++filled;
                }
                totalFilled += filled;

                // Build greedy mesh for this chunk so we render an optimized model
                if (!GreedyMesher::MeshChunk(*ch)) {
                    Log::Warning("GreedyMesher failed to mesh chunk: " + KeyFor(cx,cy,cz));
                }
                g_chunkMap.emplace(KeyFor(cx,cy,cz), std::move(ch));
            }
        }
        Log::Info(std::string("Chunks total filled blocks: ") + std::to_string(totalFilled));
    }

    void Shutdown() {
        g_generator.reset();
        g_chunkMap.clear();
    }

    void Render(const Camera& cam) {
        Input::Update();
        if (Input::WasPressed("debug_toggle")) {
            g_drawVoxelDebug = !g_drawVoxelDebug;
            Log::Info(std::string("Voxel debug: ") + (g_drawVoxelDebug ? "ON" : "OFF"));
        }

        const int S = Chunk::SIZE;
        static bool drawnTestPlane = false;
        for (auto &p : g_chunkMap) {
            Chunk* ch = p.second.get();
            if (g_drawVoxelDebug) {
                Color dbgCol = {150, 111, 51, 255};
                for (int x = 0; x < S; ++x) {
                    for (int y = 0; y < S; ++y) {
                        for (int z = 0; z < S; ++z) {
                            BlockId id = ch->Get(x,y,z);
                            if (id == 0) continue;
                            Vector3 pos = { ch->GetCoord().x * S + static_cast<float>(x), ch->GetCoord().y * S + static_cast<float>(y), ch->GetCoord().z * S + static_cast<float>(z) };
                            DrawCube(pos, 1.0f, 1.0f, 1.0f, dbgCol);
                        }
                    }
                }
            } else {
                if (ch->hasModel) {
                    // Draw the chunk model; place it at chunk origin
                    Vector3 origin = { static_cast<float>(ch->GetCoord().x * S), static_cast<float>(ch->GetCoord().y * S), static_cast<float>(ch->GetCoord().z * S) };
                    // Ensure the chunk model's material is bound to the atlas at draw time (override stale state)
                    try {
                        Texture2D &atlas = AssetManager::GetTexture("atlas");
                        if (ch->model.materialCount > 0) {
                            SetMaterialTexture(&ch->model.materials[0], MATERIAL_MAP_ALBEDO, atlas);
                            SetMaterialTexture(&ch->model.materials[0], MATERIAL_MAP_DIFFUSE, atlas);
                            ch->model.materials[0].maps[MATERIAL_MAP_ALBEDO].color = WHITE;
                            {
                                auto c = ch->GetCoord();
                                Log::Info(std::string("ChunkManager: bound atlas to chunk model at ") + std::to_string(c.x) + "," + std::to_string(c.y) + "," + std::to_string(c.z));
                            }
                        }
                    } catch (...) {}
                    DrawModel(ch->model, origin, 1.0f, WHITE);
                    // DEBUG: draw a billboarded atlas texture at the chunk center so we can verify texturing per-chunk
                    Vector3 center = { origin.x + S*0.5f, origin.y + S*0.5f, origin.z + S*0.5f };
                    try {
                        Texture2D &atlas = AssetManager::GetTexture("atlas");
                        Rectangle src = { 0.0f, 0.0f, static_cast<float>(atlas.width), static_cast<float>(atlas.height) };
                        Vector2 dstSize = { 0.6f, 0.6f };
                        DrawBillboardRec((Camera&)cam, atlas, src, center, dstSize, WHITE);
                    } catch (...) {
                        // fallback to magenta sphere when atlas not available
                        DrawSphere(center, 0.15f, MAGENTA);
                    }

                    // One-time test: draw a billboarded textured quad at the first chunk center to ensure texturing works
                    if (!drawnTestPlane) {
                        drawnTestPlane = true;
                        try {
                            Texture2D &atlas = AssetManager::GetTexture("atlas");
                            // source rect covers the whole texture; destination size set to chunk-size in world units
                            Rectangle src = { 0.0f, 0.0f, static_cast<float>(atlas.width), static_cast<float>(atlas.height) };
                            Vector2 dstSize = { static_cast<float>(S), static_cast<float>(S) };
                            Vector3 pos = { origin.x + S*0.5f, origin.y + 0.5f, origin.z + S*0.5f };
                            DrawBillboardRec((Camera&)cam, atlas, src, pos, dstSize, WHITE);
                            Log::Info(std::string("TestBillboard: drawn atlas at chunk center pos=(") + std::to_string(pos.x) + "," + std::to_string(pos.y) + "," + std::to_string(pos.z) + ")");
                        } catch (...) {
                            // ignore
                        }
                    }

                    // One-time test: create a simple GenMeshCube model, bind atlas to it and draw next to the chunk
                    static bool drawnTestModel = false;
                    if (!drawnTestModel) {
                        drawnTestModel = true;
                        try {
                            Texture2D &atlas = AssetManager::GetTexture("atlas");
                            Mesh cubeMesh = GenMeshCube(1.0f, 1.0f, 1.0f);
                            Model cubeModel = LoadModelFromMesh(cubeMesh);
                            UnloadMesh(cubeMesh);
                            if (cubeModel.materialCount > 0) {
                                SetMaterialTexture(&cubeModel.materials[0], MATERIAL_MAP_ALBEDO, atlas);
                                SetMaterialTexture(&cubeModel.materials[0], MATERIAL_MAP_DIFFUSE, atlas);
                                cubeModel.materials[0].maps[MATERIAL_MAP_ALBEDO].color = WHITE;
                                int mid = cubeModel.materials[0].maps[MATERIAL_MAP_ALBEDO].texture.id;
                                int sid = static_cast<int>(cubeModel.materials[0].shader.id);
                                Log::Info(std::string("TestModel: cubeModel material shader=") + std::to_string(sid) + " tex=" + std::to_string(mid));
                            }
                            // draw it slightly above the chunk origin for visibility
                            Vector3 testPos = { origin.x + S + 1.5f, origin.y + 0.5f, origin.z + 0.0f };
                            DrawModel(cubeModel, testPos, 1.0f, WHITE);
                            UnloadModel(cubeModel);
                        } catch (...) {
                            Log::Info("TestModel: failed to create/draw cubeModel");
                        }
                    }
                } else {
                    bool any = false;
                    for (int x = 0; x < S && !any; ++x) for (int y = 0; y < S && !any; ++y) for (int z = 0; z < S; ++z) if (ch->Get(x,y,z) != 0) { any = true; break; }
                    if (any) {
                        Color dirtCol = {150, 111, 51, 200};
                        Vector3 center = { ch->GetCoord().x * S + S*0.5f, ch->GetCoord().y * S + S*0.5f, ch->GetCoord().z * S + S*0.5f };
                        DrawCube(center, static_cast<float>(S), static_cast<float>(S), static_cast<float>(S), dirtCol);
                    }
                }
            }
        }
    }
}

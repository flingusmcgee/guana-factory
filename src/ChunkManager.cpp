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
    static bool g_drawVoxelDebug = true; // default to voxel debug so terrain is immediately visible

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
    // previously used for a one-time debug draw; unused now
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
                    // Draw the chunk model as-is. We intentionally avoid binding any atlas texture here
                    // so the rendering path remains the same as prior to adding per-block texture plumbing.
                    DrawModel(ch->model, origin, 1.0f, WHITE);
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

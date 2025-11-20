#include "ChunkManager.h"
#include "Chunk.h"
#include "WorldGenerator.h"
#include "HeightmapGenerator.h"
#include "Log.h"
#include "GreedyMesher.h"
#include <vector>
#include <memory>
#include <unordered_map>
#include <string>

namespace {
    std::unique_ptr<WorldGenerator> g_generator;
    std::unordered_map<std::string, std::shared_ptr<Chunk>> g_chunkMap;
    ChunkRegistry g_registry;
    static inline std::string KeyFor(int x,int y,int z) { return std::to_string(x) + "," + std::to_string(y) + "," + std::to_string(z); }
}

namespace ChunkManager {
    ChunkRegistry& GetRegistry() {
        return g_registry;
    }

    std::shared_ptr<Chunk> GetChunkByPath(const ChunkPath& path) {
        return g_registry.Get(path);
    }

    void SaveAllChunks(const std::string& basePath) {
        auto chunks = g_registry.GetAll();
        int saved = 0;
        int failed = 0;

        for (auto& chunk : chunks) {
            if (chunk && chunk->Save(basePath)) {
                ++saved;
            } else {
                ++failed;
            }
        }

        Log::Info("ChunkManager::SaveAllChunks - Saved " + std::to_string(saved) + " chunks, " + 
                  std::to_string(failed) + " failed (total " + std::to_string(chunks.size()) + ")");
    }

    bool LoadChunk(const ChunkPath& path, const std::string& basePath) {
        auto chunk = std::make_shared<Chunk>();
        chunk->InitWithPath(path);
        
        if (chunk->Load(basePath)) {
            if (!GreedyMesher::MeshChunk(*chunk)) {
                Log::Warning("ChunkManager::LoadChunk - Failed to mesh loaded chunk " + path.ToHexString());
            }

            if (g_registry.Add(chunk)) {
                Log::Info("ChunkManager::LoadChunk - Successfully loaded and registered chunk " + path.ToHexString());
                return true;
            } else {
                Log::Warning("ChunkManager::LoadChunk - Failed to register loaded chunk " + path.ToHexString() + " (already exists?)");
                return false;
            }
        } else {
            Log::Warning("ChunkManager::LoadChunk - Failed to load chunk data from " + basePath);
            return false;
        }
    }

    void Init() {
        g_generator = std::make_unique<HeightmapGenerator>();
        // Generate 8x8 = 64 chunks around origin
        const int S = Chunk::SIZE;
        int totalFilled = 0;
        g_chunkMap.clear();
        g_registry.Clear();
        
        for (int cx = -4; cx <= 3; ++cx) {
            for (int cz = -4; cz <= 3; ++cz) {
                int cy = 0;
                auto ch = std::make_shared<Chunk>();
                ch->Init(ChunkCoord{cx,cy,cz});
                ChunkPath p;
                p.Append(cx, cy, cz);
                ch->SetPath(p);
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

                if (!GreedyMesher::MeshChunk(*ch)) {
                    Log::Warning("GreedyMesher failed for chunk " + KeyFor(cx, cy, cz));
                }

                Log::Info(std::string("Chunk created: ") + ch->GetIdentifier() + 
                          " at coord (" + std::to_string(cx) + "," + std::to_string(cy) + "," + std::to_string(cz) + ")");
                
                g_chunkMap.emplace(KeyFor(cx,cy,cz), ch);
                
                g_registry.Add(ch);
            }
        }
        Log::Info(std::string("Chunks total filled blocks: ") + std::to_string(totalFilled));
    }

    void Shutdown() {
        g_generator.reset();
        g_chunkMap.clear();
    }

    void Render(const Camera& cam) {
        (void)cam; // cam is unused for now; silence -Werror=unused-parameter
        const int S = Chunk::SIZE;
        for (auto &p : g_chunkMap) {
            Chunk* ch = p.second.get();
            if (!ch->hasModel) {
                if (!GreedyMesher::MeshChunk(*ch)) {
                    Log::Warning("GreedyMesher remesh failed for chunk " + KeyFor(ch->GetCoord().x, ch->GetCoord().y, ch->GetCoord().z));
                    continue;
                }
            }

            Vector3 origin = {
                static_cast<float>(ch->GetCoord().x * S),
                static_cast<float>(ch->GetCoord().y * S),
                static_cast<float>(ch->GetCoord().z * S)
            };

            DrawModel(ch->model, origin, 1.0f, WHITE);

            Color outline = BLACK;
            for (const Quad& quad : ch->quads) {
                Vector3 a = { origin.x + quad.corners[0].x, origin.y + quad.corners[0].y, origin.z + quad.corners[0].z };
                Vector3 b = { origin.x + quad.corners[1].x, origin.y + quad.corners[1].y, origin.z + quad.corners[1].z };
                Vector3 c = { origin.x + quad.corners[2].x, origin.y + quad.corners[2].y, origin.z + quad.corners[2].z };
                Vector3 d = { origin.x + quad.corners[3].x, origin.y + quad.corners[3].y, origin.z + quad.corners[3].z };

                DrawLine3D(a, b, outline);
                DrawLine3D(b, c, outline);
                DrawLine3D(c, d, outline);
                DrawLine3D(d, a, outline);

                DrawLine3D(a, c, outline);
                DrawLine3D(b, d, outline);
            }
        }
    }
}


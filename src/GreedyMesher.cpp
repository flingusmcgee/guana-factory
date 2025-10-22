#include "GreedyMesher.h"
#include "Log.h"
#include "AssetManager.h"
#include <string>
#include <vector>
#include <cstring>
#include <algorithm>

// This implements a standard greedy meshing algorithm adapted from Mikola Lysenko's technique:
// For each axis (0=x,1=y,2=z), iterate slices orthogonal to that axis, build a 2D mask of visible faces
// and greedily merge rectangles of the same material into quads.

using std::vector;

// Helper to push a quad (two triangles). Assumes CCW winding when looking at the face.
static void PushQuad(vector<float>& verts, vector<float>& normals, vector<float>& uvs, vector<unsigned short>& indices,
                     float ax, float ay, float az, float bx, float by, float bz, float cx, float cy, float cz, float dx, float dy, float dz,
                     float nx, float ny, float nz, Rectangle uvrect, bool flip=false) {
    unsigned short base = static_cast<unsigned short>(verts.size() / 3);
    // a,b,c,d are corners in order
    verts.push_back(ax); verts.push_back(ay); verts.push_back(az);
    verts.push_back(bx); verts.push_back(by); verts.push_back(bz);
    verts.push_back(cx); verts.push_back(cy); verts.push_back(cz);
    verts.push_back(dx); verts.push_back(dy); verts.push_back(dz);

    for (int i = 0; i < 4; ++i) { normals.push_back(nx); normals.push_back(ny); normals.push_back(nz); }
    // uv mapping using uvrect (normalized 0..1)
    uvs.push_back(uvrect.x);                uvs.push_back(uvrect.y);
    uvs.push_back(uvrect.x + uvrect.width); uvs.push_back(uvrect.y);
    uvs.push_back(uvrect.x + uvrect.width); uvs.push_back(uvrect.y + uvrect.height);
    uvs.push_back(uvrect.x);                uvs.push_back(uvrect.y + uvrect.height);

    if (!flip) {
        // two triangles (0,1,2) and (0,2,3)
        indices.push_back(base + 0); indices.push_back(base + 1); indices.push_back(base + 2);
        indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 3);
    } else {
        // flipped winding for back-facing faces: (0,2,1) and (0,3,2)
        indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 1);
        indices.push_back(base + 0); indices.push_back(base + 3); indices.push_back(base + 2);
    }
}

// Check if a block is solid (non-zero)
static inline bool Solid(const Chunk& c, int x, int y, int z) {
    return c.Get(x,y,z) != 0;
}

bool GreedyMesher::MeshChunk(Chunk& chunk) {
    // default sampler that queries only local chunk blocks; used by the older API
    auto sampler = [&](int gx, int gy, int gz)->BlockId {
        // convert global coords to local chunk coords by subtracting origin
        int ox = chunk.GetCoord().x * Chunk::SIZE;
        int oy = chunk.GetCoord().y * Chunk::SIZE;
        int oz = chunk.GetCoord().z * Chunk::SIZE;
        int lx = gx - ox;
        int ly = gy - oy;
        int lz = gz - oz;
        if (lx < 0 || lx >= Chunk::SIZE || ly < 0 || ly >= Chunk::SIZE || lz < 0 || lz >= Chunk::SIZE) return 0;
        return chunk.Get(lx,ly,lz);
    };
    return MeshChunk(chunk, sampler);
}

bool GreedyMesher::MeshChunk(Chunk& chunk, std::function<BlockId(int,int,int)> neighborSampler) {
    const int S = Chunk::SIZE;
    vector<float> verts; verts.reserve(8192);
    vector<float> normals; normals.reserve(8192);
    vector<float> uvs; uvs.reserve(8192);
    vector<unsigned short> indices; indices.reserve(8192);

    // For each axis
    for (int axis = 0; axis < 3; ++axis) {

        // iterate slices along 'axis'
        for (int d = 0; d < S; ++d) {
            // Build mask for this slice: 2D array of SxS entries
            // mask value: 0 = nothing, >0 = positive face for BlockId, <0 = negative face for BlockId (-id)
            std::vector<int> mask(S * S, 0);

            for (int i = 0; i < S; ++i) {
                for (int j = 0; j < S; ++j) {
                    int x = 0, y = 0, z = 0;
                    // map (i,j,d) to (x,y,z) depending on axis
                    if (axis == 0) { x = d; y = i; z = j; }
                    else if (axis == 1) { x = i; y = d; z = j; }
                    else { x = i; y = j; z = d; }

                    // Use sampler for neighbor queries; convert local -> global coordinates
                    int gx = chunk.GetCoord().x * S + x;
                    int gy = chunk.GetCoord().y * S + y;
                    int gz = chunk.GetCoord().z * S + z;
                    BlockId bid = chunk.Get(x,y,z);
                    // neighbor global coords on +axis
                    int ngx = gx + (axis==0?1:0);
                    int ngy = gy + (axis==1?1:0);
                    int ngz = gz + (axis==2?1:0);
                    BlockId nbid = neighborSampler(ngx, ngy, ngz);

                    if (bid != 0 && nbid == 0) {
                        // this block has a +face exposed
                        mask[i + j * S] = static_cast<int>(bid); // positive id
                    } else if (bid == 0 && nbid != 0) {
                        // neighbor has a +face exposed which is our -face
                        mask[i + j * S] = -static_cast<int>(nbid); // negative id encodes face on neighbor
                    } else {
                        mask[i + j * S] = 0;
                    }
                }
            }

            // Greedy merge on mask (i across S, j down S)
            for (int j = 0; j < S; ++j) {
                for (int i = 0; i < S; ++i) {
                    int m = mask[i + j * S];
                    if (m == 0) continue;
                    // find width
                    int w;
                    for (w = 1; i + w < S && mask[(i + w) + j * S] == m; ++w) {}
                    // find height
                    int h;
                    bool done = false;
                    for (h = 1; j + h < S; ++h) {
                        for (int k = 0; k < w; ++k) {
                            if (mask[(i + k) + (j + h) * S] != m) { done = true; break; }
                        }
                        if (done) break;
                    }

                    // Emit quad covering (i..i+w-1, j..j+h-1) on slice d
                    // Convert quad to world coords depending on axis and whether face is + or -
                    float ax, ay, az, bx, by, bz, cx, cy, cz, dx, dy, dz;
                    float nxn=0, nyn=0, nzn=0;
                    if (axis == 0) {
                        // x = d or x = d (face on x plane). For + face, plane at x=d+1
                        int faceSign = (m > 0) ? 1 : -1;
                        int x0 = d + (faceSign == 1 ? 1 : 0);
                        nxn = (faceSign == 1) ? 1.0f : -1.0f; nyn = 0; nzn = 0;
                        // corners in (x,y,z)
                        ax = x0; ay = i;     az = j;
                        bx = x0; by = i;     bz = j + w;
                        cx = x0; cy = i + h; cz = j + w;
                        dx = x0; dy = i + h; dz = j;
                    } else if (axis == 1) {
                        int faceSign = (m > 0) ? 1 : -1;
                        int y0 = d + (faceSign == 1 ? 1 : 0);
                        nxn = 0; nyn = (faceSign == 1) ? 1.0f : -1.0f; nzn = 0;
                        ax = i; ay = y0; az = j;
                        bx = i + w; by = y0; bz = j;
                        cx = i + w; cy = y0; cz = j + h;
                        dx = i;     dy = y0; dz = j + h;
                    } else {
                        int faceSign = (m > 0) ? 1 : -1;
                        int z0 = d + (faceSign == 1 ? 1 : 0);
                        nxn = 0; nyn = 0; nzn = (faceSign == 1) ? 1.0f : -1.0f;
                        ax = i; ay = j;     az = z0;
                        bx = i + w; by = j;     bz = z0;
                        cx = i + w; cy = j + h; cz = z0;
                        dx = i;     dy = j + h; dz = z0;
                    }

                    // Convert to chunk-local coordinates (we'll draw the Model at the chunk origin)
                    float wx[4], wy[4], wz[4];
                    for (int q = 0; q < 4; ++q) {
                        float lx, ly, lz;
                        if (q == 0) { lx = ax; ly = ay; lz = az; }
                        else if (q == 1) { lx = bx; ly = by; lz = bz; }
                        else if (q == 2) { lx = cx; ly = cy; lz = cz; }
                        else { lx = dx; ly = dy; lz = dz; }
                        wx[q] = lx; wy[q] = ly; wz[q] = lz;
                    }

                    bool flip = ( (m < 0) );
                    int faceSign = (m > 0) ? 1 : -1;
                    BlockId bid = static_cast<BlockId>( (m > 0) ? m : -m );
                    // faceIndex omitted: we no longer consult an atlas for per-face UVs.
                    // Use a neutral/default uvrect so generated meshes keep a consistent (blank) UV layout.
                    int faceIndex = axis * 2 + (faceSign == 1 ? 0 : 1);
                    Rectangle uvrect = { 0.0f, 0.0f, 0.0f, 0.0f };
                    // silence -Wunused-variable for variables kept for future plumbing
                    (void)bid; (void)faceIndex;
                    PushQuad(verts, normals, uvs, indices, wx[0], wy[0], wz[0], wx[1], wy[1], wz[1], wx[2], wy[2], wz[2], wx[3], wy[3], wz[3], nxn, nyn, nzn, uvrect, flip);

                    // zero out mask for area
                    for (int yy = 0; yy < h; ++yy) {
                        for (int xx = 0; xx < w; ++xx) {
                            mask[(i + xx) + (j + yy) * S] = 0;
                        }
                    }
                }
            }
        }
    }

    if (verts.empty()) return true;

    // Build raylib Mesh as before
    Mesh m;
    memset(&m, 0, sizeof(Mesh));
    m.vertexCount = static_cast<int>(verts.size() / 3);
    m.triangleCount = static_cast<int>(indices.size() / 3);

    m.vertices = (float*)RL_MALLOC(sizeof(float) * verts.size()); memcpy(m.vertices, verts.data(), sizeof(float) * verts.size());
    m.normals = (float*)RL_MALLOC(sizeof(float) * normals.size()); memcpy(m.normals, normals.data(), sizeof(float) * normals.size());
    m.texcoords = (float*)RL_MALLOC(sizeof(float) * uvs.size()); memcpy(m.texcoords, uvs.data(), sizeof(float) * uvs.size());
    m.indices = (unsigned short*)RL_MALLOC(sizeof(unsigned short) * indices.size()); memcpy(m.indices, indices.data(), sizeof(unsigned short) * indices.size());

    UploadMesh(&m, false);

    // Debug dump: log first few vertex/uv/normal entries and OpenGL handles (vao/vbo)
    try {
        int vc = m.vertexCount;
        int toDump = std::min(vc, 16);
        std::string dump = std::string("GreedyMesher: MeshDump vertexCount=") + std::to_string(vc) + " triCount=" + std::to_string(m.triangleCount);
        dump += std::string(" vaoId=") + std::to_string(m.vaoId);
        Log::Info(dump);

        if (m.vertices) {
            std::string pos = "GreedyMesher: positions:";
            for (int i = 0; i < toDump; ++i) {
                pos += " (" + std::to_string(m.vertices[i*3]) + "," + std::to_string(m.vertices[i*3 + 1]) + "," + std::to_string(m.vertices[i*3 + 2]) + ")";
            }
            Log::Info(pos);
        }
        if (m.texcoords) {
            std::string tcs = "GreedyMesher: texcoords:";
            for (int i = 0; i < toDump; ++i) {
                tcs += " (" + std::to_string(m.texcoords[i*2]) + "," + std::to_string(m.texcoords[i*2 + 1]) + ")";
            }
            Log::Info(tcs);
        } else {
            Log::Info("GreedyMesher: Mesh has no texcoords pointer (null)");
        }
        if (m.normals) {
            std::string nms = "GreedyMesher: normals:";
            for (int i = 0; i < toDump; ++i) {
                nms += " (" + std::to_string(m.normals[i*3]) + "," + std::to_string(m.normals[i*3 + 1]) + "," + std::to_string(m.normals[i*3 + 2]) + ")";
            }
            Log::Info(nms);
        }

        if (m.vboId) {
            // vboId is an array pointer; attempt to read first few entries
            std::string vbo = "GreedyMesher: vboIds:";
            // We don't know the count; log first 4 slots safely
            for (int i = 0; i < 4; ++i) {
                unsigned int id = m.vboId[i];
                vbo += " " + std::to_string(id);
            }
            Log::Info(vbo);
        } else {
            Log::Info("GreedyMesher: Mesh has no vboId array (null)");
        }
    } catch (...) {
        Log::Info("GreedyMesher: MeshDump failed (exception)");
    }

    Model model = LoadModelFromMesh(m);
    UnloadMesh(m);

    // Log model mesh/vao/vbo ids to verify attributes are present in the uploaded model
    try {
        int meshCount = model.meshCount;
        Log::Info(std::string("GreedyMesher: Model meshCount=") + std::to_string(meshCount));
        for (int mi = 0; mi < meshCount; ++mi) {
            Mesh &mm = model.meshes[mi];
            std::string mid = std::string("GreedyMesher: model.meshes[") + std::to_string(mi) + "] vao=" + std::to_string(mm.vaoId);
            Log::Info(mid);
            if (mm.vboId) {
                std::string vbo = std::string("GreedyMesher: model.meshes[") + std::to_string(mi) + "] vboIds:";
                for (int k = 0; k < 4; ++k) vbo += " " + std::to_string(mm.vboId[k]);
                Log::Info(vbo);
            }
        }
    } catch (...) {}

    // No atlas/texture binding here. We intentionally avoid assigning any external
    // texture to the model's material so the mesh draws with the default material.
    // This keeps greedy meshing and block geometry intact while deferring any
    // texture/atlas plumbing until later.

    if (chunk.hasModel) UnloadModel(chunk.model);
    chunk.model = model; chunk.hasModel = true;

    Log::Info(std::string("Greedy mesher produced verts=") + std::to_string(m.vertexCount) + " tri=" + std::to_string(m.triangleCount));

    return true;
}

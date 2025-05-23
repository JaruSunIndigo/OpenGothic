#pragma once

#include <zenkit/Mesh.hh>
#include <zenkit/MultiResolutionMesh.hh>
#include <zenkit/SoftSkinMesh.hh>
#include <zenkit/Material.hh>

#include <Tempest/Vec>
#include <utility>

#include "resources.h"

class Bounds;

class PackedMesh {
  public:
    using Vertex        = Resources::Vertex;
    using VertexA       = Resources::VertexA;

    enum {
      MaxVert     = 64,
      MaxPrim     = 64,
      MaxInd      = MaxPrim * 3,
      MaxMeshlets = 16,
      };

    enum PkgType {
      PK_Visual,
      PK_VisualLnd,
      PK_VisualMorph,
      PK_Physic,
      };

    struct SubMesh final {
      zenkit::Material material;
      size_t           iboOffset = 0;
      size_t           iboLength = 0;
      };

    struct Cluster final {
      Tempest::Vec3 pos;
      float         r = 0;
      };

    std::vector<Vertex>   vertices;
    std::vector<VertexA>  verticesA;
    std::vector<uint32_t> indices;
    std::vector<uint8_t>  indices8;

    std::vector<SubMesh>  subMeshes;
    std::vector<Cluster>  meshletBounds;

    std::vector<uint32_t> verticesId; // only for morph meshes
    bool                  isUsingAlphaTest = true;

    PackedMesh(const zenkit::MultiResolutionMesh& mesh, PkgType type);
    PackedMesh(const zenkit::Mesh& mesh, PkgType type);
    PackedMesh(const zenkit::SoftSkinMesh& mesh);

    void debug(std::ostream &out) const;

    std::pair<Tempest::Vec3,Tempest::Vec3> bbox() const;

  private:
    Tempest::Vec3 mBbox[2];

    struct Prim {
      uint32_t primId = 0;
      uint32_t mat    = 0;
      };

    struct SkeletalData {
      Tempest::Vec3 localPositions[4] = {};
      uint8_t       boneIndices[4]    = {};
      float         weights[4]        = {};
      };

    using  Vert = std::pair<uint32_t,uint32_t>;
    struct PrimitiveHeap;
    struct Meshlet {
      Vert          vert   [MaxVert] = {};
      uint8_t       indexes[MaxInd ] = {};
      uint8_t       vertSz           = 0;
      uint8_t       indSz            = 0;
      Cluster       bounds;

      void    flush(std::vector<Vertex>& vertices,
                    std::vector<uint32_t>& indices, std::vector<uint8_t>& indices8,
                    std::vector<Cluster>& instances, const zenkit::Mesh& mesh);

      void    flush(std::vector<Vertex>& vertices, std::vector<VertexA>& verticesA,
                    std::vector<uint32_t>& indices, std::vector<uint8_t>& indices8,
                    std::vector<uint32_t>* verticesId, const std::vector<zenkit::Vec3>& vbo,
                    const std::vector<zenkit::MeshWedge>& wedgeList,
                    const std::vector<SkeletalData>* skeletal);
      bool    validate() const;

      bool    insert(const Vert& a, const Vert& b, const Vert& c);
      void    clear();
      void    updateBounds(const zenkit::Mesh& mesh);
      void    updateBounds(const zenkit::MultiResolutionMesh& mesh);
      void    updateBounds(const std::vector<zenkit::Vec3>& vbo);
      bool    canMerge(const Meshlet& other) const;
      bool    hasIntersection(const Meshlet& other) const;
      float   qDistance(const Meshlet& other) const;
      void    merge(const Meshlet& other);
      };

    bool   addTriangle(Meshlet& dest, const zenkit::Mesh* mesh, const zenkit::SubMesh* proto_mesh, size_t id);

    void   packPhysics(const zenkit::Mesh& mesh, PkgType type);
    void   packMeshletsLnd(const zenkit::Mesh& mesh);
    void   packMeshletsObj(const zenkit::MultiResolutionMesh& mesh, PkgType type,
                           const std::vector<SkeletalData>* skeletal);

    std::vector<Meshlet> buildMeshlets(const zenkit::Mesh* mesh, const zenkit::SubMesh* proto_mesh,
                                       PrimitiveHeap& heap, std::vector<bool>& used);

    void   computeBbox();

    void   dbgUtilization(const std::vector<Meshlet>& meshlets);
    void   dbgMeshlets(const zenkit::Mesh& mesh, const std::vector<Meshlet*>& meshlets);
  };


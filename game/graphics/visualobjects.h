#pragma once

#include <Tempest/Signal>

#include "objectsbucket.h"

class SceneGlobals;
class RtScene;
class Landscape;
class Sky;
class AnimMesh;

class VisualObjects final {
  public:
    VisualObjects(const SceneGlobals& globals, const std::pair<Tempest::Vec3, Tempest::Vec3>& bbox);
    ~VisualObjects();

    ObjectsBucket::Item get(const StaticMesh& mesh, const Material& mat,
                            size_t iboOffset, size_t iboLength, bool staticDraw);
    ObjectsBucket::Item get(const StaticMesh& mesh, const Material& mat,
                            size_t iboOff, size_t iboLen,
                            const Tempest::StorageBuffer& desc,
                            const Bounds& bbox, ObjectsBucket::Type bucket);
    ObjectsBucket::Item get(const AnimMesh& mesh, const Material& mat,
                            size_t iboOff, size_t iboLen,
                            const MatrixStorage::Id& anim);
    ObjectsBucket::Item get(const Material& mat);

    MatrixStorage::Id   getMatrixes(Tempest::BufferHeap heap, size_t boneCnt);
    auto                matrixSsbo (Tempest::BufferHeap heap, uint8_t fId) const -> const Tempest::StorageBuffer&;

    void prepareUniforms();
    void preFrameUpdate (uint8_t fId);
    void visibilityPass (const Frustrum fr[]);

    void drawTranslucent(Tempest::Encoder<Tempest::CommandBuffer>& enc, uint8_t fId);
    void drawWater      (Tempest::Encoder<Tempest::CommandBuffer>& enc, uint8_t fId);
    void drawGBuffer    (Tempest::Encoder<Tempest::CommandBuffer>& enc, uint8_t fId);
    void drawShadow     (Tempest::Encoder<Tempest::CommandBuffer>& enc, uint8_t fId, int layer);
    void drawHiZ        (Tempest::Encoder<Tempest::CommandBuffer>& enc, uint8_t fId);

    void resetIndex();
    void resetTlas();
    void recycle(Tempest::DescriptorSet&& del);

    bool updateRtScene(RtScene& out, const Landscape& land);

  private:
    ObjectsBucket& getBucket(ObjectsBucket::Type type, const Material& mat,
                             const StaticMesh* st, const AnimMesh* anim, const Tempest::StorageBuffer* desc);
    void           mkIndex();
    void           commitUbo(uint8_t fId);

    const SceneGlobals&                         globals;
    VisibilityGroup                             visGroup;
    MatrixStorage                               matrix;

    std::vector<std::unique_ptr<ObjectsBucket>> buckets;
    std::vector<ObjectsBucket*>                 index;
    size_t                                      lastSolidBucket = 0;

    std::vector<Tempest::DescriptorSet>         recycled[Resources::MaxFramesInFlight];
    uint8_t                                     recycledId = 0;

    bool                                        needtoInvalidateTlas = false;

  friend class ObjectsBucket;
  friend class ObjectsBucket::Item;
  };


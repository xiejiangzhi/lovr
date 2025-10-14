
#include "joltc.h"

#ifndef JOLT_C_EXT_H_
#define JOLT_C_EXT_H_ 1

typedef struct JPH_SoftBodySharedSettings	JPH_SoftBodySharedSettings;

typedef enum JPH_SoftBodyBendType {
  JPH_SoftBodyBendType_None,
  JPH_SoftBodyBendType_Distance,
  JPH_SoftBodyBendType_Dihedral,

	_JPH_SoftBodyBendType_Count,
	_JPH_SoftBodyBendType_Force32 = 0x7fffffff
} JPH_SoftBodyBendType;

JPH_CAPI JPH_SoftBodySharedSettings* JPH_SoftBodySharedSettings_CreateByVertices(
  const float* vertices, size_t vs_total, // vertices = vs_total * 3
  const float* vertices_inv_mass, float vertex_radius,
  const uint32_t* faces, size_t faces_total, // faces: vs index list, 3 points per face

  // constraints
  const uint32_t* edges, size_t edges_total, // edges: vs index list, 2 points per edge
  const uint32_t* volumes, size_t volumes_total, // volumes: vs index list, 4 points per volums

  JPH_SoftBodyBendType bend_type,
  const float vertex_compliance[5] // Compliance, ShearCompliance, BendCompliance, LRAType, LRAMaxDistMul
);

JPH_CAPI void JPH_SoftBodySharedSettings_Destroy(JPH_SoftBodySharedSettings* settings);

JPH_CAPI void JPH_SoftBodySharedSettings_Destroy(JPH_SoftBodySharedSettings* settings);

JPH_CAPI JPH_SoftBodyCreationSettings* JPH_SoftBodyCreationSettings_CreateBySharedSettings(
  JPH_SoftBodySharedSettings* shared_settings, const JPH_RVec3* pos, JPH_Quat* rot,
  JPH_ObjectLayer numObjectLayers,
  float pressure, bool update_position
);

JPH_CAPI uint32_t JPH_SoftBody_GetNumVertices(const JPH_Body* body);
JPH_CAPI uint32_t JPH_SoftBody_GetNumFaces(const JPH_Body* body);

JPH_CAPI uint32_t JPH_SoftBody_GetVertices(const JPH_Body* body, float* vertices);
JPH_CAPI uint32_t JPH_SoftBody_GetFaces(const JPH_Body* body, uint32_t* faces);

#endif // JOLT_C_EXT_H_

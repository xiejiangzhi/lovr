#include "ext.h"

#include <Jolt/Core/Core.h>

JPH_SUPPRESS_WARNING_PUSH
JPH_SUPPRESS_WARNINGS

#include "Jolt/Jolt.h"
#include "Jolt/RegisterTypes.h"
#include "Jolt/Core/Factory.h"
#include "Jolt/Core/TempAllocator.h"
#include "Jolt/Core/JobSystemThreadPool.h"
#include "Jolt/Physics/PhysicsSettings.h"
#include "Jolt/Physics/PhysicsSystem.h"
#include "Jolt/Physics/Collision/BroadPhase/BroadPhaseLayerInterfaceMask.h"
#include "Jolt/Physics/Collision/BroadPhase/ObjectVsBroadPhaseLayerFilterMask.h"
#include "Jolt/Physics/Collision/ObjectLayerPairFilterMask.h"
#include "Jolt/Physics/Collision/BroadPhase/BroadPhaseLayerInterfaceTable.h"
#include "Jolt/Physics/Collision/BroadPhase/ObjectVsBroadPhaseLayerFilterTable.h"
#include "Jolt/Physics/Collision/ObjectLayerPairFilterTable.h"
#include "Jolt/Physics/Collision/CastResult.h"
#include "Jolt/Physics/Collision/CollidePointResult.h"
#include "Jolt/Physics/Collision/CollideShape.h"
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/CollisionDispatch.h>
#include <Jolt/Physics/Collision/EstimateCollisionResponse.h>
#include <Jolt/Physics/Collision/ShapeCast.h>
#include <Jolt/Physics/Collision/SimShapeFilter.h>
#include "Jolt/Physics/Collision/Shape/PlaneShape.h"
#include "Jolt/Physics/Collision/Shape/BoxShape.h"
#include "Jolt/Physics/Collision/Shape/SphereShape.h"
#include "Jolt/Physics/Collision/Shape/TriangleShape.h"
#include "Jolt/Physics/Collision/Shape/CapsuleShape.h"
#include "Jolt/Physics/Collision/Shape/TaperedCapsuleShape.h"
#include "Jolt/Physics/Collision/Shape/CylinderShape.h"
#include "Jolt/Physics/Collision/Shape/TaperedCylinderShape.h"
#include "Jolt/Physics/Collision/Shape/ConvexHullShape.h"
#include "Jolt/Physics/Collision/Shape/MeshShape.h"
#include "Jolt/Physics/Collision/Shape/HeightFieldShape.h"
#include "Jolt/Physics/Collision/Shape/StaticCompoundShape.h"
#include "Jolt/Physics/Collision/Shape/MutableCompoundShape.h"
#include "Jolt/Physics/Collision/Shape/DecoratedShape.h"
#include "Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h"
#include "Jolt/Physics/Collision/Shape/ScaledShape.h"
#include "Jolt/Physics/Collision/Shape/OffsetCenterOfMassShape.h"
#include "Jolt/Physics/Collision/Shape/EmptyShape.h"
#include "Jolt/Physics/Body/BodyCreationSettings.h"
#include "Jolt/Physics/Body/BodyActivationListener.h"
#include "Jolt/Physics/SoftBody/SoftBodySharedSettings.h"
#include "Jolt/Physics/SoftBody/SoftBodyCreationSettings.h"
#include "Jolt/Physics/SoftBody/SoftBodyMotionProperties.h"
#include "Jolt/Physics/Collision/RayCast.h"
#include "Jolt/Physics/Collision/BroadPhase/BroadPhaseQuery.h"
#include "Jolt/Physics/Collision/NarrowPhaseQuery.h"
#include "Jolt/Physics/Constraints/SpringSettings.h"
#include "Jolt/Physics/Constraints/FixedConstraint.h"
#include "Jolt/Physics/Constraints/PointConstraint.h"
#include "Jolt/Physics/Constraints/DistanceConstraint.h"
#include "Jolt/Physics/Constraints/HingeConstraint.h"
#include "Jolt/Physics/Constraints/SliderConstraint.h"
#include "Jolt/Physics/Constraints/ConeConstraint.h"
#include "Jolt/Physics/Constraints/SwingTwistConstraint.h"
#include "Jolt/Physics/Constraints/SixDOFConstraint.h"
#include "Jolt/Physics/Constraints/GearConstraint.h"
#include "Jolt/Physics/Constraints/PulleyConstraint.h"
#include "Jolt/Physics/Character/Character.h"
#include "Jolt/Physics/Character/CharacterVirtual.h"
#include "Jolt/Physics/Collision/PhysicsMaterialSimple.h"
#include "Jolt/Physics/Collision/GroupFilterTable.h"
#include "Jolt/Physics/Body/BodyLockMulti.h"
#include "Jolt/Physics/Ragdoll/Ragdoll.h"
#include "Jolt/Physics/Vehicle/WheeledVehicleController.h"
#include "Jolt/Physics/Vehicle/MotorcycleController.h"
#include "Jolt/Physics/Vehicle/TrackedVehicleController.h"

// All Jolt symbols are in the JPH namespace
using namespace JPH;

#define DEF_MAP_DECL(JoltType, c_type)        \
    static inline const JPH::JoltType& As##JoltType(const c_type& t) {    \
        return reinterpret_cast<const JPH::JoltType&>(t);              \
    }                                                               \
    static inline const JPH::JoltType* As##JoltType(const c_type* t) {    \
        return reinterpret_cast<const JPH::JoltType*>(t);              \
    }                                                               \
    static inline JPH::JoltType& As##JoltType(c_type& t) {                \
        return reinterpret_cast<JPH::JoltType&>(t);                    \
    }                                                               \
    static inline JPH::JoltType* As##JoltType(c_type* t) {                \
        return reinterpret_cast<JPH::JoltType*>(t);                    \
    }                                                               \
    static inline const c_type& To##JoltType(const JPH::JoltType& t) {    \
        return reinterpret_cast<const c_type&>(t);                 \
    }                                                               \
    static inline const c_type* To##JoltType(const JPH::JoltType* t) {    \
        return reinterpret_cast<const c_type*>(t);                 \
    }                                                               \
    static inline c_type& To##JoltType(JPH::JoltType& t) {                \
        return reinterpret_cast<c_type&>(t);                       \
    }                                                               \
    static inline c_type* To##JoltType(JPH::JoltType* t) {                \
        return reinterpret_cast<c_type*>(t);                       \
    }

DEF_MAP_DECL(SoftBodySharedSettings, JPH_SoftBodySharedSettings)
DEF_MAP_DECL(SoftBodyCreationSettings, JPH_SoftBodyCreationSettings)
DEF_MAP_DECL(Body, JPH_Body)

JPH_SoftBodySharedSettings* JPH_SoftBodySharedSettings_CreateByVertices(
  const float* vertices, size_t vs_total, // vertices = vs_total * 3
  const float* vertices_inv_mass, // vs_total
  const uint32_t* faces, size_t faces_total, // faces: vs index list, 3 points per face

  // constraints
  const uint32_t* edges, size_t edges_total, // edges: vs index list, 2 points per edge
  const uint32_t* volumes, size_t volumes_total, // edges: vs index list, 2 points per edge

  JPH_SoftBodyBendType bend_type,
  const float vertex_compliance[3] // Compliance, ShearCompliance, BendCompliance
) {
	// Create settings
	JPH::SoftBodySharedSettings *settings = new JPH::SoftBodySharedSettings;

  if (vertices && vs_total > 0) {
    for (size_t i = 0; i < vs_total; ++i) {
      JPH::SoftBodySharedSettings::Vertex v;
      size_t idx = i * 3;
      Vec3(vertices[idx], vertices[idx + 1], vertices[idx + 2]).StoreFloat3(&v.mPosition);
      v.mInvMass = vertices_inv_mass[i];
      settings->mVertices.push_back(v);
    }
  } else {
    return NULL;
  }

  if (faces && faces_total > 0) {
    for (size_t i = 0; i < faces_total; ++i) {
      JPH::SoftBodySharedSettings::Face f;
      size_t idx = i * 3;
      f.mVertex[0] = faces[idx];
      f.mVertex[1] = faces[idx + 1];
      f.mVertex[2] = faces[idx + 2];
      settings->AddFace(f);
    }
  }

  if (edges && edges_total > 0) {
    for (size_t i = 0; i < edges_total; ++i) {
      JPH::SoftBodySharedSettings::Edge e;
      size_t idx = i * 2;
      e.mVertex[0] = edges[idx];
      e.mVertex[1] = edges[idx + 1];
      settings->mEdgeConstraints.push_back(e);
    }
    settings->CalculateEdgeLengths();
  }

  if (volumes && volumes_total > 0) {
    for (size_t i = 0; i < volumes_total; ++i) {
      JPH::SoftBodySharedSettings::Volume v;
      size_t idx = i * 4;
      v.mVertex[0] = volumes[idx];
      v.mVertex[1] = volumes[idx + 1];
      v.mVertex[2] = volumes[idx + 2];
      v.mVertex[3] = volumes[idx + 3];
      settings->mVolumeConstraints.push_back(v);
    }
	  settings->CalculateVolumeConstraintVolumes();
  }

  const SoftBodySharedSettings::VertexAttributes &inVertexAttributes = {
    vertex_compliance[0], vertex_compliance[1], vertex_compliance[2]
  };

	// Create constraints
	settings->CreateConstraints(&inVertexAttributes, 1, (JPH::SoftBodySharedSettings::EBendType)bend_type);

	// Optimize the settings
	settings->Optimize();

	return ToSoftBodySharedSettings(settings);
}

void JPH_SoftBodySharedSettings_Destroy(JPH_SoftBodySharedSettings* settings) {
  if (settings) {
		delete AsSoftBodySharedSettings(settings);
	}
}

JPH_SoftBodyCreationSettings* JPH_SoftBodyCreationSettings_CreateBySharedSettings(
  JPH_SoftBodySharedSettings* shared_settings, const JPH_RVec3* pos, JPH_Quat* rot,
  JPH_ObjectLayer numObjectLayers, float pressure, int32_t update_position
) {
  JPH::SoftBodyCreationSettings *creation_settings = new JPH::SoftBodyCreationSettings(
    AsSoftBodySharedSettings(shared_settings),
    JPH::Vec3(pos->x, pos->y, pos->z),
    rot != nullptr ? JPH::Quat(rot->x, rot->y, rot->z, rot->w) : JPH::Quat::sIdentity(),
    numObjectLayers
  );
  creation_settings->mPressure = pressure;
  creation_settings->mUpdatePosition = update_position ? true : false;
  return ToSoftBodyCreationSettings(creation_settings);
}

uint32_t JPH_SoftBody_GetNumVertices(const JPH_Body* body) {
  if (!AsBody(body)->IsSoftBody()) { return 0; }
  JPH::SoftBodyMotionProperties *p = (JPH::SoftBodyMotionProperties*)(AsBody(body)->GetMotionProperties());
  return p->GetVertices().size();
}
uint32_t JPH_SoftBody_GetNumFaces(const JPH_Body* body) {
  if (!AsBody(body)->IsSoftBody()) { return 0; }
  JPH::SoftBodyMotionProperties *p = (JPH::SoftBodyMotionProperties*)(AsBody(body)->GetMotionProperties());
	return p->GetFaces().size();
}

uint32_t JPH_SoftBody_GetVertices(const JPH_Body* body, float* outVertices) {
  if (!AsBody(body)->IsSoftBody()) { return 0; }

  JPH::SoftBodyMotionProperties *p = (JPH::SoftBodyMotionProperties*)(AsBody(body)->GetMotionProperties());
  auto &vertices = p->GetVertices();
  for (size_t i = 0; i < vertices.size(); i++) {
    size_t vi = i * 3;
    outVertices[vi] = vertices[i].mPosition.GetX();
    outVertices[vi + 1] = vertices[i].mPosition.GetY();
    outVertices[vi + 2] = vertices[i].mPosition.GetZ();
  }
  return vertices.size();
}

uint32_t JPH_SoftBody_GetFaces(const JPH_Body* body, uint32_t* outFaces) {
  if (!AsBody(body)->IsSoftBody()) { return 0; }
  JPH::SoftBodyMotionProperties *p = (JPH::SoftBodyMotionProperties*)(AsBody(body)->GetMotionProperties());
  auto &faces = p->GetFaces();
  for (size_t i = 0; i < faces.size(); i++) {
    size_t fi = i * 3;
    outFaces[fi] = faces[i].mVertex[0];
    outFaces[fi + 1] = faces[i].mVertex[1];
    outFaces[fi + 2] = faces[i].mVertex[2];
  }
  return faces.size();
}
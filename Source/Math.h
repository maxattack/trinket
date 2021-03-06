// Trinket Game Engine
// (C) 2020 Max Kaufmann <max.kaufmann@gmail.com>

#pragma once
#include "Common.h"
#include <foundation/PxTransform.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/transform.hpp>
#include <assimp/vector2.h>
#include <assimp/vector3.h>
#include <assimp/matrix4x4.h>

#define RPOSE_IDENTITY  (RPose(ForceInit::Default))
#define HPOSE_IDENTITY (HPose(ForceInit::Default))
#define RELATIVE_POSE_IDENTITY (RelativePose(ForceInit::Default))

// TODO: move all these conversions to a dedicated file?
inline const physx::PxVec3& ToPX(const vec3& v) { return reinterpret_cast<const physx::PxVec3&>(v); }
inline const physx::PxQuat& ToPX(const quat& q) { return reinterpret_cast<const physx::PxQuat&>(q); }
inline const aiVector2D& ToAI(const vec2& v) { return reinterpret_cast<const aiVector2D&>(v); }
inline const aiVector3D& ToAI(const vec3& v) { return reinterpret_cast<const aiVector3D&>(v); }

inline const vec3& FromPX(const physx::PxVec3& v) { return reinterpret_cast<const vec3&>(v); }
inline const quat& FromPX(const physx::PxQuat& q) { return reinterpret_cast<const quat&>(q); }
inline const vec2& FromAI(const aiVector2D& v) { return reinterpret_cast<const vec2&>(v); }
inline const vec3& FromAI(const aiVector3D& v) { return reinterpret_cast<const vec3&>(v); }
inline const mat4 FromAI(const aiMatrix4x4& m) { return glm::transpose(reinterpret_cast<const mat4&>(m)); }

struct RPose {
	
	// rigid poses simplify things by omitting scale

	quat rotation;
	vec3 position;

	RPose() noexcept = default;
	RPose(const RPose&) noexcept = default;
	RPose(RPose&&) noexcept = default;
	RPose& operator=(const RPose&) noexcept = default;

	explicit RPose(ForceInit) noexcept : rotation(1.f, 0.f, 0.f, 0.f), position(0.f, 0.f, 0.f) {}
	explicit RPose(vec3 aPosition) noexcept : rotation(1.f, 0.f, 0.f, 0.f), position(aPosition) {}
	explicit RPose(quat aRotation) noexcept : rotation(aRotation), position(0.f, 0.f, 0.f) {}
	explicit RPose(quat aRotation, vec3 aPosition) noexcept : rotation(aRotation), position(aPosition) {}
	explicit RPose(const physx::PxTransform& px) noexcept : rotation(FromPX(px.q)), position(FromPX(px.p)) {}

	mat4 ToMatrix() const { 
		//return glm::translate(position) * mat4(rotation); 
		const mat3 result(rotation);
		return mat4(
			vec4(result[0], 0.f),
			vec4(result[1], 0.f),
			vec4(result[2], 0.f),
			vec4(position, 1.f)
		);
	}
	physx::PxTransform ToPhysX() const { return physx::PxTransform(ToPX(position), ToPX(rotation)); }

	RPose operator*(const RPose& rhs) const { return RPose(rotation * rhs.rotation, position + rotation * (rhs.position)); }

	RPose Inverse() const {
		let inverseRotation = glm::inverse(rotation);
		return RPose(inverseRotation, inverseRotation * (-position));
	}

	vec3 TransformPosition(const vec3& localPosition) const { return position + rotation * localPosition; }
	vec3 TransformVector(const vec3& localVector) const { return rotation * localVector; }

	vec3 InvTransformPosition(const vec3& worldPosition) const { return glm::inverse(rotation) * (worldPosition - position); }
	vec3 InvTransformVector(const vec3& worldVector) const { return glm::inverse(rotation) * worldVector; }

	vec3 Right() const { return rotation * vec3(1,0,0); }
	vec3 Up() const { return rotation * vec3(0,1,0); }
	vec3 Forward() const { return rotation * vec3(0,0,1); }

	static RPose NLerp(const RPose& lhs, const RPose& rhs, float t) {
		return RPose(
			glm::normalize(glm::lerp(lhs.rotation, rhs.rotation, t)),
			glm::mix(lhs.position, rhs.position, t)
		);
	}
};

struct HPose {

	// "Hierarchy" Pose Transforms are similar, but not strictly analogous,
	// to affine transforms. In particular, the scales are multiplied 
	// component-wise, without respect to rotation, so skew cannot be
	// introduced.

	union {
		RPose rpose;
		struct {
			quat rotation;
			vec3 position;
		};
	};
	vec3 scale;

	HPose() noexcept = default;
	HPose(const HPose&) noexcept = default;
	HPose(HPose&&) noexcept = default;
	HPose& operator=(const HPose&) noexcept = default;

	explicit HPose(ForceInit) noexcept : rotation(1.f, 0.f, 0.f, 0.f), position(0.f, 0.f, 0.f), scale(1.f, 1.f, 1.f) {}
	explicit HPose(vec3 aPosition) noexcept : rotation(1.f, 0.f, 0.f, 0.f), position(aPosition), scale(1.f, 1.f, 1.f) {}
	explicit HPose(quat aRotation) noexcept : rotation(aRotation), position(0.f, 0.f, 0.f), scale(1.f, 1.f, 1.f) {}
	explicit HPose(quat aRotation, vec3 aPosition) noexcept : rotation(aRotation), position(aPosition), scale(1.f, 1.f, 1.f) {}
	explicit HPose(RPose apose) noexcept : rotation(apose.rotation), position(apose.position), scale(1.f, 1.f, 1.f) {}
	explicit HPose(quat aRotation, vec3 aPosition, vec3 aScale) noexcept : rotation(aRotation), position(aPosition), scale(aScale) {}
	explicit HPose(physx::PxTransform& px) noexcept : HPose(RPose(px)) {}

	mat4 ToMatrix() const { 
		//return glm::translate(position) * mat4(rotation) * glm::scale(scale); 
		const mat3 result(rotation);
		return mat4(
			vec4(scale.x * result[0], 0.f),
			vec4(scale.y * result[1], 0.f),
			vec4(scale.z * result[2], 0.f),
			vec4(position, 1.f)
		);
	}

	physx::PxTransform ToPhysX() const { return rpose.ToPhysX(); }

	HPose operator*(const HPose& rhs) const {
		return HPose(
			rotation * rhs.rotation,
			position + rotation * (scale * rhs.position),
			scale * rhs.scale
		);
	}

	HPose Inverse() const {
		let inverseRotation = glm::inverse(rotation);
		let inverseScale = 1.f / scale;
		return HPose(inverseRotation, inverseRotation * (-position * inverseScale), inverseScale);
	}

	vec3 TransformPosition(const vec3& localPosition) const { return position + rotation * (scale * localPosition); }
	vec3 TransformVector(const vec3& localVector) const { return rotation * (scale * localVector); }

	vec3 InvTransformPosition(const vec3& worldPosition) const { return glm::inverse(rotation) * ((worldPosition - position) / scale); }
	vec3 InvTransformVector(const vec3& worldVector) const { return glm::inverse(rotation) * (worldVector / scale); }

	vec3 Right() const { return rotation * vec3(1, 0, 0); }
	vec3 Up() const { return rotation * vec3(0, 1, 0); }
	vec3 Forward() const { return rotation * vec3(0, 0, 1); }

};

union PoseMask {

	struct {
		uint8 ignorePosition : 1;
		uint8 ignoreRotation : 1;
		uint8 ignoreScale : 1;
	};
	struct {
		uint8 allIgnoreBits : 3;
	};
	uint8 ignoreFlags;

	PoseMask() noexcept = default;
	PoseMask(const PoseMask&) noexcept = default;
	PoseMask(PoseMask&&) noexcept = default;
	PoseMask& operator=(const PoseMask&) = default;

	explicit PoseMask(ForceInit) noexcept : ignoreFlags(0) {}
	explicit PoseMask(uint8 aFlags) noexcept : ignoreFlags(aFlags) {}
	explicit PoseMask(bool aPosition, bool aRotation, bool aScale) noexcept : ignorePosition(aPosition), ignoreRotation(aRotation), ignoreScale(aScale) {}

	RPose Apply(const RPose& pose) const {
		let p = float(ignorePosition);
		let r = float(ignoreRotation);
		return RPose(
			quat(r, 0.f, 0.f, 0.f) + (1.f - r) * pose.rotation,
			(1.f - p) * pose.position
		);
	}

	HPose Apply(const HPose& pose) const { 
		//return HPose(
		//	ignoreRotation ? quat(1, 0, 0, 0) : pose.rotation,
		//	ignorePosition ? vec3(0, 0, 0) : pose.position,
		//	ignoreScale ? vec3(1, 1, 1) : pose.scale
		//);
		let p = float(ignorePosition);
		let r = float(ignoreRotation);
		let s = float(ignoreScale);
		return HPose(
			quat(r, 0.f, 0.f, 0.f) + (1.f - r) * pose.rotation,
			(1.f - p) * pose.position,
			vec3(s, s, s) + (1.f - s) * pose.scale
		);
	}

	HPose Concat(const HPose& lhs, const HPose& rhs) const { 
		return Apply(lhs) * rhs; 
	}
	
	HPose Rebase(const HPose& lhs, const HPose& desiredPose) const {
		// desiredPose = lhs * relative
		// relative = lhs^-1 * world
		return allIgnoreBits == 7 ? desiredPose : Apply(lhs).Inverse() * desiredPose;
		
	}
};

inline bool ContainsNaN(float f) { return glm::isnan(f); }
inline bool ContainsNaN(const vec3& v) { return glm::isnan(v.x) || glm::isnan(v.y) || glm::isnan(v.z); }
inline bool ContainsNaN(const quat& q) { return glm::isnan(q.x) || glm::isnan(q.y) || glm::isnan(q.z) || glm::isnan(q.w); }
inline bool ContainsNaN(const RPose& p) { return ContainsNaN(p.position) || ContainsNaN(p.rotation); }
inline bool ContainsNaN(const HPose& p) { return ContainsNaN(p.rpose) || ContainsNaN(p.scale); }
inline bool IsNormalized(const quat& q) { return glm::epsilonEqual(glm::dot(q, q), 1.f, 0.00001f); }
inline bool IsNormalized(const RPose& p) { return IsNormalized(p.rotation); }
inline bool IsNormalized(const HPose& p) { return IsNormalized(p.rotation); }

// TODO: Splines
#pragma once

#include "Source Entity.h"
#include "IEngineTrace.h"
#include "../../Utility/Signature Handler/signatures.h"
#include "../../Utility/ConsoleLogging.h"

MAKE_SIG(CCollisionProperty_WorldSpaceCenter, "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8D 99 ? ? ? ? E8", CLIENT_DLL, 
	vec*, BaseEntity*)

class CCollisionProperty : public ICollideable_t
{
public:
	// Methods of ICollideable
	virtual I_handle_entity* GetEntityHandle();
	virtual const vec& OBBMinsPreScaled();
	virtual const vec& OBBMaxsPreScaled();
	virtual const vec& OBBMins();
	virtual const vec& OBBMaxs();
	virtual void			WorldSpaceTriggerBounds(vec* pVecWorldMins, vec* pVecWorldMaxs) const;
	virtual bool			TestCollision(const ray_t& ray, unsigned int fContentsMask, trace_t& tr);
	virtual bool			TestHitboxes(const ray_t& ray, unsigned int fContentsMask, trace_t& tr);
	virtual int				GetCollisionModelIndex();
	virtual const model_t* GetCollisionModel();
	virtual const vec& GetCollisionOrigin() const;
	virtual const qangle& GetCollisionAngles() const;
	virtual const matrix3x4_t& CollisionToWorldTransform() const;
	virtual int		GetSolid();
	virtual int				GetSolidFlags() const;
	virtual I_client_unknown*			GetIClientUnknown();
	virtual int				GetCollisionGroup() const;
	virtual void			WorldSpaceSurroundingBounds(vec* pVecMins, vec* pVecMaxs);
	virtual bool			ShouldTouchTrigger(int triggerSolidFlags) const;
	virtual const matrix3x4_t* GetRootParentToWorldTransform() const;

public:
	// Spatial partition management
	void			CreatePartitionHandle();
	void			DestroyPartitionHandle();
	unsigned short	GetPartitionHandle() const;

	// Marks the spatial partition dirty
	void			MarkPartitionHandleDirty();

	// Sets the collision bounds + the size (OBB)
	void			SetCollisionBounds(const vec& mins, const vec& maxs);

	// Rebuilds the scaled bounds from the pre-scaled bounds after a model's scale has changed
	void			RefreshScaledCollisionBounds(void);

	// Sets special trigger bounds. The bloat amount indicates how much bigger the 
	// trigger bounds should be beyond the bounds set in SetCollisionBounds
	// This method will also set the FSOLID flag FSOLID_USE_TRIGGER_BOUNDS
	void			UseTriggerBounds(bool bEnable, float flBloat = 0.0f);

	// Sets the method by which the surrounding collision bounds is set
	// You must pass in values for mins + maxs if you select the USE_SPECIFIED_BOUNDS type. 
	void			SetSurroundingBoundsType(int type, const vec* pMins = NULL, const vec* pMaxs = NULL);

	// Sets the solid type (which type of collision representation)
	void			SetSolid(int val);

	// Methods related to size. The OBB here is measured in CollisionSpace
	// (specified by GetCollisionToWorld)
	const vec& OBBSize() const;

	// Returns a radius (or the square of the radius) of a sphere 
	// *centered at the world space center* bounding the collision representation 
	// of the entity. NOTE: The world space center *may* move when the entity rotates.
	float			BoundingRadius() const;
	float			BoundingRadius2D() const;

	// Returns the center of the OBB in collision space
	const vec& OBBCenter() const;

	// center point of entity measured in world space
	// NOTE: This point *may* move when the entity moves depending on
	// which solid type is being used.
	inline vec* WorldSpaceCenter()
	{
		auto* pEnt = dynamic_cast<BaseEntity*>(this);
		if (pEnt != nullptr)
			return Sig::CCollisionProperty_WorldSpaceCenter(pEnt);
		
		FAIL_LOG("ENT IS NULL");
		return nullptr;
	}

	// Methods related to solid flags
	void			ClearSolidFlags(void);
	void			RemoveSolidFlags(int flags);
	void			AddSolidFlags(int flags);
	bool			IsSolidFlagSet(int flagMask) const;
	void		 	SetSolidFlags(int flags);
	bool			IsSolid() const;

	// Updates the spatial partition
	void			UpdatePartition();

	// Are the bounds defined in entity space?
	bool			IsBoundsDefinedInEntitySpace() const;

	// Transforms a point in OBB space to world space
	const vec& CollisionToWorldSpace(const vec& in, vec* pResult) const;

	// Transforms a point in world space to OBB space
	const vec& WorldToCollisionSpace(const vec& in, vec* pResult) const;

	// Transforms a direction in world space to OBB space
	const vec& WorldDirectionToCollisionSpace(const vec& in, vec* pResult) const;

	// Selects a random point in the bounds given the normalized 0-1 bounds 
	void			RandomPointInBounds(const vec& vecNormalizedMins, const vec& vecNormalizedMaxs, vec* pPoint) const;

	// Is a worldspace point within the bounds of the OBB?
	bool			IsPointInBounds(const vec& vecWorldPt) const;

	// Computes a bounding box in world space surrounding the collision bounds
	void			WorldSpaceAABB(vec* pWorldMins, vec* pWorldMaxs) const;

	// Get the collision space mins directly
	const vec& CollisionSpaceMins(void) const;

	// Get the collision space maxs directly
	const vec& CollisionSpaceMaxs(void) const;

	// Computes a "normalized" point (range 0,0,0 - 1,1,1) in collision space
	// Useful for things like getting a point 75% of the way along z on the OBB, for example
	const vec& NormalizedToCollisionSpace(const vec& in, vec* pResult) const;

	// Computes a "normalized" point (range 0,0,0 - 1,1,1) in world space
	const vec& NormalizedToWorldSpace(const vec& in, vec* pResult) const;

	// Transforms a point in world space to normalized space
	const vec& WorldToNormalizedSpace(const vec& in, vec* pResult) const;

	// Transforms a point in collision space to normalized space
	const vec& CollisionToNormalizedSpace(const vec& in, vec* pResult) const;

	// Computes the nearest point in the OBB to a point specified in world space
	void			CalcNearestPoint(const vec& vecWorldPt, vec* pVecNearestWorldPt) const;

	// Computes the distance from a point in world space to the OBB
	float			CalcDistanceFromPoint(const vec& vecWorldPt) const;

	// Does a rotation make us need to recompute the surrounding box?
	bool			DoesRotationInvalidateSurroundingBox() const;

	// Does VPhysicsUpdate make us need to recompute the surrounding box?
	bool			DoesVPhysicsInvalidateSurroundingBox() const;

	// Marks the entity has having a dirty surrounding box
	void			MarkSurroundingBoundsDirty();

	// Compute the largest dot product of the OBB and the specified direction vec
	float			ComputeSupportMap(const vec& vecDirection) const;

private:
	// Transforms an AABB measured in collision space to a box that surrounds it in world space
	void CollisionAABBToWorldAABB(const vec& entityMins, const vec& entityMaxs, vec* pWorldMins, vec* pWorldMaxs) const;

	// Expand trigger bounds..
	void ComputeVPhysicsSurroundingBox(vec* pVecWorldMins, vec* pVecWorldMaxs);

	// Expand trigger bounds..
	bool ComputeHitboxSurroundingBox(vec* pVecWorldMins, vec* pVecWorldMaxs);
	bool ComputeEntitySpaceHitboxSurroundingBox(vec* pVecWorldMins, vec* pVecWorldMaxs);

	// Computes the surrounding collision bounds based on whatever algorithm we want...
	void ComputeCollisionSurroundingBox(bool bUseVPhysics, vec* pVecWorldMins, vec* pVecWorldMaxs);

	// Computes the surrounding collision bounds from the the OBB (not vphysics)
	void ComputeRotationExpandedBounds(vec* pVecWorldMins, vec* pVecWorldMaxs);

	// Computes the surrounding collision bounds based on whatever algorithm we want...
	void ComputeSurroundingBox(vec* pVecWorldMins, vec* pVecWorldMaxs);

	// Check for untouch
	void CheckForUntouch();

	// Updates the spatial partition
	void UpdateServerPartitionMask();
};
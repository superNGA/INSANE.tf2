#pragma once

// SDK
#include "Basic Structures.h"
#include "ClientClass.h"

// UTILITY
#include "../../Utility/Interface.h"


/*
* MATCH THE INHERITANCE ORDER CAREFULLY ELSE WON'T WORK!
	I_handle_entity ->I_client_unknown		\
					I_client_renderable		 \
											  --> I_client_entity -> CBaseEntity -> (other derived classes are entity specific)
					I_client_networkable	 /
					I_client_thinkable		/
* MATCH THE INHERITANCE ORDER CAREFULLY ELSE WON'T WORK!
*/

//======================= NETVARS =======================

struct RoundStats_t;


// forward declaring classes
class I_client_networkable;
class I_client_renderable;
class I_client_unknown;
class BaseEntity;
class I_client_thinkable;
class ICollideable_t;
class baseWeapon;

// forward declaring structs
struct ray_t;

class I_client_entity_list
{
public:
	// Get IClientNetworkable interface for specified entity
	virtual I_client_networkable* GetClientNetworkable(int entnum) = 0;
	virtual I_client_networkable* GetClientNetworkableFromHandle(CBaseHandle hEnt) = 0;
	virtual I_client_unknown* GetClientUnknownFromHandle(CBaseHandle hEnt) = 0;

	// NOTE: This function is only a convenience wrapper.
	// It returns GetClientNetworkable( entnum )->GetIClientEntity().
	virtual BaseEntity* GetClientEntity(int entnum) = 0;
	virtual BaseEntity* GetClientEntityFromHandle(CBaseHandle hEnt) = 0;

	// Returns number of entities currently in use
	virtual int					NumberOfEntities(bool bIncludeNonNetworkable) = 0;

	// Returns highest index actually used
	virtual int					GetHighestEntityIndex(void) = 0;

	// Sizes entity list to specified size
	virtual void				SetMaxEntities(int maxents) = 0;
	virtual int					GetMaxEntities() = 0;

	BaseEntity* GetClientEntityFromUserID(int userID);
};

MAKE_INTERFACE_VERSION(IClientEntityList, "VClientEntityList003", I_client_entity_list, "client.dll")

class I_handle_entity
{
public:
	virtual ~I_handle_entity() {}
	virtual void SetRefEHandle(const CBaseHandle& handle) = 0;
	virtual int	 GetRefEHandle() const = 0; // Supposed to reutrn CBaseHandle, but 4 bytes is 4 bytes :)
};

class ICollideable_t
{
public:
	// Gets at the entity handle associated with the collideable
	virtual I_handle_entity* GetEntityHandle() = 0;

	// These methods return the bounds of an OBB measured in "collision" space
	// which can be retreived through the CollisionToWorldTransform or
	// GetCollisionOrigin/GetCollisionAngles methods
	virtual const vec& OBBMinsPreScaled() const = 0;
	virtual const vec& OBBMaxsPreScaled() const = 0;
	virtual const vec& OBBMins() const = 0;
	virtual const vec& OBBMaxs() const = 0;

	// Returns the bounds of a world-space box used when the collideable is being traced
	// against as a trigger. It's only valid to call these methods if the solid flags
	// have the FSOLID_USE_TRIGGER_BOUNDS flag set.
	virtual void			WorldSpaceTriggerBounds(vec* pVecWorldMins, vec* pVecWorldMaxs) const = 0;

	// custom collision test
	virtual bool			TestCollision(const ray_t& ray, unsigned int fContentsMask, int64_t& tr) = 0;

	// Perform hitbox test, returns true *if hitboxes were tested at all*!!
	virtual bool			TestHitboxes(const ray_t& ray, unsigned int fContentsMask, int64_t& tr) = 0;

	// Returns the BRUSH model index if this is a brush model. Otherwise, returns -1.
	virtual int				GetCollisionModelIndex() = 0;

	// Return the model, if it's a studio model.
	virtual const model_t* GetCollisionModel() = 0;

	// Get angles and origin.
	virtual const vec& GetCollisionOrigin() const = 0;
	virtual const qangle& GetCollisionAngles() const = 0;
	virtual const matrix3x4_t& CollisionToWorldTransform() const = 0;

	// Return a SOLID_ define.
	virtual int				GetSolid() const = 0;
	virtual int				GetSolidFlags() const = 0;

	// Gets at the containing class...
	virtual I_client_unknown* GetIClientUnknown() = 0;

	// We can filter out collisions based on collision group
	virtual int				GetCollisionGroup() const = 0;

	// Returns a world-aligned box guaranteed to surround *everything* in the collision representation
	// Note that this will surround hitboxes, trigger bounds, physics.
	// It may or may not be a tight-fitting box and its volume may suddenly change
	virtual void			WorldSpaceSurroundingBounds(vec* pVecMins, vec* pVecMaxs) = 0;

	virtual bool			ShouldTouchTrigger(int triggerSolidFlags) const = 0;

	// returns NULL unless this collideable has specified FSOLID_ROOT_PARENT_ALIGNED
	virtual const matrix3x4_t* GetRootParentToWorldTransform() const = 0;
};

class I_client_unknown : public I_handle_entity
{
public:
	virtual ICollideable_t* GetCollideable() const = 0;
	virtual I_client_networkable* GetClientNetworkable() = 0;
	virtual I_client_renderable* GetClientRenderable() = 0;
	virtual BaseEntity* GetI_client_entity() = 0;
	//virtual c_base_entity* GetBaseEntity() = 0;
	virtual BaseEntity* GetBaseEntity() = 0;
	virtual I_client_thinkable* GetClientThinkable() = 0;
};

class I_client_renderable
{
public:
	// Gets at the containing class...
	virtual I_client_unknown* GetI_client_unknown() = 0;

	// Data accessors
	virtual vec&					GetRenderOrigin(void) = 0;
	virtual qangle&					GetRenderAngles(void) = 0;
	virtual bool					ShouldDraw(void) = 0;
	virtual bool					IsTransparent(void) = 0;
	virtual bool					UsesPowerOfTwoFrameBufferTexture() = 0;
	virtual bool					UsesFullFrameBufferTexture() = 0;

	virtual void* GetShadowHandle() const = 0;

	// Used by the leaf system to store its render handle.
	virtual void* RenderHandle() = 0;

	virtual const model_t* GetModel() const = 0;
	virtual int						DrawModel(int flags) = 0;

	// Get the body parameter
	virtual int		GetBody() = 0;

	// Determine alpha and blend amount for transparent objects based on render state info
	virtual void	ComputeFxBlend() = 0;
	virtual int		GetFxBlend(void) = 0;

	// Determine the color modulation amount
	virtual void	GetColorModulation(float* color) = 0;

	// Returns false if the entity shouldn't be drawn due to LOD. 
	// (NOTE: This is no longer used/supported, but kept in the vtable for backwards compat)
	virtual bool	LODTest() = 0;

	// Call this to get the current bone transforms for the model.
	// currentTime parameter will affect interpolation
	// nMaxBones specifies how many matrices pBoneToWorldOut can hold. (Should be greater than or
	// equal to studiohdr_t::numbones. Use MAXSTUDIOBONES to be safe.)
	virtual bool	SetupBones(matrix3x4_t* pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime) = 0;

	virtual void	SetupWeights(const void* pBoneToWorld, int nFlexWeightCount, float* pFlexWeights, float* pFlexDelayedWeights) = 0;
	virtual void	DoAnimationEvents(void) = 0;

	// Return this if you want PVS notifications. See IPVSNotify for more info.	
	// Note: you must always return the same value from this function. If you don't,
	// undefined things will occur, and they won't be good.
	virtual void* GetPVSNotifyInterface() = 0;

	// Returns the bounds relative to the origin (render bounds)
	virtual void	GetRenderBounds(vec& mins, vec& maxs) = 0;

	// returns the bounds as an AABB in worldspace
	virtual void	GetRenderBoundsWorldspace(vec& mins, vec& maxs) = 0;

	// These normally call through to GetRenderAngles/GetRenderBounds, but some entities custom implement them.
	virtual void	GetShadowRenderBounds(vec& mins, vec& maxs, ShadowType_t shadowType) = 0;

	// Should this object be able to have shadows cast onto it?
	virtual bool	ShouldReceiveProjectedTextures(int flags) = 0;

	// These methods return true if we want a per-renderable shadow cast direction + distance
	virtual bool	GetShadowCastDistance(float* pDist, ShadowType_t shadowType) const = 0;
	virtual bool	GetShadowCastDirection(vec* pDirection, ShadowType_t shadowType) const = 0;

	// Other methods related to shadow rendering
	virtual bool	IsShadowDirty() = 0;
	virtual void	MarkShadowDirty(bool bDirty) = 0;

	// Iteration over shadow hierarchy
	virtual I_client_renderable* GetShadowParent() = 0;
	virtual I_client_renderable* FirstShadowChild() = 0;
	virtual I_client_renderable* NextShadowPeer() = 0;

	// Returns the shadow cast type
	virtual ShadowType_t ShadowCastType() = 0;

	// Create/get/destroy model instance
	virtual void CreateModelInstance() = 0;
	virtual void* GetModelInstance() = 0;

	// Returns the transform from RenderOrigin/RenderAngles to world
	virtual const matrix3x4_t& RenderableToWorldTransform() = 0;

	// Attachments
	virtual int LookupAttachment(const char* pAttachmentName) = 0;
	virtual	bool GetAttachment(int number, vec& origin, qangle& angles) = 0;
	virtual bool GetAttachment(int number, matrix3x4_t& matrix) = 0;

	// Rendering clip plane, should be 4 floats, return value of NULL indicates a disabled render clip plane
	virtual float* GetRenderClipPlane(void) = 0;

	// Get the skin parameter
	virtual int		GetSkin() = 0;

	// Is this a two-pass renderable?
	virtual bool	IsTwoPass(void) = 0;

	virtual void	OnThreadedDrawSetup() = 0;

	virtual bool	UsesFlexDelayedWeights() = 0;

	virtual void	RecordToolMessage() = 0;

	virtual bool	IgnoresZBuffer(void) const = 0;
};

class I_client_networkable
{
public:
	// Gets at the containing class...
	virtual I_client_unknown* GetI_client_unknown() = 0;

	// Called by the engine when the server deletes the entity.
	virtual void			Release() = 0;

	// Supplied automatically by the IMPLEMENT_CLIENTCLASS macros.
	virtual ClientClass* GetClientClass() = 0;

	// This tells the entity what the server says for ShouldTransmit on this entity.
	// Note: This used to be EntityEnteredPVS/EntityRemainedInPVS/EntityLeftPVS.
	virtual void			NotifyShouldTransmit(ShouldTransmitState_t state) = 0;

	virtual void			OnPreDataChanged(DataUpdateType_t updateType) = 0;
	virtual void			OnDataChanged(DataUpdateType_t updateType) = 0;

	// Called when data is being updated across the network.
	// Only low-level entities should need to know about these.
	virtual void			PreDataUpdate(DataUpdateType_t updateType) = 0;
	virtual void			PostDataUpdate(DataUpdateType_t updateType) = 0;


	// Objects become dormant on the client if they leave the PVS on the server.
	virtual bool			IsDormant(void) = 0;

	// Ent Index is the server handle used to reference this entity.
	// If the index is < 0, that indicates the entity is not known to the server
	virtual int				entindex(void) const = 0;

	// Server to client entity message received
	virtual void			ReceiveMessage(int classID, bf_read& msg) = 0;

	// Get the base pointer to the networked data that GetClientClass->m_pRecvTable starts at.
	// (This is usually just the "this" pointer).
	virtual void* GetDataTableBasePtr() = 0;

	// Tells the entity that it's about to be destroyed due to the client receiving
	// an uncompressed update that's caused it to destroy all entities & recreate them.
	virtual void			SetDestroyedOnRecreateEntities(void) = 0;

	virtual void			OnDataUnchangedInPVS() = 0;
};

class I_client_thinkable
{
public:
	// Gets at the containing class...
	virtual I_client_unknown* GetI_client_unknown() = 0;

	virtual void				ClientThink() = 0;

	// Called when you're added to the think list.
	// GetThinkHandle's return value must be initialized to INVALID_THINK_HANDLE.
	virtual ClientThinkHandle_t	GetThinkHandle() = 0;
	virtual void				SetThinkHandle(ClientThinkHandle_t hThink) = 0;

	// Called by the client when it deletes the entity.
	virtual void				Release() = 0;
};
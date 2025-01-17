#pragma once
#include "Basic Structures.h"

/*
* MATCH THE INHERITANCE ORDER CAREFULLY ELSE WON'T WORK!
	I_handle_entity ->I_client_unknown		\
					I_client_renderable		 \
											  --> I_client_entity -> CBaseEntity -> (other derived classes are entity specific)
					I_client_networkable	 /
					I_client_thinkable		/
* MATCH THE INHERITANCE ORDER CAREFULLY ELSE WON'T WORK!
*/

class I_client_networkable;
class I_client_renderable;
class I_client_unknown;
class I_client_entity;
class I_client_thinkable;
class c_base_entity;
class c_base_animating;

class I_client_entity_list
{
public:
	// Get IClientNetworkable interface for specified entity
	virtual I_client_networkable* GetClientNetworkable(int entnum) = 0;
	virtual I_client_networkable* GetClientNetworkableFromHandle(CBaseHandle hEnt) = 0;
	virtual I_client_unknown* GetClientUnknownFromHandle(CBaseHandle hEnt) = 0;

	// NOTE: This function is only a convenience wrapper.
	// It returns GetClientNetworkable( entnum )->GetIClientEntity().
	virtual I_client_entity* GetClientEntity(int entnum) = 0;
	virtual I_client_entity* GetClientEntityFromHandle(CBaseHandle hEnt) = 0;

	// Returns number of entities currently in use
	virtual int					NumberOfEntities(bool bIncludeNonNetworkable) = 0;

	// Returns highest index actually used
	virtual int					GetHighestEntityIndex(void) = 0;

	// Sizes entity list to specified size
	virtual void				SetMaxEntities(int maxents) = 0;
	virtual int					GetMaxEntities() = 0;
};

class I_handle_entity
{
public:
	virtual ~I_handle_entity() {}
	virtual void SetRefEHandle(const CBaseHandle& handle) = 0;
	virtual const CBaseHandle& GetRefEHandle() const = 0;
};

class I_client_unknown : public I_handle_entity
{
public:
	virtual ICollideable* GetCollideable() = 0;
	virtual I_client_networkable* GetClientNetworkable() = 0;
	virtual I_client_renderable* GetClientRenderable() = 0;
	virtual I_client_entity* GetI_client_entity() = 0;
	//virtual c_base_entity* GetBaseEntity() = 0;
	virtual c_base_entity* GetBaseEntity() = 0;
	virtual I_client_thinkable* GetClientThinkable() = 0;
};

class I_client_renderable
{
public:
	// Gets at the containing class...
	virtual I_client_unknown* GetI_client_unknown() = 0;

	// Data accessors
	virtual vec						const& GetRenderOrigin(void) = 0;
	virtual qangle					const& GetRenderAngles(void) = 0;
	virtual bool					ShouldDraw(void) = 0;
	virtual bool					IsTransparent(void) = 0;
	virtual bool					UsesPowerOfTwoFrameBufferTexture() = 0;
	virtual bool					UsesFullFrameBufferTexture() = 0;

	virtual void* GetShadowHandle() const = 0;

	// Used by the leaf system to store its render handle.
	virtual void* RenderHandle() = 0;

	// Render baby!
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
	virtual void* GetClientClass() = 0;

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

class I_client_entity : public I_client_unknown, public I_client_renderable, public I_client_networkable, public I_client_thinkable
{
public:
	// Delete yourself.
	virtual void			Release(void) = 0;

	// Network origin + angles
	virtual const vec& GetAbsOrigin(void) const = 0;
	virtual const qangle& GetAbsAngles(void) const = 0;

	virtual CMouthInfo* GetMouth(void) = 0;

	// Retrieve sound spatialization info for the specified sound on this entity
	// Return false to indicate sound is not audible
	virtual bool			GetSoundSpatialization(SpatializationInfo_t& info) = 0;

	int get_active_weapon_handle();
};

class c_base_entity : public I_client_entity
{
public:
	/* no need to include all those bullshit virtual functions, just make whatever you need locally and call function via index if needed*/
};
//class c_base_entity : public I_client_entity
//{
//public:
//	virtual	void FireBullets();
//	virtual	void ModifyFireBulletsDamage();
//	virtual	bool ShouldDrawUnderwaterBulletBubbles();
//	virtual	bool ShouldDrawWaterImpacts();
//	virtual	bool HandleShotImpactingWater();	//5
//	virtual	ITraceFilter* DispatchTraceAttack();
//	virtual	void TraceAttack();
//	virtual	void DoImpactEffect();
//	virtual	void MakeTracer();
//	virtual	int GetTracerAttachment();			//10
//	virtual	int BloodColor();
//	virtual	const char* GetTracerType();
//	virtual	void Spawn();
//	virtual	void SpawnClientEntity();
//	virtual	void Precache();					//15
//	virtual	void Activate();
//	virtual	void ParseMapData();
//	virtual	bool KeyValue();
//	virtual	bool KeyValue2();
//	virtual	bool KeyValue3(); //20
//	virtual	bool GetKeyValue();
//	virtual	bool Init();
//	virtual	c_base_animating* GetBaseAnimating();
//	virtual	void SetClassname();
//	virtual	void SetRefEHandle(); //25
//	virtual	void RecordToolMessage();
//	virtual	void Release();
//	virtual	c_base_entity* GetBaseEntity();
//	virtual	bool IsTransparent();
//	virtual	bool IsTwoPass();
//	virtual	bool UsesPowerOfTwoFrameBufferTexture();
//	virtual	bool UsesFullFrameBufferTexture();
//	virtual	void IgnoresZBuffer();
//	virtual	void DrawModel();
//	virtual	void ComputeFxBlend();
//	virtual	int GetFxBlend();
//	virtual	bool LODTest();
//	virtual	void GetRenderBounds();
//	virtual	void GetRenderBoundsWorldspace();
//	virtual	void GetShadowRenderBounds();
//	virtual	void GetColorModulation();
//	virtual	void OnThreadedDrawSetup();
//	virtual	void TestCollision();
//	virtual	void TestHitboxes();
//	virtual	void GetAttackDamageScale();
//	virtual	void NotifyShouldTransmit();
//	virtual	void PreDataUpdate();
//	virtual	void PostDataUpdate();
//	virtual	void OnDataUnchangedInPVS();
//	virtual	void ValidateModelIndex();
//	virtual	void SetDormant();
//	virtual	bool IsDormant();
//	virtual	void SetDestroyedOnRecreateEntities();
//	virtual	void GetEFlags();
//	virtual	void SetEFlags();
//	virtual	void entindex();
//	virtual	void ReceiveMessage();
//	virtual	void* GetDataTableBasePtr();
//	virtual	void ClientThink();
//	virtual	void SetThinkHandle();
//	virtual	void ShouldSavePhysics();
//	virtual	void OnSave();
//	virtual	void OnRestore();
//	virtual	void ObjectCaps();
//	virtual	void Save();
//	virtual	void Restore();
//	virtual	void CreateVPhysics();
//	virtual	void VPhysicsDestroyObject();
//	virtual	void VPhysicsUpdate();
//	virtual	void VPhysicsGetObjectList();
//	virtual	void VPhysicsIsFlesh();
//	virtual	void SetupBones();
//	virtual	void SetupWeights();
//	virtual	bool UsesFlexDelayedWeights();
//	virtual	void DoAnimationEvents();
//	virtual	void AddEntity();
//	virtual	void CalcOverrideModelIndex();
//	virtual	void ComputeWorldSpaceSurroundingBox();
//	virtual	void GetHealthBarHeightOffset();
//	virtual	void GetSolidFlags();
//	virtual	CMouthInfo* GetMouth();
//	virtual	void GetSoundSpatialization();
//	virtual	void LookupAttachment();
//	virtual	void GetAttachment();
//	virtual	void GetAttachmentVelocity();
//	virtual	void GetTeam();
//	virtual	void GetTeamNumber();
//	virtual	void ChangeTeam();
//	virtual	void GetRenderTeamNumber();
//	virtual	void InSameTeam();
//	virtual	void InLocalTeam();
//	virtual	void IsValidIDTarget();
//	virtual	void GetIDString();
//	virtual	void ModifyEmitSoundParams();
//	virtual	void InitializeAsClientEntity();
//	virtual	void Simulate();
//	virtual	void OnDataChanged();
//	virtual	void OnPreDataChanged();
//	virtual	void GetClientVehicle();
//	virtual	void GetAimEntOrigin();
//	virtual	void GetToolRecordingState();
//	virtual	void CleanupToolRecordingState();
//	virtual	void GetCollideType();
//	virtual	bool ShouldDraw();
//	virtual	void UpdateVisibility();
//	virtual	void IsSelfAnimating();
//	virtual	void OnLatchInterpolatedVariables();
//	virtual	void OnNewModel();
//	virtual	void OnNewParticleEffect();
//	virtual	void ResetLatched();
//	virtual	void Interpolate();
//	virtual	void IsSubModel();
//	virtual	void CreateLightEffects();
//	virtual	void Clear();
//	virtual	void DrawBrushModel();
//	virtual	void GetTextureAnimationStartTime();
//	virtual	void TextureAnimationWrapped();
//	virtual	void SetNextClientThink();
//	virtual	void SetHealth();
//	virtual	void GetHealth();
//	virtual	void GetMaxHealth();
//	virtual	void IsVisibleToTargetID();
//	virtual	void IsHealthBarVisible();
//	virtual	void ShouldReceiveProjectedTextures();
//	virtual	bool IsShadowDirty();
//	virtual	void MarkShadowDirty();
//	virtual	void AddDecal();
//	virtual	void AddColoredDecal();
//	virtual	void IsClientCreated();
//	virtual	void UpdateOnRemove();
//	virtual	void SUB_Remove();
//	virtual	void SetPredictable();
//	virtual	void RestoreData();
//	virtual	void DamageDecal();
//	virtual	void DecalTrace();
//	virtual	void ImpactTrace();
//	virtual	void ShouldPredict();
//	virtual	void Think();
//	virtual	void CanBePoweredUp();
//	virtual	void AttemptToPowerup();
//	virtual	void IsCurrentlyTouching();
//	virtual	void StartTouch();
//	virtual	void Touch();
//	virtual	void EndTouch();
//	virtual	void PhysicsSolidMaskForEntity();
//	virtual	void PhysicsSimulate();
//	virtual	void IsAlive();
//	virtual	void IsPlayer();
//	virtual	void IsBaseCombatCharacter();
//	virtual	void MyCombatCharacterPointer();
//	virtual	void IsNPC();
//	virtual	void IsNextBot();
//	virtual	void IsBaseObject();
//	virtual	void IsBaseCombatWeapon();
//	virtual	void MyCombatWeaponPointer();
//	virtual	void IsCombatItem();
//	virtual	void IsBaseTrain();
//	virtual	void ShouldCollide();
//	virtual	void SetViewOffset();
//	virtual	int GetBody();
//	virtual	int GetSkin();
//	virtual	void ShouldInterpolate();
//	virtual	void GetClientSideFade();
//	virtual	void BoneMergeFastCullBloat();
//	virtual	void OnPredictedEntityRemove();
//	virtual	void GetShadowCastDistance();
//	virtual	void GetShadowCastDirection();
//	virtual	void GetShadowUseOtherEntity();
//	virtual	void SetShadowUseOtherEntity();
//	virtual	void AddRagdollToFadeQueue();
//	virtual	void IsDeflectable();
//	virtual	void GetStudioBody();
//	virtual	void PerformCustomPhysics();
//};
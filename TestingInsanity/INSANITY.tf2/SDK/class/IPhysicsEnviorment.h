#pragma once
#include "IPhysicsObject.h"

// Forward declerations
struct vec;
struct qangle;

// Don't ever change these values, or face all kinds of subtle gameplay changes
const float PHYENV_MAX_VELOCITY				= 2000.0f;
const float PHYENV_MAX_ANGULAR_VELOCITY		= 360.0f * 10.0f;

const float DEFAULT_MIN_FRICTION_MASS	= 10.0f;
const float DEFAULT_MAX_FRICTION_MASS	= 2500.0f;
struct physics_performanceparams_t
{
	int		maxCollisionsPerObjectPerTimestep;		// object will be frozen after this many collisions (visual hitching vs. CPU cost)
	int		maxCollisionChecksPerTimestep;			// objects may penetrate after this many collision checks (can be extended in AdditionalCollisionChecksThisTick)
	float	maxVelocity;							// limit world space linear velocity to this (in / s)
	float	maxAngularVelocity;						// limit world space angular velocity to this (degrees / s)
	float	lookAheadTimeObjectsVsWorld;			// predict collisions this far (seconds) into the future
	float	lookAheadTimeObjectsVsObject;			// predict collisions this far (seconds) into the future
	float	minFrictionMass;						// min mass for friction solves (constrains dynamic range of mass to improve stability)
	float	maxFrictionMass;						// mas mass for friction solves

	void Defaults()
	{
		maxCollisionsPerObjectPerTimestep	= 6;
		maxCollisionChecksPerTimestep		= 250;
		maxVelocity							= PHYENV_MAX_VELOCITY;
		maxAngularVelocity					= PHYENV_MAX_ANGULAR_VELOCITY;
		lookAheadTimeObjectsVsWorld			= 1.0f;
		lookAheadTimeObjectsVsObject		= 0.5f;
		minFrictionMass						= DEFAULT_MIN_FRICTION_MASS;
		maxFrictionMass						= DEFAULT_MAX_FRICTION_MASS;
	}
};

class IPhysicsEnvironment
{
public:
	virtual ~IPhysicsEnvironment(void) {}

	virtual void SetDebugOverlay(void* debugOverlayFactory) = 0;
	virtual void* GetDebugOverlay(void) = 0;

	// gravity is a 3-vec in in/s^2
	virtual void			SetGravity(const vec& gravityvec) = 0;
	virtual void			GetGravity(vec* pGravityvec) const = 0;

	// air density is in kg / m^3 (water is 1000)
	// This controls drag, air that is more dense has more drag.
	virtual void			SetAirDensity(float density) = 0;
	virtual float			GetAirDensity(void) const = 0;

	// object creation
	// create a polygonal object.  pCollisionModel was created by the physics builder DLL in a pre-process.
	virtual IPhysicsObject* CreatePolyObject(const void* pCollisionModel, int materialIndex, const vec& position, const qangle& angles, objectparams_t* pParams) = 0;
	// same as above, but this one cannot move or rotate (infinite mass/inertia)
	virtual IPhysicsObject* CreatePolyObjectStatic(const void* pCollisionModel, int materialIndex, const vec& position, const qangle& angles, int* pParams) = 0;
	// Create a perfectly spherical object
	virtual IPhysicsObject* CreateSphereObject(float radius, int materialIndex, const vec& position, const qangle& angles, int* pParams, bool isStatic) = 0;
	// destroy an object created with CreatePolyObject() or CreatePolyObjectStatic()
	virtual void DestroyObject(void*) = 0;

	// Create a polygonal fluid body out of the specified collision model
	// This object will affect any other objects that collide with the collision model
	virtual void* CreateFluidController(void* pFluidObject, int* pParams) = 0;
	// Destroy an object created with CreateFluidController()
	virtual void DestroyFluidController(void* asas) = 0;

	// Create a simulated spring that connects 2 objects
	virtual void* CreateSpring(void* pObjectStart, void* pObjectEnd, void* pParams) = 0;
	virtual void DestroySpring(void*) = 0;

	// Create a constraint in the space of pReferenceObject which is attached by the constraint to pAttachedObject
	virtual void* CreateRagdollConstraint(void* pReferenceObject, void* pAttachedObject, void* pGroup, const int& ragdoll) = 0;
	virtual void* CreateHingeConstraint(void* pReferenceObject, void* pAttachedObject, void* pGroup, const int& hinge) = 0;
	virtual void* CreateFixedConstraint(void* pReferenceObject, void* pAttachedObject, void* pGroup, const int& fixed) = 0;
	virtual void* CreateSlidingConstraint(void* pReferenceObject, void* pAttachedObject, void* pGroup, const int& sliding) = 0;
	virtual void* CreateBallsocketConstraint(void* pReferenceObject, void* pAttachedObject, void* pGroup, const int& ballsocket) = 0;
	virtual void* CreatePulleyConstraint(void* pReferenceObject, void* pAttachedObject, void* pGroup, const int& pulley) = 0;
	virtual void* CreateLengthConstraint(void* pReferenceObject, void* pAttachedObject, void* pGroup, const int& length) = 0;

	virtual void DestroyConstraint(void*) = 0;

	virtual void* CreateConstraintGroup(const int& groupParams) = 0;
	virtual void DestroyConstraintGroup(void* pGroup) = 0;

	virtual void* CreateShadowController(void* pObject, bool allowTranslation, bool allowRotation) = 0;
	virtual void						DestroyShadowController(void*) = 0;

	virtual void* CreatePlayerController(void* pObject) = 0;
	virtual void						DestroyPlayerController(void*) = 0;

	virtual void* CreateMotionController(void* pHandler) = 0;
	virtual void						DestroyMotionController(void* pController) = 0;

	virtual void* CreateVehicleController(void* pVehicleBodyObject, const int& params, unsigned int nVehicleType, void* pGameTrace) = 0;
	virtual void						DestroyVehicleController(void*) = 0;

	// install a function to filter collisions/penentration
	virtual void			SetCollisionSolver(void* pSolver) = 0;

	// run the simulator for deltaTime seconds
	virtual void			Simulate(float deltaTime) = 0;
	// true if currently running the simulator (i.e. in a callback during physenv->Simulate())
	virtual bool			IsInSimulation() const = 0;

	// Manage the timestep (period) of the simulator.  The main functions are all integrated with
	// this period as dt.
	virtual float			GetSimulationTimestep() const = 0;
	virtual void			SetSimulationTimestep(float timestep) = 0;

	// returns the current simulation clock's value.  This is an absolute time.
	virtual float			GetSimulationTime() const = 0;
	virtual void			ResetSimulationClock() = 0;
	// returns the current simulation clock's value at the next frame.  This is an absolute time.
	virtual float			GetNextFrameTime(void) const = 0;

	// Collision callbacks (game code collision response)
	virtual void			SetCollisionEventHandler(void* pCollisionEvents) = 0;
	virtual void			SetObjectEventHandler(void* pObjectEvents) = 0;
	virtual	void			SetConstraintEventHandler(void* pConstraintEvents) = 0;

	virtual void			SetQuickDelete(bool bQuick) = 0;

	virtual int				GetActiveObjectCount() const = 0;
	virtual void			GetActiveObjects(void** pOutputObjectList) const = 0;
	virtual const void** GetObjectList(int* pOutputObjectCount) const = 0;
	virtual bool			TransferObject(void* pObject, IPhysicsEnvironment* pDestinationEnvironment) = 0;

	virtual void			CleanupDeleteList(void) = 0;
	virtual void			EnableDeleteQueue(bool enable) = 0;

	// Save/Restore methods
	virtual bool			Save(const int& params) = 0;
	virtual void			PreRestore(const int& params) = 0;
	virtual bool			Restore(const int& params) = 0;
	virtual void			PostRestore() = 0;

	// Debugging:
	virtual bool IsCollisionModelUsed(int* pCollide) const = 0;

	// Physics world version of the enginetrace API:
	virtual void TraceRay(const int& ray, unsigned int fMask, int* pTraceFilter, int* pTrace) = 0;
	virtual void SweepCollideable(const int* pCollide, const vec& vecAbsStart, const vec& vecAbsEnd,
		const qangle& vecAngles, unsigned int fMask, int* pTraceFilter, int* pTrace) = 0;

	// performance tuning
	virtual void GetPerformanceSettings(physics_performanceparams_t* pOutput) const = 0;
	virtual void SetPerformanceSettings(const physics_performanceparams_t* pSettings) = 0;

	// perf/cost statistics
	virtual void ReadStats(void* pOutput) = 0;
	virtual void ClearStats() = 0;

	virtual unsigned int	GetObjectSerializeSize(void* pObject) const = 0;
	virtual void			SerializeObjectToBuffer(void* pObject, unsigned char* pBuffer, unsigned int bufferSize) = 0;
	virtual void* UnserializeObjectFromBuffer(void* pGameData, unsigned char* pBuffer, unsigned int bufferSize, bool enableCollisions) = 0;


	virtual void EnableConstraintNotify(bool bEnable) = 0;
	virtual void DebugCheckContacts(void) = 0;

	//char m_bullshit[128] = "EAT SHIT NIGGA";
};
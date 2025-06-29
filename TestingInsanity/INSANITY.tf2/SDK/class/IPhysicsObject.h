#pragma once
struct vec;
struct qangle;
class matrix3x4_t;

struct objectparams_t
{
	void Init()
	{
		massCenterOverride = nullptr;
		pName = nullptr;
		pGameData = nullptr;
		mass = 0.0f;
		inertia = 0.0f;
		damping = 0.0f;
		rotdamping = 0.0f;
		rotInertiaLimit = 0.0f;
		volume = 0.0f;
		dragCoefficient = 0.0f;
		enableCollisions = false;
	}
	vec*		massCenterOverride;
	float		mass;
	float		inertia;
	float		damping;
	float		rotdamping;
	float		rotInertiaLimit;
	const char* pName;				// used only for debugging
	void*		pGameData;
	float		volume;
	float		dragCoefficient;
	bool		enableCollisions;
};

static const objectparams_t g_PhysDefaultObjectParams = {
	nullptr,
		1.0f, //mass
		1.0f, // inertia
		0.1f, // damping
		0.1f, // rotdamping
		0.05f, // rotIntertiaLimit
		"DEFAULT",
		nullptr,// game data
		0.f, // volume (leave 0 if you don't have one or call physcollision->CollideVolume() to compute it)
		1.0f, // drag coefficient
		true,// enable collisions?
};

class IPhysicsObject
{
public:
	virtual ~IPhysicsObject(void) {}

	// returns true if this object is static/unmoveable
	// NOTE: returns false for objects that are not created static, but set EnableMotion(false);
	// Call IsMoveable() to find if the object is static OR has motion disabled
	virtual bool			IsStatic() const = 0;
	virtual bool			IsAsleep() const = 0;
	virtual bool			IsTrigger() const = 0;
	virtual bool			IsFluid() const = 0;		// fluids are special triggers with fluid controllers attached, they return true to IsTrigger() as well!
	virtual bool			IsHinged() const = 0;
	virtual bool			IsCollisionEnabled() const = 0;
	virtual bool			IsGravityEnabled() const = 0;
	virtual bool			IsDragEnabled() const = 0;
	virtual bool			IsMotionEnabled() const = 0;
	virtual bool			IsMoveable() const = 0;	 // legacy: IsMotionEnabled() && !IsStatic()
	virtual bool			IsAttachedToConstraint(bool bExternalOnly) const = 0;
	virtual void			EnableCollisions(bool enable) = 0;
	virtual void			EnableGravity(bool enable) = 0;
	virtual void			EnableDrag(bool enable) = 0;
	virtual void			EnableMotion(bool enable) = 0;
	virtual void			SetGameData(void* pGameData) = 0;
	virtual void*			GetGameData(void) const = 0;
	virtual void			SetGameFlags(unsigned short userFlags) = 0;
	virtual unsigned short	GetGameFlags(void) const = 0;
	virtual void			SetGameIndex(unsigned short gameIndex) = 0;
	virtual unsigned short	GetGameIndex(void) const = 0;
	virtual void			SetCallbackFlags(unsigned short callbackflags) = 0;
	virtual unsigned short	GetCallbackFlags(void) const = 0;
	virtual void			Wake(void) = 0;
	virtual void			Sleep(void) = 0;
	virtual void			RecheckCollisionFilter() = 0;
	virtual void			RecheckContactPoints() = 0;
	virtual void			SetMass(float mass) = 0;
	virtual float			GetMass(void) const = 0;
	virtual float			GetInvMass(void) const = 0;
	virtual vec				GetInertia(void) const = 0;
	virtual vec				GetInvInertia(void) const = 0;
	virtual void			SetInertia(const vec& inertia) = 0;
	virtual void			SetDamping(const float* speed, const float* rot) = 0;
	virtual void			GetDamping(float* speed, float* rot) const = 0;
	virtual void			SetDragCoefficient(float* pDrag, float* pAngularDrag) = 0;
	virtual void			SetBuoyancyRatio(float ratio) = 0;			// Override bouyancy
	virtual int				GetMaterialIndex() const = 0;
	virtual void			SetMaterialIndex(int materialIndex) = 0;
	virtual unsigned int	GetContents() const = 0;
	virtual void			SetContents(unsigned int contents) = 0;
	virtual float			GetSphereRadius() const = 0;
	virtual float			GetEnergy() const = 0;
	virtual vec				GetMassCenterLocalSpace() const = 0;
	virtual void			SetPosition(const vec& worldPosition, const qangle& angles, bool isTeleport) = 0;
	virtual void			SetPositionMatrix(const matrix3x4_t& matrix, bool isTeleport) = 0;
	virtual void			GetPosition(vec* worldPosition, qangle* angles) const = 0;
	virtual void			GetPositionMatrix(matrix3x4_t* positionMatrix) const = 0;
	virtual void			SetVelocity(const vec* velocity, const void* angularVelocity) = 0;
	virtual void			SetVelocityInstantaneous(const vec* velocity, const void* angularVelocity) = 0;
	virtual void			GetVelocity(vec* velocity, void* angularVelocity) const = 0;
	virtual void			AddVelocity(const vec* velocity, const void* angularVelocity) = 0;
	virtual void			GetVelocityAtPoint(const vec& worldPosition, vec* pVelocity) const = 0;
	virtual void			GetImplicitVelocity(vec* velocity, void* angularVelocity) const = 0;
	virtual void			LocalToWorld(vec* worldPosition, const vec& localPosition) const = 0;
	virtual void			WorldToLocal(vec* localPosition, const vec& worldPosition) const = 0;
	virtual void			LocalToWorldvec(vec* worldvec, const vec& localvec) const = 0;
	virtual void			WorldToLocalvec(vec* localvec, const vec& worldvec) const = 0;
	virtual void			ApplyForceCenter(const vec& forcevec) = 0;
	virtual void			ApplyForceOffset(const vec& forcevec, const vec& worldPosition) = 0;
	virtual void			ApplyTorqueCenter(const float& torque) = 0;
	virtual void			CalculateForceOffset(const vec& forcevec, const vec& worldPosition, vec* centerForce, void* centerTorque) const = 0;
	virtual void			CalculateVelocityOffset(const vec& forcevec, const vec& worldPosition, vec* centerVelocity, void* centerAngularVelocity) const = 0;
	virtual float			CalculateLinearDrag(const vec& unitDirection) const = 0;
	virtual float			CalculateAngularDrag(const vec& objectSpaceRotationAxis) const = 0;
	virtual bool			GetContactPoint(vec* contactPoint, IPhysicsObject** contactObject) const = 0;
	virtual void			SetShadow(float maxSpeed, float maxAngularSpeed, bool allowPhysicsMovement, bool allowPhysicsRotation) = 0;
	virtual void			UpdateShadow(const vec& targetPosition, const qangle& targetAngles, bool tempDisableGravity, float timeOffset) = 0;
	virtual int				GetShadowPosition(vec* position, qangle* angles) const = 0;
	virtual void*			GetShadowController(void) const = 0;
	virtual void			RemoveShadowController() = 0;
	virtual float			ComputeShadowControl(const int& params, float secondsToArrival, float dt) = 0;
	virtual const void*		GetCollide(void) const = 0;
	virtual const char*		GetName() const = 0;
	virtual void			BecomeTrigger() = 0;
	virtual void			RemoveTrigger() = 0;
	virtual void			BecomeHinged(int localAxis) = 0;
	virtual void			RemoveHinged() = 0;
	virtual void*			CreateFrictionSnapshot() = 0;
	virtual void			DestroyFrictionSnapshot(void* pSnapshot) = 0;
	virtual void			OutputDebugInfo() const = 0;

	void*			idk1;// = nullptr;
	void*			idk2;// = nullptr;
	void*			idk3;// = nullptr;
	void*			idk4;// = nullptr;

	vec				m_vDragBases;
	vec				m_vAngDragBases;

	bool			m_shadowTempGravityDisable : 5;
	bool			m_hasTouchedDynamic : 1;
	bool			m_asleepSinceCreation : 1;
	bool			m_forceSilentDelete : 1;
	unsigned char	m_sleepState : 2;
	unsigned char	m_hingedAxis : 3;
	unsigned char	m_collideType : 3;
	unsigned short	m_gameIndex;

	unsigned short	m_materialIndex;
	unsigned short	m_activeIndex;

	unsigned short	m_callbacks;
	unsigned short	m_gameFlags;
	unsigned int	m_contentsMask;

	float			m_volume;
	float			m_buoyancyRatio;
	float			m_dragCoefficient;
	float			m_angDragCoefficient;
};
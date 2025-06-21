#pragma once

#define MAX_WEAPON_STRING 80
#define MAX_WEAPON_PREFIX 16
#define MAX_WEAPON_AMMO_NAME 32
#define NUM_SHOOT_SOUND_TYPES 16

enum ProjectileType_t : unsigned int
{
	TF_PROJECTILE_NONE,
	TF_PROJECTILE_BULLET,
	TF_PROJECTILE_ROCKET,
	TF_PROJECTILE_PIPEBOMB,
	TF_PROJECTILE_PIPEBOMB_REMOTE,
	TF_PROJECTILE_SYRINGE,
	TF_PROJECTILE_FLARE,
	TF_PROJECTILE_JAR,
	TF_PROJECTILE_ARROW,
	TF_PROJECTILE_FLAME_ROCKET,
	TF_PROJECTILE_JAR_MILK,
	TF_PROJECTILE_HEALING_BOLT,
	TF_PROJECTILE_ENERGY_BALL,
	TF_PROJECTILE_ENERGY_RING,
	TF_PROJECTILE_PIPEBOMB_PRACTICE,
	TF_PROJECTILE_CLEAVER,
	TF_PROJECTILE_STICKY_BALL,
	TF_PROJECTILE_CANNONBALL,
	TF_PROJECTILE_BUILDING_REPAIR_BOLT,
	TF_PROJECTILE_FESTIVE_ARROW,
	TF_PROJECTILE_THROWABLE,
	TF_PROJECTILE_SPELL,
	TF_PROJECTILE_FESTIVE_JAR,
	TF_PROJECTILE_FESTIVE_HEALING_BOLT,
	TF_PROJECTILE_BREADMONSTER_JARATE,
	TF_PROJECTILE_BREADMONSTER_MADMILK,

	TF_PROJECTILE_GRAPPLINGHOOK,
	TF_PROJECTILE_SENTRY_ROCKET,
	TF_PROJECTILE_BREAD_MONSTER,

	TF_NUM_PROJECTILES
};

class FileWeaponInfo_t
{
public:
	void* pVTable;

	bool bParsedScript;
	bool bLoadedHudElements;

	// SHARED
	char szClassName[MAX_WEAPON_STRING];
	char szPrintName[MAX_WEAPON_STRING]; // Name for showing in HUD, etc.

	char szViewModel[MAX_WEAPON_STRING];       // View model of this weapon
	char szWorldModel[MAX_WEAPON_STRING];      // Model of this weapon seen carried by the player
	char szAnimationPrefix[MAX_WEAPON_PREFIX]; // Prefix of the animations that should be used by the player carrying this weapon
	int  iSlot;                                // inventory slot.
	int  iPosition;                            // position in the inventory slot.
	int  iMaxClip1;                            // max primary clip size (-1 if no clip)
	int  iMaxClip2;                            // max secondary clip size (-1 if no clip)
	int  iDefaultClip1;                        // amount of primary ammo in the gun when it's created
	int  iDefaultClip2;                        // amount of secondary ammo in the gun when it's created
	int  iWeight;                              // this value used to determine this weapon's importance in autoselection.
	int  iRumbleEffect;                        // Which rumble effect to use when fired? (xbox)
	bool bAutoSwitchTo;                        // whether this weapon should be considered for autoswitching to
	bool bAutoSwitchFrom;                      // whether this weapon can be autoswitched away from when picking up another weapon or ammo
	int  iFlags;                               // miscellaneous weapon flags
	char szAmmo1[MAX_WEAPON_AMMO_NAME];        // "primary" ammo type
	char szAmmo2[MAX_WEAPON_AMMO_NAME];        // "secondary" ammo type

	// Sound blocks
	char aShootSounds[NUM_SHOOT_SOUND_TYPES][MAX_WEAPON_STRING];

	int  iAmmoType;
	int  iAmmo2Type;
	bool m_bMeleeWeapon; // Melee weapons can always "fire" regardless of ammo.

	// This tells if the weapon was built right-handed (defaults to true).
	// This helps cl_righthand make the decision about whether to flip the model or not.
	bool m_bBuiltRightHanded;
	bool m_bAllowFlipping; // False to disallow flipping the model, regardless of whether
	// it is built left or right handed.

	// CLIENT DLL
	// Sprite data, read from the data file
	int   iSpriteCount;
	void* iconActive;
	void* iconInactive;
	void* iconAmmo;
	void* iconAmmo2;
	void* iconCrosshair;
	void* iconAutoaim;
	void* iconZoomedCrosshair;
	void* iconZoomedAutoaim;
	void* iconSmall;

	// TF2 specific
	bool bShowUsageHint; // if true, then when you receive the weapon, show a hint about it
};


struct WeaponData_t
{
	int		m_nDamage;
	int		m_nBulletsPerShot;
	float	m_flRange;			// <-- NOTE : This is not the melee range!! just some constant that's stuck at 8192.0f.
	float	m_flSpread;
	float	m_flPunchAngle;
	float	m_flTimeFireDelay;				// Time to delay between firing
	float	m_flTimeIdle;					// Time to idle after firing
	float	m_flTimeIdleEmpty;				// Time to idle after firing last bullet in clip
	float	m_flTimeReloadStart;			// Time to start into a reload (ie. shotgun)
	float	m_flTimeReload;					// Time to reload
	bool	m_bDrawCrosshair;				// Should the weapon draw a crosshair
	ProjectileType_t m_iProjectile;					// The type of projectile this mode fires
	int		m_iAmmoPerShot;					// How much ammo each shot consumes
	float	m_flProjectileSpeed;			// Start speed for projectiles (nail, etc.); NOTE: union with something non-projectile
	float	m_flSmackDelay;					// how long after swing should damage happen for melee weapons
	bool	m_bUseRapidFireCrits;
};


class CTFWeaponInfo : public FileWeaponInfo_t
{
public:
	WeaponData_t* GetWeaponData(int iWeapon = 0) { return &m_WeaponData[iWeapon]; }

	WeaponData_t	m_WeaponData[2];

	int		m_iWeaponType;

	// Grenade.
	bool	m_bGrenade;
	float	m_flDamageRadius;
	float	m_flPrimerTime;
	bool	m_bLowerWeapon;
	bool	m_bSuppressGrenTimer;

	// Skins
	bool	m_bHasTeamSkins_Viewmodel;
	bool	m_bHasTeamSkins_Worldmodel;

	// Muzzle flash
	char	m_szMuzzleFlashModel[128];
	float	m_flMuzzleFlashModelDuration;
	char	m_szMuzzleFlashParticleEffect[128];

	// Tracer
	char	m_szTracerEffect[128];

	// Eject Brass
	bool	m_bDoInstantEjectBrass;
	char	m_szBrassModel[128];

	// Explosion Effect
	char	m_szExplosionSound[128];
	char	m_szExplosionEffect[128];
	char	m_szExplosionPlayerEffect[128];
	char	m_szExplosionWaterEffect[128];

	bool	m_bDontDrop;
};
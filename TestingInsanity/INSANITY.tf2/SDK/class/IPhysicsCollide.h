#pragma once
#include "../../Utility/Interface.h"

// Forward declares
struct vec;
struct qangle;

class IPhysicsCollision
{
public:
	virtual ~IPhysicsCollision(void) {}

	// produce a convex element from verts (convex hull around verts)
	virtual void * ConvexFromVerts(vec * *pVerts, int vertCount) = 0;
	// produce a convex element from planes (csg of planes)
	virtual void* ConvexFromPlanes(float* pPlanes, int planeCount, float mergeDistance) = 0;
	// calculate volume of a convex element
	virtual float			ConvexVolume(void* pConvex) = 0;

	virtual float			ConvexSurfaceArea(void* pConvex) = 0;
	// store game-specific data in a convex solid
	virtual void			SetConvexGameData(void* pConvex, unsigned int gameData) = 0;
	// If not converted, free the convex elements with this call
	virtual void			ConvexFree(void* pConvex) = 0;
	virtual void* BBoxToConvex(const vec& mins, const vec& maxs) = 0;
	// produce a convex element from a convex polyhedron
	virtual void* ConvexFromConvexPolyhedron(const int& ConvexPolyhedron) = 0;
	// produce a set of convex triangles from a convex polygon, normal is assumed to be on the side with forward point ordering, which should be clockwise, output will need to be able to hold exactly (iPointCount-2) convexes
	virtual void			ConvexesFromConvexPolygon(const vec& vPolyNormal, const vec* pPoints, int iPointCount, void** pOutput) = 0;

	// concave objects
	// create a triangle soup
	virtual void* PolysoupCreate(void) = 0;
	// destroy the container and memory
	virtual void			PolysoupDestroy(void* pSoup) = 0;
	// add a triangle to the soup
	virtual void			PolysoupAddTriangle(void* pSoup, const vec& a, const vec& b, const vec& c, int materialIndex7bits) = 0;
	// convert the convex into a compiled collision model
	virtual void* ConvertPolysoupToCollide(void* pSoup, bool useMOPP) = 0;

	// Convert an array of convex elements to a compiled collision model (this deletes the convex elements)
	virtual void* ConvertConvexToCollide(void** pConvex, int convexCount) = 0;
	virtual void* ConvertConvexToCollideParams(void** pConvex, int convexCount, const int& convertParams) = 0;
	// Free a collide that was created with ConvertConvexToCollide()
	virtual void			DestroyCollide(void* pCollide) = 0;

	// Get the memory size in bytes of the collision model for serialization
	virtual int				CollideSize(void* pCollide) = 0;
	// serialize the collide to a block of memory
	virtual int				CollideWrite(char* pDest, void* pCollide, bool bSwap = false) = 0;
	// unserialize the collide from a block of memory
	virtual void* UnserializeCollide(char* pBuffer, int size, int index) = 0;

	// compute the volume of a collide
	virtual float			CollideVolume(void* pCollide) = 0;
	// compute surface area for tools
	virtual float			CollideSurfaceArea(void* pCollide) = 0;

	// Get the support map for a collide in the given direction
	virtual vec			CollideGetExtent(const void* pCollide, const vec& collideOrigin, const qangle& collideAngles, const vec& direction) = 0;

	// Get an AABB for an oriented collision model
	virtual void			CollideGetAABB(vec* pMins, vec* pMaxs, const void* pCollide, const vec& collideOrigin, const qangle& collideAngles) = 0;

	virtual void			CollideGetMassCenter(void* pCollide, vec* pOutMassCenter) = 0;
	virtual void			CollideSetMassCenter(void* pCollide, const vec& massCenter) = 0;
	// get the approximate cross-sectional area projected orthographically on the bbox of the collide
	// NOTE: These are fractional areas - unitless.  Basically this is the fraction of the OBB on each axis that
	// would be visible if the object were rendered orthographically.
	// NOTE: This has been precomputed when the collide was built or this function will return 1,1,1
	virtual vec			CollideGetOrthographicAreas(const void* pCollide) = 0;
	virtual void			CollideSetOrthographicAreas(void* pCollide, const vec& areas) = 0;

	// query the vcollide index in the physics model for the instance
	virtual int				CollideIndex(const void* pCollide) = 0;

	// Convert a bbox to a collide
	virtual void* BBoxToCollide(const vec& mins, const vec& maxs) = 0;
	virtual int				GetConvexesUsedInCollideable(const void* pCollideable, void** pOutputArray, int iOutputArrayLimit) = 0;


	// Trace an AABB against a collide
	virtual void TraceBox(const vec& start, const vec& end, const vec& mins, const vec& maxs, const void* pCollide, const vec& collideOrigin, const qangle& collideAngles, void* ptr) = 0;
	virtual void TraceBox(const int& ray, const void* pCollide, const vec& collideOrigin, const qangle& collideAngles, void* ptr) = 0;
	virtual void TraceBox(const int& ray, unsigned int contentsMask, void* pConvexInfo, const void* pCollide, const vec& collideOrigin, const qangle& collideAngles, void* ptr) = 0;

	// Trace one collide against another
	virtual void TraceCollide(const vec& start, const vec& end, const void* pSweepCollide, const qangle& sweepAngles, const void* pCollide, const vec& collideOrigin, const qangle& collideAngles, void* ptr) = 0;

	// relatively slow test for box vs. truncated cone
	virtual bool			IsBoxIntersectingCone(const vec& boxAbsMins, const vec& boxAbsMaxs, const int& cone) = 0;

	// loads a set of solids into a void
	virtual void			VCollideLoad(void* pOutput, int solidCount, const char* pBuffer, int size, bool swap = false) = 0;
	// destroyts the set of solids created by VCollideLoad
	virtual void			VCollideUnload(void* pVCollide) = 0;

	// begins parsing a vcollide.  NOTE: This keeps pointers to the text
	// If you free the text and call members of IVPhysicsKeyParser, it will crash
	virtual void* VPhysicsKeyParserCreate(const char* pKeyData) = 0;
	// Free the parser created by VPhysicsKeyParserCreate
	virtual void			VPhysicsKeyParserDestroy(void* pParser) = 0;

	// creates a list of verts from a collision mesh
	virtual int				CreateDebugMesh(void const* pCollisionModel, vec** outVerts) = 0;
	// destroy the list of verts created by CreateDebugMesh
	virtual void			DestroyDebugMesh(int vertCount, vec* outVerts) = 0;

	// create a queryable version of the collision model
	virtual void* CreateQueryModel(void* pCollide) = 0;
	// destroy the queryable version
	virtual void			DestroyQueryModel(void* pQuery) = 0;

	virtual IPhysicsCollision* ThreadContextCreate(void) = 0;
	virtual void			ThreadContextDestroy(IPhysicsCollision* pThreadContex) = 0;

	virtual void* CreateVirtualMesh(const int& params) = 0;
	virtual bool			SupportsVirtualMesh() = 0;


	virtual bool			GetBBoxCacheSize(int* pCachedSize, int* pCachedCount) = 0;


	// extracts a polyhedron that defines a void's shape
	virtual int* PolyhedronFromConvex(void* const pConvex, bool bUseTempPolyhedron) = 0;

	// dumps info about the collide to Msg()
	virtual void			OutputDebugInfo(const void* pCollide) = 0;
	virtual unsigned int	ReadStat(int statID) = 0;
};

MAKE_INTERFACE_VERSION(iPhysicsCollision, "VPhysicsCollision007", IPhysicsCollision, VPHYSICS_DLL)
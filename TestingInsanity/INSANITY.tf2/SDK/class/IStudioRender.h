#pragma once
#include "IAppSystem.h"
#include "../../Utility/Interface Handler/Interface.h"
#include "../../Utility/Signature Handler/signatures.h"

class IMaterial;
enum OverrideType_t;

//MAKE_SIG(IStudioRender_ForcedMaterialOverride, "48 89 91 ? ? ? ? 44 89 81", STUDIORENDER_DLL, void, void*, IMaterial*, OverrideType_t)


class IStudioRender : public IAppSystem
{
public:
	virtual void BeginFrame(void) = 0;
	virtual void EndFrame(void) = 0;

	// Used for the mat_stub console command.
	virtual void Mat_Stub(void* pMatSys) = 0;

	// Updates the rendering configuration 
	virtual void UpdateConfig(const void* config) = 0;
	virtual void GetCurrentConfig(void* config) = 0;

	// Load, unload model data
	virtual bool LoadModel(void* pStudioHdr, void* pVtxData, void* pHardwareData) = 0;
	virtual void UnloadModel(void* pHardwareData) = 0;

	// Refresh the studiohdr since it was lost...
	virtual void RefreshStudioHdr(void* pStudioHdr, void* pHardwareData) = 0;

	// This is needed to do eyeglint and calculate the correct texcoords for the eyes.
	virtual void SetEyeViewTarget(const void* pStudioHdr, int nBodyIndex, const vec& worldPosition) = 0;

	// Methods related to lighting state
	// NOTE: SetAmbientLightColors assumes that the arraysize is the same as 
	// returned from GetNumAmbientLightSamples
	virtual int GetNumAmbientLightSamples() = 0;
	virtual const vec* GetAmbientLightDirections() = 0;
	virtual void SetAmbientLightColors(const void* pAmbientOnlyColors) = 0;
	virtual void SetAmbientLightColors(const vec* pAmbientOnlyColors) = 0;
	virtual void SetLocalLights(int numLights, const void* pLights) = 0;

	// Sets information about the camera location + orientation
	virtual void SetViewState(const vec& viewOrigin, const vec& viewRight,
		const vec& viewUp, const vec& viewPlaneNormal) = 0;

	// Allocates flex weights for use in rendering
	// NOTE: Pass in a non-null second parameter to lock delayed flex weights
	virtual void LockFlexWeights(int nWeightCount, float** ppFlexWeights, float** ppFlexDelayedWeights = NULL) = 0;
	virtual void UnlockFlexWeights() = 0;

	// Used to allocate bone matrices to be used to pass into DrawModel
	virtual matrix3x4_t* LockBoneMatrices(int nBoneCount) = 0;
	virtual void UnlockBoneMatrices() = 0;

	// LOD stuff
	virtual int GetNumLODs(const void* hardwareData) const = 0;
	virtual float GetLODSwitchValue(const void* hardwareData, int lod) const = 0;
	virtual void SetLODSwitchValue(void* hardwareData, int lod, float switchValue) = 0;

	// Sets the color/alpha modulation
	virtual void SetColorModulation(float const* pColor) = 0;
	virtual void SetAlphaModulation(float flAlpha) = 0;

	// Draws the model
	virtual void DrawModel(void* pResults, const void* info,
		matrix3x4_t* pBoneToWorld, float* pFlexWeights, float* pFlexDelayedWeights, const vec& modelOrigin, int flags) = 0;

	// Methods related to static prop rendering
	virtual void DrawModelStaticProp(const void* drawInfo, const matrix3x4_t& modelToWorld, int flags) = 0;
	virtual void DrawStaticPropDecals(const void* drawInfo, const matrix3x4_t& modelToWorld) = 0;
	virtual void DrawStaticPropShadows(const void* drawInfo, const matrix3x4_t& modelToWorld, int flags) = 0;

	// Causes a material to be used instead of the materials the model was compiled with
	virtual void ForcedMaterialOverride(IMaterial* newMaterial, int nOverrideType) = 0;

	// Create, destroy list of decals for a particular model
	virtual int CreateDecalList(void* pHardwareData) = 0;
	virtual void DestroyDecalList(int handle) = 0;

	// Add decals to a decal list by doing a planar projection along the ray
	// The BoneToWorld matrices must be set before this is called
	virtual void AddDecal(int handle, void* pStudioHdr, matrix3x4_t* pBoneToWorld,
		const void* ray, const vec& decalUp, IMaterial* pDecalMaterial, float radius, int body, bool noPokethru = false, int maxLODToDecal = 0) = 0;

	// Compute the lighting at a point and normal
	virtual void ComputeLighting(const vec* pAmbient, int lightCount,
		void* pLights, const vec& pt, const vec& normal, vec& lighting) = 0;

	// Compute the lighting at a point, constant directional component is passed
	// as flDirectionalAmount
	virtual void ComputeLightingConstDirectional(const vec* pAmbient, int lightCount,
		void* pLights, const vec& pt, const vec& normal, vec& lighting, float flDirectionalAmount) = 0;

	// Shadow state (affects the models as they are rendered)
	virtual void AddShadow(IMaterial* pMaterial, void* pProxyData, void* m_pFlashlightState = NULL, void* pWorldToTexture = NULL, void* pFlashlightDepthTexture = NULL) = 0;
	virtual void ClearAllShadows() = 0;

	// Gets the model LOD; pass in the screen size in pixels of a sphere 
	// of radius 1 that has the same origin as the model to get the LOD out...
	virtual int ComputeModelLod(void* pHardwareData, float unitSphereSize, float* pMetric = NULL) = 0;

	// Return a number that is usable for budgets, etc.
	// Things that we care about:
	// 1) effective triangle count (factors in batch sizes, state changes, etc)
	// 2) texture memory usage
	// Get Triangles returns the LOD used
	virtual void GetPerfStats(void* pResults, const void* info, void* pSpewBuf = NULL) const = 0;
	virtual void GetTriangles(const void* info, matrix3x4_t* pBoneToWorld, void* out) = 0;

	// Returns materials used by a particular model
	virtual int GetMaterialList(void* pStudioHdr, int count, IMaterial** ppMaterials) = 0;
	virtual int GetMaterialListFromBodyAndSkin(unsigned short studio, int nSkin, int nBody, int nCountOutputMaterials, IMaterial** ppOutputMaterials) = 0;
	// draw an array of models with the same state
	virtual void DrawModelArray(const void* drawInfo, int arrayCount, void* pInstanceData, int instanceStride, int flags) = 0;

	virtual void GetMaterialOverride(IMaterial** ppOutForcedMaterial, void* pOutOverrideType) = 0;


	/*inline void ForcedMaterialOverride(IMaterial* pMat, OverrideType_t iOverrideType)
	{
		return Sig::IStudioRender_ForcedMaterialOverride(this, pMat, iOverrideType);
	}*/
};


MAKE_INTERFACE_VERSION(iStudioRender, "VStudioRender025", IStudioRender, STUDIORENDER_DLL)
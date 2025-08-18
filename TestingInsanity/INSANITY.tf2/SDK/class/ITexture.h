#pragma once

class ITexture
{
public:
	// Various texture polling methods
	virtual const char* GetName(void) const = 0;
	virtual int GetMappingWidth() const = 0;
	virtual int GetMappingHeight() const = 0;
	virtual int GetActualWidth() const = 0;
	virtual int GetActualHeight() const = 0;
	virtual int GetNumAnimationFrames() const = 0;
	virtual bool IsTranslucent() const = 0;
	virtual bool IsMipmapped() const = 0;

	virtual void GetLowResColorSample(float s, float t, float* color) const = 0;

	// Gets texture resource data of the specified type.
	// Params:
	//		eDataType		type of resource to retrieve.
	//		pnumBytes		on return is the number of bytes available in the read-only data buffer or is undefined
	// Returns:
	//		pointer to the resource data, or NULL
	virtual void* GetResourceData(unsigned long eDataType, size_t* pNumBytes) const = 0;

	// Methods associated with reference count
	virtual void IncrementReferenceCount(void) = 0;
	virtual void DecrementReferenceCount(void) = 0;

	inline void AddRef() { IncrementReferenceCount(); }
	inline void Release() { DecrementReferenceCount(); }

	// Used to modify the texture bits (procedural textures only)
	virtual void SetTextureRegenerator(void* pTextureRegen) = 0;

	// Reconstruct the texture bits in HW memory

	// If rect is not specified, reconstruct all bits, otherwise just
	// reconstruct a subrect.
	virtual void Download(void* pRect = 0, int nAdditionalCreationFlags = 0) = 0;

	// Uses for stats. . .get the approximate size of the texture in it's current format.
	virtual int GetApproximateVidMemBytes(void) const = 0;

	// Returns true if the texture data couldn't be loaded.
	virtual bool IsError() const = 0;

	// NOTE: Stuff after this is added after shipping HL2.

	// For volume textures
	virtual bool IsVolumeTexture() const = 0;
	virtual int GetMappingDepth() const = 0;
	virtual int GetActualDepth() const = 0;

	virtual void GetImageFormat() const = 0;
	virtual void GetNormalDecodeMode() const = 0;

	// Various information about the texture
	virtual bool IsRenderTarget() const = 0;
	virtual bool IsCubeMap() const = 0;
	virtual bool IsNormalMap() const = 0;
	virtual bool IsProcedural() const = 0;

	virtual void DeleteIfUnreferenced() = 0;

#if defined( _X360 )
	virtual bool ClearTexture(int r, int g, int b, int a) = 0;
	virtual bool CreateRenderTargetSurface(int width, int height, ImageFormat format, bool bSameAsTexture) = 0;
#endif

	// swap everything except the name with another texture
	virtual void SwapContents(ITexture* pOther) = 0;

	// Retrieve the vtf flags mask
	virtual unsigned int GetFlags(void) const = 0;

	// Force LOD override (automatically downloads the texture)
	virtual void ForceLODOverride(int iNumLodsOverrideUpOrDown) = 0;

	// Save texture to a file.
	virtual bool SaveToFile(const char* fileName) = 0;

	// Copy this texture, which must be a render target or a renderable texture, to the destination texture, 
	// which must have been created with the STAGING bit.
	virtual void CopyToStagingTexture(ITexture* pDstTex) = 0;

	// Set that this texture should return true for the call "IsError"
	virtual void SetErrorTexture(bool bIsErrorTexture) = 0;
};


class ITextureInternal : public ITexture
{
public:

	virtual void Bind(int sampler) = 0;
	virtual void Bind(int sampler1, int nFrame, int sampler2) = 0;

	// Methods associated with reference counting
	virtual int GetReferenceCount() = 0;

	virtual void GetReflectivity(vec& reflectivity) = 0;

	// Set this as the render target, return false for failure
	virtual bool SetRenderTarget(int nRenderTargetID) = 0;

	// Releases the texture's hw memory
	virtual void ReleaseMemory() = 0;

	// Called before Download() on restore. Gives render targets a change to change whether or
	// not they force themselves to have a separate depth buffer due to AA.
	virtual void OnRestore() = 0;

	// Resets the texture's filtering and clamping mode
	virtual void SetFilteringAndClampingMode(bool bOnlyLodValues = false) = 0;

	// Used by tools.... loads up the non-fallback information about the texture 
	virtual void Precache() = 0;

	// Stretch blit the framebuffer into this texture.
	virtual void CopyFrameBufferToMe(int nRenderTargetID = 0, void* pSrcRect = NULL, void* pDstRect = NULL) = 0;
	virtual void CopyMeToFrameBuffer(int nRenderTargetID = 0, void* pSrcRect = NULL, void* pDstRect = NULL) = 0;

	virtual ITexture* GetEmbeddedTexture(int nIndex) = 0;

	// Get the shaderapi texture handle associated w/ a particular frame
	virtual int64_t GetTextureHandle(int nFrame, int nTextureChannel = 0) = 0;
};
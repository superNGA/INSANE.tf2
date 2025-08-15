#pragma once

#include "Basic Structures.h"
#include "../../Utility/Interface Handler/Interface.h"


typedef unsigned short ClientRenderHandle_t;
typedef unsigned short ClientShadowHandle_t;
typedef unsigned short ClientLeafShadowHandle_t;
class I_client_renderable;


class IClientLeafSystem
{
private:
	virtual void PaddingFn_0() const = 0;
	virtual void PaddingFn_1() const = 0;
	virtual void PaddingFn_2() const = 0;
	virtual void PaddingFn_3() const = 0;

public :
	// Adds and removes renderables from the leaf lists
	virtual void AddRenderable(I_client_renderable* pRenderable, renderGroup_t group) = 0;

	// This tells if the renderable is in the current PVS. It assumes you've updated the renderable
	// with RenderableChanged() calls
	virtual bool IsRenderableInPVS(I_client_renderable* pRenderable) = 0;

	virtual void SetSubSystemDataInLeaf(int leaf, int nSubSystemIdx, void* pData) = 0;
	virtual void* GetSubSystemDataInLeaf(int leaf, int nSubSystemIdx) = 0;


	virtual void SetDetailObjectsInLeaf(int leaf, int firstDetailObject, int detailObjectCount) = 0;
	virtual void GetDetailObjectsInLeaf(int leaf, int& firstDetailObject, int& detailObjectCount) = 0;

	// Indicates which leaves detail objects should be rendered from, returns the detais objects in the leaf
	virtual void DrawDetailObjectsInLeaf(int leaf, int frameNumber, int& firstDetailObject, int& detailObjectCount) = 0;

	// Should we draw detail objects (sprites or models) in this leaf (because it's close enough to the view)
	// *and* are there any objects in the leaf?
	virtual bool ShouldDrawDetailObjectsInLeaf(int leaf, int frameNumber) = 0;

	// Call this when a renderable origin/angles/bbox parameters has changed
	virtual void RenderableChanged(ClientRenderHandle_t handle) = 0;

	// Set a render group
	virtual void SetRenderGroup(ClientRenderHandle_t handle, renderGroup_t group) = 0;

	// Computes which leaf translucent objects should be rendered in
	virtual void ComputeTranslucentRenderLeaf(int count, const void* pLeafList, const void* pLeafFogVolumeList, int frameNumber, int viewID) = 0;

	// Put renderables into their appropriate lists.
	virtual void BuildRenderablesList(const void* info) = 0;

	// Put renderables in the leaf into their appropriate lists.
	virtual void CollateViewModelRenderables(void* opaqueList, void* translucentList) = 0;

	// Call this to deactivate static prop rendering..
	virtual void DrawStaticProps(bool enable) = 0;

	// Call this to deactivate small object rendering
	virtual void DrawSmallEntities(bool enable) = 0;

	// The following methods are related to shadows...
	virtual ClientLeafShadowHandle_t AddShadow(ClientShadowHandle_t userId, unsigned short flags) = 0;
	virtual void RemoveShadow(ClientLeafShadowHandle_t h) = 0;

	// Project a shadow
	virtual void ProjectShadow(ClientLeafShadowHandle_t handle, int nLeafCount, const int* pLeafList) = 0;

	// Project a projected texture spotlight
	virtual void ProjectFlashlight(ClientLeafShadowHandle_t handle, int nLeafCount, const int* pLeafList) = 0;

	// Find all shadow casters in a set of leaves
	virtual void EnumerateShadowsInLeaves(int leafCount, void* pLeaves, void* pEnum) = 0;

	// Fill in a list of the leaves this renderable is in.
	// Returns -1 if the handle is invalid.
	virtual int GetRenderableLeaves(ClientRenderHandle_t handle, int leaves[128]) = 0;

	// Get leaves this renderable is in
	virtual bool GetRenderableLeaf(ClientRenderHandle_t handle, int* pOutLeaf, const int* pInIterator = 0, int* pOutIterator = 0) = 0;

	// Use alternate translucent sorting algorithm (draw translucent objects in the furthest leaf they lie in)
	virtual void EnableAlternateSorting(ClientRenderHandle_t handle, bool bEnable) = 0;
};

MAKE_INTERFACE_VERSION(iClientLeafSystem, "ClientLeafSystem002", IClientLeafSystem, CLIENT_DLL)
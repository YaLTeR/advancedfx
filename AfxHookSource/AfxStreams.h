#pragma once

// Copyright (c) advancedfx.org
//
// Last changes:
// 2016-09-23 dominik.matrixstorm.com
//
// First changes:
// 2015-06-26 dominik.matrixstorm.com

#include "SourceInterfaces.h"
#include "AfxInterfaces.h"
#include "AfxClasses.h"
#include "WrpConsole.h"
#include "AfxShaders.h"
#include "csgo_Stdshader_dx9_Hooks.h"

#define AFX_SHADERS_CSGO 0

#if AFX_SHADERS_CSGO
#include <shaders/build/afxHook_splinerope_ps20.h>
#include <shaders/build/afxHook_splinerope_ps20b.h>
#include <shaders/build/afxHook_splinecard_vs20.h>
#include <shaders/build/afxHook_spritecard_vs20.h>
#include <shaders/build/afxHook_spritecard_ps20.h>
#include <shaders/build/afxHook_spritecard_ps20b.h>
#include <shaders/build/afxHook_vertexlit_and_unlit_generic_ps20.h>
#include <shaders/build/afxHook_vertexlit_and_unlit_generic_ps20b.h>
#include <shaders/build/afxHook_vertexlit_and_unlit_generic_ps30.h>
#endif

#include <shared/bvhexport.h>

#include <string>
#include <list>
#include <queue>
#include <map>
#include <mutex>
#include <condition_variable>


class CAfxStreams;

extern CAfxStreams g_AfxStreams;


class IAfxContextHook abstract
{
public:
	//
	// General state:
	//
	// These functions should be synced into child queues / contexts by the implementation
	// because all contexts might need to know about these global states.

	virtual void RenderBegin(IAfxMatRenderContext * ctx) = 0;

	virtual void RenderEnd(void) = 0;

	virtual void DrawingHudBegin(void) = 0;

	virtual void DrawingSkyBoxViewBegin(void) = 0;

	virtual void DrawingSkyBoxViewEnd(void) = 0;

	//
	// Context specific functions:

	/// <returns>Return the material you want to use (instead).</returns>
	virtual SOURCESDK::IMaterial_csgo * MaterialHook(SOURCESDK::IMaterial_csgo * material) = 0;

	virtual void DrawInstances(int nInstanceCount, const SOURCESDK::MeshInstanceData_t_csgo *pInstance) = 0;

	virtual void Draw(IAfxMesh * am, int firstIndex = -1, int numIndices = 0) = 0;

	virtual void Draw_2(IAfxMesh * am, SOURCESDK::CPrimList_csgo *pLists, int nLists) = 0;

	virtual void DrawModulated(IAfxMesh * am, const SOURCESDK::Vector4D_csgo &vecDiffuseModulation, int firstIndex = -1, int numIndices = 0) = 0;

	/// <remarks>
	/// This is a good chance for an implmentation
	//  to hook and follow the subcontext actually,
	/// also a good place to inject markers for the rendering state
	/// (so we get to know about the sub-queue / sub-context).
	/// </remarks>
	virtual void QueueFunctorInternal(IAfxCallQueue * aq, SOURCESDK::CSGO::CFunctor *pFunctor) = 0;

	virtual void SetVertexShader(CAfx_csgo_ShaderState & state) = 0;

	virtual void SetPixelShader(CAfx_csgo_ShaderState & state) = 0;

	//
	// Helper functions:

	/// <remarks>
	/// The functor passed will be executed on the leaf queue / context,
	/// meaning you can i.e. handle DirectX Render States directly here
	/// in the desired order.
	/// <remarks>
	virtual void LeafExecute(SOURCESDK::CSGO::CFunctor * functor) = 0;

};

class CAfxRecordStream;

class CAfxStream
{
public:
	CAfxStream()
	{
	}

	//
	// Polymorphism:

	virtual CAfxStream * AsAfxStream(void) { return this; }
	virtual CAfxRecordStream * AsAfxRecordStream(void) { return 0; }

	//
	// Hooks:

	virtual void LevelShutdown(void)
	{
	}

protected:
	virtual ~CAfxStream()
	{
	}

private:
};

class CAfxSafeRefCounted
	: public SOURCESDK::IRefCounted_csgo
{
public:
	CAfxSafeRefCounted()
		: m_RefCount(0)
	{
	}

	//
	// SOURCESDK::IRefCounted_csgo:

	virtual int AddRef(void)
	{
		m_RefMutex.lock();

		++m_RefCount;
		int result = m_RefCount;

		m_RefMutex.unlock();

		return m_RefCount;
	}

	virtual int Release(void)
	{
		m_RefMutex.lock();

		--m_RefCount;

		int result = m_RefCount;

		m_RefMutex.unlock();

		if (0 == result)
			delete this;

		return result;
	}

protected:
	virtual ~CAfxSafeRefCounted()
	{
	}

private:
	std::mutex m_RefMutex;
	int m_RefCount;
};

class CAfxFunctor abstract
	: public SOURCESDK::CSGO::CFunctor
{
public:
	CAfxFunctor()
		: m_RefCount(0)
	{
	}

	//
	// SOURCESDK::IRefCounted_csgo:

	virtual int AddRef(void)
	{
		m_RefMutex.lock();

		++m_RefCount;
		int result = m_RefCount;

		m_RefMutex.unlock();

		return m_RefCount;
	}

	virtual int Release(void)
	{
		m_RefMutex.lock();

		--m_RefCount;

		int result = m_RefCount;

		m_RefMutex.unlock();

		if (0 == result)
			delete this;

		return result;
	}

protected:
	virtual ~CAfxFunctor()
	{
	}

private:
	std::mutex m_RefMutex;
	int m_RefCount;
};

class CAfxBaseFxStream;

class CAfxRenderViewStream
{
public:
	enum StreamCaptureType
	{
		SCT_Normal,
		SCT_Depth24,
		SCT_Depth24ZIP,
		SCT_DepthF,
		SCT_DepthFZIP
	};

	CAfxRenderViewStream();

	//
	// Reference counting:

	virtual int AddRef(void)
	{
		m_RefMutex.lock();

		++m_RefCount;
		int result = m_RefCount;

		m_RefMutex.unlock();

		return m_RefCount;
	}

	virtual int Release(void)
	{
		m_RefMutex.lock();

		--m_RefCount;

		int result = m_RefCount;

		m_RefMutex.unlock();

		if (0 == result)
			delete this;

		return result;
	}

	//
	// Lock functions:
	//
	// These should be used when accessing concurrent resources on the stream.

	void InterLock(void)
	{
		std::unique_lock<std::mutex> lock(m_LockMutex);

		m_LockCondition.wait(lock, [this]() { return m_LockCount == 0; });

		++m_LockCount;
	}

	void InterLockIncrement(void)
	{
		std::unique_lock<std::mutex> lock(m_LockMutex);
		
		++m_LockCount;
	}

	void InterLockDecrement(void)
	{
		bool notify = false;

		{
			std::unique_lock<std::mutex> lock(m_LockMutex);
			--m_LockCount;

			notify = m_LockCount == 0;
		}

		if(notify)
			m_LockCondition.notify_one();		
	}

	//
	//

	virtual CAfxRenderViewStream * AsAfxRenderViewStream(void) { return this; }

	virtual CAfxBaseFxStream * AsAfxBaseFxStream(void) { return 0; }

	char const * AttachCommands_get(void);
	void AttachCommands_set(char const * value);

	char const * DetachCommands_get(void);
	void DetachCommands_set(char const * value);

	bool DrawHud_get(void);
	void DrawHud_set(bool value);

	bool DrawViewModel_get(void);
	void DrawViewModel_set(bool value);

	StreamCaptureType StreamCaptureType_get(void);
	void StreamCaptureType_set(StreamCaptureType value);

	virtual void LevelShutdown(void)
	{
	}

	//
	// State information:

	virtual void OnRenderBegin()
	{
	}

	virtual void OnRenderEnd()
	{
	}

protected:
	StreamCaptureType m_StreamCaptureType;
	bool m_DrawingSkyBox;

	virtual ~CAfxRenderViewStream();

private:
	std::mutex m_RefMutex;
	std::mutex m_LockMutex;
	std::condition_variable m_LockCondition;
	int m_LockCount;
	int m_RefCount;
	bool m_DrawViewModel;
	bool m_DrawHud;
	std::string m_AttachCommands;
	std::string m_DetachCommands;
};

class CAfxRenderViewStreamInterLock
{
public:
	CAfxRenderViewStreamInterLock(CAfxRenderViewStream * stream)
	: m_Stream(stream)
	{
		if(m_Stream)
			m_Stream->InterLock();
	}

	~CAfxRenderViewStreamInterLock()
	{
		if (m_Stream)
			m_Stream->InterLockDecrement();
	}

private:
	CAfxRenderViewStream * m_Stream;
};

class CAfxSingleStream;
class CAfxTwinStream;

class CAfxRecordStream
: public CAfxStream
{
public:
	CAfxRecordStream(char const * streamName);

	virtual CAfxRecordStream * AsAfxRecordStream(void) { return this; }

	virtual CAfxSingleStream * AsAfxSingleStream(void) { return 0; }
	virtual CAfxTwinStream * AsAfxTwinStream(void) { return 0; }

	char const * StreamName_get(void);

	bool Record_get(void);
	void Record_set(bool value);

	/// <remarks>This is called regardless of Record value.</remarks>
	void RecordStart();

	/// <remarks>This is only called between RecordStart and RecordEnd and only if Record is true.</remarks>
	bool CreateCapturePath(const std::wstring & takeDir, int frameNumber, wchar_t const * fileExtension, std::wstring &outPath);

	/// <remarks>This is called regardless of Record value.</remarks>
	void RecordEnd();

private:
	std::string m_StreamName;
	std::wstring m_CapturePath;
	bool m_Record;
	bool m_TriedCreatePath;
	bool m_SucceededCreatePath;
};

class CAfxSingleStream
:  public CAfxRecordStream
{
public:
	CAfxSingleStream(char const * streamName, CAfxRenderViewStream * stream);

	virtual CAfxSingleStream * AsAfxSingleStream(void) { return this; }

	virtual void LevelShutdown(void);

	CAfxRenderViewStream * Stream_get(void);

protected:
	virtual ~CAfxSingleStream();

private:
	CAfxRenderViewStream * m_Stream;

};

class CAfxTwinStream
: public CAfxRecordStream
{
public:
	enum StreamCombineType
	{
		SCT_ARedAsAlphaBColor,
		SCT_AColorBRedAsAlpha
	};

	/// <remarks>Takes ownership of given streams.</remarks>
	CAfxTwinStream(char const * streamName, CAfxRenderViewStream * streamA, CAfxRenderViewStream * streamB, StreamCombineType streamCombineType);

	virtual CAfxTwinStream * AsAfxTwinStream(void) { return this; }

	virtual void LevelShutdown(void);

	CAfxRenderViewStream * StreamA_get();
	CAfxRenderViewStream * StreamB_get();

	StreamCombineType StreamCombineType_get(void);
	void StreamCombineType_set(StreamCombineType value);

protected:
	virtual ~CAfxTwinStream();

private:
	CAfxRenderViewStream * m_StreamA;
	CAfxRenderViewStream * m_StreamB;
	StreamCombineType m_StreamCombineType;
};


extern bool g_DebugEnabled;


class CAfxBaseFxStream
: public CAfxRenderViewStream
{
public:
	class CActionKey
	{
	public:
		std::string m_Name;

		CActionKey(char const * name)
		: m_Name(name)
		{
		}

		CActionKey(const CActionKey & key, bool toLower)
		: m_Name(key.m_Name)
		{
			if(toLower) ToLower();
		}

		CActionKey(const CActionKey & x)
		: m_Name(x.m_Name)
		{
		}
		
		bool operator < (const CActionKey & y) const
		{
			return m_Name.compare(y.m_Name) < 0;
		}

		void ToLower(void);

	private:
	};

private:
	class CAfxBaseFxStreamContextHook;

public:

	class CAction
	{
	public:
		CAction()
		: m_RefCount(0)
		, m_IsStockAction(false)
		, m_Key("(unnamed)")
		{
		}

		void AddRef(void)
		{
			++m_RefCount;
		}

		void Release(void)
		{
			--m_RefCount;
			if(0 == m_RefCount)
				delete this;
		}

		int GetRefCount(void)
		{
			return m_RefCount;
		}

		bool IsStockAction_get(void)
		{
			return m_IsStockAction;
		}

		void IsStockAction_set(bool value)
		{
			m_IsStockAction = value;
		}
		
		CActionKey const & Key_get(void)
		{
			return m_Key;
		}

		void Key_set(CActionKey const & value)
		{
			m_Key = value;
		}

		virtual CAction * ResolveAction(SOURCESDK::IMaterial_csgo * material)
		{
			return this;
		}

		virtual void LevelShutdown(void)
		{
		}

		virtual void AfxUnbind(CAfxBaseFxStreamContextHook * ch)
		{
		}

		virtual SOURCESDK::IMaterial_csgo * MaterialHook(CAfxBaseFxStreamContextHook * ch, SOURCESDK::IMaterial_csgo * material)
		{
			return material;
		}

		virtual void DrawInstances(CAfxBaseFxStreamContextHook * ch, int nInstanceCount, const SOURCESDK::MeshInstanceData_t_csgo *pInstance )
		{
			ch->GetCtx()->GetOrg()->DrawInstances(nInstanceCount, pInstance);
		}

		virtual void Draw(CAfxBaseFxStreamContextHook * ch, IAfxMesh * am, int firstIndex = -1, int numIndices = 0)
		{
			am->GetParent()->Draw(firstIndex, numIndices);
		}

		virtual void Draw_2(CAfxBaseFxStreamContextHook * ch, IAfxMesh * am, SOURCESDK::CPrimList_csgo *pLists, int nLists)
		{
			am->GetParent()->Draw(pLists, nLists);
		}

		virtual void DrawModulated(CAfxBaseFxStreamContextHook * ch, IAfxMesh * am, const SOURCESDK::Vector4D_csgo &vecDiffuseModulation, int firstIndex = -1, int numIndices = 0 )
		{
			am->GetParent()->DrawModulated(vecDiffuseModulation, firstIndex, numIndices);
		}

		virtual void SetVertexShader(CAfxBaseFxStreamContextHook * ch, CAfx_csgo_ShaderState & state)
		{
		}

		virtual void SetPixelShader(CAfxBaseFxStreamContextHook * ch, CAfx_csgo_ShaderState & state)
		{
		}

	protected:
		bool m_IsStockAction;
		CActionKey m_Key;

		virtual ~CAction()
		{
		}

		virtual CAction * SafeSubResolveAction(CAction * action, SOURCESDK::IMaterial_csgo * material)
		{
			if(action)
				return action->ResolveAction(material);
			
			return action;
		}

	private:
		int m_RefCount;
	};

	static void Console_ListActions(void)
	{
		m_Shared.Console_ListActions();
	}

	static void Console_AddReplaceAction(IWrpCommandArgs * args)
	{
		m_Shared.Console_AddReplaceAction(args);
	}

	static CAction * GetAction(CActionKey const & key)
	{
		return m_Shared.GetAction(key);
	}

	static bool RemoveAction(CActionKey const & key)
	{
		return m_Shared.RemoveAction(key);
	}

	CAfxBaseFxStream();

	virtual void OnRenderBegin(void);

	virtual void OnRenderEnd(void);

	void Console_ActionFilter_Add(const char * expression, CAction * action);
	void Console_ActionFilter_Print(void);
	void Console_ActionFilter_Remove(int id);
	void Console_ActionFilter_Move(int id, int moveBeforeId);

	virtual CAfxBaseFxStream * AsAfxBaseFxStream(void) { return this; }

	virtual void LevelShutdown(void);

	CAction * RetrieveAction(SOURCESDK::IMaterial_csgo * material);

	CAction * ClientEffectTexturesAction_get(void);
	void ClientEffectTexturesAction_set(CAction * value);

	CAction * WorldTexturesAction_get(void);
	void WorldTexturesAction_set(CAction * value);

	CAction * SkyBoxTexturesAction_get(void);
	void SkyBoxTexturesAction_set(CAction * value);

	CAction * StaticPropTexturesAction_get(void);
	void StaticPropTexturesAction_set(CAction * value);

	CAction * CableAction_get(void);
	void CableAction_set(CAction * value);

	CAction * PlayerModelsAction_get(void);
	void PlayerModelsAction_set(CAction * value);

	CAction * WeaponModelsAction_get(void);
	void WeaponModelsAction_set(CAction * value);

	CAction * StatTrakAction_get(void);
	void StatTrakAction_set(CAction * value);

	CAction * ShellModelsAction_get(void);
	void ShellModelsAction_set(CAction * value);

	CAction * OtherModelsAction_get(void);
	void OtherModelsAction_set(CAction * value);

	CAction * DecalTexturesAction_get(void);
	void DecalTexturesAction_set(CAction * value);

	CAction * EffectsAction_get(void);
	void EffectsAction_set(CAction * value);

	CAction * ShellParticleAction_get(void);
	void ShellParticleAction_set(CAction * value);

	CAction * OtherParticleAction_get(void);
	void OtherParticleAction_set(CAction * value);

	CAction * StickerAction_get(void);
	void StickerAction_set(CAction * value);

	CAction * ErrorMaterialAction_get(void);
	void ErrorMaterialAction_set(CAction * value);

	CAction * OtherAction_get(void);
	void OtherAction_set(CAction * value);

	CAction * WriteZAction_get(void);
	void WriteZAction_set(CAction * value);

	CAction * DevAction_get(void);

	CAction * OtherEngineAction_get(void);

	CAction * OtherSpecialAction_get(void);

	CAction * VguiAction_get(void);

	bool TestAction_get(void);
	void TestAction_set(bool value);

	float DepthVal_get(void);
	void DepthVal_set(float value);

	float DepthValMax_get(void);
	void DepthValMax_set(float value);

	float SmokeOverlayAlphaFactor_get(void);
	void SmokeOverlayAlphaFactor_set(float value);
	
	bool DebugPrint_get(void);
	void DebugPrint_set(bool value);

	void InvalidateMap(void);

	/*
	/// <pram name="to24">false: depth24 to depth; true: depth to depth24</param>
	/// <pram name="depth24ZIP">if not to24, then set SCT_Normal otherwise: false: set capturetype SCT_Depth24; true: set capturetype SCT_Depth24ZIP</param>
	void ConvertStreamDepth(bool to24, bool depth24ZIP);

	/// <pram name="to24">false: depth24 to depth; true: depth to depth24</param>
	void ConvertDepthActions(bool to24);
	*/

protected:
	class CShared
	{
	public:
		CShared();
		~CShared();

		void AddRef();
		void Release();

		void Console_ListActions(void);
		void Console_AddReplaceAction(IWrpCommandArgs * args);
		CAction * GetAction(CActionKey const & key);
		bool RemoveAction(CActionKey const & key);

		void LevelShutdown(void);

		//CAfxBaseFxStream * m_ActiveBaseFxStream;

		CAction * DrawAction_get(void);
		CAction * NoDrawAction_get(void);
		CAction * DepthAction_get(void);
		//CAction * Depth24Action_get(void);
		CAction * MaskAction_get(void);
		CAction * WhiteAction_get(void);
		CAction * BlackAction_get(void);
	
	private:
		int m_RefCount;
		int m_ShutDownLevel;
		std::map<CActionKey, CAction *> m_Actions;
		CAction * m_DrawAction;
		CAction * m_NoDrawAction;
		CAction * m_DebugDumpAction;
		CAction * m_DepthAction;
		//CAction * m_Depth24Action;
		CAction * m_MaskAction;
		CAction * m_WhiteAction;
		CAction * m_BlackAction;

		void CreateStdAction(CAction * & stdTarget, CActionKey const & key, CAction * action);
		void CreateAction(CActionKey const & key, CAction * action, bool isStockAction = false);
		bool Console_CheckActionKey(CActionKey & key);
	};

	static CShared m_Shared;

	CAction * m_ClientEffectTexturesAction;
	CAction * m_WorldTexturesAction;
	CAction * m_SkyBoxTexturesAction;
	CAction * m_StaticPropTexturesAction;
	CAction * m_CableAction;
	CAction * m_PlayerModelsAction;
	CAction * m_WeaponModelsAction;
	CAction * m_StatTrakAction;
	CAction * m_ShellModelsAction;
	CAction * m_OtherModelsAction;
	CAction * m_DecalTexturesAction;
	CAction * m_EffectsAction;
	CAction * m_ShellParticleAction;
	CAction * m_OtherParticleAction;
	CAction * m_StickerAction;
	CAction * m_ErrorMaterialAction;
	CAction * m_OtherAction;
	CAction * m_WriteZAction;
	CAction * m_DevAction;
	CAction * m_OtherEngineAction;
	CAction * m_OtherSpecialAction;
	CAction * m_VguiAction;
	bool m_TestAction;
	float m_DepthVal;
	float m_DepthValMax;
	float m_SmokeOverlayAlphaFactor;

	virtual ~CAfxBaseFxStream();

	void SetActionAndInvalidateMap(CAction * & target, CAction * src);
	void SetAction(CAction * & target, CAction * src);

private:
	class CActionDebugDump
	: public CAction
	{
	public:
		CActionDebugDump()
		{
		}

		virtual void AfxUnbind(CAfxBaseFxStreamContextHook * ch);
	};

#if AFX_SHADERS_CSGO
	class CActionAfxVertexLitGenericHookKey
	{
	public:
		ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps30::AFXMODE_e AFXMODE;
		float AlphaTestReference;

		CActionAfxVertexLitGenericHookKey()
		{
		}

		CActionAfxVertexLitGenericHookKey(
			ShaderCombo_afxHook_vertexlit_and_unlit_generic_ps30::AFXMODE_e a_AFXMODE,
			float a_AlphaTestReference)
		: AFXMODE(a_AFXMODE)
		, AlphaTestReference(a_AlphaTestReference)
		{
		}

		CActionAfxVertexLitGenericHookKey(const CActionAfxVertexLitGenericHookKey & x)
		: AFXMODE(x.AFXMODE)
		, AlphaTestReference(x.AlphaTestReference)
		{
		}
		
		bool operator < (const CActionAfxVertexLitGenericHookKey & y) const
		{
			if(this->AFXMODE < y.AFXMODE)
				return true;

			return this->AFXMODE == y.AFXMODE && this->AlphaTestReference < y.AlphaTestReference;
		}
	};

	class CActionAfxVertexLitGenericHook
	: public CAction
	{
	public:
		CActionAfxVertexLitGenericHook(CActionAfxVertexLitGenericHookKey & key);

		virtual bool CheckIsCompatible(IMaterial_csgo * material)
		{
			if(!material)
				return false;

			char const * shaderName = material->GetShaderName();

			return !strcmp(shaderName, "VertexLitGernic")
				|| !strcmp(shaderName, "UnlitGeneric");
		}

		virtual void AfxUnbind(IAfxMatRenderContext * ctx);

		virtual IMaterial_csgo * MaterialHook(IAfxMatRenderContext * ctx, IMaterial_csgo * material);

		virtual void SetPixelShader(CAfx_csgo_ShaderState & state);

	private:
		static csgo_Stdshader_dx9_Combos_vertexlit_and_unlit_generic_ps20 m_Combos_ps20;
		static csgo_Stdshader_dx9_Combos_vertexlit_and_unlit_generic_ps20b m_Combos_ps20b;
		static csgo_Stdshader_dx9_Combos_vertexlit_and_unlit_generic_ps30 m_Combos_ps30;
		CActionAfxVertexLitGenericHookKey m_Key;
	};

	class CActionUnlitGenericFallback
	: public CActionAfxVertexLitGenericHook
	{
	public:
		CActionUnlitGenericFallback(CActionAfxVertexLitGenericHookKey & key, char const * unlitGenericFallbackMaterialName);

		virtual IMaterial_csgo * MaterialHook(IAfxMatRenderContext * ctx, IMaterial_csgo * material);

	protected:
		virtual ~CActionUnlitGenericFallback();

	private:
		CAfxMaterial * m_Material;
		std::string m_MaterialName;
	};
#endif

	class CActionNoDraw
	: public CAction
	{
	public:
		CActionNoDraw()
		: CAction()
		{
		}

		virtual void AfxUnbind(CAfxBaseFxStreamContextHook * ch);

		virtual SOURCESDK::IMaterial_csgo * MaterialHook(CAfxBaseFxStreamContextHook * ch, SOURCESDK::IMaterial_csgo * material);

/*		We could speed up a bit here, but I am not sure if it's safe to do this,
        so we just let it draw for now and block the drawing instead.

		virtual void Draw(IAfxMesh * am, int firstIndex = -1, int numIndices = 0)
		{
			am->GetParent()->MarkAsDrawn();
		}

		virtual void Draw_2(IAfxMesh * am, CPrimList_csgo *pLists, int nLists)
		{
			am->GetParent()->MarkAsDrawn();
		}

		virtual void DrawModulated(IAfxMesh * am, const Vector4D_csgo &vecDiffuseModulation, int firstIndex = -1, int numIndices = 0 )
		{
			am->GetParent()->MarkAsDrawn();
		}

		virtual void DrawInstances(IAfxMatRenderContext * ctx, int nInstanceCount, const MeshInstanceData_t_csgo *pInstance )
		{
			return;
		}
*/
	};

	class CActionDraw
	: public CAction
	{
	public:
		CActionDraw()
		: CAction()
		{
		}

		virtual bool CheckIsCompatible(SOURCESDK::IMaterial_csgo * material)
		{
			return true;
		}
	};

#if AFX_SHADERS_CSGO
	class CActionAfxSpritecardHookKey
	{
	public:
		ShaderCombo_afxHook_spritecard_ps20b::AFXMODE_e AFXMODE;
		float AlphaTestReference;

		CActionAfxSpritecardHookKey()
		{
		}

		CActionAfxSpritecardHookKey(
			ShaderCombo_afxHook_spritecard_ps20b::AFXMODE_e a_AFXMODE,
			float a_AlphaTestReference)
		: AFXMODE(a_AFXMODE)
		, AlphaTestReference(a_AlphaTestReference)
		{
		}

		CActionAfxSpritecardHookKey(const CActionAfxSpritecardHookKey & x)
		: AFXMODE(x.AFXMODE)
		, AlphaTestReference(x.AlphaTestReference)
		{
		}
		
		bool operator < (const CActionAfxSpritecardHookKey & y) const
		{
			if(this->AFXMODE < y.AFXMODE)
				return true;

			return this->AFXMODE == y.AFXMODE && this->AlphaTestReference < y.AlphaTestReference;
		}
	};

	class CActionAfxSpritecardHook
	: public CAction
	{
	public:
		CActionAfxSpritecardHook(CActionAfxSpritecardHookKey & key);

		virtual void AfxUnbind(IAfxMatRenderContext * ctx);

		virtual IMaterial_csgo * MaterialHook(IAfxMatRenderContext * ctx, IMaterial_csgo * material);

		virtual void SetVertexShader(CAfx_csgo_ShaderState & state);
		virtual void SetPixelShader(CAfx_csgo_ShaderState & state);

	private:
		static csgo_Stdshader_dx9_Combos_splinecard_vs20 m_Combos_splinecard_vs20;
		static csgo_Stdshader_dx9_Combos_spritecard_vs20 m_Combos_spritecard_vs20;
		static csgo_Stdshader_dx9_Combos_spritecard_ps20 m_Combos_ps20;
		static csgo_Stdshader_dx9_Combos_spritecard_ps20b m_Combos_ps20b;
		CActionAfxSpritecardHookKey m_Key;
	};

	class CActionAfxSplineRopeHookKey
	{
	public:
		ShaderCombo_afxHook_splinerope_ps20b::AFXMODE_e AFXMODE;
		float AlphaTestReference;

		CActionAfxSplineRopeHookKey()
		{
		}

		CActionAfxSplineRopeHookKey(
			ShaderCombo_afxHook_splinerope_ps20b::AFXMODE_e a_AFXMODE,
			float a_AlphaTestReference)
		: AFXMODE(a_AFXMODE)
		, AlphaTestReference(a_AlphaTestReference)
		{
		}

		CActionAfxSplineRopeHookKey(const CActionAfxSplineRopeHookKey & x)
		: AFXMODE(x.AFXMODE)
		, AlphaTestReference(x.AlphaTestReference)
		{
		}
		
		bool operator < (const CActionAfxSplineRopeHookKey & y) const
		{
			if(this->AFXMODE < y.AFXMODE)
				return true;

			return this->AFXMODE == y.AFXMODE && this->AlphaTestReference < y.AlphaTestReference;
		}
	};

	class CActionAfxSplineRopeHook
	: public CAction
	{
	public:
		CActionAfxSplineRopeHook(CActionAfxSplineRopeHookKey & key);

		virtual void AfxUnbind(IAfxMatRenderContext * ctx);

		virtual IMaterial_csgo * MaterialHook(IAfxMatRenderContext * ctx, IMaterial_csgo * material);

		virtual void SetPixelShader(CAfx_csgo_ShaderState & state);

	private:
		static csgo_Stdshader_dx9_Combos_splinerope_ps20 m_Combos_ps20;
		static csgo_Stdshader_dx9_Combos_splinerope_ps20b m_Combos_ps20b;
		CActionAfxSplineRopeHookKey m_Key;
	};
#endif

	class CActionReplace
	: public CAction
	{
	public:
		CActionReplace(
			char const * materialName,
			CAction * fallBackAction);

		void OverrideColor(float const color[3])
		{
			m_OverrideColor = true;
			m_Color[0] = color[0];
			m_Color[1] = color[1];
			m_Color[2] = color[2];
		}

		void OverrideBlend(float blend)
		{
			m_OverrideBlend = true;
			m_Blend = blend;
		}

		void OverrideDepthWrite(bool depthWrite)
		{
			m_OverrideDepthWrite = true;
			m_DepthWrite = depthWrite;
		}

		virtual CAction * ResolveAction(SOURCESDK::IMaterial_csgo * material);

		virtual void AfxUnbind(CAfxBaseFxStreamContextHook * ch);

		virtual SOURCESDK::IMaterial_csgo * MaterialHook(CAfxBaseFxStreamContextHook * ch, SOURCESDK::IMaterial_csgo * material);

 		virtual void DrawInstances(CAfxBaseFxStreamContextHook * ch, int nInstanceCount, const SOURCESDK::MeshInstanceData_t_csgo *pInstance )
		{
			if(m_OverrideColor || m_OverrideBlend)
			{
				SOURCESDK::MeshInstanceData_t_csgo * first = const_cast<SOURCESDK::MeshInstanceData_t_csgo *>(pInstance);

				for(int i = 0; i < nInstanceCount; ++i)
				{
					if(m_OverrideColor)
					{
						first->m_DiffuseModulation.x = m_Color[0];
						first->m_DiffuseModulation.y = m_Color[1];
						first->m_DiffuseModulation.z = m_Color[2];
					}
					if(m_OverrideBlend)
					{
						first->m_DiffuseModulation.w = m_Blend;
					}
 
					++first;
				}
			}
 
			ch->GetCtx()->GetOrg()->DrawInstances(nInstanceCount, pInstance); 
		} 

	protected:
		virtual ~CActionReplace();

	private:
		CAfxMaterial * m_Material;
		std::string m_MaterialName;
		CAction * m_FallBackAction;
		bool m_OverrideColor;
		float m_Color[3];
		bool m_OverrideBlend;
		float m_Blend;
		bool m_OverrideDepthWrite;
		bool m_DepthWrite;

		void EnsureMaterial(void);
		void ExamineMaterial(SOURCESDK::IMaterial_csgo * material, bool & outSplinetype, bool & outUseinstancing);
	};

	class CActionDebugDepth
	: public CAction
	{
	public:
		CActionDebugDepth(CAction * fallBackAction);

		virtual CAction * ResolveAction(SOURCESDK::IMaterial_csgo * material);

		virtual void AfxUnbind(CAfxBaseFxStreamContextHook * ch);

		virtual SOURCESDK::IMaterial_csgo * MaterialHook(CAfxBaseFxStreamContextHook * ch, SOURCESDK::IMaterial_csgo * material);

	protected:
		~CActionDebugDepth();
	private:
		CAction * m_FallBackAction;

		CAfxMaterial * m_DebugDepthMaterial;
		WrpConVarRef * m_MatDebugDepthVal;
		WrpConVarRef * m_MatDebugDepthValMax;
		bool m_Initalized;
		std::mutex m_InitalizedMutex;
	};

#if AFX_SHADERS_CSGO
	class CActionStandardResolve
	: public CAction
	{
	public:
		enum ResolveFor {
			RF_DrawDepth,
			RF_DrawDepth24,
			RF_GreenScreen,
			RF_Black,
			RF_White
		};

		CActionStandardResolve(ResolveFor resolveFor, CAction * fallBackAction);

		virtual CAction * ResolveAction(IMaterial_csgo * material);

	protected:
		~CActionStandardResolve();
	
	private:
		class CShared
		{
		public:
			CShared();
			~CShared();

			void AddRef();
			void Release();

			void LevelShutdown(IAfxStreams4Stream * streams);

			CAction * GetStdDepthAction();
			CAction * GetStdDepth24Action();
			CAction * GetStdMatteAction();
			CAction * GetStdBlackAction();
			CAction * GetStdWhiteAction();

			CAction * GetSplineRopeHookAction(CActionAfxSplineRopeHookKey & key);
			CAction * GetSpritecardHookAction(CActionAfxSpritecardHookKey & key);
			CAction * GetVertexLitGenericHookAction(CActionAfxVertexLitGenericHookKey & key);

		private:
			int m_RefCount;
			int m_ShutDownLevel;

			CAction * m_StdDepthAction;
			CAction * m_StdDepth24Action;
			CAction * m_StdMatteAction;
			CAction * m_StdBlackAction;
			CAction * m_StdWhiteAction;

			std::map<CActionAfxSplineRopeHookKey, CActionAfxSplineRopeHook *> m_SplineRopeHookActions;
			std::map<CActionAfxSpritecardHookKey, CActionAfxSpritecardHook *> m_SpritecardHookActions;
			std::map<CActionAfxVertexLitGenericHookKey, CActionAfxVertexLitGenericHook *> m_VertexLitGenericHookActions;

			void InvalidateSplineRopeHookActions();
			void InvalidateSpritecardHookActions();
			void InvalidateVertexLitGenericHookActions();
		};

		static CShared m_Shared;


		ResolveFor m_ResolveFor;
		CAction * m_FallBackAction;
	};
#endif

	class CActionFilterValue
	{
	public:
		CActionFilterValue()
		: m_MatchAction(0)
		{
		}

		CActionFilterValue(char const * matchString, CAction * matchAction)
		: m_MatchString(matchString)
		, m_MatchAction(matchAction)
		{
			if(matchAction) matchAction->AddRef();
		}

		CActionFilterValue(const CActionFilterValue & x)
		: m_MatchString(x.m_MatchString)
		, m_MatchAction(x.m_MatchAction)
		{
			if(m_MatchAction) m_MatchAction->AddRef();
		}

		~CActionFilterValue()
		{
			if(m_MatchAction) m_MatchAction->Release();
		}

		CActionFilterValue & operator= (const CActionFilterValue & x)
		{
			this->m_MatchString = x.m_MatchString;
			this->m_MatchAction = x.m_MatchAction;
			if(m_MatchAction) m_MatchAction->AddRef();
			return *this;
		}

		char const * GetMatchString(void)
		{
			return m_MatchString.c_str();
		}

		CAction * GetMatchAction(void)
		{
			return m_MatchAction;
		}

		bool CalcMatch(char const * targetString);

	private:
		std::string m_MatchString;
		CAction * m_MatchAction;
	};

	class CAfxBaseFxStreamContextHook
		: public IAfxContextHook
	{
	public:
		CAfxBaseFxStreamContextHook(CAfxBaseFxStream * stream, CAfxBaseFxStreamContextHook * cloneFrom)
			: m_Stream(stream)
			, m_CurrentAction(0)
		{
			stream->AddRef();

			if (cloneFrom)
			{
				m_DrawingSkyBoxView = cloneFrom->m_DrawingSkyBoxView;
			}
			else
			{
				m_DrawingSkyBoxView = false;
			}
		}

		CAfxBaseFxStream * GetStream()
		{
			return m_Stream;
		}

		IAfxMatRenderContext * GetCtx()
		{
			return m_Ctx;
		}

		bool DrawingSkyBoxView_get(void)
		{
			return m_DrawingSkyBoxView;
		}

		//
		// IAfxContextHook:

		virtual void RenderBegin(IAfxMatRenderContext * ctx);

		virtual void RenderEnd(void);

		virtual void DrawingHudBegin(void);

		virtual void DrawingSkyBoxViewBegin(void);

		virtual void DrawingSkyBoxViewEnd(void);

		virtual SOURCESDK::IMaterial_csgo * MaterialHook(SOURCESDK::IMaterial_csgo * material);

		virtual void DrawInstances(int nInstanceCount, const SOURCESDK::MeshInstanceData_t_csgo *pInstance);

		virtual void Draw(IAfxMesh * am, int firstIndex = -1, int numIndices = 0);

		virtual void Draw_2(IAfxMesh * am, SOURCESDK::CPrimList_csgo *pLists, int nLists);

		virtual void DrawModulated(IAfxMesh * am, const SOURCESDK::Vector4D_csgo &vecDiffuseModulation, int firstIndex = -1, int numIndices = 0);

		virtual void QueueFunctorInternal(IAfxCallQueue * aq, SOURCESDK::CSGO::CFunctor *pFunctor);

		virtual void SetVertexShader(CAfx_csgo_ShaderState & state);

		virtual void SetPixelShader(CAfx_csgo_ShaderState & state);

		virtual void LeafExecute(SOURCESDK::CSGO::CFunctor * functor);

	protected:
		~CAfxBaseFxStreamContextHook()
		{
			m_Stream->Release();
		}

	private:
		class CLeafExecuteFunctor
			: public CAfxFunctor
		{
		public:
			CLeafExecuteFunctor(SOURCESDK::CSGO::CFunctor * functor)
				: m_Functor(functor)
			{
				functor->AddRef();
			}

			virtual void operator()();

		protected:
			virtual ~CLeafExecuteFunctor()
			{
				m_Functor->Release();
			}

		private:
			SOURCESDK::CSGO::CFunctor * m_Functor;

		};

		class CRenderBeginFunctor
			: public CAfxFunctor
		{
		public:
			CRenderBeginFunctor(CAfxBaseFxStreamContextHook * ch)
				: m_Ch(ch)
			{

			}


			virtual void operator()();
		
		private:
			CAfxBaseFxStreamContextHook * m_Ch;

		};

		class CRenderEndFunctor
			: public CAfxFunctor
		{
		public:
			virtual void operator()();
		};

		CAfxBaseFxStream * m_Stream;
		IAfxMatRenderContext * m_Ctx;
		bool m_DrawingSkyBoxView;
		CAfxBaseFxStream::CAction * m_CurrentAction;

		void BindAction(CAction * action)
		{
			if (m_CurrentAction)
			{
				m_CurrentAction->AfxUnbind(this);
				m_CurrentAction->Release();
			}
			if (action)
			{
				action->AddRef();
			}
			m_CurrentAction = action;
		}
	};

	bool m_DebugPrint;
	std::map<CAfxMaterialKey, CAction *> m_Map;
	std::list<CActionFilterValue> m_ActionFilter;

	CAction * CAfxBaseFxStream::GetAction(SOURCESDK::IMaterial_csgo * material);
	CAction * CAfxBaseFxStream::GetAction(SOURCESDK::IMaterial_csgo * material, CAction * action);

	/*
	void ConvertDepthAction(CAction * & action, bool to24);
	*/
};

class CAfxDepthStream
: public CAfxBaseFxStream
{
public:
	CAfxDepthStream() : CAfxBaseFxStream()
	{
		/*
		SetAction(m_ClientEffectTexturesAction, m_Shared.DepthAction_get());
		SetAction(m_WorldTexturesAction, m_Shared.DepthAction_get());
		SetAction(m_SkyBoxTexturesAction, m_Shared.DepthAction_get());
		SetAction(m_StaticPropTexturesAction, m_Shared.DepthAction_get());
		SetAction(m_CableAction, m_Shared.DepthAction_get());
		SetAction(m_PlayerModelsAction, m_Shared.DepthAction_get());
		SetAction(m_WeaponModelsAction, m_Shared.DepthAction_get());
		SetAction(m_StatTrakAction, m_Shared.DepthAction_get());
		SetAction(m_ShellModelsAction, m_Shared.DepthAction_get());
		SetAction(m_OtherModelsAction, m_Shared.DepthAction_get());
		SetAction(m_DecalTexturesAction, m_Shared.DepthAction_get());
		SetAction(m_EffectsAction, m_Shared.DepthAction_get());
		SetAction(m_ShellParticleAction, m_Shared.DepthAction_get());
		SetAction(m_OtherParticleAction, m_Shared.DepthAction_get());
		SetAction(m_StickerAction, m_Shared.DepthAction_get());
		SetAction(m_ErrorMaterialAction, m_Shared.DepthAction_get());
		SetAction(m_OtherAction, m_Shared.DepthAction_get());
		*/

		SetAction(m_ClientEffectTexturesAction, m_Shared.NoDrawAction_get());
		SetAction(m_WorldTexturesAction, m_Shared.DepthAction_get());
		SetAction(m_SkyBoxTexturesAction, m_Shared.DepthAction_get());
		SetAction(m_StaticPropTexturesAction, m_Shared.DepthAction_get());
		SetAction(m_CableAction, m_Shared.NoDrawAction_get());
		SetAction(m_PlayerModelsAction, m_Shared.DepthAction_get());
		SetAction(m_WeaponModelsAction, m_Shared.DepthAction_get());
		SetAction(m_StatTrakAction, m_Shared.DepthAction_get());
		SetAction(m_ShellModelsAction, m_Shared.DepthAction_get());
		SetAction(m_OtherModelsAction, m_Shared.DepthAction_get());
		SetAction(m_DecalTexturesAction, m_Shared.NoDrawAction_get());
		SetAction(m_EffectsAction, m_Shared.NoDrawAction_get());
		SetAction(m_ShellParticleAction, m_Shared.NoDrawAction_get());
		SetAction(m_OtherParticleAction, m_Shared.NoDrawAction_get());
		SetAction(m_StickerAction, m_Shared.NoDrawAction_get());
		SetAction(m_ErrorMaterialAction, m_Shared.DepthAction_get());
		SetAction(m_OtherAction, m_Shared.DepthAction_get());
	}

protected:
	virtual ~CAfxDepthStream() {}
};

class CAfxMatteWorldStream
: public CAfxBaseFxStream
{
public:
	CAfxMatteWorldStream() : CAfxBaseFxStream()
	{
		/*
		SetAction(m_ClientEffectTexturesAction, m_Shared.DrawAction_get());
		SetAction(m_WorldTexturesAction, m_Shared.DrawAction_get());
		SetAction(m_SkyBoxTexturesAction, m_Shared.DrawAction_get());
		SetAction(m_StaticPropTexturesAction, m_Shared.DrawAction_get());
		SetAction(m_CableAction,  m_Shared.DrawAction_get());
		SetAction(m_PlayerModelsAction, m_Shared.NoDrawAction_get());
		SetAction(m_WeaponModelsAction, m_Shared.NoDrawAction_get());
		SetAction(m_StatTrakAction, m_Shared.NoDrawAction_get());
		SetAction(m_ShellModelsAction, m_Shared.NoDrawAction_get());
		SetAction(m_OtherModelsAction, m_Shared.DrawAction_get());
		SetAction(m_DecalTexturesAction, m_Shared.DrawAction_get());
		SetAction(m_EffectsAction, m_Shared.DrawAction_get());
		SetAction(m_ShellParticleAction, m_Shared.NoDrawAction_get());
		SetAction(m_OtherParticleAction, m_Shared.DrawAction_get());
		SetAction(m_StickerAction, m_Shared.NoDrawAction_get());
		SetAction(m_ErrorMaterialAction, m_Shared.DrawAction_get());
		SetAction(m_OtherAction, m_Shared.DrawAction_get());
		*/

		SetAction(m_ClientEffectTexturesAction, m_Shared.DrawAction_get());
		SetAction(m_WorldTexturesAction, m_Shared.DrawAction_get());
		SetAction(m_SkyBoxTexturesAction, m_Shared.DrawAction_get());
		SetAction(m_StaticPropTexturesAction, m_Shared.DrawAction_get());
		SetAction(m_CableAction,  m_Shared.DrawAction_get());
		SetAction(m_PlayerModelsAction, m_Shared.NoDrawAction_get());
		SetAction(m_WeaponModelsAction, m_Shared.NoDrawAction_get());
		SetAction(m_StatTrakAction, m_Shared.NoDrawAction_get());
		SetAction(m_ShellModelsAction, m_Shared.NoDrawAction_get());
		SetAction(m_OtherModelsAction, m_Shared.DrawAction_get());
		SetAction(m_DecalTexturesAction, m_Shared.DrawAction_get());
		SetAction(m_EffectsAction, m_Shared.DrawAction_get());
		SetAction(m_ShellParticleAction, m_Shared.NoDrawAction_get());
		SetAction(m_OtherParticleAction, m_Shared.DrawAction_get());
		SetAction(m_StickerAction, m_Shared.NoDrawAction_get());
		SetAction(m_ErrorMaterialAction, m_Shared.DrawAction_get());
		SetAction(m_OtherAction, m_Shared.DrawAction_get());
	}

protected:
	virtual ~CAfxMatteWorldStream() {}
};

class CAfxDepthWorldStream
: public CAfxBaseFxStream
{
public:
	CAfxDepthWorldStream() : CAfxBaseFxStream()
	{
		/*
		SetAction(m_ClientEffectTexturesAction, m_Shared.DepthAction_get());
		SetAction(m_WorldTexturesAction, m_Shared.DepthAction_get());
		SetAction(m_SkyBoxTexturesAction, m_Shared.DepthAction_get());
		SetAction(m_StaticPropTexturesAction, m_Shared.DepthAction_get());
		SetAction(m_CableAction, m_Shared.DepthAction_get());
		SetAction(m_PlayerModelsAction, m_Shared.NoDrawAction_get());
		SetAction(m_WeaponModelsAction, m_Shared.NoDrawAction_get());
		SetAction(m_StatTrakAction, m_Shared.NoDrawAction_get());
		SetAction(m_ShellModelsAction, m_Shared.NoDrawAction_get());
		SetAction(m_OtherModelsAction, m_Shared.DepthAction_get());
		SetAction(m_DecalTexturesAction, m_Shared.DepthAction_get());
		SetAction(m_EffectsAction, m_Shared.DepthAction_get());
		SetAction(m_ShellParticleAction, m_Shared.NoDrawAction_get());
		SetAction(m_OtherParticleAction, m_Shared.DepthAction_get());
		SetAction(m_StickerAction, m_Shared.NoDrawAction_get());
		SetAction(m_ErrorMaterialAction, m_Shared.DepthAction_get());
		SetAction(m_OtherAction, m_Shared.DepthAction_get());
		*/

		SetAction(m_ClientEffectTexturesAction, m_Shared.NoDrawAction_get());
		SetAction(m_WorldTexturesAction, m_Shared.DepthAction_get());
		SetAction(m_SkyBoxTexturesAction, m_Shared.DepthAction_get());
		SetAction(m_StaticPropTexturesAction, m_Shared.DepthAction_get());
		SetAction(m_CableAction, m_Shared.NoDrawAction_get());
		SetAction(m_PlayerModelsAction, m_Shared.NoDrawAction_get());
		SetAction(m_WeaponModelsAction, m_Shared.NoDrawAction_get());
		SetAction(m_StatTrakAction, m_Shared.NoDrawAction_get());
		SetAction(m_ShellModelsAction, m_Shared.NoDrawAction_get());
		SetAction(m_OtherModelsAction, m_Shared.DepthAction_get());
		SetAction(m_DecalTexturesAction, m_Shared.NoDrawAction_get());
		SetAction(m_EffectsAction, m_Shared.NoDrawAction_get());
		SetAction(m_ShellParticleAction, m_Shared.NoDrawAction_get());
		SetAction(m_OtherParticleAction, m_Shared.NoDrawAction_get());
		SetAction(m_StickerAction, m_Shared.NoDrawAction_get());
		SetAction(m_ErrorMaterialAction, m_Shared.DepthAction_get());
		SetAction(m_OtherAction, m_Shared.DepthAction_get());
	}

protected:
	virtual ~CAfxDepthWorldStream() {}
};

class CAfxMatteEntityStream
: public CAfxBaseFxStream
{
public:
	CAfxMatteEntityStream() : CAfxBaseFxStream()
	{
		/*
		SetAction(m_ClientEffectTexturesAction, m_Shared.MaskAction_get());
		SetAction(m_WorldTexturesAction, m_Shared.MaskAction_get());
		SetAction(m_SkyBoxTexturesAction, m_Shared.MaskAction_get());
		SetAction(m_StaticPropTexturesAction, m_Shared.MaskAction_get());
		SetAction(m_CableAction, m_Shared.MaskAction_get());
		SetAction(m_PlayerModelsAction, m_Shared.DrawAction_get());
		SetAction(m_WeaponModelsAction, m_Shared.DrawAction_get());
		SetAction(m_StatTrakAction, m_Shared.DrawAction_get());
		SetAction(m_ShellModelsAction, m_Shared.DrawAction_get());
		SetAction(m_OtherModelsAction, m_Shared.MaskAction_get());
		SetAction(m_DecalTexturesAction, m_Shared.MaskAction_get());
		SetAction(m_EffectsAction, m_Shared.MaskAction_get());
		SetAction(m_ShellParticleAction, m_Shared.DrawAction_get());
		SetAction(m_OtherParticleAction, m_Shared.MaskAction_get());
		SetAction(m_StickerAction, m_Shared.DrawAction_get());
		SetAction(m_ErrorMaterialAction, m_Shared.MaskAction_get());
		SetAction(m_OtherAction, m_Shared.MaskAction_get());
		*/

		SetAction(m_ClientEffectTexturesAction, m_Shared.NoDrawAction_get());
		SetAction(m_WorldTexturesAction, m_Shared.MaskAction_get());
		SetAction(m_SkyBoxTexturesAction, m_Shared.MaskAction_get());
		SetAction(m_StaticPropTexturesAction, m_Shared.MaskAction_get());
		SetAction(m_CableAction, m_Shared.NoDrawAction_get());
		SetAction(m_PlayerModelsAction, m_Shared.DrawAction_get());
		SetAction(m_WeaponModelsAction, m_Shared.DrawAction_get());
		SetAction(m_StatTrakAction, m_Shared.DrawAction_get());
		SetAction(m_ShellModelsAction, m_Shared.DrawAction_get());
		SetAction(m_OtherModelsAction, m_Shared.MaskAction_get());
		SetAction(m_DecalTexturesAction, m_Shared.NoDrawAction_get());
		SetAction(m_EffectsAction, m_Shared.NoDrawAction_get());
		SetAction(m_ShellParticleAction, m_Shared.DrawAction_get());
		SetAction(m_OtherParticleAction, m_Shared.NoDrawAction_get());
		SetAction(m_StickerAction, m_Shared.DrawAction_get());
		SetAction(m_ErrorMaterialAction, m_Shared.MaskAction_get());
		SetAction(m_OtherAction, m_Shared.NoDrawAction_get());

		this->SmokeOverlayAlphaFactor_set(0.01f);
		Console_ActionFilter_Add("effects/overlaysmoke", m_Shared.NoDrawAction_get());
	}

protected:
	virtual ~CAfxMatteEntityStream() {}
};

class CAfxDepthEntityStream
: public CAfxDepthStream
{
};

class CAfxAlphaMatteStream
: public CAfxBaseFxStream
{
public:
	CAfxAlphaMatteStream() : CAfxBaseFxStream()
	{
		/*
		SetAction(m_ClientEffectTexturesAction, m_Shared.BlackAction_get());
		SetAction(m_WorldTexturesAction, m_Shared.BlackAction_get());
		SetAction(m_SkyBoxTexturesAction, m_Shared.BlackAction_get());
		SetAction(m_StaticPropTexturesAction, m_Shared.BlackAction_get());
		SetAction(m_CableAction, m_Shared.BlackAction_get());
		SetAction(m_PlayerModelsAction, m_Shared.WhiteAction_get());
		SetAction(m_WeaponModelsAction, m_Shared.WhiteAction_get());
		SetAction(m_StatTrakAction, m_Shared.WhiteAction_get());
		SetAction(m_ShellModelsAction, m_Shared.WhiteAction_get());
		SetAction(m_OtherModelsAction, m_Shared.BlackAction_get());
		SetAction(m_DecalTexturesAction, m_Shared.BlackAction_get());
		SetAction(m_EffectsAction, m_Shared.BlackAction_get());
		SetAction(m_ShellParticleAction, m_Shared.WhiteAction_get());
		SetAction(m_OtherParticleAction, m_Shared.BlackAction_get());
		SetAction(m_StickerAction, m_Shared.WhiteAction_get());
		SetAction(m_ErrorMaterialAction, m_Shared.BlackAction_get());
		SetAction(m_OtherAction, m_Shared.BlackAction_get());
		*/
		SetAction(m_ClientEffectTexturesAction, m_Shared.NoDrawAction_get());
		SetAction(m_WorldTexturesAction, m_Shared.BlackAction_get());
		SetAction(m_SkyBoxTexturesAction, m_Shared.BlackAction_get());
		SetAction(m_StaticPropTexturesAction, m_Shared.BlackAction_get());
		SetAction(m_CableAction, m_Shared.NoDrawAction_get());
		SetAction(m_PlayerModelsAction, m_Shared.WhiteAction_get());
		SetAction(m_WeaponModelsAction, m_Shared.WhiteAction_get());
		SetAction(m_StatTrakAction, m_Shared.WhiteAction_get());
		SetAction(m_ShellModelsAction, m_Shared.WhiteAction_get());
		SetAction(m_OtherModelsAction, m_Shared.BlackAction_get());
		SetAction(m_DecalTexturesAction, m_Shared.NoDrawAction_get());
		SetAction(m_EffectsAction, m_Shared.NoDrawAction_get());
		SetAction(m_ShellParticleAction, m_Shared.NoDrawAction_get());
		SetAction(m_OtherParticleAction, m_Shared.NoDrawAction_get());
		SetAction(m_StickerAction, m_Shared.WhiteAction_get());
		SetAction(m_ErrorMaterialAction, m_Shared.BlackAction_get());
		SetAction(m_OtherAction, m_Shared.BlackAction_get());

		this->SmokeOverlayAlphaFactor_set(0.01f);
		Console_ActionFilter_Add("effects/overlaysmoke", m_Shared.NoDrawAction_get());
	}

protected:
	virtual ~CAfxAlphaMatteStream() {}
};

class CAfxAlphaEntityStream
: public CAfxBaseFxStream
{
public:
	CAfxAlphaEntityStream() : CAfxBaseFxStream()
	{
		SetAction(m_ClientEffectTexturesAction, m_Shared.DrawAction_get());
		SetAction(m_WorldTexturesAction, m_Shared.DrawAction_get());
		SetAction(m_SkyBoxTexturesAction, m_Shared.DrawAction_get());
		SetAction(m_StaticPropTexturesAction, m_Shared.DrawAction_get());
		SetAction(m_CableAction, m_Shared.DrawAction_get());
		SetAction(m_PlayerModelsAction, m_Shared.DrawAction_get());
		SetAction(m_WeaponModelsAction, m_Shared.DrawAction_get());
		SetAction(m_StatTrakAction, m_Shared.DrawAction_get());
		SetAction(m_ShellModelsAction, m_Shared.DrawAction_get());
		SetAction(m_OtherModelsAction, m_Shared.DrawAction_get());
		SetAction(m_DecalTexturesAction, m_Shared.DrawAction_get());
		SetAction(m_EffectsAction, m_Shared.DrawAction_get());
		SetAction(m_ShellParticleAction, m_Shared.DrawAction_get());
		SetAction(m_OtherParticleAction, m_Shared.DrawAction_get());
		SetAction(m_StickerAction, m_Shared.DrawAction_get());
		SetAction(m_ErrorMaterialAction, m_Shared.DrawAction_get());
		SetAction(m_WriteZAction, m_Shared.DrawAction_get());
		SetAction(m_OtherAction, m_Shared.DrawAction_get());
	}

protected:
	virtual ~CAfxAlphaEntityStream() {}
};

class CAfxAlphaWorldStream
: public CAfxBaseFxStream
{
public:
	CAfxAlphaWorldStream() : CAfxBaseFxStream()
	{
		/*
		SetAction(m_ClientEffectTexturesAction, m_Shared.DrawAction_get());
		SetAction(m_WorldTexturesAction, m_Shared.DrawAction_get());
		SetAction(m_SkyBoxTexturesAction, m_Shared.DrawAction_get());
		SetAction(m_StaticPropTexturesAction, m_Shared.DrawAction_get());
		SetAction(m_CableAction, m_Shared.DrawAction_get());
		SetAction(m_PlayerModelsAction, m_Shared.NoDrawAction_get());
		SetAction(m_WeaponModelsAction, m_Shared.NoDrawAction_get());
		SetAction(m_StatTrakAction, m_Shared.NoDrawAction_get());
		SetAction(m_ShellModelsAction,  m_Shared.NoDrawAction_get());
		SetAction(m_OtherModelsAction,  m_Shared.DrawAction_get());
		SetAction(m_DecalTexturesAction, m_Shared.DrawAction_get());
		SetAction(m_EffectsAction, m_Shared.DrawAction_get());
		SetAction(m_ShellParticleAction, m_Shared.NoDrawAction_get());
		SetAction(m_OtherParticleAction, m_Shared.DrawAction_get());
		SetAction(m_StickerAction, m_Shared.NoDrawAction_get());
		SetAction(m_ErrorMaterialAction, m_Shared.DrawAction_get());
		SetAction(m_OtherAction, m_Shared.DrawAction_get());
		*/

		SetAction(m_ClientEffectTexturesAction, m_Shared.DrawAction_get());
		SetAction(m_WorldTexturesAction, m_Shared.DrawAction_get());
		SetAction(m_SkyBoxTexturesAction, m_Shared.DrawAction_get());
		SetAction(m_StaticPropTexturesAction, m_Shared.DrawAction_get());
		SetAction(m_CableAction, m_Shared.DrawAction_get());
		SetAction(m_PlayerModelsAction, m_Shared.NoDrawAction_get());
		SetAction(m_WeaponModelsAction, m_Shared.NoDrawAction_get());
		SetAction(m_StatTrakAction, m_Shared.NoDrawAction_get());
		SetAction(m_ShellModelsAction, m_Shared.NoDrawAction_get());
		SetAction(m_OtherModelsAction, m_Shared.DrawAction_get());
		SetAction(m_DecalTexturesAction, m_Shared.DrawAction_get());
		SetAction(m_EffectsAction, m_Shared.DrawAction_get());
		SetAction(m_ShellParticleAction, m_Shared.DrawAction_get());
		SetAction(m_OtherParticleAction, m_Shared.DrawAction_get());
		SetAction(m_StickerAction, m_Shared.NoDrawAction_get());
		SetAction(m_ErrorMaterialAction, m_Shared.DrawAction_get());
		SetAction(m_OtherAction, m_Shared.DrawAction_get());
	}

protected:
	virtual ~CAfxAlphaWorldStream() {}
};

class CAfxFileTracker
{
public:
	void TrackFile(char const * filePath);

	void WaitForFiles(unsigned int maxUnfinishedFiles);

private:
	std::queue<std::string> m_FilePaths;

};

class CAfxStreams
: public IAfxBaseClientDllLevelShutdown
, public IAfxBaseClientDllView_Render
{
public:
	CAfxStreams();
	~CAfxStreams();

	void OnMaterialSystem(SOURCESDK::IMaterialSystem_csgo * value);
	void OnAfxBaseClientDll(IAfxBaseClientDll * value);
	void OnShaderShadow(SOURCESDK::IShaderShadow_csgo * value);

	/// <remarks>This function can be called from diffrent threads, but only one thread at a time.</remarks>
	void OnSetVertexShader(CAfx_csgo_ShaderState & state);

	/// <remarks>This function can be called from diffrent threads, but only one thread at a time.</remarks>
	void OnSetPixelShader(CAfx_csgo_ShaderState & state);

	void OnDrawingHud(void);

	void OnDrawingSkyBoxViewBegin(void);

	void OnDrawingSkyBoxViewEnd(void);

	void Console_RecordName_set(const char * value);
	const char * Console_RecordName_get();

	void Console_PresentRecordOnScreen_set(bool value);
	bool Console_PresentRecordOnScreen_get();

	void Console_StartMovieWav_set(bool value);
	bool Console_StartMovieWav_get();	

	void Console_MatForceTonemapScale_set(float value);
	float Console_MatForceTonemapScale_get();

	void Console_RecordFormat_set(const char * value);
	const char * Console_RecordFormat_get();

	void Console_Record_Start();
	void Console_Record_End();
	void Console_AddStream(const char * streamName);
	void Console_AddBaseFxStream(const char * streamName);
	void Console_AddDepthStream(const char * streamName);
	void Console_AddMatteWorldStream(const char * streamName);
	void Console_AddDepthWorldStream(const char * streamName);
	void Console_AddMatteEntityStream(const char * streamName);
	void Console_AddDepthEntityStream(const char * streamName);
	void Console_AddAlphaMatteStream(const char * streamName);
	void Console_AddAlphaEntityStream(const char * streamName);
	void Console_AddAlphaWorldStream(const char * streamName);
	void Console_AddAlphaMatteEntityStream(const char * streamName);
	void Console_PrintStreams();
	void Console_RemoveStream(const char * streamName);
	void Console_EditStream(const char * streamName, IWrpCommandArgs * args);
	void Console_ListActions(void);
	void Console_Bvh(IWrpCommandArgs * args);
	void Console_GameRecording(IWrpCommandArgs * args);

	/// <param name="index">stream name to preview or empty string if to preview nothing.</param>
	void Console_PreviewStream(const char * streamName);

	void DebugDump(IAfxMatRenderContextOrg * ctxp);

	virtual SOURCESDK::IMaterialSystem_csgo * GetMaterialSystem(void);
	virtual IAfxFreeMaster * GetFreeMaster(void);
	virtual SOURCESDK::IShaderShadow_csgo * GetShaderShadow(void);

	virtual std::wstring GetTakeDir(void);

	virtual void LevelShutdown(IAfxBaseClientDll * cl);

	virtual void View_Render(IAfxBaseClientDll * cl, SOURCESDK::vrect_t_csgo *rect);

private:
	class CFreeDelegate : public IAfxFreeable
	{
	public:
		CFreeDelegate(IAfxFreeMaster * freeMaster, CAfxStreams * classPtr, void (CAfxStreams::*classFn)(void))
		: m_FreeMaster(freeMaster)
		, m_ClassPtr(classPtr)
		, m_ClassFn(classFn)
		{
			m_FreeMaster->AfxFreeable_Register(this);
		}

		~CFreeDelegate()
		{
			AfxFree();
		}

		virtual void AfxFree(void)
		{
			if(m_ClassPtr) { (m_ClassPtr->*m_ClassFn)(); m_ClassPtr = 0; }
			if(m_FreeMaster) { m_FreeMaster->AfxFreeable_Unregister(this); m_FreeMaster = 0; }
		}

	private:
		IAfxFreeMaster * m_FreeMaster;
		CAfxStreams * m_ClassPtr;
		void (CAfxStreams::*m_ClassFn)(void);

	};

	class CImageBuffer
	{
	public:
		enum ImageBufferPixelFormat
		{
			IBPF_BGR,
			IBPF_BGRA,
			IBPF_A,
			IBPF_ZFloat
		};

		void * Buffer;

		ImageBufferPixelFormat PixelFormat;
		int Width;
		int Height;
		size_t ImagePitch;
		size_t ImageBytes;

		CImageBuffer();
		~CImageBuffer();

		bool AutoRealloc(ImageBufferPixelFormat pixelFormat, int width, int height);

	private:
		size_t m_BufferBytesAllocated;
	};

	class CEntityBvhCapture
	{
	public:
		enum Origin_e {
			O_Net,
			O_View
		};

		enum Angles_e {
			A_Net,
			A_View
		};

		CEntityBvhCapture(int entityIndex, Origin_e origin, Angles_e angles);
		~CEntityBvhCapture();

		void StartCapture(std::wstring const & takePath, double frameTime);
		void EndCapture(void);

		void CaptureFrame(void);

		int EntityIndex_get(void) { return m_EntityIndex; }
		Origin_e Origin_get(void) { return m_Origin; }
		Angles_e Angles_get(void) { return m_Angles; }

	private:
		Origin_e m_Origin;
		Angles_e m_Angles;
		int m_EntityIndex;
		BvhExport * m_BvhExport;
	};

	std::string m_RecordName;
	bool m_PresentRecordOnScreen;
	bool m_StartMovieWav;
	bool m_StartMovieWavUsed;
	CFreeDelegate * m_OnAfxBaseClientDll_Free;
	SOURCESDK::IMaterialSystem_csgo * m_MaterialSystem;
	IAfxBaseClientDll * m_AfxBaseClientDll;
	SOURCESDK::IShaderShadow_csgo * m_ShaderShadow;
	std::list<CAfxRecordStream *> m_Streams;
	CAfxRecordStream * m_PreviewStream;
	bool m_Recording;
	int m_Frame;
	bool m_CamBvh;
	std::list<CEntityBvhCapture *> m_EntityBvhCaptures;
	bool m_GameRecording;
	std::mutex m_ContextToStreamMutex;

	WrpConVarRef * m_HostFrameRate;
	int m_OldMatQueueMode;
	WrpConVarRef * m_MatPostProcessEnableRef;
	int m_OldMatPostProcessEnable;
	WrpConVarRef * m_MatDynamicTonemappingRef;
	int m_OldMatDynamicTonemapping;
	WrpConVarRef * m_MatMotionBlurEnabledRef;
	int m_OldMatMotionBlurEnabled;
	WrpConVarRef * m_MatForceTonemapScale;
	float m_OldMatForceTonemapScale;
	float m_NewMatForceTonemapScale;
	float m_OriginalColorModulation[4];
	bool m_ColorModulationOverride;
	bool m_BlendOverride;
	float m_OverrideColor[4];
	CImageBuffer m_BufferA;
	CImageBuffer m_BufferB;
	std::wstring m_TakeDir;
	bool m_FormatBmpAndNotTga;
	//ITexture_csgo * m_RgbaRenderTarget;
	SOURCESDK::ITexture_csgo * m_RenderTargetDepthF;
	//CAfxMaterial * m_ShowzMaterial;
	DWORD m_Current_View_Render_ThreadId;

	void SetCurrent_View_Render_ThreadId(DWORD id);

	DWORD GetCurrent_View_Render_ThreadId();

	void OnAfxBaseClientDll_Free(void);

	void Console_EditStream(CAfxStream * stream, IWrpCommandArgs * args);

	void Console_EditStream(CAfxRenderViewStream * stream, IWrpCommandArgs * args);

	bool Console_CheckStreamName(char const * value);

	bool Console_ToAfxAction(char const * value, CAfxBaseFxStream::CAction * & action);
	char const * Console_FromAfxAction(CAfxBaseFxStream::CAction * action);

	bool Console_ToStreamCombineType(char const * value, CAfxTwinStream::StreamCombineType & streamCombineType);
	char const * Console_FromStreamCombineType(CAfxTwinStream::StreamCombineType streamCombineType);

	bool Console_ToStreamCaptureType(char const * value, CAfxRenderViewStream::StreamCaptureType & StreamCaptureType);
	char const * Console_FromStreamCaptureType(CAfxRenderViewStream::StreamCaptureType StreamCaptureType);

	bool CheckCanFeedStreams(void);

	void BackUpMatVars();
	void SetMatVarsForStreams();
	void RestoreMatVars();
	void EnsureMatVars();

	void AddStream(CAfxRecordStream * stream);

	void CreateRenderTargets(SOURCESDK::IMaterialSystem_csgo * materialSystem);

	bool CaptureStreamToBuffer(CAfxRenderViewStream * stream, CImageBuffer & buffer, IAfxMatRenderContextOrg * ctxp, bool isInPreview);

	bool WriteBufferToFile(const CImageBuffer & buffer, const std::wstring & path, bool ifZip);

	IAfxContextHook * FindHook(IAfxMatRenderContext * ctx);
};

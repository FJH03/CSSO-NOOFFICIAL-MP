//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef BASEMODELPANEL_H
#define BASEMODELPANEL_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui/IScheme.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/EditablePanel.h>
#include "GameEventListener.h"
#include "KeyValues.h"

class C_SceneEntity;


class CModelPanelModel : public C_BaseFlex
{
public:
	CModelPanelModel(){}
	DECLARE_CLASS( CModelPanelModel, C_BaseFlex );

	virtual bool IsMenuModel() const{ return true; }
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CModelPanelModelAnimation
{
public:
	CModelPanelModelAnimation()
	{
		m_pszName = NULL;
		m_pszSequence = NULL;
		m_pszActivity = NULL;
		m_pPoseParameters = NULL;
		m_bDefault = false;
	}

	~CModelPanelModelAnimation()
	{
		if ( m_pszName && m_pszName[0] )
		{
			delete [] m_pszName;
			m_pszName = NULL;
		}

		if ( m_pszSequence && m_pszSequence[0] )
		{
			delete [] m_pszSequence;
			m_pszSequence = NULL;
		}

		if ( m_pszActivity && m_pszActivity[0] )
		{
			delete [] m_pszActivity;
			m_pszActivity = NULL;
		}

		if ( m_pPoseParameters )
		{
			m_pPoseParameters->deleteThis();
			m_pPoseParameters = NULL;
		}
	}

public:
	const char	*m_pszName;
	const char	*m_pszSequence;
	const char	*m_pszActivity;
	KeyValues	*m_pPoseParameters;
	bool		m_bDefault;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CModelPanelAttachedModelInfo
{
public:
	CModelPanelAttachedModelInfo()
	{
		m_pszModelName = NULL;
		m_nSkin = 0;
	}

	~CModelPanelAttachedModelInfo()
	{
		if ( m_pszModelName && m_pszModelName[0] )
		{
			delete [] m_pszModelName;
			m_pszModelName = NULL;
		}
	}

public:
	const char	*m_pszModelName;
	int			m_nSkin;
};

#define MATERIAL_MAX_LIGHT_COUNT 4
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CModelPanelModelInfo
{
public:
	CModelPanelModelInfo()
		: m_mapBodygroupValues( DefLessFunc( int ) )
	{
		m_pszModelName = NULL;
		m_pszModelName_HWM = NULL;
		m_nSkin = -1;
		m_vecAbsAngles.Init();
		m_vecOriginOffset.Init();
		m_vecFramedOriginOffset.Init();
		m_nNumLightDescs = 0;
		m_vecAmbientLight.Init( 0.4f, 0.4f, 0.4f );
	}

	~CModelPanelModelInfo()
	{
		if ( m_pszModelName && m_pszModelName[0] )
		{
			delete [] m_pszModelName;
			m_pszModelName = NULL;
		}

		if ( m_pszModelName_HWM && m_pszModelName_HWM[0] )
		{
			delete [] m_pszModelName_HWM;
			m_pszModelName_HWM = NULL;
		}

		if ( m_pszVCD && m_pszVCD[0] )
		{
			delete [] m_pszVCD;
			m_pszVCD = NULL;
		}

		m_Animations.PurgeAndDeleteElements();
		m_AttachedModelsInfo.PurgeAndDeleteElements();
	}

public:
	const char	*m_pszModelName;
	const char	*m_pszModelName_HWM;
	int			m_nSkin;
	const char	*m_pszVCD;
	Vector		m_vecAbsAngles;
	Vector		m_vecOriginOffset;
	Vector2D	m_vecViewportOffset;
	Vector		m_vecFramedOriginOffset;
	CUtlMap< int, int > m_mapBodygroupValues;
	LightDesc_t m_pLightDesc[MATERIAL_MAX_LIGHT_COUNT];
	int			m_nNumLightDescs;
	Vector		m_vecAmbientLight;

	CUtlVector<CModelPanelModelAnimation*>		m_Animations;
	CUtlVector<CModelPanelAttachedModelInfo*>	m_AttachedModelsInfo;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CModelPanel: public vgui::EditablePanel, public CGameEventListener
{
//	typedef vgui::EditablePanel BaseClass;
public:
	DECLARE_CLASS_SIMPLE( CModelPanel, vgui::EditablePanel );

	CModelPanel( vgui::Panel *parent, const char *name );
	virtual ~CModelPanel();

	virtual void Paint();
	virtual void ApplySettings( KeyValues *inResourceData );
	virtual void OnCommand( const char *command ) OVERRIDE;
	virtual void DeleteVCDData( void );
	virtual void DeleteModelData( void );

	virtual void SetFOV( int nFOV ){ m_nFOV = nFOV; }
	virtual void SetPanelDirty( void ){ m_bPanelDirty = true; }
	virtual bool SetSequence( const char *pszSequence );
	virtual void SetSkin( int nSkin );
	void SetBodyGroup( const char* pszBodyGroupName, int nGroup );

	MESSAGE_FUNC_PARAMS( OnAddAnimation, "AddAnimation", data );
	MESSAGE_FUNC_PARAMS( OnSetAnimation, "SetAnimation", data );

	// Manipulation.
	virtual void OnMousePressed ( vgui::MouseCode code );
	virtual void OnMouseReleased( vgui::MouseCode code );
	virtual void OnCursorMoved( int x, int y );

	void		RotateYaw( float flDelta );
	void		RotatePitch( float flDelta );

	void	SetDefaultAnimation( const char *pszName );
	void	SwapModel( const char *pszName, const char *pszAttached = NULL );

	virtual void ParseModelInfo( KeyValues *inResourceData );
	void ParseLightInfo( KeyValues *inResourceData );

	void		ClearAttachedModelInfos( void );

	void		CalculateFrameDistance( void );
	void		ZoomToFrameDistance( void );

	void		UpdateModel();
public: // IGameEventListener:
	virtual void FireGameEvent( IGameEvent * event );

protected:
	virtual void SetupModel( void );
	virtual void SetupVCD( void );
	virtual const char *GetModelName( void );

private:
	void InitCubeMaps();
	int FindAnimByName( const char *pszName );
	void CalculateFrameDistanceInternal( const model_t *pModel );

	void EnableMouseCapture( bool enable, vgui::MouseCode code = vgui::MouseCode( -1 ) );
	bool WarpMouse( int &x, int &y );

public:
	int								m_nFOV;
	float							m_flFrameDistance;
	bool							m_bStartFramed;
	CModelPanelModelInfo			*m_pModelInfo;
	Vector							m_vecCameraPos;
	QAngle							m_angCameraAng;

	CHandle<CModelPanelModel>				m_hModel;
	CUtlVector<CHandle<C_BaseAnimating> >	m_AttachedModels;

	CHandle<C_SceneEntity>			m_hScene;

private:
	bool	m_bPanelDirty;
	int		m_iDefaultAnimation;

	bool	m_bAllowOffscreen;

	CTextureReference m_DefaultEnvCubemap;
	CTextureReference m_DefaultHDREnvCubemap;

	int m_nClickStartX;
	int m_nClickStartY;
	int m_nManipStartX;
	int m_nManipStartY;
	bool m_bMousePressed;
	vgui::MouseCode m_nCaptureMouseCode;
	int m_xoffset, m_yoffset;
	bool m_bAllowRotation;
	bool m_bAllowPitch;

	CPanelAnimationVar( float, m_flMaxPitch, "max_pitch", "90" );
};


#endif // BASEMODELPANEL_H
//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef CS_CLIENTMODE_H
#define CS_CLIENTMODE_H
#ifdef _WIN32
#pragma once
#endif

#include "clientmode_shared.h"
#include "counterstrikeviewport.h"
#include "colorcorrectionmgr.h"
#include "c_cs_player.h"

enum PostProcessEffect_t
{
	POST_EFFECT_DEFAULT = 0,
	POST_EFFECT_LOW_HEATH,			// 1st person	event
	POST_EFFECT_VERY_LOW_HEATH,
	POST_EFFECT_IN_BUY_MENU,		// 1st person	event
	POST_EFFECT_DEATH_CAM,				// 1st person	event
	POST_EFFECT_SPECTATING,					// 1st person	state
	POST_EFFECT_IN_FIRE,
	POST_EFFECT_ZOOMED_RIFLE,
	POST_EFFECT_ZOOMED_SNIPER,
	POST_EFFECT_ZOOMED_SNIPER_MOVING,
	POST_EFFECT_UNDER_WATER,
	POST_EFFECT_ROUND_END_VIA_BOMBING,
	POST_EFFECT_SPEC_CAMERA_LERPING,
	POST_EFFECT_MAP_CONTROLLED,
	POST_EFFECT_DEATH_CAM_BODYSHOT,				// 1st person	event
	POST_EFFECT_DEATH_CAM_HEADSHOT,				// 1st person	event
	NUM_POST_EFFECTS
};

enum RoundStatus_t
{
	ROUND_UNKNOWN = 0,
	ROUND_STARTED,
	ROUND_ENDED,
	ROUND_ENDED_VIA_BOMBING
};

struct PostProcessEffectParams_t
{
	float fLocalContrastStrength;
	float fLocalContrastEdgeStrength;
	float fVignetteStart;
	float fVignetteEnd;
	float fVignetteBlurStrength;
	float fFadeToBlackStrength;
};

class ClientModeCSNormal : public ClientModeShared 
{
DECLARE_CLASS( ClientModeCSNormal, ClientModeShared );

private:

// IClientMode overrides.
public:

					ClientModeCSNormal();

	virtual void	Init();
	virtual void	InitViewport();
	virtual void	Update();

	virtual void	LevelShutdown( void );

	virtual int		KeyInput( int down, ButtonCode_t keynum, const char *pszCurrentBinding );

	virtual float	GetViewModelFOV( void );

	virtual void	FireGameEvent( IGameEvent *event );

	virtual bool	ShouldDrawViewModel( void );

	virtual bool	CanRecordDemo( char *errorMsg, int length ) const;

	// [menglish] Save server information shown to the client in a persistent place
	virtual wchar_t* GetServerName() { return m_pServerName; }
	virtual void SetServerName(wchar_t* name);
	virtual wchar_t* GetMapName() { return m_pMapName; }
	virtual void SetMapName(wchar_t* name);
	
	virtual void	DoPostScreenSpaceEffects( const CViewSetup *pSetup );

	virtual void	UpdateColorCorrectionWeights( void );
	virtual void	OnColorCorrectionWeightsReset( void );

	void			LoadPostProcessParamsFromFile( const char* pFileName = NULL );
	void			UpdatePostProcessingEffects();
	void			PostProcessLerpTo( PostProcessEffect_t effectID, float fFadeDuration = 0.75f, const PostProcessParameters_t* pTargetParams = NULL );
	void			PostProcessLerpTo( PostProcessEffect_t effectID, const C_PostProcessController* pPPController );
	void			DoPostProcessParamLerp();
	void			LerpPostProcessParam(	float fAmount, PostProcessParameters_t& result, const PostProcessParameters_t& from,
		const PostProcessParameters_t& to ) const;
	PostProcessEffect_t PostProcessEffectFromName( const char* pName ) const;
	void			GetDefaultPostProcessingParams( C_CSPlayer* pPlayer, PostProcessEffectParams_t* pParams );

	virtual void	OverrideView( CViewSetup* pSetup );

private:
	wchar_t			m_pServerName[256];
	wchar_t			m_pMapName[256];
	
	ClientCCHandle_t	m_CCDeathHandle;	// handle to death cc effect
	float				m_CCDeathPercent;
	ClientCCHandle_t	m_CCFreezePeriodHandle_CT;
	float				m_CCFreezePeriodPercent_CT;
	ClientCCHandle_t	m_CCFreezePeriodHandle_T;
	float				m_CCFreezePeriodPercent_T;

	RoundStatus_t		m_iRoundStatus;

	float				m_fDelayedCTWinTime;

	static PostProcessParameters_t ms_postProcessParams[];
	static const char* ms_postProcessEffectNames[];
	PostProcessEffect_t m_activePostProcessEffect;
	PostProcessEffect_t m_lastPostProcessEffect;
	const C_PostProcessController* m_pActivePostProcessController;
	CountdownTimer m_postProcessEffectCountdown;
	PostProcessParameters_t m_postProcessLerpStartParams;
	PostProcessParameters_t m_postProcessLerpEndParams;
	PostProcessParameters_t m_postProcessCurrentParams;

	int m_nRoundMVP;
};


extern IClientMode *GetClientModeNormal();
extern ClientModeCSNormal* GetClientModeCSNormal();


#endif // CS_CLIENTMODE_H

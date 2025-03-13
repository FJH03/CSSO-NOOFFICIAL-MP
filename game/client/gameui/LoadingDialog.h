//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef LOADINGDIALOG_H
#define LOADINGDIALOG_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include <vgui_controls/HTML.h>
#include "GameEventListener.h"

//-----------------------------------------------------------------------------
// Purpose: Dialog for displaying level loading status
//-----------------------------------------------------------------------------
class CLoadingDialog : public vgui::Frame, public CGameEventListener
{
	DECLARE_CLASS_SIMPLE( CLoadingDialog, vgui::Frame ); 
public:
	CLoadingDialog( vgui::Panel *parent );
	~CLoadingDialog();

	void Open();
	bool SetProgressPoint(float fraction);
	void SetStatusText(const char *statusText);
	void SetSecondaryProgress(float progress);
	void SetSecondaryProgressText(const char *statusText);
	bool SetShowProgressText( bool show );

	void DisplayGenericError(const char *failureReason, const char *extendedReason = NULL);
	void DisplayVACBannedError();
	void DisplayNoSteamConnectionError();
	void DisplayLoggedInElsewhereError();

	// IGameEventListener
	virtual void FireGameEvent( IGameEvent* event );
protected:
	virtual void OnCommand(const char *command);
	virtual void PerformLayout();
	virtual void OnThink();
	virtual void OnClose();
	virtual void OnKeyCodeTyped(vgui::KeyCode code);
	virtual void OnKeyCodePressed(vgui::KeyCode code);
	virtual void PaintBackground( void );
	void SetupControlSettings();
private:
	void SetupControlSettings( bool bForceShowProgressText );
	void SetupControlSettingsForErrorDisplay( const char *settingsFile );
	void HideOtherDialogs( bool bHide );
	void SetMapName( const char* mapname );

	vgui::ProgressBar	*m_pProgress;
	vgui::ProgressBar	*m_pProgress2;
	vgui::Label			*m_pInfoLabel;
	vgui::Label			*m_pTimeRemainingLabel;
	vgui::Button		*m_pCancelButton;
	vgui::Panel			*m_pLoadingBackground;
	vgui::Label			*m_pMapNameLabel;
	vgui::ImagePanel	*m_pMapImage;
	vgui::Label			*m_pGameModeNameLabel;
	vgui::Label			*m_pGameModeDescriptionLabel;

	bool	m_bShowingSecondaryProgress;
	float	m_flSecondaryProgress;
	float	m_flLastSecondaryProgressUpdateTime;
	float	m_flSecondaryProgressStartTime;
	bool	m_bCenter;
	bool	m_bConsoleStyle;
	float	m_flProgressFraction;

	char m_szMapName[64];
	int m_iGameMode;
	bool m_bMapNameChanged;
	CPanelAnimationVar( int, m_iAdditionalIndentX, "AdditionalIndentX", "0" );
	CPanelAnimationVar( int, m_iAdditionalIndentY, "AdditionalIndentY", "0" );
};

// singleton accessor
CLoadingDialog *LoadingDialog();


#endif // LOADINGDIALOG_H

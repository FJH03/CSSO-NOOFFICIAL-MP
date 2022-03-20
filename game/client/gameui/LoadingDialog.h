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

	void SetExtendedServerInfo( KeyValues* pExtendedServerInfo );
	void ResetExtendedServerInfo();

protected:
	virtual void OnCommand(const char *command);
	virtual void PerformLayout();
	virtual void OnThink();
	virtual void OnClose();
	virtual void OnKeyCodeTyped(vgui::KeyCode code);
	virtual void OnKeyCodePressed(vgui::KeyCode code);
	virtual void PaintBackground( void );
	
private:
	void SetupControlSettings();
	void SetupControlSettingsForErrorDisplay( const char *settingsFile );
	void HideOtherDialogs( bool bHide );

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
	bool	m_bExtendedServerInfoLoaded;

	CPanelAnimationVar( int, m_iAdditionalIndentX, "AdditionalIndentX", "0" );
	CPanelAnimationVar( int, m_iAdditionalIndentY, "AdditionalIndentY", "0" );
};

// singleton accessor
CLoadingDialog *LoadingDialog();


#endif // LOADINGDIALOG_H

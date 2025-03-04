//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef CSTRIKEROTATINGPLAYERPANEL_H
#define CSTRIKEROTATINGPLAYERPANEL_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>

class CCSRotatingPlayerImagePanel: public vgui::Panel
{
	typedef vgui::Panel BaseClass;

public:
	CCSRotatingPlayerImagePanel( vgui::Panel *pParent, const char *pName );

	// Manipulation.
	virtual void OnMousePressed ( vgui::MouseCode code );
	virtual void OnMouseReleased( vgui::MouseCode code );
	virtual void OnCursorMoved( int x, int y );

public:
	float m_flModelYaw; // sent to UpdateImageEntity to rotate the model

private:
	void EnableMouseCapture( bool enable, vgui::MouseCode code = vgui::MouseCode( -1 ) );
	bool WarpMouse( int &x, int &y );
	void RotateYaw( float flDelta );

	int m_nClickStartX;
	int m_nClickStartY;
	int m_nManipStartX;
	int m_nManipStartY;
	bool m_bMousePressed;
	vgui::MouseCode m_nCaptureMouseCode;
	int m_xoffset, m_yoffset;
};

#endif // CSTRIKEBUYMENU_H

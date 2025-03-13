//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef VECTORIMAGEPANEL_H
#define VECTORIMAGEPANEL_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Panel.h>

struct Bitmap_t;

namespace vgui
{
//-----------------------------------------------------------------------------
// Purpose: A VGUI panel that renders SVG images
//-----------------------------------------------------------------------------
class VectorImagePanel : public Panel
{
	typedef Panel BaseClass;
public:
	VectorImagePanel( Panel *parent, const char *panelName );
	virtual ~VectorImagePanel();

public:
	virtual void ApplySettings( KeyValues *inResourceData );
	virtual void Paint();

	void SetTexture( const char *szFilePath );
	void SetRepeatsCount( int repeats ) { m_nRepeatsCount = repeats; }
	void DestroyTexture();
	void SetRenderSize( int wide, int tall );
	void SetMirrorX( bool state );
	void SetMirrorY( bool state );

private:
	int m_nTextureId;
	int m_iRenderSize[2];
	int m_iRepeatMargin[2];
	int m_nRepeatsCount; // how many times we need to render it over and over?
	bool m_bMirrorX;
	bool m_bMirrorY;
	float texCoords[4]; // s0, t0, s1, t1
};

}

#endif // VECTORIMAGEPANEL_H
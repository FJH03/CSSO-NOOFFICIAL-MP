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

private:
	int	m_nTextureId;
};

}

#endif // VECTORIMAGEPANEL_H
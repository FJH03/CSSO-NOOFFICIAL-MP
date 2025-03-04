//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include <vgui_controls/Panel.h>
#include <vgui_controls/VectorImagePanel.h>
#include <vgui/ISurface.h>
#include <bitmap/bitmap.h>
#include <KeyValues.h>
#include "filesystem.h"
#include "VGuiMatSurface/IMatSystemSurface.h"

#include "lunasvg/lunasvg.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
using namespace lunasvg;

DECLARE_BUILD_FACTORY( VectorImagePanel );

//-----------------------------------------------------------------------------
// Purpose: Check box image
//-----------------------------------------------------------------------------
VectorImagePanel::VectorImagePanel( Panel *parent, const char *name ): Panel( parent, name )
{
	m_nTextureId = -1;
	m_iRenderSize[0] = m_iRenderSize[1] = 0;
}

VectorImagePanel::~VectorImagePanel()
{
	DestroyTexture();
}

void VectorImagePanel::SetTexture( const char *szFilePath )
{
	DestroyTexture();

	char szFullPath[MAX_PATH];
	g_pFullFileSystem->RelativePathToFullPath( szFilePath, "MOD", szFullPath, sizeof( szFullPath ) );

	std::unique_ptr<Document> document = Document::loadFromFile( szFullPath ); // load the svg

	if ( !document )
	{
		Warning( "VectorImagePanel: %s failed to load file \"%s\".\n", GetName(), szFilePath );
		DestroyTexture();
		return;
	}

	Bitmap bitmap = document->renderToBitmap( m_iRenderSize[0], m_iRenderSize[1] ); // render the svg

	if ( !bitmap.valid() )
	{
		Warning( "VectorImagePanel: %s failed to render file \"%s\".\n", GetName(), szFilePath );
		DestroyTexture();
		return;
	}

	if ( m_nTextureId == -1 )
	{
		m_nTextureId = vgui::surface()->CreateNewTextureID( true );
	}

	int wide = bitmap.width();
	int tall = bitmap.height();
	SetSize( wide, tall );
	vgui::surface()->DrawSetTextureRGBA( m_nTextureId, bitmap.data(), wide, tall, 1, true );
}

void VectorImagePanel::DestroyTexture()
{
	if ( m_nTextureId != -1 )
	{
		vgui::surface()->DestroyTextureID( m_nTextureId );
		m_nTextureId = -1;
	}
}

void VectorImagePanel::ApplySettings( KeyValues *inResourceData )
{
	BaseClass::ApplySettings( inResourceData );

	GetSize( m_iRenderSize[0], m_iRenderSize[1] ); // cache the original panel size since its changed in SetTexture below

	const char *szSVGPath = inResourceData->GetString( "image", NULL );
	if ( szSVGPath )
	{
		SetTexture( szSVGPath );
	}

	m_iRepeatMargin[0] = inResourceData->GetInt( "repeat_xpos", 0 );
	m_iRepeatMargin[1] = inResourceData->GetInt( "repeat_ypos", 0 );
	m_nRepeatsCount = inResourceData->GetInt( "repeats_count", 1 );
}

void VectorImagePanel::Paint()
{
	if ( m_nTextureId == -1 )
		return;

	int wide, tall;
	vgui::surface()->DrawGetTextureSize( m_nTextureId, wide, tall );

	vgui::surface()->DrawSetTexture( m_nTextureId );
	vgui::surface()->DrawSetColor( GetFgColor() );

	g_pMatSystemSurface->DisableClipping( true );
	for ( int i = 0; i < m_nRepeatsCount; i++ )
	{
		vgui::surface()->DrawTexturedRect( m_iRepeatMargin[0] * i, m_iRepeatMargin[1] * i, (m_iRepeatMargin[0] * i) + wide, (m_iRepeatMargin[1] * i) + tall);
	}
	g_pMatSystemSurface->DisableClipping( false );
}
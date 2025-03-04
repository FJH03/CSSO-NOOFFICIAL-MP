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
	m_iRepeatMargin[0] = m_iRepeatMargin[1] = 0;
	m_nRepeatsCount = 1;
	m_bMirrorX = false;
	m_bMirrorY = false;
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

void VectorImagePanel::SetRenderSize( int wide, int tall )
{
	m_iRenderSize[0] = wide;
	m_iRenderSize[1] = tall;
	SetSize( wide, tall );
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

	int alignScreenWide, alignScreenTall;
	surface()->GetScreenSize( alignScreenWide, alignScreenTall );
	ComputePos( this, inResourceData->GetString( "repeat_xpos", NULL ), m_iRepeatMargin[0], m_iRenderSize[0],
				alignScreenWide, m_iBaseResolutionOverride[0], m_iBaseResolutionOverride[1], true, OP_SET );
	ComputePos( this, inResourceData->GetString( "repeat_ypos", NULL ), m_iRepeatMargin[1], m_iRenderSize[1],
				alignScreenTall, m_iBaseResolutionOverride[0], m_iBaseResolutionOverride[1], false, OP_SET );
	m_nRepeatsCount = inResourceData->GetInt( "repeats_count", 1 );

	m_bMirrorX = inResourceData->GetBool( "mirror_x" );
	m_bMirrorY = inResourceData->GetBool( "mirror_y" );
}

void VectorImagePanel::Paint()
{
	if ( m_nTextureId == -1 )
		return;

	int wide, tall, textureWide, textureTall;
	GetSize( wide, tall );
	vgui::surface()->DrawGetTextureSize( m_nTextureId, textureWide, textureTall );

	vgui::surface()->DrawSetTexture( m_nTextureId );
	vgui::surface()->DrawSetColor( GetFgColor() );

	g_pMatSystemSurface->DisableClipping( true );
	for ( int i = 0; i < m_nRepeatsCount; i++ )
	{
		// PiMoN TODO: this is some brutal hackery to only get the needed rect size to render instead of the
		// entire texture which often might have lots of empty space that sometimes can instead be emo-pattern
		// tldr: source needs power-of-2 texture so for that it fills the empty space either with nothing (good)
		// or emo pattern (bad) and I couldnt understand why
		int x0 = m_iRepeatMargin[0] * i;
		int x1 = x0 + wide;
		int y0 = m_iRepeatMargin[1] * i;
		int y1 = y0 + tall;
		float texs0 = m_bMirrorX ? (float)wide / (float)textureWide : 0.0f; // xpos / texture wide, always 0
		float text0 = m_bMirrorY ? (float)tall / (float)textureTall : 0.0f; // ypos / texture tall, always 0
		float texs1 = m_bMirrorX ? 0.0f : (float)wide / (float)textureWide;
		float text1 = m_bMirrorY ? 0.0f : (float)tall / (float)textureTall;
		vgui::surface()->DrawTexturedSubRect( x0, y0, x1, y1, texs0, text0, texs1, text1 );
	}
	g_pMatSystemSurface->DisableClipping( false );
}
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
#include "tier2/fileutils.h"

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
	// don't even bother doing anything without a file
	if ( !szFilePath )
		return;

	DestroyTexture();

	FileHandle_t f = g_pFullFileSystem->Open( szFilePath, "rt" );
	if ( !f )
	{
		Warning( "VectorImagePanel: %s failed to open file \"%s\".\n", GetName(), szFilePath );
		return;
	}

	// read the whole thing into memory
	int size = g_pFullFileSystem->Size( f );
	// read into temporary memory block
	int nBufSize = size + 1;
	if ( IsXbox() )
	{
		nBufSize = AlignValue( nBufSize, 512 );
	}
	char *pMem = (char *) malloc( nBufSize );
	int bytesRead = g_pFullFileSystem->ReadEx( pMem, nBufSize, size, f );
	Assert( bytesRead <= size );
	pMem[bytesRead] = 0;
	g_pFullFileSystem->Close( f );
	std::unique_ptr<Document> document = Document::loadFromData( pMem ); // load the svg
	free( pMem );

	if ( !document )
	{
		Warning( "VectorImagePanel: %s failed to load file \"%s\".\n", GetName(), szFilePath );
		return;
	}

	Bitmap bitmap = document->renderToBitmap( m_iRenderSize[0], m_iRenderSize[1] ); // render the svg

	if ( !bitmap.valid() )
	{
		Warning( "VectorImagePanel: %s failed to render file \"%s\".\n", GetName(), szFilePath );
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

	int textureWide, textureTall;
	vgui::surface()->DrawGetTextureSize( m_nTextureId, textureWide, textureTall );

	texCoords[0] = m_bMirrorX ? (float) wide / (float) textureWide : 0.0f;
	texCoords[1] = m_bMirrorY ? (float) tall / (float) textureTall : 0.0f;
	texCoords[2] = m_bMirrorX ? 0.0f : (float) wide / (float) textureWide;
	texCoords[3] = m_bMirrorY ? 0.0f : (float) tall / (float) textureTall;
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

void VectorImagePanel::SetMirrorX( bool state )
{
	int wide, tall, textureWide, textureTall;
	GetSize( wide, tall );
	vgui::surface()->DrawGetTextureSize( m_nTextureId, textureWide, textureTall );

	m_bMirrorX = state;
	texCoords[0] = m_bMirrorX ? (float) wide / (float) textureWide : 0.0f;
	texCoords[2] = m_bMirrorX ? 0.0f : (float) wide / (float) textureWide;
}

void VectorImagePanel::SetMirrorY( bool state )
{
	int wide, tall, textureWide, textureTall;
	GetSize( wide, tall );
	vgui::surface()->DrawGetTextureSize( m_nTextureId, textureWide, textureTall );

	m_bMirrorY = state;
	texCoords[1] = m_bMirrorY ? (float) tall / (float) textureTall : 0.0f;
	texCoords[3] = m_bMirrorY ? 0.0f : (float) tall / (float) textureTall;
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

	int wide, tall;
	GetSize( wide, tall );

	vgui::surface()->DrawSetTexture( m_nTextureId );
	vgui::surface()->DrawSetColor( GetFgColor() );

	g_pMatSystemSurface->DisableClipping( true );
	for ( int i = 0; i < m_nRepeatsCount; i++ )
	{
		int x0 = m_iRepeatMargin[0] * i;
		int x1 = x0 + wide;
		int y0 = m_iRepeatMargin[1] * i;
		int y1 = y0 + tall;
		vgui::surface()->DrawTexturedSubRect( x0, y0, x1, y1, texCoords[0], texCoords[1], texCoords[2], texCoords[3] );
	}
	g_pMatSystemSurface->DisableClipping( false );
}
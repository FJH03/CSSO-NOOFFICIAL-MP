//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include <vgui/IPanel.h>
#include <vgui/ISurface.h>
#include <vgui_controls/SVGImage.h>
#include <vgui_controls/Controls.h>
#include <Color.h>
#include "filesystem.h"
#include "lunasvg/lunasvg.h"
#include "VGuiMatSurface/IMatSystemSurface.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
using namespace lunasvg;

//-----------------------------------------------------------------------------
// Purpose: Conctructor. Start with default position and default color.
//-----------------------------------------------------------------------------
SVGImage::SVGImage()
{
	SetPos( 0, 0 );
	SetSize( 0, 0 );
	SetColor( Color( 255, 255, 255, 255 ) );

	m_flTexCoords[0] = m_flTexCoords[1] = m_flTexCoords[2] = m_flTexCoords[3] = 0;
	m_nTextureID = 0;
	m_bMirrored[0] = m_bMirrored[1] = false;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
SVGImage::~SVGImage()
{
	Evict();
}

bool SVGImage::SetTexture( const char* pszFilePath )
{
	// don't even bother doing anything without a file
	if ( !pszFilePath )
		return false;

	Evict();

	FileHandle_t f = g_pFullFileSystem->Open( pszFilePath, "rt" );
	if ( !f )
	{
		Warning( "SVGImage: failed to open file \"%s\".\n", pszFilePath );
		return false;
	}

	// read the whole thing into memory
	int size = g_pFullFileSystem->Size( f );
	// read into temporary memory block
	int nBufSize = size + 1;
	if ( IsXbox() )
	{
		nBufSize = AlignValue( nBufSize, 512 );
	}
	char* pMem = (char*) malloc( nBufSize );
	int bytesRead = g_pFullFileSystem->ReadEx( pMem, nBufSize, size, f );
	Assert( bytesRead <= size );
	pMem[bytesRead] = 0;
	g_pFullFileSystem->Close( f );
	std::unique_ptr<Document> document = Document::loadFromData( pMem ); // load the svg
	free( pMem );

	if ( !document )
	{
		Warning( "SVGImage: failed to load file \"%s\".\n", pszFilePath );
		return false;
	}

	Bitmap bitmap = document->renderToBitmap( m_nSize[0], m_nSize[1] ); // render the svg

	if ( !bitmap.valid() )
	{
		Warning( "SVGImage: failed to render file \"%s\".\n", pszFilePath );
		return false;
	}
	bitmap.convertToRGBA(); // PiMoN: fuck lunasvg devs for making me add this after their dumb update!

	int wide = bitmap.width();
	int tall = bitmap.height();

	m_nTextureID = surface()->CreateNewTextureID( true );
	surface()->DrawSetTextureRGBA( m_nTextureID, bitmap.data(), wide, tall, 1, true );
	SetSize( wide, tall );
	m_nContentSize[0] = wide;
	m_nContentSize[1] = tall;

	int textureWide, textureTall;
	vgui::surface()->DrawGetTextureSize( m_nTextureID, textureWide, textureTall );

	m_flTexCoords[0] = m_bMirrored[0] ? (float) wide / (float) textureWide : 0.0f;
	m_flTexCoords[1] = m_bMirrored[1] ? (float) tall / (float) textureTall : 0.0f;
	m_flTexCoords[2] = m_bMirrored[0] ? 0.0f : (float) wide / (float) textureWide;
	m_flTexCoords[3] = m_bMirrored[1] ? 0.0f : (float) tall / (float) textureTall;

	return true;
}

void SVGImage::SetMirroredX( bool bState )
{
	if ( m_bMirrored[0] != bState )
	{
		V_swap( m_flTexCoords[0], m_flTexCoords[2] );
		m_bMirrored[0] = bState;
	}
}

void SVGImage::SetMirroredY( bool bState )
{
	if ( m_bMirrored[1] != bState )
	{
		V_swap( m_flTexCoords[1], m_flTexCoords[3] );
		m_bMirrored[1] = bState;
	}
}

void SVGImage::Paint()
{
	if ( m_nTextureID == 0 )
		return;

	surface()->DrawSetTexture( m_nTextureID );
	surface()->DrawSetColor( m_clrRender );

	g_pMatSystemSurface->DisableClipping( true );
	surface()->DrawTexturedSubRect( m_nPos[0], m_nPos[1], m_nPos[0] + m_nSize[0], m_nPos[1] + m_nSize[1], m_flTexCoords[0], m_flTexCoords[1], m_flTexCoords[2], m_flTexCoords[3] );
	g_pMatSystemSurface->DisableClipping( false );
}

void SVGImage::SetPos( int x, int y )
{
	m_nPos[0] = x;
	m_nPos[1] = y;
}

void SVGImage::GetContentSize( int& w, int& h )
{
	w = m_nContentSize[0];
	h = m_nContentSize[1];
}

void SVGImage::GetSize( int& w, int& h )
{
	w = m_nSize[0];
	h = m_nSize[1];
}

void SVGImage::SetSize( int w, int h )
{
	m_nSize[0] = w;
	m_nSize[1] = h;
	m_nContentSize[0] = m_nContentSize[1] = 0; // we don't know what the content size is yet
}

void SVGImage::SetColor( Color clr )
{
	m_clrRender = clr;
}

bool SVGImage::Evict()
{
	if ( m_nTextureID != 0 )
	{
		surface()->DestroyTextureID( m_nTextureID );
		m_nTextureID = 0;
		return true;
	}

	return false;
}

HTexture SVGImage::GetID()
{
	return m_nTextureID;
}


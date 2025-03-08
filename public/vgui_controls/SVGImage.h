//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef SVGIMAGE_H
#define SVGIMAGE_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui/VGUI.h>
#include <Color.h>
#include <vgui/IImage.h>

namespace vgui
{

class Panel;

//-----------------------------------------------------------------------------
// Purpose: SVG image control
//-----------------------------------------------------------------------------
class SVGImage : public IImage
{
public:
	SVGImage();
	virtual ~SVGImage();

	virtual void Paint();
	virtual void SetPos( int x, int y );
	virtual void GetContentSize( int& wide, int& tall );
	virtual void GetSize( int& wide, int& tall );
	virtual void SetSize( int wide, int tall );
	virtual void SetColor( Color clr );
	virtual bool Evict();
	virtual int GetNumFrames() { return 1; } // SVG images always have 1 frame
	virtual void SetFrame( int nFrame ) {};
	virtual HTexture GetID();
	virtual void SetRotation( int iRotation ) {}

	bool SetTexture( const char* pszFilePath );
	void SetMirroredX( bool bState );
	void SetMirroredY( bool bState );

private:
	int m_nPos[2];
	int m_nSize[2];
	int m_nContentSize[2];
	float m_flTexCoords[4];
	Color m_clrRender;
	HTexture m_nTextureID;
	bool m_bMirrored[2];
};

} // namespace vgui

#endif // SVGIMAGE_H

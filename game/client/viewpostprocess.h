//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================

#ifndef VIEWPOSTPROCESS_H
#define VIEWPOSTPROCESS_H

#if defined( _WIN32 )
#pragma once
#endif

#ifdef CSTRIKE_DLL
#include "cs_shareddefs.h"
#endif

void DoEnginePostProcessing( int x, int y, int w, int h, bool bFlashlightIsOn, bool bPostVGui = false );
void DoImageSpaceMotionBlur( const CViewSetup &view, int x, int y, int w, int h );
void DumpTGAofRenderTarget( const int x, const int y, const int width, const int height, const char *pFilename );

void DoBlurFade( float flStrength, float flDesaturate, int x, int y, int w, int h );

void ApplyIronSightScopeEffect( int x, int y, int w, int h, CViewSetup *viewSetup );

#endif // VIEWPOSTPROCESS_H

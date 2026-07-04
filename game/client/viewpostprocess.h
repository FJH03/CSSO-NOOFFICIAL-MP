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

#include "postprocess_shared.h"

void DoEnginePostProcessing( int x, int y, int w, int h, bool bFlashlightIsOn, bool bPostVGui = false );
void DoImageSpaceMotionBlur( const CViewSetup &view, int x, int y, int w, int h );
void DumpTGAofRenderTarget( const int x, const int y, const int width, const int height, const char *pFilename );

void UpdateMaterialSystemTonemapScalar();
float GetCurrentTonemapScale();
void SetOverrideTonemapScale( bool bEnableOverride, float flTonemapScale );

void DoBlurFade( float flStrength, float flDesaturate, int x, int y, int w, int h );

void SetViewFadeParams( byte r, byte g, byte b, byte a, bool bModulate );

void ApplyIronSightScopeEffect( int x, int y, int w, int h, CViewSetup *viewSetup );

#endif // VIEWPOSTPROCESS_H

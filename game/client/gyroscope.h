//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Gyroscope input support for Android devices
//
//=============================================================================//

#ifndef GYROSCOPE_H
#define GYROSCOPE_H

#ifdef _WIN32
#pragma once
#endif

//-----------------------------------------------------------------------------
// Gyroscope CVARs
//-----------------------------------------------------------------------------
extern ConVar gyroscope;
extern ConVar gyroscope_sensitivity;
extern ConVar gyroscope_reverse_x;
extern ConVar gyroscope_reverse_y;

//-----------------------------------------------------------------------------
// Gyroscope functions
//-----------------------------------------------------------------------------
void Gyro_Init( void );
void Gyro_Shutdown( void );
void Gyro_Update( float *yaw, float *pitch );
void Gyro_Reset( void );
int  Gyro_IsEnabled( void );

#endif // GYROSCOPE_H

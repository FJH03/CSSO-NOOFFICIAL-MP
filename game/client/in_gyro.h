//========= Gyroscope Input Support ============//
// Reads gyroscope data from the input system (SDL2 sensor API) and applies
// it to the player view angles during CreateMove.
//==============================================//

#pragma once

// Called once during CInput::Init_All
void Gyro_Init( void );
void Gyro_ApplyMove( float frametime );

//========= CS:SO Android  ============//
//
// cl_identification.h — hardware-based persistent unique ID
//
//=================================//

#ifndef CL_IDENTIFICATION_H
#define CL_IDENTIFICATION_H

#ifdef _WIN32
#pragma once
#endif

// Call once during engine client init
void ID_Init( void );

// Returns the unique ID string
const char *ID_GetUniqueIDString( void );

#endif // CL_IDENTIFICATION_H

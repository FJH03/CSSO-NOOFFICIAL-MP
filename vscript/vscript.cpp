//========== Copyright © 2008, Valve Corporation, All rights reserved. ========
//
// Purpose:
//
//=============================================================================

#include "interface.h"
#include "vscript/ivscript.h"
#include "squirrel/vsquirrel/vsquirrel.h"
#include "vstdlib/random.h"
#include "tier1/tier1.h"


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
class CScriptManager : public CTier1AppSystem<IScriptManager>
{
public:
	CScriptManager()
	{
	}

	IScriptVM *CreateVM()
	{
		IScriptVM *pVM = ScriptCreateSquirrelVM();

		AssertMsg( pVM, "Unknown script language\n" );
		if ( pVM )
		{
			pVM->Init();
			ScriptRegisterFunction( pVM, RandomFloat, "Generate a random floating point number within a range, inclusive" );
			ScriptRegisterFunction( pVM, RandomInt, "Generate a random integer within a range, inclusive" );
		}
		return pVM;
	}

	void DestroyVM( IScriptVM *p )
	{
		if ( p )
		{
			p->Shutdown();
			ScriptDestroySquirrelVM( p );
		}
	}
};

//-----------------------------------------------------------------------------
// Singleton
//-----------------------------------------------------------------------------
CScriptManager g_ScriptManager;

EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CScriptManager, IScriptManager, VSCRIPT_INTERFACE_VERSION, g_ScriptManager );



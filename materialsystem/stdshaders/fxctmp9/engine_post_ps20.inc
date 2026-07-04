#include "shaderlib/cshader.h"
class engine_post_ps20_Static_Index
{
private:
	int m_nTOOL_MODE;
#ifdef _DEBUG
	bool m_bTOOL_MODE;
#endif
public:
	void SetTOOL_MODE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nTOOL_MODE = i;
#ifdef _DEBUG
		m_bTOOL_MODE = true;
#endif
	}
	void SetTOOL_MODE( bool i )
	{
		m_nTOOL_MODE = i ? 1 : 0;
#ifdef _DEBUG
		m_bTOOL_MODE = true;
#endif
	}
private:
	int m_nDEPTH_BLUR_ENABLE;
#ifdef _DEBUG
	bool m_bDEPTH_BLUR_ENABLE;
#endif
public:
	void SetDEPTH_BLUR_ENABLE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDEPTH_BLUR_ENABLE = i;
#ifdef _DEBUG
		m_bDEPTH_BLUR_ENABLE = true;
#endif
	}
	void SetDEPTH_BLUR_ENABLE( bool i )
	{
		m_nDEPTH_BLUR_ENABLE = i ? 1 : 0;
#ifdef _DEBUG
		m_bDEPTH_BLUR_ENABLE = true;
#endif
	}
public:
	engine_post_ps20_Static_Index( )
	{
#ifdef _DEBUG
		m_bTOOL_MODE = false;
#endif // _DEBUG
		m_nTOOL_MODE = 0;
#ifdef _DEBUG
		m_bDEPTH_BLUR_ENABLE = false;
#endif // _DEBUG
		m_nDEPTH_BLUR_ENABLE = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllStaticVarsDefined = m_bTOOL_MODE && m_bDEPTH_BLUR_ENABLE;
		Assert( bAllStaticVarsDefined );
#endif // _DEBUG
		return ( 24 * m_nTOOL_MODE ) + ( 48 * m_nDEPTH_BLUR_ENABLE ) + 0;
	}
};
#define shaderStaticTest_engine_post_ps20 psh_forgot_to_set_static_TOOL_MODE + psh_forgot_to_set_static_DEPTH_BLUR_ENABLE + 0
class engine_post_ps20_Dynamic_Index
{
private:
	int m_nAA_ENABLE;
#ifdef _DEBUG
	bool m_bAA_ENABLE;
#endif
public:
	void SetAA_ENABLE( int i )
	{
		Assert( i >= 0 && i <= 0 );
		m_nAA_ENABLE = i;
#ifdef _DEBUG
		m_bAA_ENABLE = true;
#endif
	}
	void SetAA_ENABLE( bool i )
	{
		m_nAA_ENABLE = i ? 1 : 0;
#ifdef _DEBUG
		m_bAA_ENABLE = true;
#endif
	}
private:
	int m_nCOL_CORRECT_NUM_LOOKUPS;
#ifdef _DEBUG
	bool m_bCOL_CORRECT_NUM_LOOKUPS;
#endif
public:
	void SetCOL_CORRECT_NUM_LOOKUPS( int i )
	{
		Assert( i >= 0 && i <= 3 );
		m_nCOL_CORRECT_NUM_LOOKUPS = i;
#ifdef _DEBUG
		m_bCOL_CORRECT_NUM_LOOKUPS = true;
#endif
	}
	void SetCOL_CORRECT_NUM_LOOKUPS( bool i )
	{
		m_nCOL_CORRECT_NUM_LOOKUPS = i ? 1 : 0;
#ifdef _DEBUG
		m_bCOL_CORRECT_NUM_LOOKUPS = true;
#endif
	}
private:
	int m_nFADE_TO_BLACK;
#ifdef _DEBUG
	bool m_bFADE_TO_BLACK;
#endif
public:
	void SetFADE_TO_BLACK( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFADE_TO_BLACK = i;
#ifdef _DEBUG
		m_bFADE_TO_BLACK = true;
#endif
	}
	void SetFADE_TO_BLACK( bool i )
	{
		m_nFADE_TO_BLACK = i ? 1 : 0;
#ifdef _DEBUG
		m_bFADE_TO_BLACK = true;
#endif
	}
private:
	int m_nFADE_TYPE;
#ifdef _DEBUG
	bool m_bFADE_TYPE;
#endif
public:
	void SetFADE_TYPE( int i )
	{
		Assert( i >= 0 && i <= 2 );
		m_nFADE_TYPE = i;
#ifdef _DEBUG
		m_bFADE_TYPE = true;
#endif
	}
	void SetFADE_TYPE( bool i )
	{
		m_nFADE_TYPE = i ? 1 : 0;
#ifdef _DEBUG
		m_bFADE_TYPE = true;
#endif
	}
private:
	int m_nVOMIT_ENABLE;
#ifdef _DEBUG
	bool m_bVOMIT_ENABLE;
#endif
public:
	void SetVOMIT_ENABLE( int i )
	{
		Assert( i >= 0 && i <= 0 );
		m_nVOMIT_ENABLE = i;
#ifdef _DEBUG
		m_bVOMIT_ENABLE = true;
#endif
	}
	void SetVOMIT_ENABLE( bool i )
	{
		m_nVOMIT_ENABLE = i ? 1 : 0;
#ifdef _DEBUG
		m_bVOMIT_ENABLE = true;
#endif
	}
public:
	engine_post_ps20_Dynamic_Index()
	{
#ifdef _DEBUG
		m_bAA_ENABLE = false;
#endif // _DEBUG
		m_nAA_ENABLE = 0;
#ifdef _DEBUG
		m_bCOL_CORRECT_NUM_LOOKUPS = false;
#endif // _DEBUG
		m_nCOL_CORRECT_NUM_LOOKUPS = 0;
#ifdef _DEBUG
		m_bFADE_TO_BLACK = false;
#endif // _DEBUG
		m_nFADE_TO_BLACK = 0;
#ifdef _DEBUG
		m_bFADE_TYPE = false;
#endif // _DEBUG
		m_nFADE_TYPE = 0;
#ifdef _DEBUG
		m_bVOMIT_ENABLE = false;
#endif // _DEBUG
		m_nVOMIT_ENABLE = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllDynamicVarsDefined = m_bAA_ENABLE && m_bCOL_CORRECT_NUM_LOOKUPS && m_bFADE_TO_BLACK && m_bFADE_TYPE && m_bVOMIT_ENABLE;
		Assert( bAllDynamicVarsDefined );
#endif // _DEBUG
		return ( 1 * m_nAA_ENABLE ) + ( 1 * m_nCOL_CORRECT_NUM_LOOKUPS ) + ( 4 * m_nFADE_TO_BLACK ) + ( 8 * m_nFADE_TYPE ) + ( 24 * m_nVOMIT_ENABLE ) + 0;
	}
};
#define shaderDynamicTest_engine_post_ps20 psh_forgot_to_set_dynamic_AA_ENABLE + psh_forgot_to_set_dynamic_COL_CORRECT_NUM_LOOKUPS + psh_forgot_to_set_dynamic_FADE_TO_BLACK + psh_forgot_to_set_dynamic_FADE_TYPE + psh_forgot_to_set_dynamic_VOMIT_ENABLE + 0

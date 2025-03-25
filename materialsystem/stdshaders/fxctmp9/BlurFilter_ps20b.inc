#include "shaderlib/cshader.h"
class blurfilter_ps20b_Static_Index
{
private:
	int m_nKERNEL;
#ifdef _DEBUG
	bool m_bKERNEL;
#endif
public:
	void SetKERNEL( int i )
	{
		Assert( i >= 0 && i <= 4 );
		m_nKERNEL = i;
#ifdef _DEBUG
		m_bKERNEL = true;
#endif
	}
	void SetKERNEL( bool i )
	{
		m_nKERNEL = i ? 1 : 0;
#ifdef _DEBUG
		m_bKERNEL = true;
#endif
	}
private:
	int m_nCLEAR_COLOR;
#ifdef _DEBUG
	bool m_bCLEAR_COLOR;
#endif
public:
	void SetCLEAR_COLOR( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nCLEAR_COLOR = i;
#ifdef _DEBUG
		m_bCLEAR_COLOR = true;
#endif
	}
	void SetCLEAR_COLOR( bool i )
	{
		m_nCLEAR_COLOR = i ? 1 : 0;
#ifdef _DEBUG
		m_bCLEAR_COLOR = true;
#endif
	}
private:
	int m_nAPPROX_SRGB_ADAPTER;
#ifdef _DEBUG
	bool m_bAPPROX_SRGB_ADAPTER;
#endif
public:
	void SetAPPROX_SRGB_ADAPTER( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nAPPROX_SRGB_ADAPTER = i;
#ifdef _DEBUG
		m_bAPPROX_SRGB_ADAPTER = true;
#endif
	}
	void SetAPPROX_SRGB_ADAPTER( bool i )
	{
		m_nAPPROX_SRGB_ADAPTER = i ? 1 : 0;
#ifdef _DEBUG
		m_bAPPROX_SRGB_ADAPTER = true;
#endif
	}
public:
	blurfilter_ps20b_Static_Index( )
	{
#ifdef _DEBUG
		m_bKERNEL = false;
#endif // _DEBUG
		m_nKERNEL = 0;
#ifdef _DEBUG
		m_bCLEAR_COLOR = false;
#endif // _DEBUG
		m_nCLEAR_COLOR = 0;
#ifdef _DEBUG
		m_bAPPROX_SRGB_ADAPTER = false;
#endif // _DEBUG
		m_nAPPROX_SRGB_ADAPTER = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllStaticVarsDefined = m_bKERNEL && m_bCLEAR_COLOR && m_bAPPROX_SRGB_ADAPTER;
		Assert( bAllStaticVarsDefined );
#endif // _DEBUG
		return ( 1 * m_nKERNEL ) + ( 5 * m_nCLEAR_COLOR ) + ( 10 * m_nAPPROX_SRGB_ADAPTER ) + 0;
	}
};
#define shaderStaticTest_blurfilter_ps20b psh_forgot_to_set_static_KERNEL + psh_forgot_to_set_static_CLEAR_COLOR + psh_forgot_to_set_static_APPROX_SRGB_ADAPTER + 0
class blurfilter_ps20b_Dynamic_Index
{
public:
	blurfilter_ps20b_Dynamic_Index()
	{
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
#endif // _DEBUG
		return 0;
	}
};
#define shaderDynamicTest_blurfilter_ps20b 0

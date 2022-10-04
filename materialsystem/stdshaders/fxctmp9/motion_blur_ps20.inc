#include "shaderlib/cshader.h"
class motion_blur_ps20_Static_Index
{
public:
	motion_blur_ps20_Static_Index( )
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
#define shaderStaticTest_motion_blur_ps20 0
class motion_blur_ps20_Dynamic_Index
{
private:
	int m_nD_NUM_BLUR_SAMPLES;
#ifdef _DEBUG
	bool m_bD_NUM_BLUR_SAMPLES;
#endif
public:
	void SetD_NUM_BLUR_SAMPLES( int i )
	{
		Assert( i >= 0 && i <= 14 );
		m_nD_NUM_BLUR_SAMPLES = i;
#ifdef _DEBUG
		m_bD_NUM_BLUR_SAMPLES = true;
#endif
	}
	void SetD_NUM_BLUR_SAMPLES( bool i )
	{
		m_nD_NUM_BLUR_SAMPLES = i ? 1 : 0;
#ifdef _DEBUG
		m_bD_NUM_BLUR_SAMPLES = true;
#endif
	}
public:
	motion_blur_ps20_Dynamic_Index()
	{
#ifdef _DEBUG
		m_bD_NUM_BLUR_SAMPLES = false;
#endif // _DEBUG
		m_nD_NUM_BLUR_SAMPLES = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllDynamicVarsDefined = m_bD_NUM_BLUR_SAMPLES;
		Assert( bAllDynamicVarsDefined );
#endif // _DEBUG
		return ( 1 * m_nD_NUM_BLUR_SAMPLES ) + 0;
	}
};
#define shaderDynamicTest_motion_blur_ps20 psh_forgot_to_set_dynamic_D_NUM_BLUR_SAMPLES + 0

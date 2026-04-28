#include "gamepadui_util.h"
#include "gamepadui_interface.h"

#include "tier0/icommandline.h"
#include "tier1/strtools.h"

#if defined( __linux__ ) && defined( __GLIBC__ ) && ( __GLIBC__ > 2 || ( __GLIBC__ == 2 && __GLIBC_MINOR__ >= 31 ) )
#define GAMEPADUI_NEEDS_GLIBC_FINITE_MATH_COMPAT 1
#include <math.h>
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Josh: Unused, but referenced by imageutils.cpp
// SDK2013: not necessary here (Madi)
#ifdef HL2_RETAIL
class IVEngineClient* engine = NULL;
#endif

#if defined( GAMEPADUI_NEEDS_GLIBC_FINITE_MATH_COMPAT )
extern "C"
{
	double __acos_finite( double x ) { return acos( x ); }
	float __acosf_finite( float x ) { return acosf( x ); }
	double __asin_finite( double x ) { return asin( x ); }
	double __atan2_finite( double y, double x ) { return atan2( y, x ); }
	float __atan2f_finite( float y, float x ) { return atan2f( y, x ); }
	double __exp_finite( double x ) { return exp( x ); }
	float __expf_finite( float x ) { return expf( x ); }
	double __log_finite( double x ) { return log( x ); }
	double __pow_finite( double x, double y ) { return pow( x, y ); }
}
#endif

// Josh: Copied verbatim from basically every other module
// we have on this planet.
const char *COM_GetModDirectory()
{
    static char szModDir[ MAX_PATH ] = {};
    if ( V_strlen( szModDir ) == 0 )
    {
        const char *pszGameDir = CommandLine()->ParmValue("-game", CommandLine()->ParmValue( "-defaultgamedir", "hl2" ) );
        V_strncpy( szModDir, pszGameDir, sizeof(szModDir) );
        if ( strchr( szModDir, '/' ) || strchr( szModDir, '\\' ) )
        {
            V_StripLastDir( szModDir, sizeof(szModDir) );
            int nDirLen = V_strlen( szModDir );
            V_strncpy( szModDir, pszGameDir + nDirLen, sizeof(szModDir) - nDirLen );
        }
    }

    return szModDir;
}

int DrawPrintWrappedText(vgui::HFont font, int pX, int pY, const wchar_t* pszText, int nLength, int nMaxWidth, bool bDraw)
{
    float x = 0.0f;
    int extraY = 0;
    const int nFontTall = vgui::surface()->GetFontTall(font);
    const wchar_t* wszStrStart = pszText;
    const wchar_t* wszLastSpace = NULL;
    const wchar_t* wszEnd = pszText + nLength;

    for (const wchar_t* wsz = pszText; *wsz; wsz++)
    {
        wchar_t ch = *wsz;

        if (ch == L' ' || ch == L'\n')
            wszLastSpace = wsz;

#if USE_GETKERNEDCHARWIDTH
        wchar_t chBefore = 0;
        wchar_t chAfter = 0;
        if (wsz > pszText)
            chBefore = wsz[-1];
        chAfter = wsz[1];
        float flWide = 0.0f, flabcA = 0.0f;
        vgui::surface()->GetKernedCharWidth(font, ch, chBefore, chAfter, flWide, flabcA);
        if (ch == L' ')
            x = ceil(x);

        surface()->DrawSetTextPos(x + px, y + py);
        surface()->DrawUnicodeChar(ch);
        x += floor(flWide + 0.6);
#else
        x += vgui::surface()->GetCharacterWidth(font, ch);
#endif

        if (x >= nMaxWidth || ch == L'\n')
        {
            const wchar_t* wszNewStart = wszLastSpace ? wszLastSpace : wsz;
            if ( bDraw )
            {
                vgui::surface()->DrawSetTextPos(pX, pY);
                vgui::surface()->DrawPrintText(wszStrStart, (int)(intp)(wszNewStart - wszStrStart));
            }
            wszStrStart = wszNewStart + 1;
            wsz = wszStrStart;
            if ( ch == L'\n' )
                wsz--;
            x = 0;
            pY += nFontTall;
            extraY += nFontTall;
        }
    }

    if (wszStrStart != wszEnd && bDraw)
    {
        vgui::surface()->DrawSetTextPos(pX, pY);
        vgui::surface()->DrawPrintText(wszStrStart, (int)(intp)(wszEnd - wszStrStart));
    }

    return extraY;
}

int NextPowerOfTwo( int v )
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    return ++v;
}

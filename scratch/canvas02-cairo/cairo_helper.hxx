#ifndef _SV_SYSDATA_HXX
#include <vcl/sysdata.hxx>
#endif

#ifndef _SV_BITMAP_HXX
#include <vcl/bitmap.hxx>
#endif

int cairoHelperGetDefaultScreen( void* display );
void* cairoHelperGetGlitzDrawable( const SystemEnvData* pSysData, int width, int height );
void* cairoHelperGetGlitzSurface( const SystemEnvData* pSysData, void *drawable, int x, int y, int width, int height );
void* cairoHelperGetSurface( const SystemEnvData* pSysData, int x, int y, int width, int height );
void  cairoHelperFlush( const SystemEnvData* pSysData );
void* cairoHelperGetSurface( const SystemEnvData* pSysData, const BitmapSystemData& rBmpData, int width, int height );

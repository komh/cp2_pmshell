/*
 * cp2_pmshell : execute PMSHELL in a secondary code page
 *
 * Copyright (C) 2016 KO Myung-Hun <komh@chollian.net>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

#define INCL_DOS
#define INCL_DOSERRORS
#include <os2.h>

#include <process.h>

/* Replacement for strlen() */
int strLen( const char *s )
{
    const char *p = s;

    while( *p )
        p++;

    return p - s;
}

/* Replacement for memcpy() */
void memCpy( void *dst, const void *src, int size )
{
    int i;

    char *d = dst;
    const char *s = src;

    for( i = 0; i < size; i ++ )
        *d++ = *s++;
}

/* Calculate a length of array of ASCIIZ string */
int asczLen( PCH pchAscz )
{
    PCH pch = pchAscz;

    do
    {
        pch += strLen( pch ) + 1;
    } while( *pch );

    return pch - pchAscz;
}

int execute( const char *name )
{
    PCSZ pcszUseSecondaryCp = "WORKPLACE_PRIMARY_CP=0";
    PCSZ pcszPrimaryCp = "WORKPLACE_PRIMARY_CP";
    PSZ  pszPrimaryCpEnv;
    PCSZ pcszArgs;

    PPIB ppib;
    UCHAR szObjName[ CCHMAXPATH ] = { 0, };
    PSZ pszArgs = NULL;
    PSZ pszEnvs = NULL;
    RESULTCODES result  = { 0, };

    PSZ psz;
    APIRET rc = NO_ERROR;

    DosGetInfoBlocks( NULL, &ppib );

    /* Build up arguments with the given executable name */
    pcszArgs = ppib->pib_pchcmd + strLen( ppib->pib_pchcmd ) + 1;
    DosAllocMem( &pszArgs, strLen( name ) + 1 + strLen( pcszArgs ) + 1 + 1,
                 fALLOC );

    memCpy( pszArgs, name, strLen( name ) + 1 );
    memCpy( pszArgs + strLen( name ) + 1, pcszArgs, strLen( pcszArgs ) + 1 );
    /*
     * Additional NUL is given by DosAllocMem(), which initializes allocated
     * memory with 0
     */

    /* Add WORKPLACE_PRIMARY_CP=0 to enviroments */

    if( DosScanEnv( pcszPrimaryCp, &pszPrimaryCpEnv ) == NO_ERROR )
    {
        /* Found. Just overwrite */
        pszPrimaryCpEnv[ strLen( pszPrimaryCpEnv ) - 1 ] = '0';
    }
    else
    {
        /* Not found. Build up environments with WORKPLACE_PRIMARY_CP=0 */
        int envLen;

        envLen = asczLen( ppib->pib_pchenv );
        DosAllocMem( &pszEnvs, envLen + strLen( pcszUseSecondaryCp ) + 1 + 1,
                     fALLOC );

        memCpy( pszEnvs, ppib->pib_pchenv, envLen );
        memCpy( pszEnvs + envLen, pcszUseSecondaryCp,
                strLen( pcszUseSecondaryCp ) + 1 );
        /*
         * Additional NUL is given by DosAllocMem(), which initializes
         * allocated memory with 0
         */
    }

    rc = DosExecPgm( szObjName, sizeof (szObjName ), EXEC_SYNC,
                     pszArgs, pszEnvs, &result, name );

    DosFreeMem( pszEnvs );
    DosFreeMem( pszArgs );

    /* Return the return code of a child */
    return rc == NO_ERROR ? result.codeResult : 0xFF;
}

#ifdef __KLIBC__
#define main simple_main
#endif

int main( void )
{
    CHAR pmshell[] = "X:\\OS2\\PMSHELL.EXE";
    ULONG ulBoot;

    /* Query boot drive */
    DosQuerySysInfo( QSV_BOOT_DRIVE, QSV_BOOT_DRIVE,
                     &ulBoot, sizeof( ulBoot ));
    pmshell[ 0 ] = 'A' + ulBoot - 1;

    return execute( pmshell );
}

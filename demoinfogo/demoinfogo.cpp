//====== Copyright (c) 2012, Valve Corporation, All rights reserved. ========//
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are met:
//
// Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, 
// this list of conditions and the following disclaimer in the documentation 
// and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
// THE POSSIBILITY OF SUCH DAMAGE.
//===========================================================================//

#include "demofiledump.h"

// these settings cause it to output nothing
bool g_bDumpGameEvents = false;
bool g_bSupressFootstepEvents = true;
bool g_bShowExtraPlayerInfoInGameEvents = false;
bool g_bDumpDeaths = false;
bool g_bSupressWarmupDeaths = true;
bool g_bDumpStringTables = false;
bool g_bDumpDataTables = false;
bool g_bDumpPacketEntities = false;
bool g_bDumpNetMessages = false;

int __cdecl main( int argc, char *argv[] )
{
	CDemoFileDump DemoFileDump;

	if ( argc <= 1 )
	{
		printf( "demoinfogo filename.dem\n" );
		printf( "optional arguments:\n" \
				" -gameevents    Dump out game events.\n" \
				" -nofootsteps   Skip footstep events when dumping out game events.\n" \
				"                Should be after -gameevents.\n" \
				" -extrainfo     Show extra player info when dumping out game events.\n" \
				"                Should be after -gameevents.\n" \
				" -deathscsv     Dump out player death info in CSV form.\n" \
				" -nowarmup      Skip deaths during warm up when dumping player deaths.\n" \
				"                Should be after -deaths.\n" \
				" -stringtables  Dump string tables.\n" \
				" -datatables    Dump data tables. (send tables)\n" \
				" -packetentities Dump Packet Entities messages.\n" \
				" -netmessages   Dump net messages that are not one of the above.\n" \
				"Note: by default everything is dumped out.\n" );
		exit( 0 );
	}

	int nFileArgument = 1;
	if ( argc > 2 )
	{
		for ( int i = 1; i < argc; i++ )
		{
			// arguments start with - or /
			if ( argv[i][0] == '-' || argv[i][0] == '/' )
			{
				if ( _stricmp( &argv[i][1], "gameevents" ) == 0 )
				{
					g_bDumpGameEvents = true;
					g_bSupressFootstepEvents = false;
					g_bShowExtraPlayerInfoInGameEvents = false;
				}
				else if ( _stricmp( &argv[i][1], "nofootsteps" ) == 0 )
				{
					g_bSupressFootstepEvents = true;
				}
				else if ( _stricmp( &argv[i][1], "extrainfo" ) == 0 )
				{
					g_bShowExtraPlayerInfoInGameEvents = true;
				}
				else if ( _stricmp( &argv[i][1], "deathscsv" ) == 0 )
				{
					g_bDumpDeaths = true;
					g_bSupressWarmupDeaths = false;
				}
				else if ( _stricmp( &argv[i][1], "nowarmup" ) == 0 )
				{
					g_bSupressWarmupDeaths = true;
				}
				else if ( _stricmp( &argv[i][1], "stringtables" ) == 0 )
				{
					g_bDumpStringTables = true;
				}
				else if ( _stricmp( &argv[i][1], "datatables" ) == 0 )
				{
					g_bDumpDataTables = true;
				}
				else if ( _stricmp( &argv[i][1], "packetentities" ) == 0 )
				{
					g_bDumpPacketEntities = true;
				}
				else if ( _stricmp( &argv[i][1], "netmessages" ) == 0 )
				{
					g_bDumpNetMessages = true;
				}
			}
			else
			{
				nFileArgument = i;
			}
		}
	}
	else
	{
		// default is to dump out everything
		g_bDumpGameEvents = true;
		g_bSupressFootstepEvents = false;
		g_bShowExtraPlayerInfoInGameEvents = true;
		g_bDumpDeaths = true;
		g_bSupressWarmupDeaths = false;
		g_bDumpStringTables = true;
		g_bDumpDataTables = true;
		g_bDumpPacketEntities = true;
		g_bDumpNetMessages = true;
	}

	if( DemoFileDump.Open( argv[ nFileArgument ] ) )
	{
		DemoFileDump.DoDump();
	}

	return 1;
}

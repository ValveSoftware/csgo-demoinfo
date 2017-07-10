//====== Copyright (c) 2014, Valve Corporation, All rights reserved. ========//
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

#include <stdio.h>
#include <assert.h>
#include "demofile.h"

CDemoFile::CDemoFile()
{
}

CDemoFile::~CDemoFile()
{
	Close();
}

void CDemoFile::ReadSequenceInfo( int32 &nSeqNrIn, int32 &nSeqNrOut )
{
	if ( !m_fileBuffer.size() )
		return;

	nSeqNrIn = *( int32 * )( &m_fileBuffer[ m_fileBufferPos ] );
	m_fileBufferPos += sizeof( int32 );
	nSeqNrOut = *( int32 * )( &m_fileBuffer[ m_fileBufferPos ] );
	m_fileBufferPos += sizeof( int32 );
}

void CDemoFile::ReadCmdInfo( democmdinfo_t& info )
{
	if ( !m_fileBuffer.size() )
		return;

	memcpy( &info, &m_fileBuffer[ m_fileBufferPos ], sizeof( democmdinfo_t ) );
	m_fileBufferPos += sizeof( democmdinfo_t );
}

void CDemoFile::ReadCmdHeader( unsigned char& cmd, int32& tick, unsigned char& playerSlot )
{
	if ( !m_fileBuffer.size() )
		return;

	// Read the command
	cmd = *( unsigned char * )( &m_fileBuffer[ m_fileBufferPos ] );
	m_fileBufferPos += sizeof( unsigned char );
	
	if ( cmd <=0 )
	{
		fprintf( stderr, "CDemoFile::ReadCmdHeader: Missing end tag in demo file.\n");
		cmd = dem_stop;
		return;
	}

	assert( cmd >= 1 && cmd <= dem_lastcmd );

	// Read the timestamp
	tick = *( int32 * )( &m_fileBuffer[ m_fileBufferPos ] );
	m_fileBufferPos += sizeof( int32 );

	// read playerslot
	playerSlot = *( unsigned char * )( &m_fileBuffer[ m_fileBufferPos ] );
	m_fileBufferPos += sizeof( unsigned char );
}

int32 CDemoFile::ReadUserCmd( char *buffer, int32 &size )
{
	if ( !m_fileBuffer.size() )
		return 0;

	int32 outgoing_sequence = *( int32 * )( &m_fileBuffer[ m_fileBufferPos ] );
	m_fileBufferPos += sizeof( int32 );
	
	size = ReadRawData( buffer, size );

	return outgoing_sequence;
}

int32 CDemoFile::ReadRawData( char *buffer, int32 length )
{
	if ( !m_fileBuffer.size() )
		return 0;

	// read length of data block
	int32 size = *( int32 * )( &m_fileBuffer[ m_fileBufferPos ] );
	m_fileBufferPos += sizeof( int32 );
	
	if ( buffer && (length < size) )
	{
		fprintf( stderr, "CDemoFile::ReadRawData: buffer overflow (%i).\n", size );
		return -1;
	}

	if ( buffer )
	{
		// read data into buffer
		memcpy( buffer, &m_fileBuffer[ m_fileBufferPos ], size );
		m_fileBufferPos += size;
	}
	else
	{
		// just skip it
		m_fileBufferPos += size;
	}

	return size;
}

bool CDemoFile::Open( const char *name )
{
	Close();

	FILE *fp = NULL;
	fopen_s( &fp, name, "rb" );
	if( fp )
	{
		size_t Length;

		fseek( fp, 0, SEEK_END );
		Length = ftell( fp );
		fseek( fp, 0, SEEK_SET );

		if( Length < sizeof( m_DemoHeader ) )
		{
			fprintf( stderr, "CDemoFile::Open: file too small. %s.\n", name );
			fclose( fp );
			return false;
		}

		fread( &m_DemoHeader, 1, sizeof( m_DemoHeader ), fp );
		Length -= sizeof( m_DemoHeader );

		if ( strcmp ( m_DemoHeader.demofilestamp, DEMO_HEADER_ID ) )
		{
			fprintf( stderr, "CDemoFile::Open: %s has invalid demo header ID.\n", name );
			fclose( fp );
			return false;
		}

		if ( m_DemoHeader.demoprotocol != DEMO_PROTOCOL )
		{
			fprintf( stderr, "CDemoFile::Open: demo file protocol %i invalid, expected version is %i \n", m_DemoHeader.demoprotocol, DEMO_PROTOCOL );
			fclose( fp );
			return false;
		}

		m_fileBuffer.resize( Length );
		fread( &m_fileBuffer[ 0 ], 1, Length, fp );

		fclose( fp );
		fp = NULL;
	}

	if ( !m_fileBuffer.size() )
	{
		fprintf( stderr, "CDemoFile::Open: couldn't open file %s.\n", name );
		Close();
		return false;
	}

	m_fileBufferPos = 0;
	m_szFileName = name;
	return true;
}

void CDemoFile::Close()
{
	m_szFileName.clear();

	m_fileBufferPos = 0;
	m_fileBuffer.clear();
}

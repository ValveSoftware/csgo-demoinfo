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

#ifndef DEMOFILEDUMP_H
#define DEMOFILEDUMP_H

#include "demofile.h"
#include "demofilebitbuf.h"
#include "demofilepropdecode.h"

#include "generated_proto/netmessages_public.pb.h"

#define NET_MAX_PAYLOAD					( 262144 - 4 )		// largest message we can send in bytes
#define DEMO_RECORD_BUFFER_SIZE			( 2 * 1024 * 1024 )	// temp buffer big enough to fit both string tables and server classes

// How many bits to use to encode an edict.
#define	MAX_EDICT_BITS					11					// # of bits needed to represent max edicts
// Max # of edicts in a level
#define	MAX_EDICTS						( 1 << MAX_EDICT_BITS )

#define MAX_USERDATA_BITS				14
#define	MAX_USERDATA_SIZE				( 1 << MAX_USERDATA_BITS )
#define SUBSTRING_BITS					5

#define NUM_NETWORKED_EHANDLE_SERIAL_NUMBER_BITS	10

#define MAX_PLAYER_NAME_LENGTH			128
#define MAX_CUSTOM_FILES				4	// max 4 files
#define SIGNED_GUID_LEN					32	// Hashed CD Key (32 hex alphabetic chars + 0 terminator )

#define ENTITY_SENTINEL					9999

#define MAX_STRING_TABLES				64

struct StringTableData_t
{
	char	szName[ 64 ];
	int		nMaxEntries;
	int		nUserDataSize;      // not currently used to parse stringtable updates, kept for documentation purposes only
	int		nUserDataSizeBits;  // not currently used to parse stringtable updates, kept for documentation purposes only
	int		nUserDataFixedSize; // used to parse stringtable updates
};

// userinfo string table contains these
struct player_info_t
{
	// version for future compatibility
	uint64			version;
	// network xuid
	uint64			xuid;
	// scoreboard information
	char			name[ MAX_PLAYER_NAME_LENGTH ];
	// local server user ID, unique while server is running
	int				userID;
	// global unique player identifer
	char			guid[ SIGNED_GUID_LEN + 1 ];
	// friends identification number
	uint32			friendsID;
	// friends name
	char			friendsName[ MAX_PLAYER_NAME_LENGTH ];
	// true, if player is a bot controlled by game.dll
	bool			fakeplayer;
	// true if player is the HLTV proxy
	bool			ishltv;
	// custom files CRC for this player
	CRC32_t			customFiles[ MAX_CUSTOM_FILES ];
	// this counter increases each time the server downloaded a new file
	unsigned char	filesDownloaded;
	// entity index
	int				entityID;
};

struct ExcludeEntry
{
	ExcludeEntry( const char *pVarName, const char *pDTName, const char *pDTExcluding )
		: m_pVarName( pVarName )
		, m_pDTName( pDTName )
		, m_pDTExcluding( pDTExcluding )
	{
	}

	const char *m_pVarName;
	const char *m_pDTName;
	const char *m_pDTExcluding;
};

struct FlattenedPropEntry
{
	FlattenedPropEntry( const CSVCMsg_SendTable::sendprop_t *prop, const CSVCMsg_SendTable::sendprop_t *arrayElementProp )
		: m_prop( prop )
		, m_arrayElementProp( arrayElementProp )
	{
	}
	const CSVCMsg_SendTable::sendprop_t *m_prop;
	const CSVCMsg_SendTable::sendprop_t *m_arrayElementProp;
};

struct ServerClass_t
{
	int nClassID;
	char strName[256];
	char strDTName[256];
	int nDataTable;

	std::vector< FlattenedPropEntry > flattenedProps;
};

struct PropEntry
{
	PropEntry( FlattenedPropEntry *pFlattenedProp, Prop_t *pPropValue )
		: m_pFlattenedProp( pFlattenedProp )
		, m_pPropValue( pPropValue )
	{
	}
	~PropEntry()
	{
		delete m_pPropValue;
	}

	FlattenedPropEntry *m_pFlattenedProp;
	Prop_t *m_pPropValue;
};

struct EntityEntry
{
	EntityEntry( int nEntity, uint32 uClass, uint32 uSerialNum)
		: m_nEntity( nEntity )
		, m_uClass( uClass )
		, m_uSerialNum( uSerialNum )
	{
	}
	~EntityEntry()
	{
		for ( std::vector< PropEntry * >::iterator i = m_props.begin(); i != m_props.end(); i++ )
		{
			delete *i;
		}
	}
	PropEntry *FindProp( const char *pName )
	{
		for ( std::vector< PropEntry * >::iterator i = m_props.begin(); i != m_props.end(); i++ )
		{
			PropEntry *pProp = *i;
			if (  pProp->m_pFlattenedProp->m_prop->var_name().compare( pName ) == 0 )
			{
				return pProp;
			}
		}
		return NULL;
	}
	void AddOrUpdateProp( FlattenedPropEntry *pFlattenedProp, Prop_t *pPropValue )
	{
		//if ( m_uClass == 34 && pFlattenedProp->m_prop->var_name().compare( "m_vecOrigin" ) == 0 )
		//{
		//	printf("got vec origin!\n" );
		//}
		PropEntry *pProp = FindProp( pFlattenedProp->m_prop->var_name().c_str() );
		if ( pProp )
		{
			delete pProp->m_pPropValue;
			pProp->m_pPropValue = pPropValue;
		}
		else
		{
			pProp = new PropEntry( pFlattenedProp, pPropValue );
			m_props.push_back( pProp );
		}
	}
	int m_nEntity;
	uint32 m_uClass;
	uint32 m_uSerialNum;

	std::vector< PropEntry * > m_props;
};

enum UpdateType
{
	EnterPVS = 0,	// Entity came back into pvs, create new entity if one doesn't exist
	LeavePVS,		// Entity left pvs
	DeltaEnt,		// There is a delta for this entity.
	PreserveEnt,	// Entity stays alive but no delta ( could be LOD, or just unchanged )
	Finished,		// finished parsing entities successfully
	Failed,			// parsing error occured while reading entities
};

// Flags for delta encoding header
enum HeaderFlags
{
	FHDR_ZERO			= 0x0000,
	FHDR_LEAVEPVS		= 0x0001,
	FHDR_DELETE			= 0x0002,
	FHDR_ENTERPVS		= 0x0004,
};

class CDemoFileDump
{
public:
	CDemoFileDump() : m_nFrameNumber( 0 )
	{
	}

	~CDemoFileDump()
	{
	}

	bool Open( const char *filename ); 
	void DoDump();
	void HandleDemoPacket();

public:
	void DumpDemoPacket( CBitRead &buf, int length );
	void DumpUserMessage( const void *parseBuffer, int BufferSize );
	void MsgPrintf( const ::google::protobuf::Message& msg, int size, const char *fmt, ... );

public:
	CDemoFile m_demofile;
	CSVCMsg_GameEventList m_GameEventList;

	int m_nFrameNumber;
};

#endif // DEMOFILEDUMP_H

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

#include <stdarg.h>
#include <iostream>
#include <fstream>
#include <string>  
#include <conio.h>
#include "demofile.h"
#include "demofiledump.h"
#include "demofilepropdecode.h"
#include "enums.h"

#include "google/protobuf/descriptor.h"
#include "google/protobuf/reflection_ops.h"
#include "google/protobuf/descriptor.pb.h"

#include "generated_proto/cstrike15_usermessages_public.pb.h"
#include "generated_proto/netmessages_public.pb.h"

// file globals
static int s_nNumStringTables;
static StringTableData_t s_StringTables[ MAX_STRING_TABLES ];

static int s_nServerClassBits = 0;
static std::vector< ServerClass_t > s_ServerClasses;
static std::vector< CSVCMsg_SendTable > s_DataTables;
static std::vector< ExcludeEntry > s_currentExcludes;
static std::vector< EntityEntry * > s_Entities;

extern bool g_bDumpGameEvents;
extern bool g_bSupressFootstepEvents;
extern bool g_bShowExtraPlayerInfoInGameEvents;
extern bool g_bDumpDeaths;
extern bool g_bSupressWarmupDeaths;
extern bool g_bDumpStringTables;
extern bool g_bDumpDataTables;
extern bool g_bDumpPacketEntities;
extern bool g_bDumpNetMessages;

//TODO make these not static lmao
static bool s_bMatchStartOccured = false;
static int s_nCurrentRound = 1;
static int s_nCurrentTick = 0;
static int s_nPreviousTick = 0;
static int s_nBots = 0;
static RoundStatus s_RoundStatus;
static std::vector< Player* > s_PlayerInstances;
static std::vector< GrenadeEntity* > s_GrenadeEntities;
static std::vector< PlayerHurtEvent* > s_PlayerHurtEvents;
static std::vector< PlayerDeathEvent* > s_PlayerDeathEvents;
static std::vector< TickInfo > s_TickInfos;
static BombEntity* bomb;

__declspec( noreturn ) void fatal_errorf( const char* fmt, ... )
{
    va_list  vlist;
    char buf[ 1024 ];

    va_start( vlist, fmt);
    vsnprintf_s( buf, sizeof( buf ), fmt, vlist );
	buf[ sizeof( buf ) - 1 ] = 0;
    va_end( vlist );

    fprintf( stderr, "\nERROR: %s\n", buf );
    exit( -1 );
}

bool CDemoFileDump::Open( const char *filename )
{
	if ( !m_demofile.Open( filename ) )
	{
		fprintf( stderr, "Couldn't open '%s'\n", filename );
		return false;
	}

	return true;
}


/************************Helper Functions************************/

void CDemoFileDump::CleanUp()
{
	for ( std::vector< Player* >::iterator it = s_PlayerInstances.begin(); it != s_PlayerInstances.end(); ++it )
	{			
		delete *it;
	}
	s_PlayerInstances.clear();

	/*for ( std::vector< TickInfo >::iterator it = s_TickInfos.begin(); it != s_TickInfos.end(); ++it )
	{			
		delete *it;
	}*/
	s_TickInfos.clear();

	delete bomb;
}

template< typename T >
static void LowLevelByteSwap( T *output, const T *input )
{
	T temp = *output;
	for ( unsigned int i = 0; i < sizeof( T ); i++ )
	{
		( ( unsigned char* )&temp )[i] = ( ( unsigned char* )input )[ sizeof( T ) - ( i + 1 ) ]; 
	}
	memcpy( output, &temp, sizeof( T ) );
}

Player *CDemoFileDump::FindPlayerInstance( int userID )
{
	for ( std::vector< Player* >::iterator it = s_PlayerInstances.begin(); it != s_PlayerInstances.end(); ++it )
	{			
		if ( ( *it )->GetUserID() == userID )
		{				
			return *it;
		}
	}
	return NULL;
}

Player *CDemoFileDump::FindPlayerInstanceByGUID( int GUID )
{
	for ( std::vector< Player* >::iterator it = s_PlayerInstances.begin(); it != s_PlayerInstances.end(); ++it )
	{			
		if ( ( *it )->GetGUID() == GUID )
		{				
			return *it;
		}
	}
	return NULL;
}

//Sets grenades ready to remove
//Mollies/Smokes/Decoys are set to remove based on networked event
//Flashes and HEs are set to remove the tick after
void CDemoFileDump::SetRemoveGrenadeEntity( int entityID )
{
	for ( std::vector< GrenadeEntity* >::iterator it = s_GrenadeEntities.begin(); it != s_GrenadeEntities.end(); ++it )
	{			
		if( ( *it )->GetEntityID() == entityID )
		{
			( *it )->SetReadyToRemove( true );
			break;
		}
	}
}

const CSVCMsg_GameEventList::descriptor_t *CDemoFileDump::GetGameEventDescriptor( const CSVCMsg_GameEvent &msg, CDemoFileDump& Demo )
{
	int iDescriptor;

	for ( iDescriptor = 0; iDescriptor < Demo.m_GameEventList.descriptors().size(); iDescriptor++ )
	{
		const CSVCMsg_GameEventList::descriptor_t& Descriptor = Demo.m_GameEventList.descriptors( iDescriptor );

		if ( Descriptor.eventid() == msg.eventid() )
			break;
	}

	if ( iDescriptor == Demo.m_GameEventList.descriptors().size() )
	{
		if ( g_bDumpGameEvents )
		{
			printf( "%s", msg.DebugString().c_str() );
		}
		return NULL;
	}

	return &Demo.m_GameEventList.descriptors( iDescriptor );
}

static std::string GetNetMsgName( int Cmd )
{
	if ( NET_Messages_IsValid( Cmd ) )
	{
		return NET_Messages_Name( ( NET_Messages )Cmd );
	}
	else if ( SVC_Messages_IsValid( Cmd ) )
	{
		return SVC_Messages_Name( ( SVC_Messages )Cmd );
	}

	return "NETMSG_???";
}


/************************String Table Handling************************/
/*
void ParseStringTableUpdate( CBitRead &buf, int entries, int nMaxEntries, int user_data_size, int user_data_size_bits, int user_data_fixed_size, bool bIsUserInfo )
{
	struct StringHistoryEntry
	{
		char string[ ( 1 << SUBSTRING_BITS ) ];
	};

	int lastEntry = -1;
	int lastDictionaryIndex = -1;

	// perform integer log2() to set nEntryBits
	int nTemp = nMaxEntries;
	int nEntryBits = 0;
	while (nTemp >>= 1) ++nEntryBits;

	bool bEncodeUsingDictionaries = buf.ReadOneBit() ? true : false;

	if ( bEncodeUsingDictionaries )
	{
		printf( "ParseStringTableUpdate: Encoded with dictionaries, unable to decode.\n" );
		return;
	}

	std::vector< StringHistoryEntry > history;

	for ( int i = 0; i < entries; i++ )
	{
		int entryIndex = lastEntry + 1;

		if ( !buf.ReadOneBit() )
		{
			entryIndex = buf.ReadUBitLong( nEntryBits );
		}

		lastEntry = entryIndex;
		
		if ( entryIndex < 0 || entryIndex >= nMaxEntries )
		{
			printf( "ParseStringTableUpdate: bogus string index %i\n", entryIndex );
			return;
		}

		const char *pEntry = NULL;
		char entry[ 1024 ]; 
		char substr[ 1024 ];
		entry[ 0 ] = 0;

		if ( buf.ReadOneBit() )
		{
			bool substringcheck = buf.ReadOneBit() ? true : false;

			if ( substringcheck )
			{
				int index = buf.ReadUBitLong( 5 );
				if( size_t( index ) >= history.size() )
				{
					printf("ParseStringTableUpdate: Invalid index %d, expected < %u\n", index, (unsigned)history.size());
					exit(-1);
				}
				int bytestocopy = buf.ReadUBitLong( SUBSTRING_BITS );
				strncpy_s( entry, history[ index ].string, bytestocopy + 1 );
				buf.ReadString( substr, sizeof( substr ) );
				strcat_s( entry, substr );
			}
			else
			{
				buf.ReadString( entry, sizeof( entry ) );
			}

			pEntry = entry;
		}
		
		// Read in the user data.
		unsigned char tempbuf[ MAX_USERDATA_SIZE ];
		memset( tempbuf, 0, sizeof( tempbuf ) );
		const void *pUserData = NULL;
		int nBytes = 0;

		if ( buf.ReadOneBit() )
		{
			if ( user_data_fixed_size )
			{
				// Don't need to read length, it's fixed length and the length was networked down already.
				nBytes = user_data_size;
				assert( nBytes > 0 );
				tempbuf[ nBytes - 1 ] = 0; // be safe, clear last byte
				buf.ReadBits( tempbuf, user_data_size_bits );
			}
			else
			{
				nBytes = buf.ReadUBitLong( MAX_USERDATA_BITS );
				if ( nBytes > sizeof( tempbuf ) )
				{
					printf( "ParseStringTableUpdate: user data too large (%d bytes).", nBytes);
					return;
				}

				buf.ReadBytes( tempbuf, nBytes );
			}

			pUserData = tempbuf;
		}

		if ( pEntry == NULL )
		{
			pEntry = "";// avoid crash because of NULL strings
		}

		if ( bIsUserInfo && pUserData != NULL )
		{
			const player_info_t *pUnswappedPlayerInfo = ( const player_info_t * )pUserData;
			player_info_t playerInfo = *pUnswappedPlayerInfo;
			playerInfo.entityID = entryIndex;

			LowLevelByteSwap( &playerInfo.xuid, &pUnswappedPlayerInfo->xuid );
			LowLevelByteSwap( &playerInfo.userID, &pUnswappedPlayerInfo->userID );
			LowLevelByteSwap( &playerInfo.friendsID, &pUnswappedPlayerInfo->friendsID );

			bool bAdded = false;
			auto existing = FindPlayerByEntity(entryIndex);
			if ( !existing ) 
			{
				bAdded = true;
				s_PlayerInfos.push_back(playerInfo);
			}
			else {
				*existing = playerInfo;
			}

			if ( g_bDumpStringTables )
			{
				printf( "player info\n{\n %s:true\n xuid:%lld\n name:%s\n userID:%d\n guid:%s\n friendsID:%d\n friendsName:%s\n fakeplayer:%d\n ishltv:%d\n filesDownloaded:%d\n entityID:%d\n}\n",
					bAdded ? "adding" : "updating", playerInfo.xuid, playerInfo.name, playerInfo.userID, playerInfo.guid, playerInfo.friendsID,
					playerInfo.friendsName, playerInfo.fakeplayer, playerInfo.ishltv, playerInfo.filesDownloaded, playerInfo.entityID );
			}
		}
		else
		{
			if ( g_bDumpStringTables )
			{
				printf( " %d, %s, %d, %s \n", entryIndex, pEntry, nBytes, pUserData );
			}
		}


		if ( history.size() > 31 )
		{
			history.erase( history.begin() );
		}

		StringHistoryEntry she;
		strncpy_s( she.string, pEntry, sizeof( she.string ) - 1 );
		history.push_back( she );
	}
}*/

void RecvTable_ReadInfos( const CSVCMsg_SendTable& msg )
{
	if ( g_bDumpDataTables )
	{
		printf( "%s:%d\n", msg.net_table_name().c_str(), msg.props_size() ); 
	
		for ( int iProp=0; iProp < msg.props_size(); iProp++ )
		{
			const CSVCMsg_SendTable::sendprop_t& sendProp = msg.props( iProp );

			if ( ( sendProp.type() == DPT_DataTable ) || ( sendProp.flags() & SPROP_EXCLUDE ) )
			{
				printf( "%d:%06X:%s:%s%s\n", sendProp.type(), sendProp.flags(), sendProp.var_name().c_str(), sendProp.dt_name().c_str(), ( sendProp.flags() & SPROP_EXCLUDE ) ? " exclude" : "" );
			}
			else if ( sendProp.type() == DPT_Array )
			{
				printf( "%d:%06X:%s[%d]\n", sendProp.type(), sendProp.flags(), sendProp.var_name().c_str(), sendProp.num_elements() );
			}
			else
			{
				printf( "%d:%06X:%s:%f,%f,%08X%s\n", sendProp.type(), sendProp.flags(), sendProp.var_name().c_str(), sendProp.low_value(), sendProp.high_value(), sendProp.num_bits(), ( sendProp.flags() & SPROP_INSIDEARRAY ) ? " inside array" : "" );
			}
		}
	}
}

int CDemoFileDump::ReadFieldIndex( CBitRead &entityBitBuffer, int lastIndex, bool bNewWay )
{
	if (bNewWay)
	{
		if (entityBitBuffer.ReadOneBit())
		{
			return lastIndex + 1;
		}
	}
 
	int ret = 0;
	if (bNewWay && entityBitBuffer.ReadOneBit())
	{
		ret = entityBitBuffer.ReadUBitLong(3);  // read 3 bits
	}
	else
	{
		ret = entityBitBuffer.ReadUBitLong(7); // read 7 bits
		switch( ret & ( 32 | 64 ) )
		{
			case 32:
				ret = ( ret &~96 ) | ( entityBitBuffer.ReadUBitLong( 2 ) << 5 );
				assert( ret >= 32);
				break;
			case 64:
				ret = ( ret &~96 ) | ( entityBitBuffer.ReadUBitLong( 4 ) << 5 );
				assert( ret >= 128);
				break;
			case 96:
				ret = ( ret &~96 ) | ( entityBitBuffer.ReadUBitLong( 7 ) << 5 );
				assert( ret >= 512);
				break;
		}
	}
 
	if (ret == 0xFFF) // end marker is 4095 for cs:go
	{
		return -1;
	}
 
	return lastIndex + 1 + ret;
}

CSVCMsg_SendTable *GetTableByClassID( uint32 nClassID )
{
	for ( uint32 i = 0; i < s_ServerClasses.size(); i++ )
	{
		if ( s_ServerClasses[ i ].nClassID == nClassID )
		{
			return &(s_DataTables[ s_ServerClasses[i].nDataTable ]);
		}
	}
	return NULL;
}

CSVCMsg_SendTable *GetTableByName( const char *pName )
{
	for ( unsigned int i = 0; i < s_DataTables.size(); i++ )
	{
		if ( s_DataTables[ i ].net_table_name().compare( pName ) == 0 )
		{
			return &(s_DataTables[ i ]);
		}
	}
	return NULL;
}

FlattenedPropEntry *GetSendPropByIndex( uint32 uClass, uint32 uIndex )
{
	if ( uIndex < s_ServerClasses[ uClass ].flattenedProps.size() )
	{
		return &s_ServerClasses[ uClass ].flattenedProps[ uIndex ];
	}
	return NULL;
}

bool IsPropExcluded( CSVCMsg_SendTable *pTable, const CSVCMsg_SendTable::sendprop_t &checkSendProp )
{
	for ( unsigned int i = 0; i < s_currentExcludes.size(); i++ )
	{
		if ( pTable->net_table_name().compare( s_currentExcludes[ i ].m_pDTName ) == 0 &&
			 checkSendProp.var_name().compare( s_currentExcludes[ i ].m_pVarName ) == 0 )
		{
			return true;
		}
	}
	return false;
}

void GatherExcludes( CSVCMsg_SendTable *pTable )
{
	for ( int iProp=0; iProp < pTable->props_size(); iProp++ )
	{
		const CSVCMsg_SendTable::sendprop_t& sendProp = pTable->props( iProp );
		if ( sendProp.flags() & SPROP_EXCLUDE )
		{
			s_currentExcludes.push_back( ExcludeEntry( sendProp.var_name().c_str(), sendProp.dt_name().c_str(), pTable->net_table_name().c_str() ) );
		}

		if ( sendProp.type() == DPT_DataTable )
		{
			CSVCMsg_SendTable *pSubTable = GetTableByName( sendProp.dt_name().c_str() );
			if ( pSubTable != NULL )
			{
				GatherExcludes( pSubTable );
			}
		}
	}
}

void GatherProps( CSVCMsg_SendTable *pTable, int nServerClass );

void GatherProps_IterateProps( CSVCMsg_SendTable *pTable, int nServerClass, std::vector< FlattenedPropEntry > &flattenedProps )
{
	for ( int iProp=0; iProp < pTable->props_size(); iProp++ )
	{
		const CSVCMsg_SendTable::sendprop_t& sendProp = pTable->props( iProp );

		if ( ( sendProp.flags() & SPROP_INSIDEARRAY ) || 
			 ( sendProp.flags() & SPROP_EXCLUDE ) || 
			 IsPropExcluded( pTable, sendProp ) )
		{
			continue;
		}

		if ( sendProp.type() == DPT_DataTable )
		{
			CSVCMsg_SendTable *pSubTable = GetTableByName( sendProp.dt_name().c_str() );
			if ( pSubTable != NULL )
			{
				if ( sendProp.flags() & SPROP_COLLAPSIBLE )
				{
					GatherProps_IterateProps( pSubTable, nServerClass, flattenedProps );
				}
				else
				{
					GatherProps( pSubTable, nServerClass );
				}
			}
		}
		else
		{
			if ( sendProp.type() == DPT_Array )
			{
				flattenedProps.push_back( FlattenedPropEntry( &sendProp, &(pTable->props( iProp - 1 ) ) ) );
			}
			else
			{
				flattenedProps.push_back( FlattenedPropEntry( &sendProp, NULL ) );
			}
		}
	}
}

void GatherProps( CSVCMsg_SendTable *pTable, int nServerClass )
{
	std::vector< FlattenedPropEntry > tempFlattenedProps;
	GatherProps_IterateProps( pTable, nServerClass, tempFlattenedProps );

	std::vector< FlattenedPropEntry > &flattenedProps = s_ServerClasses[ nServerClass ].flattenedProps;
	for ( uint32 i = 0; i < tempFlattenedProps.size(); i++ )
	{
		flattenedProps.push_back( tempFlattenedProps[ i ] );
	}
}

void FlattenDataTable( int nServerClass )
{
	CSVCMsg_SendTable *pTable = &s_DataTables[ s_ServerClasses[ nServerClass ].nDataTable ];

	s_currentExcludes.clear();
	GatherExcludes( pTable );

	GatherProps( pTable, nServerClass );

	std::vector< FlattenedPropEntry > &flattenedProps = s_ServerClasses[ nServerClass ].flattenedProps;

	// get priorities
	std::vector< uint32 > priorities;
	priorities.push_back(64);
	for ( unsigned int i = 0; i < flattenedProps.size(); i++ )
	{
		uint32 priority = flattenedProps[ i ].m_prop->priority();

		bool bFound = false;
		for ( uint32 j = 0; j < priorities.size(); j++ )
		{
			if ( priorities[ j ] == priority )
			{
				bFound = true;
				break;
			}
		}

		if (!bFound)
		{
			priorities.push_back(priority);
		}
	}

	std::sort(priorities.begin(), priorities.end());

	// sort flattenedProps by priority
	uint32 start = 0;
	for (uint32 priority_index = 0; priority_index < priorities.size(); ++priority_index)
	{
		uint32 priority = priorities[priority_index];

		while( true )
		{
			uint32 currentProp = start;
			while (currentProp < flattenedProps.size()) 
			{
				const CSVCMsg_SendTable::sendprop_t *prop = flattenedProps[currentProp].m_prop;

				if (prop->priority() == priority || (priority == 64 && (SPROP_CHANGES_OFTEN & prop->flags()))) 
				{
					if ( start != currentProp )
					{
						FlattenedPropEntry temp = flattenedProps[start];
						flattenedProps[start] = flattenedProps[currentProp];
						flattenedProps[currentProp] = temp;
					}
					start++;
					break;
				}
				currentProp++;
			}

			if ( currentProp == flattenedProps.size() )
				break;
		}
	}
}


/************************Entity Helper Functions************************/

bool CDemoFileDump::ReadNewEntity( CBitRead &entityBitBuffer, EntityEntry *pEntity )
{
	bool bNewWay = ( entityBitBuffer.ReadOneBit() == 1 );  // 0 = old way, 1 = new way

	std::vector< int > fieldIndices;

	int index = -1;
	do
	{
		index = ReadFieldIndex( entityBitBuffer, index, bNewWay );
		if ( index != -1 )
		{
			fieldIndices.push_back( index );
		}
	} while (index != -1);

	CSVCMsg_SendTable *pTable = GetTableByClassID( pEntity->m_uClass );
	/*
	if ( g_bDumpPacketEntities )
	{
		printf( "Table: %s\n", pTable->net_table_name().c_str() );
	}*/
	for ( unsigned int i = 0; i < fieldIndices.size(); i++ )
	{
		FlattenedPropEntry *pSendProp = GetSendPropByIndex( pEntity->m_uClass, fieldIndices[ i ] );
		if ( pSendProp )
		{
			Prop_t *pProp = DecodeProp( entityBitBuffer, pSendProp, pEntity->m_uClass, fieldIndices[ i ], !g_bDumpPacketEntities );	
			pEntity->AddOrUpdateProp( pSendProp, pProp );
		}
		else
		{
			return false;
		}
	}

	return true;
}

EntityEntry *CDemoFileDump::FindEntity( int nEntity )
{
	for ( std::vector< EntityEntry * >::iterator i = s_Entities.begin(); i != s_Entities.end(); i++ )
	{
		if (  (*i)->m_nEntity == nEntity )
		{
			return *i;
		}
	}

	return NULL;
}

EntityEntry *CDemoFileDump::AddEntity( int nEntity, uint32 uClass, uint32 uSerialNum )
{
	// if entity already exists, then replace it, else add it
	EntityEntry *pEntity = FindEntity( nEntity );
	if ( pEntity )
	{
		pEntity->m_uClass = uClass;
		pEntity->m_uSerialNum = uSerialNum;
	}
	else
	{
		pEntity = new EntityEntry( nEntity, uClass, uSerialNum );
		s_Entities.push_back( pEntity );
	}

	return pEntity;
}

void CDemoFileDump::RemoveEntity( int nEntity )
{
	for ( std::vector< EntityEntry * >::iterator i = s_Entities.begin(); i != s_Entities.end(); i++ )
	{
		EntityEntry *pEntity = *i;
		if (  pEntity->m_nEntity == nEntity )
		{
			s_Entities.erase( i );
			delete pEntity;
			break;
		}
	}
}

void CDemoFileDump::HandleDemoPacket()
{
	democmdinfo_t	info;
	int				dummy;
	char			data[ NET_MAX_PAYLOAD ];

	m_demofile.ReadCmdInfo( info );
	m_demofile.ReadSequenceInfo( dummy, dummy ); 

	CBitRead buf( data, NET_MAX_PAYLOAD );
	int length = m_demofile.ReadRawData( ( char* )buf.GetBasePointer(), buf.GetNumBytesLeft() );
	buf.Seek( 0 );
	DumpDemoPacket( buf, length );
}

bool ReadFromBuffer( CBitRead &buffer, void **pBuffer, int& size )
{
	size = buffer.ReadVarInt32();
	if ( size < 0 || size > NET_MAX_PAYLOAD )
	{
		return false;
	}

	// Check its valid
	if ( size > buffer.GetNumBytesLeft() )
	{
		return false;
	}

	*pBuffer = malloc( size );

	// If the read buffer is byte aligned, we can parse right out of it
	if ( ( buffer.GetNumBitsRead() % 8 ) == 0 )
	{
		memcpy( *pBuffer, buffer.GetBasePointer() + buffer.GetNumBytesRead(), size );
		buffer.SeekRelative( size * 8 );
		return true;
	}

	// otherwise we have to ReadBytes() it out
	if ( !buffer.ReadBytes( *pBuffer, size ) )
	{
		return false;
	}

	return true;
}

bool ParseDataTable( CBitRead &buf )
{
	CSVCMsg_SendTable msg;
	while ( 1 )
	{
		int type = buf.ReadVarInt32();
		
		void *pBuffer = NULL;
		int size = 0;
		if ( !ReadFromBuffer( buf, &pBuffer, size ) )
		{
			printf( "ParseDataTable: ReadFromBuffer failed.\n" );
			return false;
		}
		msg.ParseFromArray( pBuffer, size );
		free( pBuffer );

		if ( msg.is_end() )
			break;

		RecvTable_ReadInfos( msg );

		s_DataTables.push_back( msg );
	}
	
	short nServerClasses = buf.ReadShort();
	assert( nServerClasses );
	for ( int i = 0; i < nServerClasses; i++ )
	{
		ServerClass_t entry;
		entry.nClassID = buf.ReadShort();
		if ( entry.nClassID >= nServerClasses )
		{
			printf( "ParseDataTable: invalid class index (%d).\n", entry.nClassID);
			return false;
		}

		int nChars;
		buf.ReadString( entry.strName, sizeof( entry.strName ), false, &nChars );
		buf.ReadString( entry.strDTName, sizeof( entry.strDTName ), false, &nChars );

		// find the data table by name
		entry.nDataTable = -1;
		for ( unsigned int j = 0; j < s_DataTables.size(); j++ )
		{
			if ( strcmp( entry.strDTName, s_DataTables[ j ].net_table_name().c_str() ) == 0 )
			{
				entry.nDataTable = j;
				break;
			}
		}

		if ( g_bDumpDataTables )
		{
			printf( "class:%d:%s:%s(%d)\n", entry.nClassID, entry.strName, entry.strDTName, entry.nDataTable );
		}
		s_ServerClasses.push_back( entry );
	}

	if ( g_bDumpDataTables )
	{
		printf( "Flattening data tables..." );
	}
	for ( int i = 0; i < nServerClasses; i++ )
	{
		FlattenDataTable( i );
	}
	if ( g_bDumpDataTables )
	{
		printf( "Done.\n" );
	}

	// perform integer log2() to set s_nServerClassBits
	int nTemp = nServerClasses;
	s_nServerClassBits = 0;
	while (nTemp >>= 1) ++s_nServerClassBits;

	s_nServerClassBits++;

	return true;
}

bool DumpStringTable( CBitRead &buf, bool bIsUserInfo )
{
	int numstrings = buf.ReadWord();
	if ( g_bDumpStringTables )
	{
		printf( "%d\n", numstrings );
	}

	if( bIsUserInfo )
	{
		if ( g_bDumpStringTables )
		{
			printf( "Clearing player info array.\n" );
		}
		//s_PlayerInfos.clear();
	}

	for ( int i = 0 ; i < numstrings; i++ )
	{
		char stringname[4096];
		
		buf.ReadString( stringname, sizeof( stringname ) );

		assert( strlen( stringname ) < 100 );

		if ( buf.ReadOneBit() == 1 )
		{
			int userDataSize = ( int )buf.ReadWord();
			assert( userDataSize > 0 );
			unsigned char *data = new unsigned char[ userDataSize + 4 ];
			assert( data );

			buf.ReadBytes( data, userDataSize );

			if( bIsUserInfo && data != NULL )
			{
				/*
				const player_info_t *pUnswappedPlayerInfo = ( const player_info_t * )data;
				player_info_t playerInfo = *pUnswappedPlayerInfo;
				playerInfo.entityID = i;

				LowLevelByteSwap( &playerInfo.xuid, &pUnswappedPlayerInfo->xuid );
				LowLevelByteSwap( &playerInfo.userID, &pUnswappedPlayerInfo->userID );
				LowLevelByteSwap( &playerInfo.friendsID, &pUnswappedPlayerInfo->friendsID );

				//shouldn't ever exist, but just incase
				auto existing = FindPlayerByEntity(i);
				if (!existing) 
				{
					if (g_bDumpStringTables) 
					{
						printf("adding:player entity:%d info:\n xuid:%lld\n name:%s\n userID:%d\n guid:%s\n friendsID:%d\n friendsName:%s\n fakeplayer:%d\n ishltv:%d\n filesDownloaded:%d\n",
							i, playerInfo.xuid, playerInfo.name, playerInfo.userID, playerInfo.guid, playerInfo.friendsID,
							playerInfo.friendsName, playerInfo.fakeplayer, playerInfo.ishltv, playerInfo.filesDownloaded);
					}
					s_PlayerInfos.push_back(playerInfo);
				}
				else 
				{
					*existing = playerInfo;
				}*/
			}
			else
			{
				if ( g_bDumpStringTables )
				{
					printf( " %d, %s, userdata[%d] \n", i, stringname, userDataSize );
				}
			}

			delete [] data;

			assert( buf.GetNumBytesLeft() > 10 );
		}
		else
		{
			if ( g_bDumpStringTables )
			{
				printf( " %d, %s \n", i, stringname );
			}
		}
	}

	// Client side stuff
	if ( buf.ReadOneBit() == 1 )
	{
		int numstrings = buf.ReadWord();
		for ( int i = 0 ; i < numstrings; i++ )
		{
			char stringname[ 4096 ];

			buf.ReadString( stringname, sizeof( stringname ) );

			if ( buf.ReadOneBit() == 1 )
			{
				int userDataSize = ( int )buf.ReadWord();
				assert( userDataSize > 0 );
				unsigned char *data = new unsigned char[ userDataSize + 4 ];
				assert( data );

				buf.ReadBytes( data, userDataSize );

				if ( i >= 2 )
				{
					if ( g_bDumpStringTables )
					{
						printf( " %d, %s, userdata[%d] \n", i, stringname, userDataSize );
					}
				}

				delete[] data;

			}
			else
			{
				if ( i >= 2 )
				{
					if ( g_bDumpStringTables )
					{
						printf( " %d, %s \n", i, stringname );
					}
				}
			}
		}
	}

	return true;
}

bool DumpStringTables( CBitRead &buf )
{
	int numTables = buf.ReadByte();
	for ( int i = 0 ; i < numTables; i++ )
	{
		char tablename[ 256 ];
		buf.ReadString( tablename, sizeof( tablename ) );

		if ( g_bDumpStringTables )
		{
			printf( "ReadStringTable:%s:", tablename );
		}

		bool bIsUserInfo = !strcmp( tablename, "userinfo" );
		if ( !DumpStringTable( buf, bIsUserInfo ) )
		{
			printf( "Error reading string table %s\n", tablename );
		}
	}

	return true;
}


/************************Message Printing************************/

void CDemoFileDump::MsgPrintf( const ::google::protobuf::Message& msg, int size, const char *fmt, ... )
{
	if ( g_bDumpNetMessages )
	{
		va_list vlist;
		const std::string& TypeName = msg.GetTypeName();

		// Print the message type and size
		printf( "---- %s (%d bytes) -----------------\n", TypeName.c_str(), size );

		va_start( vlist, fmt);
		vprintf( fmt, vlist );
		va_end( vlist );
	}
}

template < class T, int msgType >
void PrintUserMessage( CDemoFileDump& Demo, const void *parseBuffer, int BufferSize )
{
	T msg;

	if ( msg.ParseFromArray( parseBuffer, BufferSize ) )
	{
		Demo.MsgPrintf( msg, BufferSize, "%s", msg.DebugString().c_str() );
	}
}

template < class T, int msgType >
void PrintNetMessage( CDemoFileDump& Demo, const void *parseBuffer, int BufferSize )
{
	T msg;

	if ( msg.ParseFromArray( parseBuffer, BufferSize ) )
	{
		if ( msgType == svc_GameEventList )
		{
			Demo.m_GameEventList.CopyFrom( msg );
		}

		Demo.MsgPrintf( msg, BufferSize, "%s", msg.DebugString().c_str() );
	}
}

/*template <>
void PrintNetMessage< CSVCMsg_CreateStringTable, svc_CreateStringTable >( CDemoFileDump& Demo, const void *parseBuffer, int BufferSize )
{
	CSVCMsg_CreateStringTable msg;

	if ( msg.ParseFromArray( parseBuffer, BufferSize ) )
	{
		bool bIsUserInfo = !strcmp( msg.name().c_str(), "userinfo" );
		if ( g_bDumpStringTables )
		{
			printf( "CreateStringTable:%s:%d:%d:%d:%d:\n", msg.name().c_str(), msg.max_entries(), msg.num_entries(), msg.user_data_size(), msg.user_data_size_bits() );
		}
		CBitRead data( &msg.string_data()[ 0 ], msg.string_data().size() );
		ParseStringTableUpdate( data,  msg.num_entries(), msg.max_entries(), msg.user_data_size(), msg.user_data_size_bits(), msg.user_data_fixed_size(), bIsUserInfo ); 

		strcpy_s( s_StringTables[ s_nNumStringTables ].szName, msg.name().c_str() );
		s_StringTables[ s_nNumStringTables ].nMaxEntries = msg.max_entries();
		s_StringTables[ s_nNumStringTables ].nUserDataSize = msg.user_data_size();
		s_StringTables[ s_nNumStringTables ].nUserDataSizeBits = msg.user_data_size_bits();
		s_StringTables[ s_nNumStringTables ].nUserDataFixedSize = msg.user_data_fixed_size();
		s_nNumStringTables++;
	}
}

template <>
void PrintNetMessage< CSVCMsg_UpdateStringTable, svc_UpdateStringTable >( CDemoFileDump& Demo, const void *parseBuffer, int BufferSize )
{
	CSVCMsg_UpdateStringTable msg;

	if ( msg.ParseFromArray( parseBuffer, BufferSize ) )
	{
		CBitRead data( &msg.string_data()[ 0 ], msg.string_data().size() );

		if ( msg.table_id() < s_nNumStringTables && s_StringTables[ msg.table_id() ].nMaxEntries > msg.num_changed_entries() )
		{
			const StringTableData_t &table = s_StringTables[ msg.table_id() ];
			bool bIsUserInfo = !strcmp( table.szName, "userinfo" );
			if ( g_bDumpStringTables )
			{
				printf( "UpdateStringTable:%d(%s):%d:\n", msg.table_id(), table.szName, msg.num_changed_entries() );
			}
			ParseStringTableUpdate( data, msg.num_changed_entries(), table.nMaxEntries, table.nUserDataSize, table.nUserDataSizeBits, table.nUserDataFixedSize, bIsUserInfo ); 
		}
		else
		{
			printf( "Bad UpdateStringTable:%d:%d!\n", msg.table_id(), msg.num_changed_entries() );
		}
	}
}

template <>
void PrintNetMessage< CSVCMsg_SendTable, svc_SendTable >( CDemoFileDump& Demo, const void *parseBuffer, int BufferSize )
{
	CSVCMsg_SendTable msg;

	if ( msg.ParseFromArray( parseBuffer, BufferSize ) )
	{
		RecvTable_ReadInfos( msg );
	}
}

template <>
void PrintNetMessage< CSVCMsg_UserMessage, svc_UserMessage >( CDemoFileDump& Demo, const void *parseBuffer, int BufferSize )
{
	Demo.DumpUserMessage( parseBuffer, BufferSize );
}
*/

//Parses tick messages
template <>
void PrintNetMessage< CNETMsg_Tick, net_Tick >( CDemoFileDump& Demo, const void *parseBuffer, int BufferSize )
{
	CNETMsg_Tick msg;

	if ( msg.ParseFromArray( parseBuffer, BufferSize ) )
	{
		s_nPreviousTick = s_nCurrentTick;
		s_nCurrentTick = msg.tick();
		//printf("----- Parsing Tick %d -----\n", s_nCurrentTick);
	}
}

//Parses game event messages
template <>
void PrintNetMessage< CSVCMsg_GameEvent, svc_GameEvent >( CDemoFileDump& Demo, const void *parseBuffer, int BufferSize )
{
	CSVCMsg_GameEvent msg;

	if ( msg.ParseFromArray( parseBuffer, BufferSize ) )
	{
		const CSVCMsg_GameEventList::descriptor_t *pDescriptor = Demo.GetGameEventDescriptor( msg, Demo );
		if ( pDescriptor )
		{
			Demo.ParseGameEvent( msg, pDescriptor );
		}
	}
}

//Parses entity messages
template <>
void PrintNetMessage< CSVCMsg_PacketEntities, svc_PacketEntities >( CDemoFileDump& Demo, const void *parseBuffer, int BufferSize )
{
	CSVCMsg_PacketEntities msg;

	if ( msg.ParseFromArray( parseBuffer, BufferSize ) )
	{
		CBitRead entityBitBuffer( &msg.entity_data()[ 0 ], msg.entity_data().size() );
		bool bAsDelta = msg.is_delta();
		int nHeaderCount = msg.updated_entries();
		int nBaseline = msg.baseline();
		bool bUpdateBaselines = msg.update_baseline();
		int nHeaderBase = -1;
		int nNewEntity = -1;
		int UpdateFlags = 0;

		UpdateType updateType = PreserveEnt;

		while ( updateType < Finished )
		{
			nHeaderCount--;

			bool bIsEntity = ( nHeaderCount >= 0 ) ? true : false;

			if ( bIsEntity  )
			{
				UpdateFlags = FHDR_ZERO;

				nNewEntity = nHeaderBase + 1 + entityBitBuffer.ReadUBitVar();
				nHeaderBase = nNewEntity;

				// leave pvs flag
				if ( entityBitBuffer.ReadOneBit() == 0 )
				{
					// enter pvs flag
					if ( entityBitBuffer.ReadOneBit() != 0 )
					{
						UpdateFlags |= FHDR_ENTERPVS;
					}
				}
				else
				{
					UpdateFlags |= FHDR_LEAVEPVS;

					// Force delete flag
					if ( entityBitBuffer.ReadOneBit() != 0 )
					{
						UpdateFlags |= FHDR_DELETE;
					}
				}
			}

			for ( updateType = PreserveEnt; updateType == PreserveEnt; )
			{
				// Figure out what kind of an update this is.
				if ( !bIsEntity || nNewEntity > ENTITY_SENTINEL)
				{
					updateType = Finished;
				}
				else
				{
					if ( UpdateFlags & FHDR_ENTERPVS )
					{
						updateType = EnterPVS;
					}
					else if ( UpdateFlags & FHDR_LEAVEPVS )
					{
						updateType = LeavePVS;
					}
					else
					{
						updateType = DeltaEnt;
					}
				}

				switch( updateType )
				{
					case EnterPVS:	
						{
							uint32 uClass = entityBitBuffer.ReadUBitLong( s_nServerClassBits );
							uint32 uSerialNum = entityBitBuffer.ReadUBitLong( NUM_NETWORKED_EHANDLE_SERIAL_NUMBER_BITS );
							/*
							if ( g_bDumpPacketEntities )
							{
								printf( "Entity Enters PVS: id:%d, class:%d, serial:%d\n", nNewEntity, uClass, uSerialNum );
							}*/
							EntityEntry *pEntity = Demo.AddEntity( nNewEntity, uClass, uSerialNum );
							if ( !Demo.ReadNewEntity( entityBitBuffer, pEntity ) )
							{
								printf( "*****Error reading entity! Bailing on this PacketEntities!\n" );
								return;
							}
						}
						break;

					case LeavePVS:
						{
							if ( !bAsDelta )  // Should never happen on a full update.
							{
								//printf( "WARNING: LeavePVS on full update" );
								updateType = Failed;	// break out
								assert( 0 );
							}
							else
							{
								if ( g_bDumpPacketEntities )
								{
									if ( UpdateFlags & FHDR_DELETE )
									{
										//printf( "Entity leaves PVS and is deleted: id:%d\n", nNewEntity );
									}
									else
									{
										//printf( "Entity leaves PVS: id:%d\n", nNewEntity );
									}
								}
								Demo.RemoveEntity( nNewEntity );
							}
						}
						break;

					case DeltaEnt:
						{
							EntityEntry *pEntity = Demo.FindEntity( nNewEntity );
							if ( pEntity )
							{
								/*
								if ( g_bDumpPacketEntities )
								{
									printf( "Entity Delta update: id:%d, class:%d, serial:%d\n", pEntity->m_nEntity, pEntity->m_uClass, pEntity->m_uSerialNum );
								}*/
								if ( !Demo.ReadNewEntity( entityBitBuffer, pEntity ) )
								{
									printf( "*****Error reading entity! Bailing on this PacketEntities!\n" );
									return;
								}
							}
							else
							{
								assert(0);
							}
						}
						break;

					case PreserveEnt:
						{
							if ( !bAsDelta )  // Should never happen on a full update.
							{
								//printf( "WARNING: PreserveEnt on full update" );
								updateType = Failed;	// break out
								assert( 0 );
							}
							else
							{
								if ( nNewEntity >= MAX_EDICTS )
								{
									//printf( "PreserveEnt: nNewEntity == MAX_EDICTS" );
									assert( 0 );
								}
								else
								{
									if ( g_bDumpPacketEntities )
									{
										//printf( "PreserveEnt: id:%d\n", nNewEntity );
									}
								}
							}
						}
						break;

					default:
						break;
				}
			}
		}
	}
}

void CDemoFileDump::DumpUserMessage( const void *parseBuffer, int BufferSize )
{
	CSVCMsg_UserMessage userMessage;

	if ( userMessage.ParseFromArray( parseBuffer, BufferSize ) )
	{
		int Cmd = userMessage.msg_type();
		int SizeUM = userMessage.msg_data().size();
		const void *parseBufferUM = &userMessage.msg_data()[ 0 ];
		switch ( Cmd )
		{
#define HANDLE_UserMsg( _x )			case CS_UM_ ## _x: PrintUserMessage< CCSUsrMsg_ ## _x, CS_UM_ ## _x >( *this, parseBufferUM, SizeUM ); break

		default:
			// unknown user message
			break;
			HANDLE_UserMsg( VGUIMenu );
			HANDLE_UserMsg( Geiger );
			HANDLE_UserMsg( Train );
			HANDLE_UserMsg( HudText );
			HANDLE_UserMsg( SayText );
			HANDLE_UserMsg( SayText2 );
			HANDLE_UserMsg( TextMsg );
			HANDLE_UserMsg( HudMsg );
			HANDLE_UserMsg( ResetHud );
			HANDLE_UserMsg( GameTitle );
			HANDLE_UserMsg( Shake );
			HANDLE_UserMsg( Fade );
			HANDLE_UserMsg( Rumble );
			HANDLE_UserMsg( CloseCaption );
			HANDLE_UserMsg( CloseCaptionDirect );
			HANDLE_UserMsg( SendAudio );
			HANDLE_UserMsg( RawAudio );
			HANDLE_UserMsg( VoiceMask );
			HANDLE_UserMsg( RequestState );
			HANDLE_UserMsg( Damage );
			HANDLE_UserMsg( RadioText );
			HANDLE_UserMsg( HintText );
			HANDLE_UserMsg( KeyHintText );
			HANDLE_UserMsg( ProcessSpottedEntityUpdate );
			HANDLE_UserMsg( ReloadEffect );
			HANDLE_UserMsg( AdjustMoney );
			HANDLE_UserMsg( StopSpectatorMode );
			HANDLE_UserMsg( KillCam );
			HANDLE_UserMsg( DesiredTimescale );
			HANDLE_UserMsg( CurrentTimescale );
			HANDLE_UserMsg( AchievementEvent );
			HANDLE_UserMsg( MatchEndConditions );
			HANDLE_UserMsg( DisconnectToLobby );
			HANDLE_UserMsg( DisplayInventory );
			HANDLE_UserMsg( WarmupHasEnded );
			HANDLE_UserMsg( ClientInfo );
			HANDLE_UserMsg( CallVoteFailed );
			HANDLE_UserMsg( VoteStart );
			HANDLE_UserMsg( VotePass );
			HANDLE_UserMsg( VoteFailed );
			HANDLE_UserMsg( VoteSetup );
			HANDLE_UserMsg( SendLastKillerDamageToClient );
			HANDLE_UserMsg( ItemPickup );
			HANDLE_UserMsg( ShowMenu );
			HANDLE_UserMsg( BarTime );
			HANDLE_UserMsg( AmmoDenied );
			HANDLE_UserMsg( MarkAchievement );
			HANDLE_UserMsg( ItemDrop );
			HANDLE_UserMsg( GlowPropTurnOff );

#undef HANDLE_UserMsg
		}
	}
}

void CDemoFileDump::DumpDemoPacket( CBitRead &buf, int length )
{
	while ( buf.GetNumBytesRead() < length )
	{
		int Cmd = buf.ReadVarInt32();
		int Size = buf.ReadVarInt32();

		if ( buf.GetNumBytesRead() + Size > length )
		{
			const std::string& strName = GetNetMsgName( Cmd );

			fatal_errorf( "DumpDemoPacket()::failed parsing packet. Cmd:%d '%s' \n", Cmd, strName.c_str() );
		}

		switch( Cmd )
		{
#define HANDLE_NetMsg( _x )		case net_ ## _x: PrintNetMessage< CNETMsg_ ## _x, net_ ## _x >( *this, buf.GetBasePointer() + buf.GetNumBytesRead(), Size ); break
#define HANDLE_SvcMsg( _x )		case svc_ ## _x: PrintNetMessage< CSVCMsg_ ## _x, svc_ ## _x >( *this, buf.GetBasePointer() + buf.GetNumBytesRead(), Size ); break

		default:
			// unknown net message
			break;

		HANDLE_NetMsg( NOP );            	// 0
		HANDLE_NetMsg( Disconnect );        // 1
		HANDLE_NetMsg( File );              // 2
		HANDLE_NetMsg( Tick );              // 4
		//HANDLE_NetMsg( StringCmd );         // 5
		HANDLE_NetMsg( SetConVar );         // 6
		//HANDLE_NetMsg( SignonState );       // 7
		HANDLE_SvcMsg( ServerInfo );        // 8
		HANDLE_SvcMsg( SendTable );         // 9
		HANDLE_SvcMsg( ClassInfo );         // 10
		HANDLE_SvcMsg( SetPause );          // 11
		//HANDLE_SvcMsg( CreateStringTable ); // 12
		//HANDLE_SvcMsg( UpdateStringTable ); // 13
		//HANDLE_SvcMsg( VoiceInit );         // 14
		//HANDLE_SvcMsg( VoiceData );         // 15
		//HANDLE_SvcMsg( Print );             // 16
		//HANDLE_SvcMsg( Sounds );            // 17
		//HANDLE_SvcMsg( SetView );           // 18
		//HANDLE_SvcMsg( FixAngle );          // 19
		//HANDLE_SvcMsg( CrosshairAngle );    // 20
		//HANDLE_SvcMsg( BSPDecal );          // 21
		//HANDLE_SvcMsg( UserMessage );       // 23
		HANDLE_SvcMsg( GameEvent );         // 25
		HANDLE_SvcMsg( PacketEntities );    // 26
		//HANDLE_SvcMsg( TempEntities );      // 27
		//HANDLE_SvcMsg( Prefetch );          // 28
		//HANDLE_SvcMsg( Menu );              // 29
		HANDLE_SvcMsg( GameEventList );     // 30
		//HANDLE_SvcMsg( GetCvarValue );      // 31

#undef HANDLE_SvcMsg
#undef HANDLE_NetMsg
		}

		buf.SeekRelative( Size * 8 );
	}
}


/************************Display Information************************/

void CDemoFileDump::GetPlayerInfo()
{
	EntityEntry *pEntity;
	Player *pPlayer;

	for ( std::vector< Player* >::iterator it = s_PlayerInstances.begin(); it != s_PlayerInstances.end(); ++it )
	{
		pPlayer = *it;
		pEntity = FindEntity( pPlayer->entityID + 1 );

		if( pEntity && pPlayer )
		{	
			//Getting team
			PropEntry *pTeam = pEntity->FindProp( "m_iTeamNum" );
			if( pTeam ){
				pPlayer->teamID = pTeam->m_pPropValue->m_value.m_int;
			}

			//Getting xyz coordinates
			PropEntry *pXYProp = pEntity->FindProp( "m_vecOrigin" );
			PropEntry *pZProp = pEntity->FindProp( "m_vecOrigin[2]" );
			if ( pXYProp && pZProp )
			{
				pPlayer->x = pXYProp->m_pPropValue->m_value.m_vector.x;
				pPlayer->y = pXYProp->m_pPropValue->m_value.m_vector.y;
				pPlayer->z = pZProp->m_pPropValue->m_value.m_float;
			}
			//Getting aim coordinates
			PropEntry *pAngle0Prop = pEntity->FindProp( "m_angEyeAngles[0]" );
			PropEntry *pAngle1Prop = pEntity->FindProp( "m_angEyeAngles[1]" );
			if ( pAngle0Prop && pAngle1Prop )
			{
				pPlayer->eyePitch = pAngle0Prop->m_pPropValue->m_value.m_float;
				pPlayer->eyeYaw = pAngle1Prop->m_pPropValue->m_value.m_float;
			}
			//Getting health, armour, kit
			PropEntry *pHealth = pEntity->FindProp( "m_iHealth" );
			PropEntry *pArmour = pEntity->FindProp( "m_ArmorValue" );
			PropEntry *pHelmet = pEntity->FindProp( "m_bHasHelmet" );
			PropEntry* pDefuser = pEntity->FindProp("m_bHasDefuser");
			if ( pHealth && pArmour )
			{
				pPlayer->health = pHealth->m_pPropValue->m_value.m_int;
				pPlayer->armour = pArmour->m_pPropValue->m_value.m_int;
				if ( pHelmet )
				{
					pPlayer->hasHelmet = ( pHelmet->m_pPropValue->m_value.m_int == 1 ) ? true : false;
				}
				if ( pDefuser )
				{
					pPlayer->hasDefuseKit = ( pDefuser->m_pPropValue->m_value.m_int == 1 ) ? true : false;
				}
			}

			//Getting money			
			PropEntry *pMoney = pEntity->FindProp( "m_iAccount" );
			if( pMoney )
			{
				pPlayer->money = pMoney->m_pPropValue->m_value.m_int;
			}

			//Getting flashed state
			//Only gives the initial duration - doesn't decay with time
			//Just check if duration > 0, indicating player is flashed
			PropEntry *pFlash = pEntity->FindProp( "m_flFlashDuration" );
			if( pFlash ){
				pPlayer->flashDuration = pFlash->m_pPropValue->m_value.m_float;
			}

			//TODO
			//Getting active weapon and total equipment value
			//PropEntry *pActiveWeapon = pEntity->FindProp( "m_hActiveWeapon" );
		}
	}

	/*
	printf( "Player positions: \n" );
	for ( std::vector< Player* >::iterator it = s_PlayerInstances.begin(); it != s_PlayerInstances.end(); ++it )
	{			
		if ( ( *it )->GetIsConnected() )
		{				
			( *it )->Print();
		}
		//( *it )->Print();
	}*/
}


/************************Game Event Handling************************/

void CDemoFileDump::HandlePlayerConnection( const CSVCMsg_GameEvent &msg, const CSVCMsg_GameEventList::descriptor_t *pDescriptor, bool connection )
{
	int numKeys = msg.keys().size();
	int userid = -1;
	unsigned int index = -1;
	const char *name = NULL;
	bool bBot = false;
	const char *reason = NULL;
	int GUID = -1;

	for ( int i = 0; i < numKeys; i++ )
	{
		const CSVCMsg_GameEventList::key_t& Key = pDescriptor->keys( i );
		const CSVCMsg_GameEvent::key_t& KeyValue = msg.keys( i );
		if ( Key.name().compare( "userid" ) == 0 )
		{
			userid = KeyValue.val_short();
		}
		else if ( Key.name().compare( "index" ) == 0 )
		{
			index = KeyValue.val_byte();
		}
		else if ( Key.name().compare( "name" ) == 0 )
		{
			name = KeyValue.val_string().c_str();
		}
		//This is the GUID
		else if ( Key.name().compare( "networkid" ) == 0 )
		{
			bBot = ( KeyValue.val_string().compare( "BOT" ) == 0 );
			const char* GUIDString = KeyValue.val_string().c_str();
			if ( !bBot )
			{
				//TODO Probably error check this
				GUID = std::stoi( GUIDString + 10 );
			}
		}
		else if ( Key.name().compare( "bot" ) == 0 )
		{
			bBot = KeyValue.val_bool();
		}
		else if ( Key.name().compare( "reason" ) == 0 )
		{
			reason = KeyValue.val_string().c_str();
		}
	}

	if ( connection )
	{
		if ( !bBot )
		{
			Player* connectingPlayer = FindPlayerInstanceByGUID( GUID );
			//New player connecting
			if ( !connectingPlayer )
			{			
				//printf("	----- Player %d %s (id:%d) connected. EntityID: %d \n", GUID, name, userid, index + 1);
				s_PlayerInstances.push_back( new Player( GUID, index, userid, name, bBot ) );
			}
			//Existing playing reconnected
			else
			{			
				//printf("	----- Player %d %s (id:%d) reconnected. EntityID: %d \n", GUID, name, userid, index + 1);
				//Once a player reconnects they get a new userID and entityID? thanks volvo
				connectingPlayer->userID = userid;
				connectingPlayer->entityID = index;
				connectingPlayer->SetIsConnected( true );
				connectingPlayer->SetStatus( PLAYER_DEFAULT );
			}
		}
		else
		{
			//Bots don't have a GUID, and userids/entity ids aren't static. We'll find (existing) bots by name
			//Might make this a function
			Player* connectingBot = NULL;
			for ( std::vector< Player* >::iterator it = s_PlayerInstances.begin(); it != s_PlayerInstances.end(); ++it )
			{			
				if ( ( *it )->GetName().compare( name ) == 0 && ( *it )->GetIsBot() )
				{				
					connectingBot = *it;
				}
			}

			if ( !connectingBot )
			{				
				//Gives bots a unique GUID for our own reference
				GUID = -100 * ( ++s_nBots );
				printf("	----- Bot %d %s (id:%d) connected. EntityID: %d \n", GUID, name, userid, index + 1);
				s_PlayerInstances.push_back( new Player( GUID, index, userid, name, bBot ) );
			}
			else
			{
				//idk if a bot can reconnect but here it is
				//printf("	----- Bot %d %s (id:%d) reconnected. EntityID: %d \n", connectingBot->GetGUID(), name, userid, index + 1);
				connectingBot->userID = userid;
				connectingBot->entityID = index;
				connectingBot->SetIsConnected( true );
				connectingBot->SetStatus( PLAYER_DEFAULT );
			}
		}
	}
	else
	{
		if ( !bBot )
		{
			Player* disconnectingPlayer = FindPlayerInstanceByGUID( GUID );
			//If this fails to find, we have a problem lmao
			if ( disconnectingPlayer )
			{
				//printf("	----- Player %d %s (id:%d) disconnected. EntityID: %d. Reason: %s\n", GUID, name, userid, disconnectingPlayer->entityID, reason);
				disconnectingPlayer->SetIsConnected( false );
				disconnectingPlayer->entityID = -1;
				disconnectingPlayer->SetStatus( PLAYER_DEFAULT );	
			}
		}
		else
		{			
			Player* disconnectingBot = NULL;
			//Bots don't have a GUID in the networked message, so we have to find it by name
			for ( std::vector< Player* >::iterator it = s_PlayerInstances.begin(); it != s_PlayerInstances.end(); ++it )
			{			
				if ( ( *it )->GetName().compare( name ) == 0 && ( *it )->GetIsBot())
				{				
					disconnectingBot = *it;
				}
			}		
			if ( disconnectingBot )
			{
				//printf("	----- Bot %d %s (id:%d) disconnected. EntityID: %d. Reason: %s\n", disconnectingBot->GetGUID(), name, userid, disconnectingBot->entityID, reason);
				disconnectingBot->SetIsConnected( false );
				disconnectingBot->entityID = -1;
				disconnectingBot->SetStatus( PLAYER_DEFAULT );	
			}
		}
	}
}

void CDemoFileDump::HandleBombEvent( const CSVCMsg_GameEvent &msg, const CSVCMsg_GameEventList::descriptor_t *pDescriptor, BombEvent event )
{
	int userid = msg.keys( 0 ).val_short();
	int bombsite = -1;
	char* action = NULL;
	Player* player = FindPlayerInstance( userid );
	EntityEntry *pEntity = FindEntity( player->entityID + 1 );

	if ( event != B_PICK_UP && event != B_DROP )
	{
		//TODO dictionary to convert bombsite long into string
		bombsite = msg.keys( 1 ).val_long();		
	}

	switch ( event )
	{
		case B_BEGIN_PLANT:
			{
				action = "planter at";
				player->SetStatus( PLAYER_PLANTING );
				//printf( "	----- Bomb planting on %d ------\n", bombsite );
			}
			break;

		case B_ABORT_PLANT:
			{
				action = "planter at";
				player->SetStatus( PLAYER_DEFAULT );
				//printf( "	----- Bomb plant has been aborted on %d ------\n", bombsite );
			}
			break;

		case B_COMPLETE_PLANT:
			{
				action = "planter at";
				player->SetHasBomb( false );
				player->SetStatus( PLAYER_DEFAULT );
				bomb->SetX( player->x );
				bomb->SetY( player->y );
				bomb->SetZ( player->z );
				bomb->SetBombStatus( BOMB_PLANTED );
				//printf( "	----- Bomb has been planted on %d ------\n", bombsite );				
			}
			break;

		case B_BEGIN_DEFUSE:
			{
				action = "defuser at";
				player->SetStatus( PLAYER_DEFUSING );
				//printf( "	----- Bomb defusing on %d ------\n", bombsite );
			}
			break;

		case B_ABORT_DEFUSE:
			{
				action = "defuser at";
				player->SetStatus( PLAYER_DEFAULT );
				//printf( "	----- Bomb has been defused on %d ------\n", bombsite );				
			}
			break;

		case B_COMPLETE_DEFUSE:
			{
				action = "defuser at";
				player->SetStatus( PLAYER_DEFAULT );
				bomb->SetBombStatus( BOMB_DEFUSED );
				//printf( "	----- Bomb defuse has been aborted on %d ------\n", bombsite );				
			}
			break;

		case B_PICK_UP:
			{
				action = "picked up at";
				player->SetHasBomb( true );
				bomb->SetX( player->x );
				bomb->SetY( player->y );
				bomb->SetZ( player->z );
				bomb->SetBombStatus( BOMB_ON_PLAYER );
				//printf( "	----- Bomb has been picked up by %s -----\n", player->GetName().c_str() );
			}
			break;

		case B_DROP:
			{
				action = "dropped at";
				player->SetHasBomb( false );
				bomb->SetX( player->x );
				bomb->SetY( player->y );
				bomb->SetZ( player->z );
				bomb->SetBombStatus( BOMB_DROPPED );
				//printf( "	----- Bomb has been dropped by %s -----\n", player->GetName().c_str() );
			}
			break;

		case B_EXPLODE:
			{
				action = "dropped at";
				bomb->SetBombStatus( BOMB_EXPLODED );
				//printf( "	----- Bomb has exploded on %d -----\n", bombsite );
			}
			break;

		default:
			break;
	}

	//Position data
	if ( pEntity && event != B_EXPLODE )
	{
		PropEntry* pXYProp = pEntity->FindProp( "m_vecOrigin" );
		PropEntry* pZProp = pEntity->FindProp( "m_vecOrigin[2]" );
		if ( pXYProp && pZProp )
		{			
			//printf("  	----- %s %s position: %f, %f, %f\n", player->GetName().c_str(), action, pXYProp->m_pPropValue->m_value.m_vector.x, pXYProp->m_pPropValue->m_value.m_vector.y, pZProp->m_pPropValue->m_value.m_float );
		}
	}
}

void CDemoFileDump::HandleGrenadeEvent( const CSVCMsg_GameEvent &msg, const CSVCMsg_GameEventList::descriptor_t *pDescriptor, GrenadeEvent event )
{
	int userid = -1;
	int entityID = -1;
	float x, y, z = -1;
	Player* planter = NULL;

	if ( event != MOLOTOV_START && event != MOLOTOV_EXPIRE && event != MOLOTOV_EXTINGUISH )
	{
		userid = msg.keys( 0 ).val_short();
		entityID = msg.keys( 1 ).val_short();
		x = msg.keys( 2 ).val_float();
		y = msg.keys( 3 ).val_float();
		z = msg.keys( 4 ).val_float();
		planter = FindPlayerInstance( userid );
	}
	else{
		entityID = msg.keys( 0 ).val_short();
		x = msg.keys( 1 ).val_float();
		y = msg.keys( 2 ).val_float();
		z = msg.keys( 3 ).val_float();
	}

	switch ( event )
	{
		case MOLOTOV_START:
			{
				s_GrenadeEntities.push_back(new GrenadeEntity( G_MOLOTOV, x, y, z, entityID ) );
				//printf( "	----- Molotov burning at %f, %f, %f. ------\n", x, y, z );
			}
			break;

		case MOLOTOV_EXPIRE:
			{
				SetRemoveGrenadeEntity( entityID );
				//printf( "	----- Molotov expired at %f, %f, %f. ------\n", x, y, z );
			}
			break;

		case MOLOTOV_EXTINGUISH:
			{
				SetRemoveGrenadeEntity( entityID );
				//printf( "	----- Molotov extinguished at %f, %f, %f. ------\n", x, y, z );
			}
			break;

		case SMOKE_DETONATE:
			{
				s_GrenadeEntities.push_back(new GrenadeEntity( G_SMOKE, x, y, z, entityID ) );
				//printf( "	----- Smoke detonated at %f, %f, %f. Thrown by %s ------\n", x, y, z, planter->GetName().c_str() );		
			}
			break;

		case SMOKE_EXPIRED:
			{
				SetRemoveGrenadeEntity( entityID );
				//printf( "	----- Smoke expired at %f, %f, %f. Thrown by %s ------\n", x, y, z, planter->GetName().c_str() );
			}
			break;

		case FLASH_DETONATE:
			{
				s_GrenadeEntities.push_back(new GrenadeEntity( G_FLASH, x, y, z, entityID ) );
				//printf( "	----- Flashbang detonated at %f, %f, %f. Thrown by %s ------\n", x, y, z, planter->GetName().c_str() );	
			}
			break;

		case HE_DETONATE:
			{
				s_GrenadeEntities.push_back(new GrenadeEntity( G_HE, x, y, z, entityID ) );
				//printf( "	----- HE detonated at %f, %f, %f. Thrown by %s ------\n", x, y, z, planter->GetName().c_str() );
			}
			break;

		case DECOY_START:
			{
				s_GrenadeEntities.push_back(new GrenadeEntity( G_DECOY, x, y, z, entityID ) );
				//printf( "	----- Decoy started at %f, %f, %f. Thrown by %s ------\n", x, y, z, planter->GetName().c_str() );
			}
			break;

		case DECOY_FIRE:
			{
				for ( std::vector< GrenadeEntity* >::iterator it = s_GrenadeEntities.begin(); it != s_GrenadeEntities.end(); ++it )
				{			
					if( ( *it )->GetEntityID() == entityID )
					{
						( *it )->SetDecoyFiring( true );
						break;
					}
				}
				//printf( "	----- Decoy fired at %f, %f, %f. Thrown by %s ------\n", x, y, z, planter->GetName().c_str() );
			}
			break;

		case DECOY_DETONATE:
			{
				SetRemoveGrenadeEntity( entityID );
				//printf( "	----- Decoy detonated at %f, %f, %f. Thrown by %s ------\n", x, y, z, planter->GetName().c_str() );
			}
			break;

		default:
			break;
	}
}

void CDemoFileDump::HandleWeaponFire( const CSVCMsg_GameEvent &msg, const CSVCMsg_GameEventList::descriptor_t *pDescriptor )
{
	int userid = msg.keys( 0 ).val_short();
	const char *weapon = msg.keys( 1 ).val_string().c_str();
	Player* player = FindPlayerInstance( userid );

	player->SetStatus( PLAYER_FIRING );
	
	printf( "	----- %s fired weapon %s. -----\n", player->GetName().c_str(), weapon );

	/*//Probing active weapon id
	EntityEntry *pEntity = FindEntity( player->entityID + 1 );
	PropEntry *pActiveWeapon = pEntity->FindProp( "m_hActiveWeapon" );
	int test;
	int max = 11;
	int mask = ( ( 1 << max ) - 1 );
	test = pActiveWeapon->m_pPropValue->m_value.m_int & mask;
	printf( "%d\n", test );

	EntityEntry *pGun = FindEntity( test );
	if( test )
	{
		printf("found pgun\n");
		for (int i = 0; i < pGun->m_props.size(); i++)
		{
			//std::cout << pGun->m_props.at(i)->m_pFlattenedProp->m_arrayElementProp->DebugString() << std::endl;
			//std::cout << name << std::endl;
			std::cout << pGun->m_props.at(i)->m_pPropValue->m_type << std::endl;
		}
	}*/

	//TODO add weapon fire event to event array
	//is this necessary? player entities already have a field when they are firing
}

void CDemoFileDump::HandlePlayerBlind( const CSVCMsg_GameEvent &msg, const CSVCMsg_GameEventList::descriptor_t *pDescriptor )
{
	int userid = msg.keys( 0 ).val_short();
	Player* player = FindPlayerInstance( userid );

	int attackerid = msg.keys( 1 ).val_short();
	Player* attacker = FindPlayerInstance( attackerid );

	//Can tick up number of people flashed by this entityID
	int entityID = msg.keys( 2 ).val_short();
	float blindDuration = msg.keys( 3 ).val_float();

	//Sometimes finds gotv by accident??
	if ( player && attacker )
	{
		//printf( "	----- Player %s flashed by %s for %f seconds. -----\n", player->GetName().c_str(), attacker->GetName().c_str(), blindDuration );
	}
	
	//TODO add to event array
	//is this necessary? player entities already have a field when they are blind
}

void CDemoFileDump::HandlePlayerHurt( const CSVCMsg_GameEvent &msg, const CSVCMsg_GameEventList::descriptor_t *pDescriptor )
{
	int userid = msg.keys( 0 ).val_short();
	Player* player = FindPlayerInstance( userid );
	int playerGUID = player->GetGUID();

	int attackerid = msg.keys( 1 ).val_short();
	Player* attacker = FindPlayerInstance( attackerid );
	int attackerGUID = -404;
	if ( attacker )
	{
		attackerGUID = attacker->GetGUID();
	}

	int health = msg.keys( 2 ).val_byte();
	int armour = msg.keys( 3 ).val_byte();
	const char* weapon = msg.keys( 4 ).val_string().c_str();
	int healthDamage = msg.keys( 5 ).val_short();
	int armourDamage = msg.keys( 6 ).val_byte();
	int hitgroup = msg.keys( 7 ).val_byte();

	s_PlayerHurtEvents.push_back( new PlayerHurtEvent( playerGUID, attackerGUID, healthDamage, armourDamage, weapon, hitgroup ) );

	/*if ( attacker )
	{
		printf( "	----- Player %s hurt for %d damage (%d armour) by %s (weapon: %s, hitgroup: %d). -----\n", player->GetName().c_str(), healthDamage, armourDamage, attacker->GetName().c_str(), weapon, hitgroup );
	}
	else
	{
		printf( "	----- Player %s hurt for %d damage (%d armour) by world. -----\n", player->GetName().c_str(), healthDamage, armourDamage );
	}
	printf( "		----- Remaining health: %d, remaining armour: %d\n", health, armour );*/
}

void CDemoFileDump::HandlePlayerDeath( const CSVCMsg_GameEvent &msg, const CSVCMsg_GameEventList::descriptor_t *pDescriptor )
{
	int userid = msg.keys( 0 ).val_short();
	Player* player = FindPlayerInstance( userid );
	int playerGUID = player->GetGUID();

	int attackerid = msg.keys( 1 ).val_short();	
	Player* attacker = FindPlayerInstance( attackerid );
	int attackerGUID = -404;
	if ( attacker )
	{
		attackerGUID = attacker->GetGUID();
	}

	int assisterid = msg.keys( 2 ).val_short();	
	Player* assister = FindPlayerInstance( assisterid );
	int assisterGUID = -404;
	if ( assister )
	{
		assisterGUID = assister->GetGUID();
	}

	bool assistedFlash = msg.keys( 3 ).val_bool();
	const char *weaponName = msg.keys( 4 ).val_string().c_str();
	bool headshot = msg.keys( 8 ).val_bool();
	bool wallbang = msg.keys( 12 ).val_bool();
	bool throughSmoke = msg.keys( 15 ).val_bool();
	bool whileBlind = msg.keys( 16 ).val_bool();
	float distance = msg.keys( 17 ).val_float();

	s_PlayerDeathEvents.push_back( new PlayerDeathEvent( playerGUID, attackerGUID, assisterGUID, assistedFlash, weaponName, headshot, wallbang, throughSmoke, whileBlind, distance ) );

	/*if ( player )
	{
		if ( attacker )
		{
			if ( assister )
			{
				printf( "	----- Player %s died to %s (Assist: %s). -----\n", player->GetName().c_str(), attacker->GetName().c_str(), assister->GetName().c_str() );
			}
			else
			{
				printf( "	----- Player %s died to %s. -----\n", player->GetName().c_str(), attacker->GetName().c_str() );			
			}
		}
		else
		{
			printf( "	----- Player %s died. -----\n", player->GetName().c_str() );
		}
		printf( "		----- Weapon: %s \n", weaponName );

		if ( assistedFlash )
		{		
			printf( "		----- Assisted flash \n" );
		}
		if ( headshot )
		{
			printf( "		----- Headshot \n");
		}
		if ( wallbang )
		{
			printf( "		----- Wallbang \n");
		}
		if ( throughSmoke )
		{
			printf( "		----- Through smoke \n");
		}
		if ( whileBlind )
		{
			printf( "		------ While blind \n");
		}
		player->SetStatus( PLAYER_DEFAULT );
	}
	else
	{
		printf( "	----- Player %d died to disconnection. -----\n", userid );
	}*/
}

//TODO more clean up
void CDemoFileDump::TickCleanUp()
{
	for ( std::vector< Player* >::iterator it = s_PlayerInstances.begin(); it != s_PlayerInstances.end(); ++it )
	{			
		if ( ( *it )->GetIsConnected() )
		{				
			//Reset status
			( *it )->TickCleanUp();
		}
	}
	bomb->TickCleanUp( s_nCurrentTick - s_nPreviousTick );

	for ( std::vector< GrenadeEntity* >::iterator it = s_GrenadeEntities.begin(); it != s_GrenadeEntities.end(); )
	{
		( *it )->TickCleanUp( s_nCurrentTick - s_nPreviousTick );

		if ( ( *it )->GetReadyToRemove() )
		{
			delete *it;
			s_GrenadeEntities.erase(it);
		}
		else
		{
			++it;
		}
	}

	//Hurt events exist for the single tick and are cleared afterwards
	for ( std::vector< PlayerHurtEvent* >::iterator it = s_PlayerHurtEvents.begin(); it != s_PlayerHurtEvents.end(); ++it )
	{
		delete *it;
	}
	s_PlayerHurtEvents.clear();

	//Deaths events exist for the single tick and are cleared afterwards
	for ( std::vector< PlayerDeathEvent* >::iterator it = s_PlayerDeathEvents.begin(); it != s_PlayerDeathEvents.end(); ++it )
	{
		delete *it;
	}
	s_PlayerDeathEvents.clear();
}

//TODO more cleanup
void CDemoFileDump::HandleRoundCleanUp()
{
	//Call RoundCleanUp() on each player
	for ( std::vector< Player* >::iterator it = s_PlayerInstances.begin(); it != s_PlayerInstances.end(); ++it )
	{			
		if ( ( *it )->GetIsConnected() )
		{				
			( *it )->RoundCleanUp();
		}
	}
	bomb->RoundCleanUp();

	//Clear all grenades at round end
	for ( std::vector< GrenadeEntity* >::iterator it = s_GrenadeEntities.begin(); it != s_GrenadeEntities.end(); ++it )
	{			
		delete *it;
	}
	s_GrenadeEntities.clear();
}


/************************Parsing Demo************************/

void CDemoFileDump::ParseGameEvent( const CSVCMsg_GameEvent &msg, const CSVCMsg_GameEventList::descriptor_t *pDescriptor )
{
	if ( pDescriptor )
	{
		std::string gameEvent = pDescriptor->name();

		//Handling player connect events
		if ( gameEvent.compare( "player_connect" ) == 0 )
		{
			HandlePlayerConnection( msg, pDescriptor, true );
		}
		else if ( gameEvent.compare( "player_disconnect" ) == 0 )
		{
			HandlePlayerConnection( msg, pDescriptor, false );
		}
		//Round starts before match starts, so the bomb can be picked up before the match starts
		else if ( gameEvent.compare( "bomb_pickup" ) == 0 )
		{
			HandleBombEvent( msg, pDescriptor, B_PICK_UP );
		}
		else if ( gameEvent.compare( "begin_new_match" ) == 0 )
		{
			s_bMatchStartOccured = true;
			printf("----- Match has begun -----\n");
		}
		else if ( s_bMatchStartOccured )
		{
			//Handling round game events
			if (gameEvent.compare( "round_start" ) == 0 )
			{
				s_nCurrentRound++;
				s_RoundStatus = ROUND_FREEZE;
				printf("----- Round %d has started -----\n", s_nCurrentRound);
			}
			else if ( gameEvent.compare( "round_freeze_end" ) == 0 )
			{
				s_RoundStatus = ROUND_REGULAR;
				printf("----- Round has unfrozen -----\n");
			}
			else if ( gameEvent.compare( "round_end" ) == 0 )
			{
				s_RoundStatus = ROUND_AFTER;
				printf("----- Round %d has ended -----\n", s_nCurrentRound);
			}
			else if (gameEvent.compare( "round_officially_ended" ) == 0 )
			{
				this->HandleRoundCleanUp();
				printf("----- Round %d after-time has ended -----\n", s_nCurrentRound);
			}

			//Handling bomb game events
			else if ( gameEvent.compare( "bomb_beginplant" ) == 0 )
			{
				HandleBombEvent( msg, pDescriptor, B_BEGIN_PLANT );
			}
			else if ( gameEvent.compare( "bomb_abortplant" ) == 0 )
			{
				HandleBombEvent( msg, pDescriptor, B_ABORT_PLANT );
			}
			else if ( gameEvent.compare( "bomb_planted" ) == 0 )
			{
				HandleBombEvent( msg, pDescriptor, B_COMPLETE_PLANT );
			}
			else if ( gameEvent.compare( "bomb_begin_defuse" ) == 0 )
			{
				HandleBombEvent( msg, pDescriptor, B_BEGIN_DEFUSE );
			}
			else if ( gameEvent.compare( "bomb_abort_defuse" ) == 0 )
			{
				HandleBombEvent( msg, pDescriptor, B_ABORT_DEFUSE );
			}
			else if ( gameEvent.compare( "bomb_defused" ) == 0 )
			{
				HandleBombEvent( msg, pDescriptor, B_COMPLETE_DEFUSE );
			}
			else if ( gameEvent.compare( "bomb_dropped" ) == 0 )
			{
				HandleBombEvent( msg, pDescriptor, B_DROP );
			}
			else if ( gameEvent.compare( "bomb_exploded" ) == 0 )
			{
				HandleBombEvent( msg, pDescriptor, B_EXPLODE );
			}

			//Handling grenade game events
			else if ( gameEvent.compare( "inferno_startburn" ) == 0 )
			{
				HandleGrenadeEvent( msg, pDescriptor, MOLOTOV_START );
			}
			else if ( gameEvent.compare( "inferno_expire" ) == 0 )
			{
				HandleGrenadeEvent( msg, pDescriptor, MOLOTOV_EXPIRE );
			}
			else if ( gameEvent.compare( "inferno_extinguish" ) == 0 )
			{
				HandleGrenadeEvent( msg, pDescriptor, MOLOTOV_EXTINGUISH );
			}
			else if ( gameEvent.compare( "smokegrenade_detonate" ) == 0 )
			{
				HandleGrenadeEvent( msg, pDescriptor, SMOKE_DETONATE );
			}
			else if ( gameEvent.compare( "smokegrenade_expired" ) == 0 )
			{
				HandleGrenadeEvent( msg, pDescriptor, SMOKE_EXPIRED );
			}
			else if ( gameEvent.compare( "flashbang_detonate" ) == 0 )
			{
				HandleGrenadeEvent( msg, pDescriptor, FLASH_DETONATE );
			}
			else if ( gameEvent.compare( "hegrenade_detonate" ) == 0 )
			{
				HandleGrenadeEvent( msg, pDescriptor, HE_DETONATE );
			}
			else if ( gameEvent.compare( "decoy_started" ) == 0 )
			{
				HandleGrenadeEvent( msg, pDescriptor, DECOY_START );
			}
			else if ( gameEvent.compare( "decoy_firing" ) == 0 )
			{
				HandleGrenadeEvent( msg, pDescriptor, DECOY_FIRE );
			}
			else if ( gameEvent.compare( "decoy_detonate" ) == 0 )
			{
				HandleGrenadeEvent( msg, pDescriptor, DECOY_DETONATE );
			}

			//Handling player game events
			else if ( gameEvent.compare( "weapon_fire" ) == 0 )
			{
				HandleWeaponFire( msg, pDescriptor );
			}
			else if ( gameEvent.compare( "player_blind" ) == 0 )
			{
				HandlePlayerBlind( msg, pDescriptor );
			}
			else if ( gameEvent.compare( "player_hurt" ) == 0 )
			{
				HandlePlayerHurt( msg, pDescriptor );
			}
			else if ( gameEvent.compare( "player_death" ) == 0 )
			{
				HandlePlayerDeath( msg, pDescriptor );
			}
		}
	}
}

void CDemoFileDump::ParseToEnd()
{
	//TODO make an init()
	bomb = new BombEntity();
	
	while( ParseNextTick() )
	{

	}
	for ( std::vector< TickInfo >::iterator it = s_TickInfos.begin(); it != s_TickInfos.end(); ++it )
	{			
		it->Print();
	}

	std::ofstream file("output.json");
	json jOutput = s_TickInfos;
	file<<std::setw(4)<<jOutput<<std::endl;

	CleanUp();
}

bool CDemoFileDump::ParseNextTick()
{
	bool b = ParseTick();
	if ( s_bMatchStartOccured )
	{
		GetPlayerInfo();

		//Consolidate information into tick container
		TickInfo tickInfo = TickInfo( s_nCurrentTick, s_nCurrentRound, s_RoundStatus );
		for ( std::vector< Player* >::iterator it = s_PlayerInstances.begin(); it != s_PlayerInstances.end(); ++it )
		{			
			tickInfo.AddPlayer( ( *it ) );
		}
		tickInfo.AddBomb( bomb );

		for ( std::vector< GrenadeEntity* >::iterator it = s_GrenadeEntities.begin(); it != s_GrenadeEntities.end(); ++it )
		{			
			tickInfo.AddGrenade( ( *it ) );
		}

		for ( std::vector< PlayerHurtEvent* >::iterator it = s_PlayerHurtEvents.begin(); it != s_PlayerHurtEvents.end(); ++it )
		{
			tickInfo.AddPlayerHurt( ( *it ) );
		}

		for ( std::vector< PlayerDeathEvent* >::iterator it = s_PlayerDeathEvents.begin(); it != s_PlayerDeathEvents.end(); ++it )
		{
			tickInfo.AddPlayerDeath( ( *it ) );
		}

		s_TickInfos.push_back( tickInfo );
	}
	//Reset for the next tick
	TickCleanUp();
	//printf( "----- End of Tick %d -----\n", s_nCurrentTick );

	return b;
}

bool CDemoFileDump::ParseTick()
{	
	int				tick;
	unsigned char	cmd;
	unsigned char	playerSlot;
	m_demofile.ReadCmdHeader( cmd, tick, playerSlot );

	// COMMAND HANDLERS
	switch ( cmd )
	{
		case dem_synctick:
			break;

		case dem_stop:
			{
				return false;
			}
			break;

		case dem_consolecmd:
			{
				m_demofile.ReadRawData( NULL, 0 );
			}
			break;

		case dem_datatables:
			{
				char *data = ( char * )malloc( DEMO_RECORD_BUFFER_SIZE );
				CBitRead buf( data, DEMO_RECORD_BUFFER_SIZE );
				m_demofile.ReadRawData( ( char* )buf.GetBasePointer(), buf.GetNumBytesLeft() );
				buf.Seek( 0 );
				if ( !ParseDataTable( buf ) )
				{
					printf( "Error parsing data tables. \n" );
				}
				free( data );
			}
			break;

		case dem_stringtables:
			{
				char *data = ( char * )malloc( DEMO_RECORD_BUFFER_SIZE );
				CBitRead buf( data, DEMO_RECORD_BUFFER_SIZE );
				m_demofile.ReadRawData( ( char* )buf.GetBasePointer(), buf.GetNumBytesLeft() );
				buf.Seek( 0 );
				if ( !DumpStringTables( buf ) )
				{
					printf( "Error parsing string tables. \n" );
				}
				free( data );
			}
			break;

		case dem_usercmd:
			{
				int	dummy;
				m_demofile.ReadUserCmd( NULL, dummy );
			}
			break;
		
		case dem_signon:
		case dem_packet:
			{
				HandleDemoPacket();
			}
			break;

		default:
			break;
	}
	return true;
}

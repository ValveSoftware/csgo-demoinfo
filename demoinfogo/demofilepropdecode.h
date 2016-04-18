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

#ifndef DEMOFILEPROPDECODE_H
#define DEMOFILEPROPDECODE_H

enum SendPropType_t
{
	DPT_Int = 0,
	DPT_Float,
	DPT_Vector,
	DPT_VectorXY, // Only encodes the XY of a vector, ignores Z
	DPT_String,
	DPT_Array,	// An array of the base types (can't be of datatables).
	DPT_DataTable,
	DPT_Int64,
	DPT_NUMSendPropTypes
};

#define SPROP_UNSIGNED					( 1 << 0 )	// Unsigned integer data.
#define SPROP_COORD						( 1 << 1 )	// If this is set, the float/vector is treated like a world coordinate. Note that the bit count is ignored in this case.
#define SPROP_NOSCALE					( 1 << 2 )	// For floating point, don't scale into range, just take value as is.
#define SPROP_ROUNDDOWN					( 1 << 3 )	// For floating point, limit high value to range minus one bit unit
#define SPROP_ROUNDUP					( 1 << 4 )	// For floating point, limit low value to range minus one bit unit
#define SPROP_NORMAL					( 1 << 5 )	// If this is set, the vector is treated like a normal (only valid for vectors)
#define SPROP_EXCLUDE					( 1 << 6 )	// This is an exclude prop (not excludED, but it points at another prop to be excluded).
#define SPROP_XYZE						( 1 << 7 )	// Use XYZ/Exponent encoding for vectors.
#define SPROP_INSIDEARRAY				( 1 << 8 )	// This tells us that the property is inside an array, so it shouldn't be put into the flattened property list. Its array will point at it when it needs to.
#define SPROP_PROXY_ALWAYS_YES			( 1 << 9 )	// Set for datatable props using one of the default datatable proxies like SendProxy_DataTableToDataTable that always send the data to all clients.
#define SPROP_IS_A_VECTOR_ELEM			( 1 << 10 )	// Set automatically if SPROP_VECTORELEM is used.
#define SPROP_COLLAPSIBLE				( 1 << 11 )	// Set automatically if it's a datatable with an offset of 0 that doesn't change the pointer (ie: for all automatically-chained base classes).
#define SPROP_COORD_MP					( 1 << 12 ) // Like SPROP_COORD, but special handling for multiplayer games
#define SPROP_COORD_MP_LOWPRECISION 	( 1 << 13 ) // Like SPROP_COORD, but special handling for multiplayer games where the fractional component only gets a 3 bits instead of 5
#define SPROP_COORD_MP_INTEGRAL			( 1 << 14 ) // SPROP_COORD_MP, but coordinates are rounded to integral boundaries
#define SPROP_CELL_COORD				( 1 << 15 ) // Like SPROP_COORD, but special encoding for cell coordinates that can't be negative, bit count indicate maximum value
#define SPROP_CELL_COORD_LOWPRECISION 	( 1 << 16 ) // Like SPROP_CELL_COORD, but special handling where the fractional component only gets a 3 bits instead of 5
#define SPROP_CELL_COORD_INTEGRAL		( 1 << 17 ) // SPROP_CELL_COORD, but coordinates are rounded to integral boundaries
#define SPROP_CHANGES_OFTEN				( 1 << 18 )	// this is an often changed field, moved to head of sendtable so it gets a small index
#define SPROP_VARINT					( 1 << 19 )	// use var int encoded (google protobuf style), note you want to include SPROP_UNSIGNED if needed, its more efficient

#define DT_MAX_STRING_BITS			9
#define DT_MAX_STRING_BUFFERSIZE	(1<<DT_MAX_STRING_BITS)	// Maximum length of a string that can be sent.

struct Prop_t
{
	Prop_t()
	{
	}

	Prop_t( SendPropType_t type )
		: m_type( type )
		, m_nNumElements( 0 )
	{
		// this makes all possible types init to 0's
		m_value.m_vector.Init();
	}

	void Print( int nMaxElements = 0 )
	{
		if ( m_nNumElements > 0 )
		{
			printf( " Element: %d  ", ( nMaxElements ? nMaxElements : m_nNumElements ) - m_nNumElements );
		}

		switch ( m_type )
		{
			case DPT_Int:
				{
					printf( "%d\n", m_value.m_int );
				}
				break;
			case DPT_Float:
				{
					printf( "%f\n", m_value.m_float );
				}
				break;
			case DPT_Vector:
				{
					printf( "%f, %f, %f\n", m_value.m_vector.x, m_value.m_vector.y, m_value.m_vector.z );
				}
				break;
			case DPT_VectorXY:
				{
					printf( "%f, %f\n", m_value.m_vector.x, m_value.m_vector.y );
				}
				break;
			case DPT_String:
				{
					printf( "%s\n", m_value.m_pString );
				}
				break;
			case DPT_Array:
				break;
			case DPT_DataTable:
				break;
			case DPT_Int64:
				{
					printf( "%lld\n", m_value.m_int64 );
				}
				break;
		}

		if ( m_nNumElements > 1 )
		{
			Prop_t *pProp = this;
			pProp[ 1 ].Print( nMaxElements ? nMaxElements : m_nNumElements );
		}
	}

	SendPropType_t m_type;
	union
	{
		int m_int;
		float m_float;
		const char *m_pString;
		int64 m_int64;
		Vector m_vector;
	} m_value;
	int m_nNumElements;
};

struct FlattenedPropEntry;

Prop_t *DecodeProp( CBitRead &entityBitBuffer, FlattenedPropEntry *pFlattenedProp, uint32 uClass, int nFieldIndex, bool bQuiet );

#endif
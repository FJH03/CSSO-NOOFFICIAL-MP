//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//===========================================================================//

#ifndef UTLSTRINGMAP_H
#define UTLSTRINGMAP_H
#ifdef _WIN32
#pragma once
#endif

#include "utlsymbol.h"

template <class T>
class CUtlStringMap
{
public:
	CUtlStringMap( bool caseInsensitive = true ) : m_SymbolTable( 0, 32, caseInsensitive )
	{
	}

	// Get data by the string itself:
	T& operator[]( const char *pString )
	{
		CUtlSymbol symbol = m_SymbolTable.AddString( pString );
		int index = ( int )( UtlSymId_t )symbol;
		if( m_Vector.Count() <= index )
		{
			m_Vector.EnsureCount( index + 1 );
		}
		return m_Vector[index];
	}

	// Get data by the string's symbol table ID - only used to retrieve a pre-existing symbol, not create a new one!
	T& operator[]( UtlSymId_t n )
	{
		Assert( n >=0 && n <= m_Vector.Count() );
		return m_Vector[n];
	}

	const T& operator[]( UtlSymId_t n ) const
	{
		Assert( n >=0 && n <= m_Vector.Count() );
		return m_Vector[n];
	}

	bool Defined( const char *pString ) const
	{
		return m_SymbolTable.Find( pString ) != UTL_INVAL_SYMBOL;
	}

	UtlSymId_t Find( const char *pString ) const
	{
		return m_SymbolTable.Find( pString );
	}

	/// Add a string to the map and also insert an item at 
	/// its location in the same operation. Returns the 
	/// newly created index (or the one that was just 
	/// overwritten, if pString already existed.)
	UtlSymId_t Insert( const char *pString, const T &item )
	{
		CUtlSymbol symbol = m_SymbolTable.AddString( pString );
		UtlSymId_t index = symbol; // implicit coercion
		if ( m_Vector.Count() > index ) 
		{
			// this string is already in the dictionary.

		}
		else if ( m_Vector.Count() == index )
		{
			// this is the expected case when we've added one more to the tail.
			m_Vector.AddToTail( item );
		}
		else // ( m_Vector.Count() < index )
		{
			// this is a strange shouldn't-happen case.
			AssertMsg( false, "CUtlStringMap insert unexpected entries." );
			m_Vector.EnsureCount( index + 1 );
			m_Vector[index] = item;
		}
		return index;
	}

	static UtlSymId_t InvalidIndex()
	{
		return UTL_INVAL_SYMBOL;
	}

	int GetNumStrings( void ) const
	{
		return m_SymbolTable.GetNumStrings();
	}

	const char *String( int n )	const
	{
		return m_SymbolTable.String( n );
	}

	// Clear all of the data from the map
	void Clear()
	{
		m_Vector.RemoveAll();
		m_SymbolTable.RemoveAll();
	}

	void Purge()
	{
		m_Vector.Purge();
		m_SymbolTable.RemoveAll();
	}

	void PurgeAndDeleteElements()
	{
		m_Vector.PurgeAndDeleteElements();
		m_SymbolTable.RemoveAll();
	}



private:
	CUtlVector<T> m_Vector;
	CUtlSymbolTable m_SymbolTable;
};

#endif // UTLSTRINGMAP_H

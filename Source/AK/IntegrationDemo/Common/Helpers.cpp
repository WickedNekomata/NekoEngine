/*******************************************************************************
The content of this file includes portions of the AUDIOKINETIC Wwise Technology
released in source code form as part of the SDK installer package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use this file in accordance with the end user license agreement provided 
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

  Version: v2017.2.6  Build: 6636
  Copyright (c) 2006-2018 Audiokinetic Inc.
*******************************************************************************/

// Helpers.cpp
/// \file 
/// Implements all functions declared in Helpers.h

#include "stdafx.h"
#include "Helpers.h"
#include "Drawing.h"

void IntegrationDemoHelpers::WordWrapString( std::string& io_strWrapped, const size_t in_nMaxWidth )
{
	if ( in_nMaxWidth < 2 )
		return;

	size_t nLineStartIndex = 0;
	size_t nCurrentIndex = 0;
	size_t nNewLine = (size_t)-1;
	size_t nStringEndIndex = io_strWrapped.length();

	bool bFoundNonSpaceCharacter = false;
	std::string spacesAtBeginningOfLine;

	while ( nLineStartIndex < nStringEndIndex )
	{
		while ( ( nCurrentIndex < nStringEndIndex ) && ( ( nCurrentIndex - nLineStartIndex ) < in_nMaxWidth ) )
		{
			const char thisChar = io_strWrapped[nCurrentIndex];

			if ( thisChar == L'\n' )
			{
				// We've reached the end of a hardcoded newline. Let's
				// start again at the next line.
				nCurrentIndex++;
				nLineStartIndex = nCurrentIndex;
				spacesAtBeginningOfLine.clear();
				bFoundNonSpaceCharacter = false;
			}
			else if ( thisChar == L' ' )
			{
				if ( ! bFoundNonSpaceCharacter )
				{
					spacesAtBeginningOfLine.append( io_strWrapped.substr( nCurrentIndex, 1 ) );
				}

				if ( ( (nCurrentIndex+1) < nStringEndIndex )
					&& ( io_strWrapped[nCurrentIndex+1] != L' ' ) )
				{
					// This is a potential location for a newline
					nNewLine = nCurrentIndex;
				}

				nCurrentIndex++;
			}
			else
			{
				if ( ! bFoundNonSpaceCharacter )
				{
					if ( thisChar == L'*' || thisChar == L'-' )
					{
						spacesAtBeginningOfLine.append( " " );
					}
					else
					{
						bFoundNonSpaceCharacter = true;
					}
				}

				nCurrentIndex++;
			}
		}

		if ( nNewLine == (size_t)-1 )
		{
			// Couldn't find a suitable place to cut, let's quit
			return;
		}

		if ( nCurrentIndex >= nStringEndIndex )
		{
			// We reached the end of the string, we're done
			return;
		}

		io_strWrapped[nNewLine] = L'\n'; // -----8<--- Cut here ---------

		// Move to start of next line
		nLineStartIndex = nCurrentIndex = nNewLine+1; 

		if ( ( spacesAtBeginningOfLine.length() > 0 ) && ( nLineStartIndex < nStringEndIndex ) )
		{
			io_strWrapped.insert( nLineStartIndex, spacesAtBeginningOfLine );
			nStringEndIndex += spacesAtBeginningOfLine.length();
		}
		
		spacesAtBeginningOfLine.clear();
		bFoundNonSpaceCharacter = false;
		nNewLine = (size_t)-1;
	}
}


std::string IntegrationDemoHelpers::WordWrapString( const char* in_szOriginal, const size_t in_nMaxWidth )
{
	std::string strText( in_szOriginal );
	WordWrapString( strText, in_nMaxWidth );
	return strText;
}

// Replace all occurrences of a string by an other
void FindReplace(string& io_strLine, const string& in_oldString, const string& in_newString)
{
	const size_t oldSize = in_oldString.length();

	// do nothing if line is shorter than the string to find
	if (oldSize > io_strLine.length())
		return;

	const size_t newSize = in_newString.length();
	for (size_t pos = 0;; pos += newSize)
	{
		// Find the substring to replace
		pos = io_strLine.find(in_oldString, pos);
		if (pos == string::npos)  // Not found
			return;
		// Replace
		io_strLine = io_strLine.substr(0, pos) + in_newString + io_strLine.substr(pos + oldSize);
	}
}

/// Replaces tags by their platform-specific values
void ReplaceTags( string& io_strTagString )
{
	static const char* TAGS[][2] = {
		{ "<<UG_BUTTON1>>",  UG_BUTTON1_NAME  },
		{ "<<UG_BUTTON2>>",  UG_BUTTON2_NAME  },
		{ "<<UG_BUTTON3>>",  UG_BUTTON3_NAME  },
		{ "<<UG_BUTTON4>>",  UG_BUTTON4_NAME  },
		{ "<<UG_BUTTON5>>",  UG_BUTTON5_NAME  },
		{ "<<UG_BUTTON6>>",  UG_BUTTON6_NAME  },
		{ "<<UG_BUTTON7>>",  UG_BUTTON7_NAME  },
		{ "<<UG_BUTTON8>>",  UG_BUTTON8_NAME  },
		{ "<<UG_BUTTON9>>",  UG_BUTTON9_NAME  },
		{ "<<UG_BUTTON10>>", UG_BUTTON10_NAME },
		{ "<<UG_BUTTON11>>", UG_BUTTON11_NAME },
		{ "<<OS_BUTTON>>",   OS_BUTTON_NAME   },
		{ "<<HELP_BUTTON>>",   HELP_BUTTON    },
		{ "<<ACTION_BUTTON>>", ACTION_BUTTON  },
		{ "<<BACK_BUTTON>>",   BACK_BUTTON    },
		{ "<<DIRECTIONAL_TYPE>>", DIRECTIONAL_TYPE },
		{ "<<UG_RIGHT_STICK>>", UG_RIGHT_STICK },
		{ "<<UG_LEFT_STICK>>", UG_LEFT_STICK },
		{ "", "" } // This must be the last one!
	};

	// Cycle through the available tags
	for ( int i = 0; strlen( TAGS[i][0] ) > 0; i++ )
	{
		// Do a find and replace for the current tag
		FindReplace(io_strTagString, TAGS[i][0], TAGS[i][1]);
	}
}

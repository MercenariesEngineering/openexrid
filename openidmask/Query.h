/*

             *
            ***
           *****
   *********************       Mercenaries Engineering SARL
     *****************              Copyright (C) 2016
       *************
         *********        http://www.mercenaries-engineering.com
        ***********
       ****     ****
      **           **

*/

#pragma once

#include "Mask.h"

namespace openidmask
{

// Query pre processes the Mask data for a specific mask selection
// Query then provide the final mask value
// It is possible to have multiple queries on the same Mask
class Query
{
public:
	const openidmask::Mask	*TheMask;

	// Build a query object
	// The match prototype is : bool match(const char *name)
	// The mask pointer must be valid during the queries
	// The mask pointer won't be deleted by the Query object
	template<class Match>
	inline Query (const Mask *mask, Match match) : TheMask (mask)
	{
		const size_t namesN = mask->_NamesIndexes.size ();
		_State.resize (namesN);
		for (size_t i = 0; i < namesN; ++i)
			_State[i] = match (&TheMask->_Names[mask->_NamesIndexes[i]]);
	}

	// Get the pixel coverage for this query
	// This method is thread safe
	inline float getCoverage (int x, int y)
	{
		const size_t index = x+y*TheMask->_Width;
		const uint32_t begin = TheMask->_PixelsIndexes[index];
		const uint32_t end = TheMask->_PixelsIndexes[index+1];

		float coverage = 0;
		for (uint32_t i = begin; i < end; ++i)
			coverage += _State[TheMask->_Ids[i]] ? (float)TheMask->_Coverage[i] : 0.f;

		return coverage;
	}

	// Is this Id selected
	// This method is thread safe
	inline bool isSelected (uint32_t id)
	{
		return _State[id];
	}

private:

	// The match status of every name
	std::vector<bool>	_State;
};

}

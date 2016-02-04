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
#include <vector>
#include <stdint.h>

namespace openidmask
{

// A pixel sample. A fragment is composed of a name id and a coverage.
class Sample
{
public:
	inline Sample (uint32_t id, float coverage) : Id(id), Coverage (coverage) {}
	
	// The Id of the string in the strings vector associated with this coverage value.
	uint32_t	Id;
	
	// The coverage in the pixel for this string.
	// The sum of the coverage values in a pixel shoud be between [0,1].
	float		Coverage;

	// Compare the Id. Used by vector::find().
	inline bool operator== (uint32_t id) const
	{
		return Id == id;
	}
};

}

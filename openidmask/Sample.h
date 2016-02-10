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
#include <half.h>

namespace openidmask
{

// A pixel sample. A fragment is composed of a name id and a coverage.
class Sample
{
public:
	// The Id of the string in the strings vector associated with this coverage value.
	uint32_t	Id;
	
	// The values in the pixel for this string.
	// The sum of the coverage values in a pixel shoud be between [0,1].
	std::vector<half> Values;
};

}

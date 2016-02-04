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

#include "Builder.h"
#include <algorithm>

using namespace openidmask;
using namespace std;

//**********************************************************************

Builder::Builder (int width, int height) : _Width (width), 	_Height (height), _Pixels (width*height) {}

//**********************************************************************

void Builder::addCoverage (int x, int y, float coverage, uint32_t id)
{
	// The pixel sample list
	vector<Sample> &pixel = _Pixels[x+y*_Width];

	// Find a sample for this id
	vector<Sample>::iterator ite = find (pixel.begin (), pixel.end (), id);
	if (ite == pixel.end())
		// No sample yet for this object, add one entry
		pixel.emplace_back (id, coverage);
	else
		// Accumulate the coverage for this object
		ite->Coverage += coverage;
}

//**********************************************************************

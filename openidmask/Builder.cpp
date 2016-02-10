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

Builder::Builder (int width, int height, const std::vector<std::string> &slices) : _Width (width), 	_Height (height), _Pixels (width*height), _Slices (slices) {}

//**********************************************************************

void Builder::addCoverage (int x, int y, uint32_t id, const float *sliceValues)
{
	// The pixel sample list
	SampleList &sl = _Pixels[x+y*_Width];
	sl.addCoverage (id, sliceValues, (int)_Slices.size ());
}

//**********************************************************************

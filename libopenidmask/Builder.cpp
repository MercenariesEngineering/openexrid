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

using namespace openidmask;
using namespace std;

//**********************************************************************

Builder::Builder (int width, int height) : _Width (width), 	_Height (height), _NextID (0), _Pixels (width*height) {}

//**********************************************************************

void Builder::addCoverage (int x, int y, float coverage, 
	const char *objectName)
{
	// Search/insert the objectName in the name map
	const auto pairib = _NameToId.emplace (objectName, _NextID);

	// Increment the next id if an insertion occured
	_NextID += (int)pairib.second;

	// The final id for this name
	const uint32_t id = pairib.first->second;

	// The pixel sample list
	auto &pixel = _Pixels[x+y*_Width];

	// Find a sample for this id
	auto ite = find (pixel.begin (), pixel.end (), id);
	if (ite == pixel.end())
		// No sample yet for this object, add one entry
		pixel.emplace_back (id, coverage);
	else
		// Accumulate the coverage for this object
		ite->Coverage += coverage;
}

//**********************************************************************

Mask Builder::build () const
{
	return Mask (_Width, _Height, _NameToId, _Pixels);
}

//**********************************************************************

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
#include "Sample.h"

#include <vector>
#include <map>
#include <ImfCompression.h>

namespace openidmask
{

// The Mask object hold the data needed to dynamically craft the mask images.
// The Mask is built using a Builder object. It can be loaded and saved in an EXR file.
// A Query object references a Mask object to craft a specific mask image.
class Mask
{
	friend class Query;
public:

	// Build a mask using the Builder.
	Mask (const class Builder &builder, const std::vector<std::string> &names);

	// Build a Mask using an EXR file.
	// This constructor throws exceptions in case of reading issues.
	Mask (const char *filename);

	// Write the mask in an EXR file.
	void write (const char *filename, Imf::Compression compression=Imf::ZIPS_COMPRESSION) const;

private:

	// The image resolution
	int _Width, _Height;

	// For each name, the index of the begining of the string in the _Names buffer.
	std::vector<uint32_t>	_NamesIndexes;

	// All the names concatenated in a large string. The strings are C strings, 
	// with an ending \0 character.
	std::string				_Names;

	// For each pixels, the index of the first pixel sample in the _Samples vector.
	// The number of sample in the pixel p is (_PixelsIndexes[p+1]-_PixelsIndexes[p]).
	// _PixelsIndexes size is _Width*_Height+1.
	std::vector<uint32_t>	_PixelsIndexes;

	// The pixel samples concatenated in a single vector.
	std::vector<Sample>	_Samples;

	// Mask version
	const uint32_t		_Version = 1;
};

}

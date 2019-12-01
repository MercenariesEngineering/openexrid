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

#include <map>
#include <ImfPixelType.h>
#include "Mask.h"

namespace openexrid
{

// This class is used during the rendering of the image to accumulate
// the coverage data and then build a Mask object.
class Builder
{
	friend class Mask;
public:

	// Initialize a builder
	Builder (int width, int height, const std::vector<std::string> &slices);

	// Add a contribution in the pixel (x,y) for the object 'id'.
	// The sliceValues will be multiplied internally by the weight.
	// The z is the contribution depth, it is used to generate an approximate deep Z value.
	// This method is not thread safe.
	void addCoverage (int x, int y, uint32_t id, float z, float weight, const float *sliceValues);

	// Finish the builder
	// Call it once before to write
	// Once the Builder as been finished, no data can be added.
	// weightSums is the accumulated weights for each pixel
	// which will be used to normalize the contributions.
	void finish (const std::vector<float> &weightSums);

	// Finish
	// Write the mask in an EXR file.
	// names is the concatenated C strings of the object names
	// namesLength is the size of the names buffer
	// If computeDataWindow is true, compute the dataWindow of the none empty pixel and store it in the EXR header
	void write (const char *filename, const char *names, int namesLength, const char *namesHash,
		bool computeDataWindow, Imf::Compression compression=Imf::ZIPS_COMPRESSION) const;

private:

	// The image resolution.
	int	_Width, _Height;

	bool _Finished;

	// The sample list per pixel.
	std::vector<SampleList>	_Pixels;

	std::vector<std::string>	_Slices;
};

}

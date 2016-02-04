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
#include "Mask.h"

namespace openidmask
{

// This class is used during the rendering of the image to accumulate
// the coverage data and then build a Mask object.
class Builder
{
	friend class Mask;
public:

	// Initialize a builder
	Builder (int width, int height);

	// Add some coverage in the pixel (x,y) for the object named objectName.
	// The sum of the coverage of a pixel is supposed to be between [0, 1].
	// This method is not thread safe.
	void addCoverage (int x, int y, float coverage, uint32_t id);

private:

	// The image resolution.
	int	_Width, _Height;

	// The sample list per pixel.
	std::vector<std::vector<Sample> >	_Pixels;
};

}

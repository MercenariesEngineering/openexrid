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
#include <openidmask/Mask.h>
#include <ofxImageEffect.h>

// private instance data type
class Instance
{
public:
	openidmask::Mask Mask;

	// Name of the last mask file loaded
	std::string		LastMaskFilename;

	// handles to the clips we deal with
	OfxImageClipHandle outputClip;

	// handles to a our parameters
	OfxParamHandle fileParam;
	OfxParamHandle patternParam;
	OfxParamHandle colorsParam;
};

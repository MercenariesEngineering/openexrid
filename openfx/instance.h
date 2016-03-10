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
#include <openexrid/Mask.h>
#include <ofxImageEffect.h>

// private instance data type
class Instance
{
public:
	openexrid::Mask Mask;

	// Name of the last mask file loaded
	std::string		LastMaskFilename;

	// handles to the clips we deal with
	OfxImageClipHandle OutputClip;

	// handles to a our parameters
	OfxParamHandle File;
	OfxParamHandle FirstFrame;
	OfxParamHandle LastFrame;
	OfxParamHandle Before;
	OfxParamHandle After;
	OfxParamHandle Frame;
	OfxParamHandle Offset;
	OfxParamHandle MissingFrames;
	OfxParamHandle Pattern;
	OfxParamHandle Colors;
	OfxParamHandle Invert;
	OfxParamHandle Alpha;
};

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

#include "DDImage/Knobs.h"

class PickingKnob : public DD::Image::Knob
{
public:
	class DeepOpenEXRId* theOp;
	const char* Class() const { return "PickingKnob"; }

	// Nuke calls this to draw the handle, this then calls make_handle
	// which tells Nuke to call the above function when the mouse does
	// something...
	void draw_handle(DD::Image::ViewerContext* ctx);

	// And you need to implement this just to make it call draw_handle:
	bool build_handle(DD::Image::ViewerContext* ctx);

	PickingKnob(DD::Image::Knob_Closure* kc, DeepOpenEXRId* t, const char* n);
	
	float x0, y0;
	bool show;
};
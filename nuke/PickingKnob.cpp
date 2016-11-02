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

#include "PickingKnob.h"
#include "DeepOpenEXRId.h"

using namespace DD::Image;

// This is what Nuke will call once the below stuff is executed:
bool handle_cb(ViewerContext* ctx, Knob* knob, int index)
{
	if (ctx->event() == PUSH)
	{
		((PickingKnob*)knob)->show = true;
		((PickingKnob*)knob)->x0 = ctx->x();
		((PickingKnob*)knob)->y0 = ctx->y();
	}
	if (ctx->event() == RELEASE)
	{
		((PickingKnob*)knob)->show = false;
		const float x1 = ctx->x();
		const float y1 = ctx->y();
		((PickingKnob*)knob)->theOp->select(((PickingKnob*)knob)->x0,((PickingKnob*)knob)->y0,x1,y1,
			(ctx->state()&SHIFT)!=0);
	}
	return true;
}

// Nuke calls this to draw the handle, this then calls make_handle
// which tells Nuke to call the above function when the mouse does
// something...
void PickingKnob::draw_handle(ViewerContext* ctx)
{
   if (ctx->event() == DRAW_OPAQUE
        || ctx->event() == PUSH // true for clicking hit-detection
        || ctx->event() == DRAG // true for selection box hit-detection
        ) {

		if (ctx->event() == DRAW_OPAQUE && show)
		{
			const float x1 = ctx->x();
			const float y1 = ctx->y();
			glPushAttrib (GL_ENABLE_BIT|GL_CURRENT_BIT);
			glColor3f(1, 1, 1);
		
			glEnable(GL_COLOR_LOGIC_OP);
			glLogicOp(GL_XOR);
			glLineStipple(1, 0xF0F0);
			glEnable(GL_LINE_STIPPLE);

			glBegin(GL_LINE_STRIP);

				glVertex2f(x0, y0);
				glVertex2f(x1, y0);
				glVertex2f(x1, y1);
				glVertex2f(x0, y1);
				glVertex2f(x0, y0);

			glEnd();
			glPopAttrib ();
		}
      // Make clicks anywhere in the viewer call handle() with index = 0.
      // This takes the lowest precedence over, so above will be detected
      // first.
      begin_handle(Knob::ANYWHERE, ctx, handle_cb, 0 /*index*/, 0, 0, 0 /*xyz*/);
      end_handle(ctx);
    }
}

// And you need to implement this just to make it call draw_handle:
bool PickingKnob::build_handle(ViewerContext* ctx)
{
	// If your handles only work in 2D or 3D mode, only return true
	// in those cases:
	// return (ctx->transform_mode() == VIEWER_2D);
	return true;
}

PickingKnob::PickingKnob(Knob_Closure* kc, DeepOpenEXRId* t, const char* n) : Knob(kc, n), show(false)
{
	theOp = t;
}

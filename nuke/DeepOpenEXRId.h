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

#include "DDImage/DeepFilterOp.h"
#include "DDImage/Knobs.h"

#include <OpenImageIO/ustring.h>

// The DeepOpenEXRId plugin
class DeepOpenEXRId : public DD::Image::DeepFilterOp
{
	const char *_patterns;
  const char *_LPEs;
	bool _colors, _invert, _alpha, _keepVis;
	const char *_version;
	int _mode;


  struct LPEEvent
  {
    OIIO::ustring Type;
    OIIO::ustring Scattering;
    OIIO::ustring Label;
  };

  typedef std::vector<LPEEvent> LightPath;

public:
  DeepOpenEXRId(Node* node) : DeepFilterOp(node) {
  _patterns = NULL;
	_LPEs = NULL;
	_colors = false;
	_invert = false;
	_alpha = false;
	_keepVis = true;
	_version = NULL;
	_mode = 0;
  }

  const char* node_help() const;

  const char* Class() const;

  virtual Op* op();

  void knobs(DD::Image::Knob_Callback f);

  int knob_changed(DD::Image::Knob* k);

  void _validate(bool for_real);

  bool doDeepEngine(DD::Image::Box box, const DD::Image::ChannelSet& channels, DD::Image::DeepOutputPlane& plane);
  
  void select (float x0, float y0, float x1, float y1, bool invert);

  static const Description d;
private:
	bool _getNames (std::vector<std::string> &names);
	bool _getNames_v2 (std::vector<std::string> &names);
	bool _getNames_v3 (std::vector<std::string> &names);
  bool _getLightPaths (std::vector<LightPath> &lightpaths);
};

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

#include "DeepOpenEXRId.h"
#include "PickingKnob.h"
#include "../openexrid/Version.h"

#include "DDImage/MetaData.h"
#include "DDImage/Interest.h"
#include <re2/set.h>
#include <iterator>

#ifdef WIN32
#pragma warning(push, 0)
#endif
#include <OSL/oslconfig.h>
#include <OSL/optautomata.h>
#include "OSL/lpeparse.h"
#ifdef WIN32
#pragma warning(pop)
#endif
// Defined by oiio
#undef copysign
// STOP is used as a keywoard inside oslclosure.h
#undef STOP
#include <OSL/oslclosure.h>

using namespace DD::Image;
using namespace std;

static const char* CLASS = "DeepOpenEXRId";

#include <stdarg.h>
void	log (const char *fmt, ...)
{
	FILE	*f = fopen ("/tmp/exrid.log", "a");
	va_list args;
	va_start (args, fmt);
	vfprintf (f, fmt, args);
	fprintf (f, "\n");
	fclose (f);
}

// Remove the - sign
static inline const char *removeNeg (const std::string &s)
{
	if (!s.empty () && s[0] == '-')
		return s.c_str ()+1;
	return s.c_str ();
}

// Split a multi line text single line strings
static void splitPatterns (std::vector<std::string> &patterns, const char *_patterns)
{
	const char *p = _patterns;
	string str;
	while (true)
	{
		if (*p != '\r')
		{
			if (*p == '\n' || *p == '\0')
			{
				// No empty pattern, they match all the names
				if (!str.empty() && str != "-")
					patterns.push_back (str);
				if (*p == '\0')
					break;
				str = "";
			}
			else
				str += *p;
		}
		++p;
	}
}

// The halton sequence, to get a random color per id
inline float halton (float base, int id)
{
	float result = 0;
	float f = 1;
	float i = (float)id;
	while (i > 0)
	{
		f = f / base;
		result = result + f * fmodf (i, base);
		i = floorf (i / base);
	}
	return result;
}

const char* DeepOpenEXRId::node_help() const
{
return "Select the objects present in an OpenEXR/Id image using the mouse or regular expressions.";
}

const char* DeepOpenEXRId::Class() const {
return CLASS;
}

Op* DeepOpenEXRId::op()
{
	return this;
}

const char* const kapMode[] = {
  "Replace",
  "Add",
  "Remove",
  "Keep",
  NULL
};

enum TMode
{
	ModeReplace=0,
	ModeAdd,
	ModeRemove,
	ModeKeep
};

void DeepOpenEXRId::knobs(Knob_Callback f)
{
	// create the knob needed to get mouse interaction:
	CustomKnob1(PickingKnob, f, this, "kludge");

	Enumeration_knob(f, &_mode, kapMode, "pick_mode", "pick mode");

	Multiline_String_knob(f, &_patterns, "patterns", 0, 20);	
	Tooltip(f, 
		"Each line is a regular expression matching object names like :\n"
		"\"plane\": any objects with a name containing \"plane\"\n"
		"\"plane.*light\": any object with a name containing \"plane\" followed by \"light\"\n"
		"\"^plane\": any object with a name starting by \"plane\"\n"
		"\"plane$ \": any object with a name finishing by \"plane\"\n\n"
		"If the pattern is prefixed by the - character, the objects matching the pattern wille be excluded"
		);

	Multiline_String_knob(f, &_LPEs, "LPEs", 0, 20);	
	Tooltip(f, 
		"Each line is a light path expression:\n"
		"\"C.*L\": All light paths from the camera to any light (beauty)\n"
		"\"CDL\" : Direct diffuse (either reflection, transmission and volume) from all lights\n"
		"\"CD.+L\" : Indirect diffuse from all lights"
		);

	Bool_knob(f, &_colors, "colors", "false colors");
	SetFlags (f, Knob::STARTLINE);
	Tooltip(f, "Show the image with false colors");

	Bool_knob(f, &_invert, "invert", "invert");
	Tooltip(f, "Invert the selection");

	Bool_knob(f, &_alpha, "alpha", "alpha");
	Tooltip(f, "Render just the alpha");

	Bool_knob(f, &_keepVis, "keepVis", "keep visibility");
	Tooltip(f, "Keep the visibility of the previous samples. Check it to get a valid image. Uncheck it to get valid deep samples.");

	f(TEXT_KNOB, Custom, 0, "version", "version", openexrid::Version.c_str());
}

int DeepOpenEXRId::knob_changed(DD::Image::Knob* k)
{
	return 1;
}

void DeepOpenEXRId::_validate(bool for_real)
{
	DeepFilterOp::_validate(for_real);
}

bool DeepOpenEXRId::_getNames (std::vector<std::string> &names)
{
	const MetaData::Bundle &metadata = fetchMetaData ("exr/EXRIdVersion");
	const int EXRIdVersion = atoi(metadata.getString ("exr/EXRIdVersion").c_str());
  if (EXRIdVersion == 3)
    return _getNames_v3 (names);
  else
    return _getNames_v2 (names);
}

extern std::string b64decode (const std::string& str);
extern std::string inflate (const std::string& str);

bool DeepOpenEXRId::_getNames_v2 (std::vector<std::string> &names)
{
	names.clear ();

	// Get the metadata
	const MetaData::Bundle &metadata = fetchMetaData ("exr/EXRIdNames");
	const string namesPacked = metadata.getString ("exr/EXRIdNames");
	if (namesPacked.empty())
	{
		error("No EXRIdNames metadata");
		return false;
	}

	const string namesUnpacked = inflate (namesPacked);

	size_t index = 0;
	while (index < namesUnpacked.size())
	{
		names.push_back (namesUnpacked.c_str()+index);
		index = namesUnpacked.find ('\0', index)+1;
	}
	return true;
}

bool DeepOpenEXRId::_getNames_v3 (std::vector<std::string> &names)
{
	names.clear ();

	// Get the metadata
	const MetaData::Bundle &metadata = fetchMetaData ("exr/EXRIdNames");
	const string namesPacked = metadata.getString ("exr/EXRIdNames");
	if (namesPacked.empty())
	{
		error("No EXRIdNames metadata");
		return false;
	}

	const string namesUnpacked = inflate (b64decode (namesPacked));

	size_t index = 0;
	while (index < namesUnpacked.size())
	{
		size_t	eol = namesUnpacked.find ('\n', index);
		if (eol == std::string::npos)
		{
			names.push_back (namesUnpacked.substr (index));
			break;
		}
		else
			names.push_back (namesUnpacked.substr (index, eol-index));
		index = eol+1;
	}

	return true;
}

bool DeepOpenEXRId::_getLightPaths (std::vector<LightPath> &lightpaths)
{
	// Get the metadata
	const MetaData::Bundle &metadata = fetchMetaData ("exr/EXRIdPaths");
	const string pathsPacked = metadata.getString ("exr/EXRIdPaths");
	if (pathsPacked.empty())
	{
		return false;
	}

	const string pathsUnpacked = inflate (b64decode (pathsPacked));

	const char	*buffer = pathsUnpacked.c_str ();

	while (*buffer != '\0')
	{
		LightPath	lp;
		while (*buffer != '\0' && *buffer != '\n')
		{
			std::string	type, scattering, label;
			while (*buffer != '\0' && *buffer != '\t')
				type += *(buffer++);
			if (*buffer == '\t')
				++buffer;
			while (*buffer != '\0' && *buffer != '\t')
				scattering += *(buffer++);
			if (*buffer == '\t')
				++buffer;
			while (*buffer != '\0' && *buffer != '\t')
				label += *(buffer++);
			if (*buffer == '\t')
				++buffer;

			lp.emplace_back (LPEEvent {OIIO::ustring (type),OIIO::ustring (scattering),OIIO::ustring (label)});
		}

		lightpaths.emplace_back (lp);
		if (*buffer == '\n')
			++buffer;
	}

	return true;
}

bool DeepOpenEXRId::doDeepEngine(DD::Image::Box box, const ChannelSet& channels, DeepOutputPlane& plane)
{
	if (!input0() || _patterns == NULL)
		return true;

	// Split the pattern text entry in patterns
	vector<string> patterns;
	splitPatterns (patterns, _patterns);

	vector<string> lpepatterns;
	splitPatterns (lpepatterns, _LPEs);

	DeepOp* in = input0();

	// Get the Id channel
	DD::Image::ChannelSet requiredChannels = DD::Image::Mask_Alpha;
	Channel idChannel = DD::Image::getChannel ("other.Id");

	std::vector<string> names;
	if (!_getNames (names))
		return false;

	Channel lpeidChannel = Channel (-1);
	std::vector<LightPath> lightpaths;
	bool	useLightPaths = !lpepatterns.empty () && _getLightPaths (lightpaths);
	if (useLightPaths)
		lpeidChannel = DD::Image::getChannel ("other.LPEId");

	// State of each name, true selected
	vector<bool> idStates (names.size(), false);

	// Build a single regular expression automata for all the user patterns
	re2::RE2::Options options;
	re2::RE2::Anchor anchor = re2::RE2::UNANCHORED;
	re2::RE2::Set set (options, anchor);
	for (vector<string>::const_iterator itp = patterns.begin (); itp != patterns.end (); ++itp)
	{
		if (set.Add (removeNeg (*itp), NULL) < 0)
		{
			error("Bad regular expression");
			return false;
		}
	}

	if (!patterns.empty ())
	{
		if (!set.Compile ())
		{
			error("Bad regular expression");
			return false;
		}

		// Match every names against the patterns
		vector<int> matched;
		for (size_t i = 0; i < names.size(); ++i)
		{
			matched.clear ();
			set.Match (names[i], &matched);
			for (vector<int>::iterator it = matched.begin (); it != matched.end (); ++it)
			{
				// Negative match
				if (patterns[*it][0] == '-')
				{
					idStates[i] = false;
					break;
				}
				else
					idStates[i] = true;
			}
		}
	}


	// State of each light path, true selected
	vector<bool> lpeidStates (lightpaths.size (), false);

	if (useLightPaths)
	{
		OSL::NdfAutomata ndfautomata;

		for (auto &reg : lpepatterns)
		{
			const std::vector<OIIO::ustring> userEvents = {OIIO::ustring("I")};
			OSL::Parser parser (&userEvents, nullptr);
			OSL::LPexp *e = parser.parse (reg.c_str ());
			if (parser.error ())
			{
				error ("LPE: Parse error: %s", parser.getErrorMsg ());
				return false;
			}
			else
			{
				auto exp = new OSL::lpexp::Rule (e, (void*)reg.c_str ());
				exp->genAuto (ndfautomata);
			}

			delete e;
		}

		OSL::DfAutomata dfautomata;
		ndfautoToDfauto (ndfautomata, dfautomata);

		OSL::DfOptimizedAutomata	automaton;
		automaton.compileFrom (dfautomata);

		uint32_t	id = 0;
		for (const auto &lightpath : lightpaths)
		{
			int	state = 0;

			for (const auto &e : lightpath)
			{
				if ((state = automaton.getTransition (state, e.Type)) < 0 ||
					(state = automaton.getTransition (state, e.Scattering)) < 0 ||
					(state = automaton.getTransition (state, e.Label)) < 0 ||
					(state = automaton.getTransition (state, OSL::Labels::STOP)) < 0)
					break;
			}

			if (state > 0)
			{
				int nMatchExp = 0;
				const char **matchExps = 
					(const char**)automaton.getRules (state, nMatchExp);

				if (nMatchExp > 0)
				{
					for (int i = 0; i < nMatchExp; ++i)
					{
						if (matchExps[i] != NULL)
						{
							lpeidStates[id] = true;
							break;
						}
					}
				}
			}
				
			++id;
		}
	}


	DeepPlane inPlane;

	ChannelSet needed = channels;
	needed += idChannel;
	if (useLightPaths)
		needed += lpeidChannel;
	needed += Mask_DeepFront;
	needed += Mask_Alpha;

	if (!in->deepEngine(box, needed, inPlane))
		return false;
    
	plane = DeepOutputPlane(channels, box, DeepPixel::eZAscending);
	
	// For the halton colors
	const float primes[3] = {2,3,5};

	for (DD::Image::Box::iterator it = box.begin(); it != box.end(); it++) {

		DeepPixel pixel = inPlane.getPixel(it);

		DeepOutPixel pels;

		// Alpha of the previous samples
		float prevVis = 0.f;
		for (int sample = (int)pixel.getSampleCount()-1; sample >= 0; --sample)
		{
			const float	_id = pixel.getOrderedSample(sample, idChannel);
			const int	id = (int)_id;
			const bool	idselected = (id >= 0) && (id < (int)idStates.size()) && (idStates[id] != _invert);

			float		_lpeid = 0;
			int			lpeid = 0;
			bool		lpeidselected = true;
			if (useLightPaths)
			{
				_lpeid = pixel.getOrderedSample (sample, lpeidChannel);
				lpeid = (int)_lpeid;
				lpeidselected = (lpeid >= 0) && (lpeid < (int)lpeidStates.size ()) && lpeidStates[lpeid];
			}

			if (!idselected || !lpeidselected)
			{
				const float alpha = pixel.getOrderedSample(sample, Chan_Alpha);
				if (_colors)
				{
					// Display unselected samples with false colors
					foreach (channel, channels) 
					{
						if (channel == idChannel)
							pels.push_back(_id);
						else if (channel == lpeidChannel)
							pels.push_back(_lpeid);
						else if (channel == Chan_Alpha)
							pels.push_back(alpha);
						else if (channel == Chan_DeepFront || channel == Chan_DeepBack)
							pels.push_back(pixel.getOrderedSample(sample, channel));
						else
							pels.push_back (alpha*halton (primes[channel%3], id));
					}
				}
				else
				{
					// Not selected, keep the previous visibility
					prevVis += (1.f-prevVis)*alpha;
				}
			}
			else
			{
				foreach (channel, channels) 
				{
					if (channel == idChannel)
						pels.push_back(_id);
					if (channel == lpeidChannel)
						pels.push_back(_lpeid);
					else if (channel == Chan_DeepFront || channel == Chan_DeepBack)
						pels.push_back(pixel.getOrderedSample(sample, channel));
					else
					{
						const float v = pixel.getOrderedSample(sample, _alpha ? Chan_Alpha : channel);
						pels.push_back(v*(_keepVis ? (1.f-prevVis) : 1.f));
					}
				}
			}
		}

		plane.addPixel(pels);
	}

	return true;
}

// Escape the regexp characters
static std::string escapeRegExp (const char *s)
{
	const char *sc = ".^$*+?()[{\\|";
	string result = "^";
	while (*s)
	{
		if (find (sc, sc+12, *s) != sc+12)
			result += "\\\\";
		result += *s;
		++s;
	}
	result += "$";
	return result;
}

static std::string escapeBackslash (const std::string &str)
{
	string result;
	for (size_t i = 0; i < str.size(); ++i)
	{
		const char c = str[i];
		if (c == '\\')
			result += c;
		result += c;
	}
	return result;
}

// Split a string in a string set
static std::set<std::string> getOldName (const char *str)
{
	const std::string delim ("\r\n");
	set<string> result;
	const char *start = str;
	while (true)
	{
		if (*str == '\0' || delim.find (*str) != delim.npos)
		{
			if (start < str)
				result.insert (escapeBackslash (string (start, str)));
			start = str+1;
		}
		if (*str == '\0')
			break;
		++str;
	}
	return result;
}

void DeepOpenEXRId::select (float x0, float y0, float x1, float y1, bool invert)
{
	if (_mode == ModeKeep)
		return;

	DeepOp* in = input0();
	DeepPlane inPlane;

	Channel idChannel = DD::Image::getChannel ("other.Id");
	ChannelSet needed = idChannel;

	const int l = (int)floor(min(x0,x1));
	const int b = (int)floor(min(y0,y1));
	int r = (int)ceil(max(x0,x1));
	int t = (int)ceil(max(y0,y1));
	r += (int)(r==l);
	t += (int)(t==b);

	DD::Image::Box box (l,b,r,t);
	if (!in->deepEngine(box, needed, inPlane))
		return;

	// Click, not drag
	const bool click = box.w() == 1 && box.h() == 1;

	set<int> ids;
	for (DD::Image::Box::iterator it = box.begin(); it != box.end(); it++) {

		DeepPixel pixel = inPlane.getPixel(it);
		for (int sample = (int)pixel.getSampleCount()-1; sample >= 0; --sample) {
			const float _id = pixel.getOrderedSample(sample, idChannel);
			ids.insert ((int)_id);
			if (click)
				break;
		}
	}

	vector<string> id2names;
	if (!_getNames (id2names))
		return;

	// Build a final string set
	set<string> names;
	{
		set<int>::iterator ite = ids.begin();
		while (ite != ids.end())
		{
			const int id = *ite;
			if (id >= 0 && id < (int)id2names.size())
				names.insert (escapeRegExp (id2names[id].c_str()));
			++ite;
		}
	}

	set<string> oldNames = getOldName (_patterns);
	set<string> result;

	if (_mode == ModeReplace && !invert)
		result = names;
	else if (_mode == ModeReplace && invert)
		std::set_symmetric_difference (names.begin(), names.end(), oldNames.begin(), oldNames.end(), std::inserter(result, result.end()));
	else if (_mode == ModeAdd)
		std::set_union (names.begin(), names.end(), oldNames.begin(), oldNames.end(), std::inserter(result, result.end()));
	else if (_mode == ModeRemove)
		std::set_difference (oldNames.begin(), oldNames.end(), names.begin(), names.end(), std::inserter(result, result.end()));

	// Build a final string with the patterns
	string patterns;
	{
		set<string>::iterator ite = result.begin();
		while (ite != result.end())
		{
			if (!patterns.empty())
				patterns += "\n";
			patterns += *ite;
			++ite;
		}
	}
	knob("patterns")->set_text (patterns.c_str());
}

static Op* build(Node* node) { return new DeepOpenEXRId(node); }
const Op::Description DeepOpenEXRId::d(::CLASS, "Deep/DeepOpenEXRId", build);

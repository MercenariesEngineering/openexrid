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

#include <iterator>

#include "../openexrid/Version.h"

#include "DeepOpenEXRId.h"
#include "PickingKnob.h"
#include "md5.h"

#include "DDImage/MetaData.h"
#include "DDImage/Interest.h"

using namespace DD::Image;
using namespace std;

static const char* CLASS = "DeepOpenEXRId";

#include <stdarg.h>

//#define LOG
//#define TIMING_LOG

#ifdef LOG
void	log (const char *fmt, ...)
{
/*
	{
		FILE	*f = fopen ("/tmp/exrid.log", "a");
		va_list args;
		va_start (args, fmt);
		vfprintf (f, fmt, args);
		fprintf (f, "\n");
		fclose (f);
	}
*/
	{
		va_list args;
		va_start (args, fmt);
		vprintf (fmt, args);
		printf ("\n");
	}
}
#else
static inline void log (const char *fmt, ...) {}
#endif

extern std::string b64decode (const std::string& str);
extern std::string inflate (const std::string& str);

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

static void	unpackStringToArray (const std::string &s, std::vector<std::string> &array, char separator)
{
	const char	*p = s.c_str ();
	const char	*end = p + s.size ();
	while (p < end)
	{
		std::string	str;
		while (p < end && *p != separator)
			str += *(p++);
		array.emplace_back (std::move (str));
		if (p < end)
			++p;
	}
}

static std::string MD5DigestToString (const uint8_t *digest)
{
	std::string	r;
	static const char	*hex = "0123456789abcdef";
	for (int i = 0; i < 16; ++i)
	{
		r += hex[(digest[i] >> 4) & 0xf];
		r += hex[digest[i] & 0xf];
	}
	return r;
}

static std::string	hashCString (const char *p)
{
	md5_context	md5;
	md5_starts (&md5);
	md5_update (&md5, (const uint8_t*)p, (uint32_t)strlen (p));
	uint8_t	digest[16];
	md5_finish (&md5, digest);
	return MD5DigestToString (digest);
}

std::shared_ptr<DeepOpenEXRId::ExrIdData> DeepOpenEXRId::_getExrIdData ()
{
	const MetaData::Bundle &metadata = fetchMetaData (NULL);

	// Note: Add a EXRIdHash metadata to speedup the digesting of the metadata
	string hash = metadata.getString ("exr/EXRIdHash");

	// No hash present, compute one directly from the metadata
	// Note: this is fairly slow hence why we suggest adding the metadata hash
	if (hash.empty ())
	{
		const int EXRIdVersion = atoi (metadata.getString ("exr/EXRIdVersion").c_str());
		const string namesPacked = metadata.getString ("exr/EXRIdNames");
		const string pathsPacked = metadata.getString ("exr/EXRIdPaths");

		md5_context	md5;
		md5_starts (&md5);
		md5_update (&md5, (const uint8_t*)(&EXRIdVersion), sizeof (EXRIdVersion));
		md5_update (&md5, (const uint8_t*)(namesPacked.c_str ()), (uint32_t)namesPacked.size ());
		md5_update (&md5, (const uint8_t*)(pathsPacked.c_str ()), (uint32_t)pathsPacked.size ());

		uint8_t	digest[16];
		md5_finish (&md5, digest);

		hash = MD5DigestToString (digest);
	}

	return ExrIdDataCache.get (hash, [&] () -> std::shared_ptr<DeepOpenEXRId::ExrIdData>
	{
		log ("Loading EXRId data ...");

		std::shared_ptr<DeepOpenEXRId::ExrIdData>	data = std::make_shared<DeepOpenEXRId::ExrIdData> ();

		data->Hash = hash;

		const int EXRIdVersion = atoi (metadata.getString ("exr/EXRIdVersion").c_str());
		const string namesPacked = metadata.getString ("exr/EXRIdNames");
		const string pathsPacked = metadata.getString ("exr/EXRIdPaths");

		// Load names
		if (!namesPacked.empty ())
		{
			if (EXRIdVersion == 2)
			{
				const string namesUnpacked = inflate (namesPacked);
				unpackStringToArray (namesUnpacked, data->Names, '\0');
			}
			else
			{
				const string namesUnpacked = inflate (b64decode (namesPacked));
				unpackStringToArray (namesUnpacked, data->Names, '\n');
			}
		}

		// Load light paths
		if (!pathsPacked.empty())
		{
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
					
#if defined(_MSC_VER) && (_MSC_VER <= 1600)
					LPEEvent evt;
					evt.Type = OIIO::ustring (type);
					evt.Scattering = OIIO::ustring (scattering);
					evt.Label = OIIO::ustring (label);
					lp.emplace_back (evt);
#else
					lp.emplace_back (LPEEvent {OIIO::ustring (type),OIIO::ustring (scattering),OIIO::ustring (label)});
#endif
				}

				data->LightPaths.emplace_back (lp);
				if (*buffer == '\n')
					++buffer;
			}
		}

		log ("Unpacked %d names and %d lightpaths", int (data->Names.size ()), int (data->LightPaths.size ()));

		return data;
	});
}

static re2::RE2::Options	_DefaultRe2Options;

DeepOpenEXRId::NameAutomaton::NameAutomaton () :
	RegEx (re2::RE2::Options (), re2::RE2::UNANCHORED)	{}

DeepOpenEXRId::NameAutomatonPtr	DeepOpenEXRId::_getNamesAutomaton ()
{
	// Note: we could keep a hash of _patterns and invalidate it with knob_changed
	// but _patterns is not supposed to grow incontrollably, so there negigeable gain here
	std::string	hash = hashCString (_patterns);
	return NameAutomatonCache.get (hash, [&] () -> std::shared_ptr<NameAutomaton>
	{
		log ("Compiling name automaton ...");

		std::shared_ptr<NameAutomaton>	set = std::make_shared<NameAutomaton> ();

		set->Hash = hash;

		// Split the pattern text entry in patterns
		splitPatterns (set->Patterns, _patterns);
		if (set->Patterns.empty ())
			NameAutomatonPtr ();

		for (std::vector<std::string>::const_iterator pattern = set->Patterns.begin() ; pattern != set->Patterns.end() ; pattern++)
		{
			std::string	err;
			if (set->RegEx.Add (removeNeg (*pattern), &err) < 0)
			{
				((DeepOpenEXRId*)this)->error ("Bad regular expression '%s': %s", pattern->c_str (), err.c_str ());
				return std::make_shared<NameAutomaton> ();
			}
		}

		if (!set->RegEx.Compile ())
		{
			((DeepOpenEXRId*)this)->error ("Automaton compilation error");
			return std::make_shared<NameAutomaton> ();
		}

		log ("Compiled %d regexes", int (set->Patterns.size ()));

		return set;
	});
}

bool	DeepOpenEXRId::NameAutomaton::match (const std::string &name, std::vector<int> &tmp) const
{
	tmp.clear ();
	RegEx.Match (name, &tmp);

	// No match?
	if (tmp.empty ())
		return false;

	// Negative match?
	for (std::vector<int>::const_iterator patternid = tmp.begin() ; patternid != tmp.end() ; patternid++)
		if (Patterns[*patternid][0] == '-')
			return false;

	return true;
}

DeepOpenEXRId::LPEAutomatonPtr	DeepOpenEXRId::_getLPEAutomaton ()
{
	// Note: we could keep a hash of _LPEs and invalidate it with knob_changed
	// but _LPEs is not supposed to grow incontrollably, so there negigeable gain here
	std::string	hash = hashCString (_LPEs);
	return LPEAutomatonCache.get (hash, [&] () -> std::shared_ptr<LPEAutomaton>
	{
		log ("Compiling lpe automaton ...");

		std::shared_ptr<LPEAutomaton>	set = std::make_shared<LPEAutomaton> ();

		set->Hash = hash;

		splitPatterns (set->Patterns, _LPEs);
		if (set->Patterns.empty ())
			return std::make_shared<LPEAutomaton> ();

		OSL::NdfAutomata ndfautomata;
		size_t	patternid = 0;
		for (std::vector<std::string>::const_iterator pattern = set->Patterns.begin() ; pattern != set->Patterns.end() ; pattern++)
		{
			std::vector<OIIO::ustring> userEvents;
			userEvents.push_back(OIIO::ustring("I"));

			OSL::Parser parser (&userEvents, NULL);
			OSL::LPexp *e = parser.parse (removeNeg (*pattern));
			if (parser.error ())
			{
				((DeepOpenEXRId*)this)->error ("Bad light path expression '%s': %s", pattern->c_str (), parser.getErrorMsg ());
				return std::make_shared<LPEAutomaton> ();
			}
			else
			{
				auto exp = new OSL::lpexp::Rule (e, (void*)pattern->c_str ());
				exp->genAuto (ndfautomata);
			}

			delete e;
			++patternid;
		}

		OSL::DfAutomata dfautomata;
		OSL::ndfautoToDfauto (ndfautomata, dfautomata);

		set->LPEx.compileFrom (dfautomata);

		log ("Compiled %d lpes", int (set->Patterns.size ()));

		return set;
	});
}

bool	DeepOpenEXRId::LPEAutomaton::match (const LightPath &lightpath) const
{
	if (LPEx.empty())
		return false;

	int	state = 0;

	// Move the automaton event by event
	// Check that each transition doesn't move us outside the automaton
	for (LightPath::const_iterator evt = lightpath.begin() ; evt != lightpath.end() ; evt++)
		if ((state = LPEx.getTransition (state, evt->Type)) < 0 ||
			(state = LPEx.getTransition (state, evt->Scattering)) < 0 ||
			(state = LPEx.getTransition (state, evt->Label)) < 0 ||
			(state = LPEx.getTransition (state, OSL::Labels::STOP)) < 0)
			return false;

	int nMatchExp = 0;
	const char **matchExps = (const char **)LPEx.getRules (state, nMatchExp);

	// No match?
	if (nMatchExp <= 0)
		return false;

	// Negative match?
	for (int ilpe = 0; ilpe < nMatchExp; ++ilpe)
		if (matchExps[ilpe][0] == '-')
			return false;

	return true;
}

DeepOpenEXRId::StatePtr	DeepOpenEXRId::_getState (
	const ExrIdDataPtr &exrid,
	const NameAutomatonPtr &namesregex,
	const LPEAutomatonPtr &lperegex)
{
	// Quick digest of the input metadata and name patterns and lpes
	md5_context	md5;
	md5_starts (&md5);
	if (exrid != NULL)
		md5_update (&md5, (const uint8_t*)(exrid->Hash.c_str ()), (uint32_t)exrid->Hash.size ());
	if (namesregex != NULL)
		md5_update (&md5, (const uint8_t*)(namesregex->Hash.c_str ()), (uint32_t)namesregex->Hash.size ());
	if (lperegex != NULL)
		md5_update (&md5, (const uint8_t*)(lperegex->Hash.c_str ()), (uint32_t)lperegex->Hash.size ());

	uint8_t	digest[16];
	md5_finish (&md5, digest);

	std::string	hash = MD5DigestToString (digest);

	return StateCache.get (hash, [&] () -> std::shared_ptr<State>
	{
		log ("Building exrid active state ...");

		std::shared_ptr<State>	st = std::make_shared<State> ();

		// State of each name, true selected
		vector<bool>	&idStates = st->IdStates;
		idStates.resize (exrid->Names.size (), false);

		int	nids = 0;
		vector<int> matched;
		// Match every names against the patterns
		if (namesregex != NULL)
			for (size_t i = 0; i < exrid->Names.size (); ++i)
				nids += int (idStates[i] = namesregex->match (exrid->Names[i], matched));

		// State of each light path, true selected
		vector<bool>	&lpeidStates = st->LPEIdStates;
		lpeidStates.resize (exrid->LightPaths.size (), false);

		int	nlpeids = 0;
		// Match every light paths against the patterns
		if (lperegex != NULL)
			for (size_t i = 0; i < exrid->LightPaths.size (); ++i)
				nlpeids += int (lpeidStates[i] = lperegex->match (exrid->LightPaths[i]));

		log ("Generated %d active names and %d active lpes", nids, nlpeids);

		return st;
	});
}

bool DeepOpenEXRId::doDeepEngine(DD::Image::Box box, const ChannelSet& channels, DeepOutputPlane& plane)
{
	if (!input0 () || _patterns == NULL)
		return true;

#ifdef TIMING_LOG
	std::chrono::time_point<std::chrono::system_clock> _start = std::chrono::system_clock::now ();
#endif

	// Load the EXRId metadata
	ExrIdDataPtr		exrid = _getExrIdData ();
	if (exrid->Names.empty ())
		return false;

	// Load the pattern regex and lpe automata
	NameAutomatonPtr	namesregex = _getNamesAutomaton ();
	LPEAutomatonPtr		lperegex = _getLPEAutomaton ();

	// Load the active names and light paths ids
	StatePtr			st = _getState (exrid, namesregex, lperegex);

#ifdef TIMING_LOG
	std::chrono::time_point<std::chrono::system_clock> _end = std::chrono::system_clock::now ();
	auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(_end - _start);
	log ("metadata loaded in %dus", int (microseconds.count ()));
#endif

	bool	useLightPaths = lperegex != NULL && !exrid->LightPaths.empty ();

	DeepOp* in = input0();

	// Get the Id channel
	DD::Image::ChannelSet requiredChannels = DD::Image::Mask_Alpha;
	Channel idChannel = DD::Image::getChannel ("other.Id");

	// Get the LPEId channel
	Channel lpeidChannel = Channel (-1);
	if (useLightPaths)
		lpeidChannel = DD::Image::getChannel ("other.LPEId");

	if (int (lpeidChannel) < 0)
		useLightPaths = false;

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
			const bool	idselected = (st->idSelected (id) != _invert);

			float		_lpeid = 0;
			int			lpeid = 0;
			bool		lpeidselected = true;
			if (useLightPaths)
			{
				_lpeid = pixel.getOrderedSample (sample, lpeidChannel);
				lpeid = (int)_lpeid;
				lpeidselected = st->lpeidSelected (lpeid);
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
	if (!in)
		return;

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
	for (DD::Image::Box::iterator it = box.begin(); it != box.end(); it++)
	{
		DeepPixel pixel = inPlane.getPixel(it);
		size_t sample_count = pixel.getSampleCount();
		if (sample_count > 0)
		{
			if (click)
			{
				const float _id = pixel.getOrderedSample(sample_count - 1, idChannel);
				ids.insert((int)_id);
			}
			else
			{
				for (int sample = (int)sample_count - 1; sample >= 0; --sample)
				{
					const float _id = pixel.getUnorderedSample(sample, idChannel);
					ids.insert((int)_id);
				}
			}
		}
	}

	auto	exrid = _getExrIdData ();
	if (exrid == NULL)
		return;

	// Build a final string set
	set<string> names;
	for (set<int>::const_iterator id = ids.begin() ; id != ids.end() ; id++)
		if (*id >= 0 && *id < (int)exrid->Names.size())
			names.insert (escapeRegExp (exrid->Names[*id].c_str()));

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
	for (set<string>::const_iterator pattern = result.begin() ; pattern != result.end() ; pattern++)
	{
		if (!patterns.empty())
			patterns += "\n";
		patterns += *pattern;
	}
	knob("patterns")->set_text (patterns.c_str());
}

static Op* build(Node* node) { return new DeepOpenEXRId(node); }
const Op::Description DeepOpenEXRId::d(::CLASS, "Deep/DeepOpenEXRId", build);

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

#include <memory>

#if defined(_MSC_VER) && (_MSC_VER > 1600)
#define USE_MODERN_APIS
#endif

#ifdef USE_MODERN_APIS

#include <mutex>
namespace OpenEXRId
{
	using std::mutex;
	using std::lock_guard;
	using std::shared_ptr;
	using std::make_shared;
}

#else

#include <boost/thread/lock_guard.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
namespace OpenEXRId
{
	using boost::mutex;
	using boost::lock_guard;
	using boost::shared_ptr;
	using boost::make_shared;
}

#endif

#include "DDImage/DeepFilterOp.h"
#include "DDImage/Knobs.h"

#ifdef WIN32
#pragma warning(push, 0)
#endif

#if defined(_MSC_VER) && (_MSC_VER <= 1600)
#include <cmath>
#undef rintf
#undef expm1f
#endif

#include <OpenImageIO/ustring.h>
#include <re2/set.h>
#include <OSL/oslconfig.h>
#include <OSL/optautomata.h>
#include <OSL/lpeparse.h>
#include <OSL/oslclosure.h>
#ifdef WIN32
#pragma warning(pop)
#endif
// Defined by oiio
#undef copysign


// The DeepOpenEXRId plugin
class DeepOpenEXRId : public DD::Image::DeepFilterOp
{
	const char *_patterns;
	const char *_LPEs;
	bool _colors, _invert, _alpha, _keepVis;
	const char *_version;
	int _mode;

public:

	struct LPEEvent
	{
		OIIO::ustring Type;
		OIIO::ustring Scattering;
		OIIO::ustring Label;
	};

	typedef std::vector<LPEEvent> LightPath;

	DeepOpenEXRId(Node* node) : DeepFilterOp(node)
	{
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

	// A single entry hashed cache
	template<typename H, typename T>
	struct HashCache
	{
		OpenEXRId::mutex	Mutex;
		H					Hash;
		T					Value;

		T	get (const H &hash, const void * build_data)
		{
			OpenEXRId::lock_guard<OpenEXRId::mutex> guard (Mutex);
			if (hash != Hash)
			{
				Value = build (hash, build_data);
				Hash = hash;
			}
			return Value;
		}

		T	build(const H &hash, const void * build_data);
	};

	// This is the metadata we grabd from the input exrid
	struct ExrIdData
	{
		std::string					Hash;
		std::vector<std::string>	Names;
		std::vector<LightPath>		LightPaths;
	};

	typedef OpenEXRId::shared_ptr<ExrIdData>	ExrIdDataPtr;
	typedef HashCache<std::string,ExrIdDataPtr>	ExrIdDataCacheType;
	ExrIdDataCacheType	ExrIdDataCache;

	// get the cached metadata (or build it if anything has changed)
	ExrIdDataPtr				  	_getExrIdData ();

	// This is the regex matching automaton for names
	class NameAutomaton
	{
	public:
		NameAutomaton ();
		std::string					Hash;
		std::vector<std::string>	Patterns;
		re2::RE2::Set				RegEx;

		bool	match (const std::string &name, std::vector<int> &tmp) const;
	};

	typedef OpenEXRId::shared_ptr<NameAutomaton>	NameAutomatonPtr;
	typedef HashCache<std::string,NameAutomatonPtr>	NameAutomatonCacheType;
	NameAutomatonCacheType	NameAutomatonCache;

	// get the cached name matcing automaton
	NameAutomatonPtr				_getNamesAutomaton ();

	// This is the lpe matching automaton for light paths
	class LPEAutomaton
	{
	public:
		std::string					Hash;
		std::vector<std::string> 	Patterns;
		OSL::DfOptimizedAutomata	LPEx;

		bool	match (const LightPath &lightpath) const;
	};

	typedef OpenEXRId::shared_ptr<LPEAutomaton>		LPEAutomatonPtr;
	typedef HashCache<std::string,LPEAutomatonPtr>	LPEAutomatonCacheType;
	LPEAutomatonCacheType	LPEAutomatonCache;

	// get the cached lpe matching automaton
	LPEAutomatonPtr					_getLPEAutomaton ();

	// This is the activation states for names and light paths
	class State
	{
	public:
		std::vector<bool>			IdStates;
		std::vector<bool>			LPEIdStates;

		bool	idSelected (size_t id) const
			{ return id < IdStates.size () && IdStates[id]; }
		bool	lpeidSelected (size_t id) const
			{ return id < LPEIdStates.size () && LPEIdStates[id]; }

	struct BuildData
	{
		const DeepOpenEXRId::ExrIdDataPtr &exrid;
		const DeepOpenEXRId::NameAutomatonPtr &namesregex;
		const DeepOpenEXRId::LPEAutomatonPtr &lperegex;
	};
	};

	typedef OpenEXRId::shared_ptr<State> 	StatePtr;
	typedef HashCache<std::string,StatePtr>	StateCacheType;
	StateCacheType	StateCache;

	// get the result activate names and light paths
	StatePtr						_getState (
		const ExrIdDataPtr &data,
		const NameAutomatonPtr &names,
		const LPEAutomatonPtr &lpes);
};

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
#include <mutex>

#include "DDImage/DeepFilterOp.h"
#include "DDImage/Knobs.h"

#include <OpenImageIO/ustring.h>

#include <re2/set.h>

#ifdef WIN32
#pragma warning(push, 0)
#endif
#include <OSL/oslconfig.h>
#include <OSL/optautomata.h>
#include <OSL/lpeparse.h>
#include <OSL/oslclosure.h>
#ifdef WIN32
#pragma warning(pop)
#endif
// Defined by oiio
#undef copysign

// A single entry hashed cache
template<typename H, typename T>
struct HashCache
{
	std::mutex	Mutex;
	H			Hash;
	T			Value;

	template<typename Builder>
	T	get (const H &hash, Builder &&builder)
	{
		std::lock_guard<std::mutex>	guard (Mutex);
		if (hash != Hash)
		{
			Value = builder ();
			Hash = hash;
		}
		return Value;
	}
};


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

	// This is the metadata we grabd from the input exrid
	struct ExrIdData
	{
		std::string					Hash;
		std::vector<std::string>	Names;
		std::vector<LightPath>		LightPaths;
	};

	typedef std::shared_ptr<ExrIdData> ExrIdDataPtr;
	HashCache<std::string,ExrIdDataPtr>	ExrIdDataCache;

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

	typedef std::shared_ptr<NameAutomaton> NameAutomatonPtr;
	HashCache<std::string,NameAutomatonPtr>	NameAutomatonCache;

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

	typedef std::shared_ptr<LPEAutomaton> LPEAutomatonPtr;
	HashCache<std::string,LPEAutomatonPtr>	LPEAutomatonCache;

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
	};

	typedef std::shared_ptr<State> StatePtr;
	HashCache<std::string,StatePtr>	StateCache;

	// get the result activate names and light paths
	StatePtr						_getState (
		const ExrIdDataPtr &data,
		const NameAutomatonPtr &names,
		const LPEAutomatonPtr &lpes);
};

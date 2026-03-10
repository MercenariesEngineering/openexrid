
#include "ustr.h"
#include <array>
#include <atomic>
#include <mutex>
#include <unordered_set>

#include <emmintrin.h>

lpe::ustrData    lpe::ustr::_Empty { "", std::hash<std::string> {} ("") };

// ***************************************************************************

// Fast mutex, not fair
class SpinMutex
{
	std::atomic_flag	_Lock;
public:

	SpinMutex ()
		{ _Lock.clear (); }
	~SpinMutex ()
		{}

	// Spin lock mutex
	void lock ()
		{ while (!try_lock ()) _mm_pause (); }

	// Try lock
	bool try_lock ()
		{ return !_Lock.test_and_set (std::memory_order_acquire); }

	// unlock 
	void unlock ()
		{ _Lock.clear(std::memory_order_release); }
};

// ***************************************************************************

struct ustrBin
{
    SpinMutex                           Lock;
    std::unordered_set<lpe::ustrData>   Data;
};

// ***************************************************************************

static const int    ustrNBins = 1024;
static std::array<ustrBin, ustrNBins>   *ustrBins = nullptr;

// ***************************************************************************

lpe::ustr::ustr (const std::string &s)
{
    if (s.empty ())
    {
        _Data = &_Empty;
    }
    else
    {
        if (ustrBins == nullptr)
            ustrBins = new std::array<ustrBin, ustrNBins> ();
 
        ustrData    data { s, std::hash<std::string> {} (s) };
        ustrBin     &bin = (*ustrBins)[data.Hash & (ustrNBins-1)];
        std::lock_guard g (bin.Lock);
        _Data = &(*bin.Data.emplace (std::move (data)).first);
    }
}

// ***************************************************************************

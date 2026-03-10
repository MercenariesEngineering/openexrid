/*

             *
            ***
           *****
   *********************       Mercenaries Engineering SARL
     *****************              Copyright (C) 2025
       *************
         *********        http://www.mercenaries-engineering.com
        ***********
       ****     ****
      **           **

*/

#pragma once

#include <string>
#include <memory>

namespace lpe
{

struct ustrData
{
    std::string Str = std::string ();
    size_t      Hash = 0;

    bool    operator == (const ustrData &other) const
        { return Str == other.Str; }
    bool    operator != (const ustrData &other) const
        { return Str != other.Str; }
};

class ustr
{

    const ustrData  *_Data;
    static ustrData _Empty;

public:

    ustr () : _Data (&_Empty)
        {}

    ustr (const std::string &s);
    ustr    &operator = (const std::string &s)
        { return *this = ustr (s); }

    const std::string   &str () const
        { return _Data->Str; }
    const char          *c_str () const
        { return _Data->Str.c_str (); }

    size_t  hash () const
        { return _Data->Hash; }

    bool    operator < (const ustr &other) const
        { return _Data->Str < other._Data->Str; }

    bool    operator == (const ustr &other) const
        { return _Data == other._Data; }
    bool    operator == (const std::string &other) const
        { return _Data->Str == other; }
    bool    operator == (const char *other) const
        { return _Data->Str == other; }
    bool    operator != (const ustr &other) const
        { return !(*this == other); }
    bool    operator != (const std::string &other) const
        { return !(*this == other); }
    bool    operator != (const char *other) const
        { return !(*this == other); }
};

}

namespace std
{

template<>
struct hash<lpe::ustrData>
{
	size_t	operator () (const lpe::ustrData &str) const
		{ return str.Hash; }
};

template<>
struct hash<lpe::ustr>
{
	size_t	operator () (const lpe::ustr &str) const
		{ return str.hash (); }
};

}

/*
Copyright (c) 2009-2010 Sony Pictures Imageworks Inc., et al.
All Rights Reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
* Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.
* Neither the name of Sony Pictures Imageworks nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include "automata.h"
#include "ustr.h"

#include <memory>

namespace lpe {

/// Labels for light walks
///
/// This is the leftover of a class which used to hold all the labels
/// Now just acts as a little namespace for the basic definitions
///
/// NOTE: you still can assign these labels as keyword arguments to
///       closures. But we have removed the old labels array in the
///       primitives.
class Labels {
public:

    static lpe::ustr NONE;
    // Event type
    static lpe::ustr CAMERA;
    static lpe::ustr LIGHT;
    static lpe::ustr BACKGROUND;
    static lpe::ustr TRANSMIT;
    static lpe::ustr REFLECT;
    static lpe::ustr VOLUME;
    static lpe::ustr OBJECT;
    // Scattering
    static lpe::ustr DIFFUSE;  // typical 2PI hemisphere
    static lpe::ustr GLOSSY;   // blurry reflections and transmissions
    static lpe::ustr SINGULAR; // perfect mirrors and glass
    static lpe::ustr STRAIGHT; // Special case for transparent shadows

    static lpe::ustr __STOP__;     // end of a surface description
};

namespace lpexp {

// This is just a pair of states, see the use of the function genAuto in LPexp
// for a justification of this type that we use throughout all the regexp code
typedef std::pair<NdfAutomata::State *, NdfAutomata::State *> FirstLast;

/// LPexp atom type for the getType method
typedef enum {
    CAT,
    OR,
    SYMBOL,
    WILDCARD,
    REPEAT,
    NREPEAT
}Regtype;


class LPexp;
using LPexpPtr = std::shared_ptr<LPexp>;

/// Base class for a light path expression
//
/// Light path expressions are arranged as an abstract syntax tree. All the
/// nodes in that tree satisfy this interface that basicaly makes the automate
/// generation easy and clear.
///
/// The node types for this tree are:
///     CAT:      Concatenation of regexps like abcde or (abcde)
///     OR:        Ored union of two or more expressions like a|b|c|d
///     SYMBOL    Just a symbol like G or 'customlabel'
///     WILDCARD The wildcard regexp for . or [^GS]
///     REPEAT    Generic unlimited repetition of the child expression (exp)*
///     NREPEAT  Bounded repetition of the child expression like (exp){n,m}
///
class LPexp {
    public:
        virtual ~LPexp() {};

        /// Generate automata states for this subtree
        ///
        /// This method recursively builds all the needed automata states of
        /// the tree rooted by this node and returns the begin and end states
        /// for it. That means that if it were the only thing in the automata,
        /// making retvalue.first initial state and retvalue.second final state,
        /// would be the right thing to do.
        ///
        virtual FirstLast genAuto(NdfAutomata &automata)const = 0;
        /// Get the type for this node
        virtual Regtype getType()const = 0;
        /// For the parser's convenience. It is easy to implement things like a+
        /// as aa*. So the amount of regexp classes gets reduced. For doing that
        /// it needs an abstract clone function
        virtual LPexpPtr clone()const = 0;
};



/// LPexp concatenation
class Cat : public LPexp {
    public:
        virtual ~Cat();
        void append(LPexpPtr regexp);
        virtual FirstLast genAuto(NdfAutomata &automata)const;
        virtual Regtype getType()const { return CAT; };
        virtual LPexpPtr clone()const;

    protected:
        std::list<LPexpPtr> m_children;
};



/// Basic symbol like G or 'customlabel'
class Symbol : public LPexp {
    public:
        Symbol(lpe::ustr sym) { m_sym = sym; };
        virtual ~Symbol() {};

        virtual FirstLast genAuto(NdfAutomata &automata)const;
        virtual Regtype getType()const { return SYMBOL; };
        virtual LPexpPtr clone()const { return std::make_shared<Symbol> (*this); };

    protected:
        // All symbols are unique ustrings
        lpe::ustr m_sym;
};



/// Wildcard regexp
///
/// Named like this to avoid confusion with the automata Wildcard class
class Wildexp : public LPexp {
    public:
        Wildexp(SymbolSet &minus):m_wildcard(minus) {};
        virtual ~Wildexp() {};

        virtual FirstLast genAuto(NdfAutomata &automata)const;
        virtual Regtype getType()const { return WILDCARD; };
        virtual LPexpPtr clone()const { return std::make_shared<Wildexp> (*this); };

    protected:
        // And internally we use the automata's Wildcard type
        Wildcard m_wildcard;
};



/// Ored list of expressions
class Orlist : public LPexp {
    public:
        virtual ~Orlist();
        void append(LPexpPtr regexp);
        virtual FirstLast genAuto(NdfAutomata &automata)const;
        virtual Regtype getType()const { return OR; };
        virtual LPexpPtr clone()const;

    protected:
        std::list<LPexpPtr> m_children;
};



// Unlimited repeat: (exp)*
class Repeat : public LPexp {
    public:
        Repeat(LPexpPtr child):m_child(child) {};
        virtual ~Repeat() {}
        virtual FirstLast genAuto(NdfAutomata &automata)const;
        virtual Regtype getType()const { return REPEAT; };
        virtual LPexpPtr clone()const { return std::make_shared<Repeat> (m_child->clone()); };

    protected:
        LPexpPtr    m_child;
};



// Bounded repeat: (exp){m,n}
class NRepeat : public LPexp {
    public:
        NRepeat(LPexpPtr child, int min, int max):m_child(child),m_min(min),m_max(max) {};
        virtual ~NRepeat() {}
        virtual FirstLast genAuto(NdfAutomata &automata)const;
        virtual Regtype getType()const { return NREPEAT; };
        virtual LPexpPtr clone()const { return std::make_shared<NRepeat> (m_child->clone(), m_min, m_max); };

    protected:
        LPexpPtr    m_child;
        int m_min, m_max;
};



/// Toplevel rule definition
///
/// Note that although it has almost the same interface, this is not
/// a LPexp. It actually binds a light path expression to a certain rule.
/// Making the begin state initial and the end state final. It can't be
/// nested in other light path expressions, it is the root of the tree.
class Rule
{
    public:
        Rule(LPexpPtr child, void *rule):m_child(child), m_rule(rule) {};
        virtual ~Rule() {}
        void genAuto(NdfAutomata &automata)const;

    protected:
        LPexpPtr    m_child;
        // Anonymous pointer to the associated object for this rule
        void *m_rule;
};

} // namespace regexp

}

#ifndef STOCHASTIC_GRAMMAR
#define  STOCHASTIC_GRAMMAR

#include <assert.h>
#include <stdio.h>
//#include "Random.h"

/***********************************************
 ********** rhythmic grouping codes *************
************************************************/
typedef unsigned short GKEY;

/* Rules for keys - super important!
 *
 * There are two kinds of keys: terminal keys and non-terminal
 *
 * Terminal keys either directly generate themselves, or may be divided if there is a
 *  specific production rule to divide them. As an example, sg_q (quarter note) is a terminal key.
 *  It will always generate itself unless a production rule divides it.
 *
 * On the other hand sg_w2 is a non-terminal representing all the time in two bars of 4/4.
 *  sg_w2 will NEVER generate itself. Lacking a specific rule, it will auto generate two whole notes
 *
 * programmer must be aware of the difference in two places:
 *		when making production rules for a specific grammar
 *		when implementing ProductionRuleKeys::breakDown
 */


const GKEY sg_invalid = 0;		// either uninitialized rule, or return value that stops recursion.
                                // Note that this means table of production rules must have a dummy entry up front
const GKEY sg_w2 = 1;		// duration of two whole notes
const GKEY sg_w = 2;		// whole
const GKEY sg_ww = 3;		// w,w
const GKEY sg_h = 4;
const GKEY sg_hh = 5;
const GKEY sg_q = 6;
const GKEY sg_qq = 7;
const GKEY sg_e = 8;
const GKEY sg_ee = 9;

//
const GKEY sg_e3e3e3 = 10;		// three trip eights
const GKEY sg_e3 = 11;			//  trip eight

const GKEY sg_sx = 12;
const GKEY sg_sxsx = 13;

// crazy stuff for syncopation (unequal measure divisions)
const GKEY sg_68 = 14;		// the time duration of 7/8
const GKEY sg_78 = 15;		// the time duration of 7/8
const GKEY sg_98 = 16;		// the time duration of 9/8
const GKEY sg_798 = 17;		// 7/8 + 9/8 = 2w

// dotted notes
const GKEY sg_dq = 18;		// dotted quarter
const GKEY sg_dh = 19;		// dotted half
const GKEY sg_de = 20;		// dotted eighth

// odd groupings
const GKEY sg_hdq = 21;		// half + dotted Q
const GKEY sg_qhe = 22;		// q,h,e
const GKEY sg_hq = 23;	// h,q
const GKEY sg_qh = 24;	// h,q
const GKEY sg_q78 = 25;		// q + 7x8
const GKEY sg_qe68 = 26;		// q+e+6x8




const GKEY sg_first = 1;		// first valid one
const GKEY sg_last = 26;

const int fullRuleTableSize = sg_last + 1;

const int PPQ = 24;

/* class ProductionRuleKeys
 * collection of utility functions around rule keys
 */
class ProductionRuleKeys
{
public:
    static const int bufferSize = 6;	// size of a buffer that must be passed to breakDown

    // turn a key into a 0 terminated list of keys for individual notes
    // if called with a terminal key, just returns itself
    static void breakDown(GKEY key, GKEY * outKeys);

    // get the duration in clocks for a "simple" key (like q, not q+q)
    static int getDuration(GKEY key);

    static const char * toString(GKEY key);
};


/* class ProductionRuleEntry
 * A single entry in a production rule.
 * if A -> B or A -> C, then each of these would be a separate rule entry
 */
class ProductionRuleEntry
{
public:
    ProductionRuleEntry() : probability(0), code(sg_invalid)
    {
    }
    unsigned char probability;	// 1..256
    GKEY code;			// what to do if this one fires
};

inline bool operator == (const ProductionRuleEntry& a, const ProductionRuleEntry& b)
{
    return a.probability == b.probability && a.code == b.code;
}

// TODO: get rid of this
class Random
{
public:
    float get();
};

/* class ProductionRule
 * A production rule encapsulates every way that a starting symbol
 * can produce others.
 * if A -> B or A -> C, then a single production rule could represent this
 *
 */
class ProductionRule
{
public:
    static const int numEntries = 3;
    class EvaluationState
    {
    public:
        EvaluationState(Random& xr) : r(xr)
        {
        }
        const ProductionRule * rules;
        int numRules;
        Random& r;		//random number generator to use 
        virtual void writeSymbol(GKEY)
        {
        }
    };

    ProductionRule()
    {
    }

    void makeTerminal()
    {
        entries[0].code = sg_invalid;
        entries[0].probability = 255;
    }

    /* the data */

    // each possible production rule for this state
    ProductionRuleEntry entries[numEntries];

    static void evaluate(EvaluationState& es, int ruleToEval);

    static bool isGrammarValid(const ProductionRule * rules, int numRules, GKEY firstRule);
private:
    static int _evaluateRule(const ProductionRule& rule, float random);
#ifdef _MSC_VER
    bool _isValid(int index) const;
#endif
};


    // generate production, return code for what happened
inline int ProductionRule::_evaluateRule(const ProductionRule& rule, float random)
{
    assert(random >= 0 && random <= 1);
    assert(false);   // the rest is for 1..256
    //int rand = r.get() & 0xff;
    int rand = (int) (random * 256);
    //printf("evaluateRule called with rand is %d\n", rand);

    int i = 0;
    for (bool done2 = false; !done2; ++i) {
        assert(i < numEntries);
        //printf("prob[%d] is %d\n", i,  rule.entries[i].probability);
        if (rule.entries[i].probability >= rand) {
            GKEY code = rule.entries[i].code;
            //printf("rule fired on code abs val=%d\n", code);
            return code;
        }
    }
    assert(false);	// no rule fired
    return 0;
}

inline void ProductionRule::evaluate(EvaluationState& es, int ruleToEval)
{
    //printf("\n evaluate called on rule #%d\n", ruleToEval);
    const ProductionRule& rule = es.rules[ruleToEval];

#ifdef _MSC_VER
    assert(rule._isValid(ruleToEval));
#endif
    GKEY result = _evaluateRule(rule, es.r.get());
    if (result == sg_invalid)		// request to terminate recursion
    {
        GKEY code = ruleToEval;		// our "real" terminal code is our table index
        //printf("production rule #%d terminated\n", ruleToEval);
        //printf("rule terminated! execute code %s\n",  ProductionRuleKeys::toString(code));
        es.writeSymbol(code);
    } else {
        //printf("production rule #%d expanded to %d\n", ruleToEval, result);

        // need to expand,then eval all of the expanded codes

        GKEY buffer[ProductionRuleKeys::bufferSize];
        ProductionRuleKeys::breakDown(result, buffer);
        for (GKEY * p = buffer; *p != sg_invalid; ++p) {
            //printf("expanding rule #%d with %d\n", ruleToEval, *p);
            evaluate(es, *p);
        }
        //printf("done expanding %d\n", ruleToEval);
    }
}

// is the data self consistent, and appropriate for index
#ifdef _MSC_VER
inline bool ProductionRule::_isValid(int index) const
{
    if (index == sg_invalid) {
        printf("rule not allowed in first slot\n");
        return false;
    }


    if (entries[0] == ProductionRuleEntry()) {
        printf("rule at index %d is ininitizlied. bad graph (%s)\n",
            index,
            ProductionRuleKeys::toString(index));
        return false;
    }

    int last = -1;
    bool foundTerminator = false;
    for (int i = 0; !foundTerminator; ++i) {
        if (i >= numEntries) {
            printf("entries not terminated index=%d 'i' is too big: %d\n", index, i);
            return false;
        }
        const ProductionRuleEntry& e = entries[i];
        if (e.probability <= last)			// probabilities grow
        {
            printf("probability not growing is %d was %d\n", e.probability, last);
            return false;
        }
        if (e.probability == 0xff) {
            foundTerminator = true;					// must have a 255 to end it
            if (e.code == index) {
                printf("rule terminates on self: recursion not allowed\n");
                return false;
            }
        }

        if (e.code < sg_invalid || e.code > sg_last) {
            printf("rule[%d] entry[%d] had invalid code: %d\n", index, i, e.code);
            return false;
        }

        // if we are terminating recursion, then by definition our duration is correct
        if (e.code != sg_invalid) {
            // otherwise, make sure the entry has the right duration
            int entryDuration = ProductionRuleKeys::getDuration(e.code);
            int ruleDuration = ProductionRuleKeys::getDuration(index);
            if (entryDuration != ruleDuration) {
                printf("production rule[%d] (name %s) duration mismatch (time not conserved) rule dur = %d entry dur %d\n",
                    index, ProductionRuleKeys::toString(index), ruleDuration, entryDuration);
                return false;
            }
        }
    }
    return true;
}
#endif

#ifdef _MSC_VER
inline bool ProductionRule::isGrammarValid(const ProductionRule * rules, int numRules, GKEY firstRule)
{
    //printf("is grammar valid, numRules = %d first = %d\n", numRules, firstRule);
    if (firstRule < sg_first) {
        printf("first rule index (%d) bad\n", firstRule);
        return false;
    }
    if (numRules != (sg_last + 1)) {
        printf("bad number of rules\n");
        return false;
    }

    const ProductionRule& r = rules[firstRule];

    if (!r._isValid(firstRule)) {
        return false;
    }


    // now, make sure every entry goes to something real
    bool foundTerminator = false;
    for (int i = 0; !foundTerminator; ++i) {
        const ProductionRuleEntry& e = r.entries[i];
        if (e.probability == 0xff)
            foundTerminator = true;					// must have a 255 to end it	
        GKEY _newKey = e.code;
        if (_newKey != sg_invalid) {
            GKEY outKeys[4];
            ProductionRuleKeys::breakDown(_newKey, outKeys);
            for (GKEY * p = outKeys; *p != sg_invalid; ++p) {
                if (!isGrammarValid(rules, numRules, *p)) {
                    printf("followed rules to bad one\n");
                    return false;
                }
            }
        }
    }

    return true;
}
#endif

/* class StochasticGrammarDictionary
 *
 * just a collection of pre-made grammars
 *
 * 0: simple test
 * 1: mixed duration, with some trips
 * 2: some syncopation, no trips
 */
class StochasticGrammarDictionary
{
public:
    class Grammar
    {
    public:
        const ProductionRule * rules;
        int numRules;
        GKEY firstRule;
    };
    static Grammar getGrammar(int index);
    static int getNumGrammars();
private:
    static bool _didInitRules;
    static void initRules();
    static void initRule0(ProductionRule * rules);
    static void initRule1(ProductionRule * rules);
    static void initRule2(ProductionRule * rules);
    static void initRule3(ProductionRule * rules);
};



#endif

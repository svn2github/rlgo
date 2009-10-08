//----------------------------------------------------------------------------
/** @file RlWeight.h
    A single weight in the network
*/
//----------------------------------------------------------------------------

#ifndef RLWEIGHT_H
#define RLWEIGHT_H

#include "RlUtils.h"

//----------------------------------------------------------------------------
/** The following defines specify the contents of each weight.
    If a property isn't defined, then a static placeholder is used instead. 
    This avoids any further conditional compilation in client code.
    The Ensure* functions can be called to throw an exception
    if a required property isn't defined. */

/** Store eligibility traces for TD(lambda) */
//#define RL_ELIGIBILITY

/** If defined then each weight has its own step-size */
//#define RL_STEP

/** If defined then each weight has its own step-size trace parameter */
//#define RL_TRACE

/** If defined then count how many times weight is updated */
//#define RL_COUNT

//----------------------------------------------------------------------------
/** Simple class holding the learning parameters for a single feature
*/
class RlWeight
{
public:

    RlWeight();

    /** Save the weight */
    void Save(std::ostream& ostr);
    
    /** Load the weight */
    void Load(std::istream& istr);

    /** Clear all data */
    void Clear();

    //-------------------------------------------------------------------------
    // Main weight
    const RlFloat& Weight() const { return m_weight; }
    RlFloat& Weight() { return m_weight; }
    void Add(RlWeight& weight, RlFloat mul);

    //-------------------------------------------------------------------------
    // TD Lambda
    const bool& Active() const { return m_active; }
    bool& Active() { return m_active; }

    const RlFloat& Eligibility() const { return m_eligibility; }
    RlFloat& Eligibility() { return m_eligibility; }

    //-------------------------------------------------------------------------
    // Step-size adaptation
    const RlFloat& Step() const { return m_step; }
    RlFloat& Step() { return m_step; }

    const RlFloat& Trace() const { return m_trace; }
    RlFloat& Trace() { return m_trace; }

    //-------------------------------------------------------------------------
    // Occurrence counting
    int Count() const { return m_count; }
    void IncCount() { m_count ++; }
    void ResetCount() { m_count = 0; }

    //-------------------------------------------------------------------------
    // Throw an exception if property isn't defined */
    static void EnsureEligibility();
    static void EnsureStep();
    static void EnsureTrace();
    static void EnsureExtendedTrace();
    static void EnsureDeltaPhi();
    static void EnsureCount();

    static const RlFloat MIN_WEIGHT;
    static const RlFloat MAX_WEIGHT;

private:

    /** The weight corresponding to specified feature */
    RlFloat m_weight;
    
#ifdef RL_ELIGIBILITY
    /** The eligibility of this weight for credit assignment */
    RlFloat m_eligibility;
    
    /** Whether this weight is currently eligible for learning */
    bool m_active;
#else
    static RlFloat m_eligibility;
    static bool m_active;
#endif // RL_ELIGIBILITY

#ifdef RL_STEP
    /** Step-size parameter associated with this weight */
    RlFloat m_step;
#else
    static RlFloat m_step;
#endif // RL_STEP

#ifdef RL_TRACE
    /** Step-size trace parameter */
    RlFloat m_trace;
#else
    static RlFloat m_trace;
#endif // RL_TRACE

#ifdef RL_COUNT
    /** How many times this weight has been updated */
    int m_count;
#else
    static int m_count;
#endif // RL_COUNT
};

//----------------------------------------------------------------------------

#endif // RLWEIGHT_H


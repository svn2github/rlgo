//----------------------------------------------------------------------------
/** @file RlWeight.cpp
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlWeight.h"
#include "RlWeightSet.h"
#include "RlUtils.h"

using namespace std;

//----------------------------------------------------------------------------

template<class T>
T ReadValue(istream& in)
{
    T value;
    in.read((char*) &value, sizeof(float));
    return value;
}

template<class T>
void WriteValue(ostream& out, T value)
{
    out.write((char*) &value, sizeof(T));
}

//----------------------------------------------------------------------------

const RlFloat RlWeight::MAX_WEIGHT = +1000;
const RlFloat RlWeight::MIN_WEIGHT = -1000;

#ifndef RL_ELIGIBILITY
RlFloat RlWeight::m_eligibility;
bool RlWeight::m_active = false;
#endif // RL_ELIGIBILITY

#ifndef RL_STEP
RlFloat RlWeight::m_step;
#endif // RL_STEP

#ifndef RL_TRACE
RlFloat RlWeight::m_trace;
#endif // RL_TRACE

#ifndef RL_COUNT
int RlWeight::m_count = 0;
#endif // RL_COUNT

RlWeight::RlWeight()
{
    m_active = false;
    Clear();
}

void RlWeight::Clear()
{
    m_weight = 0;
    #ifdef RL_ELIGIBILITY
    m_eligibility = 0;
    m_active = false;
    #endif
    #ifdef RL_STEP
    m_step = 1;
    #endif
    #ifdef RL_TRACE
    m_trace = 0;
    #endif
    #ifdef RL_COUNT
    m_count = 0;
    #endif
}

void RlWeight::Save(ostream& ostr)
{
    WriteValue<float>(ostr, m_weight);
}

void RlWeight::Load(istream& istr)
{
    m_weight = ReadValue<float>(istr);
}

void RlWeight::Add(RlWeight& weight, RlFloat mul)
{
    m_weight += weight.m_weight * mul;
}

void RlWeight::EnsureEligibility()
{
    #ifndef RL_ELIGIBILITY
    throw SgException("Eligibility traces not defined");
    #endif // RL_ELIGIBILITY
}

void RlWeight::EnsureStep()
{
    #ifndef RL_STEP
    throw SgException("Step sizes not defined");
    #endif // RL_STEP
}

void RlWeight::EnsureTrace()
{
    #ifndef RL_TRACE
    throw SgException("Traces not defined");
    #endif // RL_TRACE
}

void RlWeight::EnsureCount()
{
    #ifndef RL_COUNT
    throw SgException("Counts not defined");
    #endif // RL_COUNT
}

//----------------------------------------------------------------------------


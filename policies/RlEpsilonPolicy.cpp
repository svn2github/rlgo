//----------------------------------------------------------------------------
/** @file RlEpsilonPolicy.cpp
    See RlEpsilonPolicy.h
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlEpsilonPolicy.h"

using namespace std;

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlEpsilonPolicy);

RlEpsilonPolicy::RlEpsilonPolicy(GoBoard& board, 
    RlPolicy* ppolicy, RlPolicy* npolicy)
:   RlPolicy(board, 0),
    m_pPolicy(ppolicy),
    m_nPolicy(npolicy),
    m_epsilon(0.1f)
{ 
}

void RlEpsilonPolicy::LoadSettings(istream& settings)
{
    int version;
    settings >> RlVersion(version, 3, 3);
    settings >> RlSetting<RlPolicy*>("PPolicy", m_pPolicy);
    settings >> RlSetting<RlPolicy*>("NPolicy", m_nPolicy);
    settings >> RlSetting<RlFloat>("Epsilon", m_epsilon);
}

SgMove RlEpsilonPolicy::SelectMove(RlState& state)
{
    bool explore = SgRandomFloat(0.0f, 1.0) < m_epsilon;

    SgMove move = SG_NULLMOVE;
    if (explore)
        move = m_pPolicy->SelectMove(state);
    if (move == SG_NULLMOVE)
        move = m_nPolicy->SelectMove(state);
    SG_ASSERT(move != SG_NULLMOVE);
    return move;
}

void RlEpsilonPolicy::SetCurrentMove(RlState& state)
{
    m_pPolicy->SetCurrentMove(state);
    m_nPolicy->SetCurrentMove(state);
}

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlEpsilonDecayPolicy);

RlEpsilonDecayPolicy::RlEpsilonDecayPolicy(GoBoard& board, 
    RlPolicy* ppolicy, RlPolicy* npolicy)
:   RlEpsilonPolicy(board, ppolicy, npolicy),
    m_initEpsilon(0.1f),
    m_episodeDecay(1.0)
{ 
}

void RlEpsilonDecayPolicy::LoadSettings(istream& settings)
{
    RlEpsilonPolicy::LoadSettings(settings);
    settings >> RlSetting<RlFloat>("EpisodeDecay", m_episodeDecay);
}

void RlEpsilonDecayPolicy::Initialise()
{
    m_initEpsilon = m_epsilon;
}

SgMove RlEpsilonDecayPolicy::SelectMove(RlState& state)
{
    RlFloat prob = powf(1 - m_epsilon, state.TimeStep() + 1);
    bool explore = SgRandomFloat(0.0f, 1.0) > prob;

    SgMove move;
    if (explore)
        move = m_pPolicy->SelectMove(state);
    else
        move = m_nPolicy->SelectMove(state);
    return move;
}

void RlEpsilonDecayPolicy::Reset() 
{ 
    m_epsilon = m_initEpsilon; 
}

void RlEpsilonDecayPolicy::NewGame() 
{ 
    m_epsilon *= (1 - m_episodeDecay);
}

//----------------------------------------------------------------------------

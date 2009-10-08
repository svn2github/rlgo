//----------------------------------------------------------------------------
/** @file RlTwoStagePolicy.cpp
    See RlTwoStagePolicy.h
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlTwoStagePolicy.h"

#include "RlSimulator.h"

using namespace std;

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlTwoStagePolicy);

RlTwoStagePolicy::RlTwoStagePolicy(GoBoard& board,
    RlPolicy* policy1, RlPolicy* policy2)
:   RlPolicy(board),
    m_policy1(policy1),
    m_policy2(policy2),
    m_episodes(0)
{
}

void RlTwoStagePolicy::LoadSettings(istream& settings)
{
    int version;
    settings >> RlVersion(version, 2, 2);
    settings >> RlSetting<RlPolicy*>("Policy1", m_policy1);
    settings >> RlSetting<RlPolicy*>("Policy2", m_policy2);
    settings >> RlSetting<RlFloat>("SwitchTime", m_switchTime);
}

SgMove RlTwoStagePolicy::SelectMove(RlState& state)
{
    RlFloat switchtime = SwitchTime(m_episodes);
    int floor = static_cast<int>(switchtime);
    RlFloat p = switchtime - floor;

    SgMove move;
    if (state.TimeStep() < floor
        || (state.TimeStep() == floor && SgRandomFloat(0.0f, 1.0) < p))
    {
        move = m_policy1->SelectMove(state);
    }
    else
    {
        move = m_policy2->SelectMove(state);
    }
    return move;
}

RlFloat RlTwoStagePolicy::SwitchTime(int episodes) const
{
    SG_UNUSED(episodes);
    return m_switchTime;
}

void RlTwoStagePolicy::Reset()
{
    m_episodes = 0;
}

void RlTwoStagePolicy::NewGame()
{
    m_episodes++;
}

void RlTwoStagePolicy::SetCurrentMove(RlState& state)
{
    m_policy1->SetCurrentMove(state);
    m_policy2->SetCurrentMove(state);
}

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlLogTwoStagePolicy);

RlLogTwoStagePolicy::RlLogTwoStagePolicy(GoBoard& board,
    RlPolicy* policy1, RlPolicy* policy2)
:   RlTwoStagePolicy(board, policy1, policy2),
    m_base(2)
{
}

void RlLogTwoStagePolicy::LoadSettings(istream& settings)
{
    RlTwoStagePolicy::LoadSettings(settings);
    settings >> RlSetting<RlFloat>("Base", m_base);
}

RlFloat RlLogTwoStagePolicy::SwitchTime(int episodes) const
{
    return logf(episodes) / logf(m_base);
}

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlLinearTwoStagePolicy);

RlLinearTwoStagePolicy::RlLinearTwoStagePolicy(GoBoard& board,
    RlPolicy* policy1, RlPolicy* policy2)
:   RlTwoStagePolicy(board, policy1, policy2)
{
}

void RlLinearTwoStagePolicy::LoadSettings(istream& settings)
{
    RlTwoStagePolicy::LoadSettings(settings);
    settings >> RlSetting<RlSimulator*>("Simulator", m_simulator);
}

RlFloat RlLinearTwoStagePolicy::SwitchTime(int episodes) const
{
    return m_switchTime * episodes / m_simulator->GetNumGames();
}

//----------------------------------------------------------------------------

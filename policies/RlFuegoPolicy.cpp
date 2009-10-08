//----------------------------------------------------------------------------
/** @file RlFuegoPolicy.cpp
    See RlFuegoPolicy.h
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlFuegoPolicy.h"

using namespace std;

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlFuegoPlayoutPolicy);

RlFuegoPlayoutPolicy::RlFuegoPlayoutPolicy(GoBoard& board, 
    RlFuegoPlayout* playout)
:   RlPolicy(board),
    m_fuegoPlayout(playout),
    m_incremental(false)
{
}

void RlFuegoPlayoutPolicy::LoadSettings(istream& settings)
{
    settings >> RlSetting<bool>("OnPolicy", m_onPolicy);
    settings >> RlSetting<RlFuegoPlayout*>("FuegoPlayout", m_fuegoPlayout);
    settings >> RlSetting<bool>("Incremental", m_incremental);
}

SgMove RlFuegoPlayoutPolicy::SelectMove(RlState& state)
{
    if (!m_incremental)
        m_fuegoPlayout->OnStart();
    SgMove move = m_fuegoPlayout->GenerateMove();
    state.SetPolicyType(m_onPolicy ? RlState::POL_ON : RlState::POL_OFF);

    // @TODO: deal with illegal moves
    if (m_board.IsLegal(move))
        return move;
    else
        return SG_PASS;
}

void RlFuegoPlayoutPolicy::SampleProbabilities(int timestep, 
    int numsamples, vector<RlFloat>& probs)
{
    SG_UNUSED(timestep);
    
    // Sample probability distribution for Fuego playout policy
    RlFloat pmass = 1.0 / static_cast<RlFloat>(numsamples);
    for (int i = 0; i < numsamples; ++i)
    {
        m_fuegoPlayout->OnStart();
        SgMove move = m_fuegoPlayout->GenerateMove();
        m_fuegoPlayout->OnEnd();
        probs[move] += pmass;
    }
}

//----------------------------------------------------------------------------

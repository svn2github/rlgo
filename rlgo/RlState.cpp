//----------------------------------------------------------------------------
/** @file RlState.cpp
    See RlState.h
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlState.h"

using namespace std;

//----------------------------------------------------------------------------

RlState::RlState()
:   m_timestep(-1),
    m_colour(SG_EMPTY),
    m_move(SG_NULLMOVE),
    m_policyType(POL_NONE),
    m_evaluated(false),
    m_activeSet(false),
    m_terminal(false)
{
    ClearBest();
}

RlState::RlState(int timestep, SgBlackWhite colour)
:   m_timestep(timestep),
    m_colour(colour),
    m_move(SG_NULLMOVE),
    m_policyType(POL_NONE),
    m_evaluated(false),
    m_activeSet(false),
    m_terminal(false)
{
    ClearBest();
}

void RlState::Resize(int activesize)
{
    m_active.Resize(activesize);
}

inline void RlState::ClearBest()
{
    // @todo: not strictly necessary, but makes debugging clearer
    m_bestMove = SG_NULLMOVE;
    m_bestEval = 0;
}

bool RlState::OnPolicy() const
{
    switch (m_policyType)
    {
    case POL_ON:
        return true;
    case POL_OFF:
        return false;
    case POL_BEST:
        return m_move == m_bestMove;
    case POL_TERMINAL:
        return true;
    default:
        return true;
    }
}

void RlState::CopyBest(const RlState& sourcestate)
{
    m_bestMove = sourcestate.m_bestMove;
    m_bestEval = sourcestate.m_bestEval;
}

//----------------------------------------------------------------------------

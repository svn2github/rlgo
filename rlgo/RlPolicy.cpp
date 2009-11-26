//----------------------------------------------------------------------------
/** @file RlPolicy.cpp

    $Id: RlPolicy.cpp,v 1.68 2008/05/29 23:20:58 silver Exp $
    $Source: /usr/cvsroot/project_explorer/rlgo/RlPolicy.cpp,v $
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlPolicy.h"

#include "RlAgent.h"
#include "RlWeightSet.h"
#include "RlEvaluator.h"
#include "RlLogger.h"
#include "RlMoveFilter.h"
#include "RlSimulator.h"

#include <math.h>

using namespace std;
using namespace GoBoardUtil;
using namespace SgPointUtil;
using namespace RlMoveUtil;

//----------------------------------------------------------------------------

RlPolicy::RlPolicy(GoBoard& board, RlEvaluator* evaluator, RlLogger* log)
:   RlAutoObject(board),
    m_evaluator(evaluator),
    m_log(log),
    m_onPolicy(RlState::POL_NONE)
{ 
}

void RlPolicy::LoadSettings(istream& settings)
{
    int version;
    settings >> RlVersion(version, 2, 1);
    settings >> RlSetting<RlEvaluator*>("Evaluator", m_evaluator);
    if (version >= 2)
        settings >> RlSetting<RlLogger*>("Log", m_log);
    settings >> RlSetting<bool>("OnPolicy", m_onPolicy);
}

void RlPolicy::Initialise()
{
    InitLogs();
}

bool RlPolicy::SearchValue(RlFloat& value) const
{
    SG_UNUSED(value);
    return false;
}

void RlPolicy::SampleProbabilities(int timestep, 
    int numsamples, vector<RlFloat>& probs)
{
    // Sample probability distribution for an unknown stochastic policy
    RlFloat pmass = 1.0 / static_cast<RlFloat>(numsamples);
    for (int i = 0; i < numsamples; ++i)
    {
        RlState state(timestep, m_board.ToPlay());
        SgMove move = SelectMove(state);
        probs[move] += pmass;
    }
}

RlFloat RlPolicy::GetProbability(SgMove move) const
{
    SG_UNUSED(move);
    throw SgException("Probability distribution not computed by this policy");
}

void RlPolicy::InitLogs()
{
    if (m_log)
    {
        m_policyLog.reset(new RlLog(this, "Policy"));
        m_policyLog->AddItem("TimeStep", 0);
        m_policyLog->AddItem("SelectedMove", string("None"));
    }
}

void RlPolicy::LogPolicy(const RlState& state, SgBlackWhite move)
{
    if (m_log && m_log->MoveLogIsActive())
    {
        m_policyLog->Log("TimeStep", state.TimeStep());
        m_policyLog->Log("SelectedMove", SgWritePoint(move));
        m_policyLog->Step();
    }
}

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlGreedy);

RlGreedy::RlGreedy(GoBoard& board, RlEvaluator* evaluator, RlHistory* history)
:   RlPolicy(board, evaluator),
    m_history(history)
{
}

void RlGreedy::LoadSettings(istream& settings)
{
    int version;
    settings >> RlVersion(version, 1, 1);
    settings >> RlSetting<RlEvaluator*>("Evaluator", m_evaluator);
    settings >> RlSetting<RlHistory*>("History", m_history);
}

void RlGreedy::InitLogs()
{
    RlPolicy::InitLogs();
    if (m_log)
        m_policyLog->AddItem("BestMove", string("None"));
}

void RlGreedy::LogPolicy(const RlState& state, SgMove move)
{
    if (m_log && m_log->MoveLogIsActive())
        m_policyLog->Log("BestMove", SgWritePoint(state.BestMove()));
    RlPolicy::LogPolicy(state, move);
}

SgMove RlGreedy::SelectMove(RlState& state)
{
    m_evaluator->FindBest(state);
    state.SetPolicyType(RlState::POL_BEST);
    SgMove move = state.BestMove();
    LogPolicy(state, move);
    return move;
}

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlRandomPolicy);

RlRandomPolicy::RlRandomPolicy(GoBoard& board, RlEvaluator* evaluator)
:   RlPolicy(board, evaluator)
{
}

SgMove RlRandomPolicy::SelectMove(RlState& state)
{
    state.SetPolicyType(m_onPolicy ? RlState::POL_ON : RlState::POL_OFF);
    const RlMoveFilter* filter = m_evaluator->GetMoveFilter();
    SgMove move = filter->GetRandomMove(state.Colour());
    LogPolicy(state, move);
    return move;
}

//----------------------------------------------------------------------------

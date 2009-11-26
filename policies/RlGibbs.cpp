//----------------------------------------------------------------------------
/** @file RlGibbs.cpp
    See RlGibbs.h
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlGibbs.h"

#include "RlEvaluator.h"
#include "RlLogger.h"
#include "RlMoveFilter.h"
#include "RlMoveUtil.h"

using namespace std;
using namespace RlMathUtil;
using namespace RlMoveUtil;

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlGibbs);

RlGibbs::RlGibbs(GoBoard& board, RlEvaluator* evaluator, RlLogger* log, 
    RlFloat temperature)
:   RlPolicy(board, evaluator, log),
    m_temperature(temperature),
    m_centre(true),
    m_redraw(false)
{ 
}

void RlGibbs::LoadSettings(istream& settings)
{
    RlPolicy::LoadSettings(settings);
    settings >> RlSetting<RlFloat>("Temperature", m_temperature);
    settings >> RlSetting<bool>("Centre", m_centre);
    settings >> RlSetting<bool>("Redraw", m_redraw);
}

void RlGibbs::Initialise()
{
    RlPolicy::Initialise();
    m_moveIndices.resize(SG_MAXPOINT + 2); // include pass move
}

SgMove RlGibbs::SelectMove(RlState& state)
{
    // Allow multiple samples to be redrawn without recalculating distribution
    bool redraw = m_redraw && state.PolicyType() == RlState::POL_BEST;
    if (!redraw)
    {
        const RlMoveFilter* filter = m_evaluator->GetMoveFilter();
        filter->GetMoveVector(state.Colour(), m_moves);
        if (!filter->ConsiderPass())
            m_moves.push_back(SG_PASS);

        EvaluateMoves(state);
        MakeGibbs(state.Colour() == SG_WHITE);
        state.SetPolicyType(RlState::POL_BEST);
    }
    
    if (m_moves.empty())
        return SG_PASS;
    int index = SelectRandom(m_probs);
    SG_ASSERT(index >= 0 && index < ssize(m_moves));
    LogPolicy(state, m_moves[index]);
    return m_moves[index];
}

void RlGibbs::EvaluateMoves(RlState& state)
{
    // Evaluate all candidate moves
    m_evals.clear();
    m_values.clear();
    for (int i = 0; i < SG_MAXPOINT + 2; ++i)
        m_moveIndices[i] = -1;
    m_changeLists.resize(m_moves.size());
    
    for (int m = 0; m < ssize(m_moves); ++m)
    {
        SgMove move = m_moves[m];
        SG_ASSERT(m_board.IsLegal(move));
        RlFloat eval = m_evaluator->EvaluateMove(move, state.Colour());
        SANITY_CHECK(eval, -RlInfinity, +RlInfinity);
        m_evals.push_back(eval);
        m_values.push_back(Logistic(eval));
        m_moveIndices[m_moves[m]] = m;
    }

    // If pass move isn't considered, assign it -infinity value
    // Otherwise the expected features for pass move can't be computed
    const RlMoveFilter* filter = m_evaluator->GetMoveFilter();
    if (!filter->ConsiderPass())
    {
        m_evals.back() = state.Colour() == SG_BLACK 
            ? -RL_MAX_EVAL : +RL_MAX_EVAL;
        m_values.back() = Logistic(m_evals.back());
    }
}

void RlGibbs::MakeGibbs(bool negate)
{
    RlFloat sign = negate ? -1.0 : +1.0;
    int size = m_evals.size();
    SG_ASSERT(size);
    m_probs.resize(size);
    
    const RlFloat maxexp = 50;
    RlFloat emax = maxexp;
    int imax = -1;

    RlFloat mean = 0; // Simple average, not weighted by probabilities
    if (m_centre)
    {
        for (int i = 0; i < size; ++i)
            mean += sign * m_evals[i];
        mean /= size;
    }
    
    RlFloat z = 0;
    for (int i = 0; i < size; ++i)
    {
        RlFloat exponent = (sign * m_evals[i] - mean) / m_temperature;
        if (exponent > emax)
        {
            emax = exponent;
            imax = i;
        }
        else
        {
            m_probs[i] = exp(exponent);
            z += m_probs[i];
        }
    }
    
    // If values are too high for exp, set probabilities to argmax
    if (imax >= 0)
    {
        for (int i = 0; i < size; ++i)
            m_probs[i] = 0;
        m_probs[imax] = 1;
    }

    // Otherwise normalise to give total probability of 1
    else
    {
        RlFloat normaliser = 1.0 / z;
        for (int i = 0; i < size; ++i)
            m_probs[i] *= normaliser;
    }

    m_statMean.Add(mean);
    m_statEntropy.Add(GetEntropy(m_probs));
    m_statDeterministic.Add(imax >= 0 ? 1.0 : 0.0);
}

RlFloat RlGibbs::GetProbability(SgMove move) const
{
    int moveindex = m_moveIndices[move];
    if (moveindex < 0)
        return 0.0;
    else
        return m_probs[moveindex];
}

void RlGibbs::InitLogs()
{
    RlPolicy::InitLogs();
    if (m_log)
    {
        m_policyLog->AddItem("Mean");
        m_policyLog->AddItem("Entropy");
        m_policyLog->AddItem("Deterministic");
    }
}

void RlGibbs::LogPolicy(const RlState& state, SgMove move)
{
    if (m_log && m_log->MoveLogIsActive())
    {
        m_policyLog->Log("Mean", m_statMean.Mean());
        m_policyLog->Log("Entropy", m_statEntropy.Mean());
        m_policyLog->Log("Deterministic", m_statDeterministic.Mean());
        m_statEntropy.Clear();
        m_statMean.Clear();
    }
    RlPolicy::LogPolicy(state, move);
}

//----------------------------------------------------------------------------

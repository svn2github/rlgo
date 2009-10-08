//----------------------------------------------------------------------------
/** @file RlTDRules.cpp
    See RlTDRules.h
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlTDRules.h"

#include "RlAgentLog.h"

using namespace std;

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlTD0);

RlTD0::RlTD0(GoBoard& board, RlWeightSet* wset, RlAgentLog* log)
:   RlLearningRule(board, wset, log)
{
}

void RlTD0::CalcDelta()
{
    m_target = m_reward + m_newValue;
    m_delta = m_target - m_oldValue;
}

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlLambdaReturn);

RlLambdaReturn::RlLambdaReturn(GoBoard& board, RlWeightSet* wset,
    RlAgentLog* log, RlFloat lambda)
:   RlLearningRule(board, wset, log),
    m_lambda(lambda)
{
}

void RlLambdaReturn::LoadSettings(istream& settings)
{
    RlLearningRule::LoadSettings(settings);
    settings >> RlSetting<RlFloat>("Lambda", m_lambda);
}

void RlLambdaReturn::CalcDelta()
{
    if (m_terminal)
    {
        m_lambdaReturn = m_reward;
    }
    else
    {
        m_lambdaReturn = m_reward 
            + m_lambdaReturn * m_lambda
            + m_newValue * (1 - m_lambda);
    }

    // Clear lambda return if off-policy
    if (!m_useOffPolicy && !m_onPolicy)
        m_lambdaReturn = m_oldValue;

    m_delta = m_lambdaReturn - m_oldValue;    
}

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlTDLambda);

RlTDLambda::RlTDLambda(GoBoard& board, RlWeightSet* wset, RlAgentLog* log,
    RlFloat lambda, bool replacing)
:   RlTD0(board, wset, log),
    m_lambda(lambda),
    m_replacing(replacing),
    m_zeroThreshold(0.001f)
{
}

void RlTDLambda::LoadSettings(istream& settings)
{
    RlTD0::LoadSettings(settings);
    settings >> RlSetting<RlFloat>("Lambda", m_lambda);
    settings >> RlSetting<bool>("Replacing", m_replacing);
    settings >> RlSetting<RlFloat>("ZeroThreshold", m_zeroThreshold);
}

void RlTDLambda::Start(RlHistory* history, int episode)
{
    RlLearningRule::Start(history, episode);

    // Clear all eligibility traces and non-zero eligibility list
    RlWeight::EnsureEligibility();
    ClearEligibility();
}

void RlTDLambda::Learn()
{
    RlWeight::EnsureEligibility();

    // Don't use off-policy experience
    if (!m_useOffPolicy && !m_onPolicy)
    {
        ClearEligibility();
        return;
    }

    CalcDelta();
    CalcLogisticGradient();
    CalcStepSize();
    LogLearn();
    DecayEligibility();
    for (RlActiveSet::Iterator i_active(m_oldState->Active()); 
        i_active; ++i_active)
    {
        RlOccur occur = i_active->m_occurrences;
        RlWeight& weight = 
            m_weightSet->Get(i_active->m_featureIndex);
        UpdateActive(weight, occur);
    }
    UpdateEligible();
    m_learnCount++;
    m_numSteps++;
}

void RlTDLambda::ClearEligibility()
{
    list<RlWeight*>::iterator i_nonZero = m_nonZero.begin();
    while (i_nonZero != m_nonZero.end())
    {
        RlWeight* weight = *i_nonZero;
        SG_ASSERT(weight->Active());
        
        weight->Eligibility() = 0;
        weight->Active() = false;
        i_nonZero = m_nonZero.erase(i_nonZero);
    }
}

void RlTDLambda::DecayEligibility()
{
    // Decay eligibility by lambda (recency weighting)
    list<RlWeight*>::iterator i_nonZero = m_nonZero.begin();
    while (i_nonZero != m_nonZero.end())
    {
        RlWeight* weight = *i_nonZero;
        SG_ASSERT(weight->Active());
        
        weight->Eligibility() *= m_lambda;
        
        // Deactivate any eligibilities that are almost zero
        if (fabs(weight->Eligibility()) <= m_zeroThreshold)
        {
            i_nonZero = m_nonZero.erase(i_nonZero);
            weight->Eligibility() = 0;
            weight->Active() = false;
        }
        else
        {
            ++i_nonZero;
        }
    }
}

void RlTDLambda::UpdateActive(RlWeight& weight, RlOccur occurrences)
{
    SG_ASSERT(occurrences > 0);
    if (m_replacing)
        weight.Eligibility() = 0;
    weight.Eligibility() += occurrences;
    weight.IncCount();
    SANITY_CHECK(weight.Eligibility(), 
        RlWeight::MIN_WEIGHT, RlWeight::MAX_WEIGHT);
    
    // Activate eligibility if not already
    if (!weight.Active() && fabs(weight.Eligibility()) > m_zeroThreshold)
    {
        m_nonZero.push_back(&weight);
        weight.Active() = true;
    }
}

void RlTDLambda::UpdateEligible()
{
    // Update weights for all non-zero eligibility traces
    for (list<RlWeight*>::iterator i_nonZero = m_nonZero.begin(); 
        i_nonZero != m_nonZero.end(); ++i_nonZero)
    {
        RlWeight* weight = *i_nonZero;
        SG_ASSERT(weight->Active());
        UpdateWeight(*weight);
    }
}

inline void RlTDLambda::UpdateWeight(RlWeight& weight)
{    
    if (!CheckOnPolicy())
        return;

    RlFloat update = m_stepSize * m_delta * weight.Eligibility();
    if (m_mse)
        update *= m_logisticGradient;
    weight.Weight() += update;

    SANITY_CHECK(weight.Weight(), RlWeight::MIN_WEIGHT, RlWeight::MAX_WEIGHT);

    if (DoLog(weight))
    {
        int id = TraceID(weight);
        LogUpdate(id, m_stepSize, m_delta, 1, update, weight.Weight());
    }
    weight.IncCount();
}

//----------------------------------------------------------------------------

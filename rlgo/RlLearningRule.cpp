//----------------------------------------------------------------------------
/** @file RlLearningRule.cpp
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlLearningRule.h"

#include "RlAgentLog.h"
#include "RlEvaluator.h"
#include "RlHistory.h"
#include "RlUtils.h"

#include <math.h>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace boost;

//----------------------------------------------------------------------------

RlLearningRule::RlLearningRule(GoBoard& board, RlWeightSet* wset,
    RlAgentLog* log)
:   RlAutoObject(board),
    m_weightSet(wset),
    m_log(log),
    m_alpha(0.1),
    m_stepSizeMode(RL_NORM),
    m_useOffPolicy(false),
    m_logistic(true),
    m_mse(false),
    m_minGrad(0.01f),
    m_oldState(0),
    m_newState(0),
    m_terminal(false),
    m_isDataSet(false)
{
}

void RlLearningRule::LoadSettings(istream& settings)
{
    int version;
    settings >> RlVersion(version, 1, 1);
    settings >> RlSetting<RlWeightSet*>("WeightSet", m_weightSet);
    settings >> RlSetting<RlFloat>("Alpha", m_alpha);
    settings >> RlSetting<int>("StepSizeMode", m_stepSizeMode);
    settings >> RlSetting<bool>("UseOffPolicy", m_useOffPolicy);
    settings >> RlSetting<bool>("Logistic", m_logistic);
    settings >> RlSetting<bool>("MSE", m_mse);
    settings >> RlSetting<RlFloat>("MinGrad", m_minGrad);
    settings >> RlSetting<RlAgentLog*>("Log", m_log);
}

void RlLearningRule::Initialise()
{
    m_learnCount = 0;
    m_numGames = 0;
    m_isDataSet = false;
    InitLogs();
}

void RlLearningRule::SetData(RlHistory* history, 
    int from, int to, int episode)
{
    // note that learning has been separated into a getting data phase, 
    // and a learning phase. This is because we do not assume that we 
    // are looking at our experience sequentially. That is why we have 
    // SetData() and Learn().
    
    // Learning times correspond to the states (before move) at which
    // we are learning from and to.
    // It is the responsibility of the caller to decide on the update-ply
    // (timesteps between from and to)

    SG_ASSERT(from >= 0 && to > from);
    m_timeStep = from;
    m_oldState = &history->GetState(from, episode);
    m_newState = &history->GetState(to, episode);
    m_terminal = m_newState->Terminal();
    m_reward = m_newState->Reward();
    m_colour = m_oldState->Colour();
    SG_ASSERT(m_oldState->Evaluated());
    CalcValues();
    SetOnPolicy(history, from, to, episode);
    m_isDataSet = true;
}

void RlLearningRule::InitLogs()
{
    if (!m_log)
        return;
    m_log->EnsureInitialised();

    m_learnLog.reset(new RlLog(this, "Learn"));
    m_gameLog.reset(new RlLog(this, "Game"));
    m_updateTrace.reset(
        new RlTrace(this, "Update"));

    for (int i = 0; i < m_log->GetNumTraceFeatures(); ++i)
        m_updateTrace->AddLog(
            m_log->GetTraceFeatureName(i),
            m_log->GetTraceFeatureIndex(i));

    m_updateTrace->AddItemToAll("Step");
    m_updateTrace->AddItemToAll("Delta");
    m_updateTrace->AddItemToAll("Occur");
    m_updateTrace->AddItemToAll("Update");
    m_updateTrace->AddItemToAll("Weight");
    
    m_learnLog->AddItem("From");
    m_learnLog->AddItem("To");
    m_learnLog->AddItem("Terminal");
    m_learnLog->AddItem("Reward");
    m_learnLog->AddItem("OldValue");
    m_learnLog->AddItem("NewValue");
    m_learnLog->AddItem("Step");
    m_learnLog->AddItem("Delta");
    m_learnLog->AddItem("Logistic");
    m_learnLog->AddItem("OnPolicy");

    m_gameLog->AddItem("Game");
    m_gameLog->AddItem("NumSteps");
    m_gameLog->AddItem("Return");
    m_gameLog->AddItem("MeanDelta");
    m_gameLog->AddItem("RMSDelta");
    m_gameLog->AddItem("CrossEntropy");
    m_gameLog->AddItem("RMSPBE");
    m_gameLog->AddItem("MeanMCError");
    m_gameLog->AddItem("RMSMCError");
}

void RlLearningRule::LogLearn()
{
    m_statDelta.Add(m_delta);
    m_statDelta2.Add(m_delta * m_delta);
    RlFloat mcerror = m_return - m_oldValue;
    m_statMCError.Add(mcerror);
    m_statMCError2.Add(mcerror * mcerror);
    m_statCrossEntropy.Add(-m_target * log(m_oldValue) 
        - (1 - m_target) * log(1 - m_oldValue));

    if (!m_log || !m_log->LogIsActive()) return;
    
    m_learnLog->Log("From", m_oldState->TimeStep());
    m_learnLog->Log("To", m_newState->TimeStep());
    m_learnLog->Log("Terminal", m_terminal);
    m_learnLog->Log("Reward", m_reward);
    m_learnLog->Log("OldValue", m_oldValue);
    m_learnLog->Log("NewValue", m_newValue);
    m_learnLog->Log("Step", m_stepSize);
    m_learnLog->Log("Delta", m_delta);
    m_learnLog->Log("Logistic", m_logisticGradient);
    m_learnLog->Log("OnPolicy", m_onPolicy);
    m_learnLog->Step();

    m_log->Debug(RlSetup::VERBOSE)
        << "Learning from time-step " << m_oldState->TimeStep()
        << " to " << m_newState->TimeStep()
        << (m_terminal ? " (terminal)" : "")
        << ": OldV = " << m_oldValue
        << ", NewV = " << m_newValue
        << ", Delta = " << m_delta
        << ", Step = " << m_stepSize
        << "\n";
}

void RlLearningRule::LogGame()
{
    if (!m_log || !m_log->LogIsActive()) return;

    // Log basic information about the game
    m_gameLog->Log("Game", m_numGames);
    m_gameLog->Log("NumSteps", m_numSteps);
    m_gameLog->Log("Return", m_return);
    m_gameLog->Log("MeanDelta", m_statDelta.Mean());
    m_gameLog->Log("RMSDelta", sqrt(m_statDelta2.Mean()));
    m_gameLog->Log("CrossEntropy", m_statCrossEntropy.Mean());
    m_gameLog->Log("MeanMCError", m_statMCError.Mean());
    m_gameLog->Log("RMSMCError", sqrt(m_statMCError2.Mean()));
    m_gameLog->Step();
    m_statDelta.Clear();
    m_statDelta2.Clear();
    m_statMCError.Clear();
    m_statMCError2.Clear();
    m_statCrossEntropy.Clear();
}

void RlLearningRule::LogUpdate(int id, RlFloat step, RlFloat delta, 
    RlOccur occur, RlFloat update, RlFloat weight)
{
    (*m_updateTrace)[id]->Log("Step", step);
    (*m_updateTrace)[id]->Log("Delta", delta);
    (*m_updateTrace)[id]->Log("Occur", occur);
    (*m_updateTrace)[id]->Log("Update", update);
    (*m_updateTrace)[id]->Log("Weight", weight);
    (*m_updateTrace)[id]->Step();

    int tracenum = m_log->GetTraceFeatureNum(id);
    m_log->Debug(RlSetup::VERBOSE)
        << "Updating "
        << m_log->GetTraceFeatureName(tracenum) << ": "
        << step << " * " << delta << " * " << occur << " = "
        << update << " -> " << weight << "\n";
}

RlFloat RlLearningRule::ApplyLogistic(RlFloat value) const
{
    if (m_logistic)
        return RlMathUtil::Logistic(value);
    else
        return value;
}

void RlLearningRule::SetOnPolicy(RlHistory* history, 
    int from, int to, int episode)
{
    // Test whether all states between t1 and t2 are on-policy
    // NB. this may be using historic argmax to decide on/off-policy
    m_onPolicy = true;
    for (int t = from; t < to; ++t)
    {
        RlState& state = history->GetState(t, episode);
        if(!state.OnPolicy())
        {
            m_onPolicy = false;
            break;
        }
    }
}

void RlLearningRule::CalcValues()
{
    m_oldValue = ApplyLogistic(m_oldState->Eval());
    if (m_terminal)
    {
        m_newValue = 0;
    }
    else
    {
        SG_ASSERT(m_newState->Evaluated());
        m_newValue = ApplyLogistic(m_newState->Eval());
    }
}

void RlLearningRule::DoLearn(RlHistory* history, int episode, int t1, int t2)
{
    if (m_log && m_log->LogIsActive())
        m_log->Debug(RlSetup::VERBOSE) << "Learning " 
            << t1 << " <- " << t2 << "\n";
    SetData(history, t1, t2, episode);
    Learn();
}

void RlLearningRule::Learn()
{
    SG_ASSERT(m_isDataSet);
    CalcDelta();
    CalcLogisticGradient();
    CalcStepSize();
    LogLearn();
    for (RlActiveSet::Iterator i_active(m_oldState->Active()); 
        i_active; ++i_active)
    {
        RlOccur occur = i_active->m_occurrences;
        RlWeight& weight = 
            m_weightSet->Get(i_active->m_featureIndex);
        UpdateWeight(weight, occur);
    }
    m_learnCount++;
    m_numSteps++;
    m_isDataSet = false;
}

void RlLearningRule::UpdateWeight(RlWeight& weight, RlOccur occurrences)
{
    if (!CheckOnPolicy())
        return;

    RlFloat update = m_stepSize * m_delta * occurrences;
    if (m_mse)
        update *= m_logisticGradient;
    weight.Weight() += update;
    weight.IncCount();

    SANITY_CHECK(weight.Weight(), RlWeight::MIN_WEIGHT, RlWeight::MAX_WEIGHT);

    if (DoLog(weight))
    {
        int id = TraceID(weight);
        LogUpdate(id, m_stepSize, m_delta, occurrences, update,
            weight.Weight());
    }
}

void RlLearningRule::CountFeatures()
{
    RlOccur numfeatures = 0;
    for (RlActiveSet::Iterator i_active(m_oldState->Active()); 
        i_active; ++i_active)
    {
        numfeatures += i_active->m_occurrences * i_active->m_occurrences;
    }    
    if (numfeatures == 0)
        m_fraction = 0;
    else
        m_fraction = 1.0 / numfeatures;
}

void RlLearningRule::CalcStepSize()
{
    switch (m_stepSizeMode)
    {
        case RL_CONSTANT:
        {
            m_stepSize = m_alpha;
            break;
        }

        case RL_NORM:
        {
            CountFeatures();
            m_stepSize = m_alpha * m_fraction;
            break;
        }

        case RL_BINARY:
        {
            RlOccur numfeatures = m_oldState->Active().GetTotalActive();
            if (numfeatures == 0)
                m_stepSize = 0;
            else
                m_stepSize = m_alpha / numfeatures;
            break;
        }
        
        case RL_RECIPROCAL:
        {
            m_stepSize = m_alpha / (m_numGames + 1);
            break;
        }
    }
    SANITY_CHECK(m_stepSize, 0, 1);
}

void RlLearningRule::CalcLogisticGradient()
{
    // d(sigma(x))/dx = sigma(x)(1-sigma(x)). 
    m_logisticGradient = m_oldValue * (1 - m_oldValue);
    if (m_logisticGradient < m_minGrad)
        m_logisticGradient = m_minGrad;
}

void RlLearningRule::Start(RlHistory* history, int episode)
{
    m_return = history->GetReturn(episode);
    m_numSteps = 0;
}

void RlLearningRule::End()
{
    LogGame();
    m_numGames++;
}

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
/** @file RlTrainer.cpp
    See RlTrainer.h
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlTrainer.h"

#include "RlEvaluator.h"
#include "RlHistory.h"
#include "RlLearningRule.h"

using namespace std;

// reset history and count num games

//----------------------------------------------------------------------------

RlTrainer::RlTrainer(GoBoard& board, RlLearningRule* rule, 
    RlHistory* history, RlEvaluator* evaluator)
:   RlAutoObject(board),
    m_learningRule(rule),
    m_history(history),
    m_evaluator(evaluator),
    m_episodes(EP_CURRENT),
    m_numReplays(1),
    m_updateRoot(true),
    m_temporalDifference(2),
    m_refreshValues(true),
    m_interleave(true),
    m_updateWeights(true)
{
}

void RlTrainer::LoadSettings(istream& settings)
{
    int version;
    settings >> RlVersion(version, 0, 0);
    settings >> RlSetting<RlLearningRule*>("LearningRule", m_learningRule);
    settings >> RlSetting<RlHistory*>("History", m_history);
    settings >> RlSetting<RlEvaluator*>("Evaluator", m_evaluator);
    settings >> RlSetting<int>("Episodes", m_episodes);
    settings >> RlSetting<int>("NumReplays", m_numReplays);
    settings >> RlSetting<bool>("UpdateRoot", m_updateRoot);
    settings >> RlSetting<int>("TemporalDifference", m_temporalDifference);
    settings >> RlSetting<bool>("RefreshValues", m_refreshValues);
    settings >> RlSetting<bool>("Interleave", m_interleave);
    settings >> RlSetting<bool>("UpdateWeights", m_updateWeights);

    if (m_temporalDifference > RlLearningRule::MAX_TD)
        throw SgException("Temporal difference exceeds maximum");
}

void RlTrainer::RefreshValue(RlState& state)
{
    if (!state.Terminal())
        m_evaluator->RefreshValue(state);
}

int RlTrainer::SelectEpisode(int replay)
{
    switch (m_episodes)
    {
        case EP_CURRENT:
            return 0;
        case EP_LAST:
            return replay % m_history->GetNumEpisodes();
        case EP_RANDOM:
            return SgRandom::Global().Int(
                m_history->GetNumEpisodes());
        default:
            throw SgException("Unknown case for selecting episodes");
            return -1;
    }
}

//----------------------------------------------------------------------------

RlEpisodicTrainer::RlEpisodicTrainer(GoBoard& board, RlLearningRule* rule, 
    RlHistory* history, RlEvaluator* evaluator)
:   RlTrainer(board, rule, history, evaluator)
{
}

void RlEpisodicTrainer::Train()
{
    m_learningRule->SetUpdateWeights(m_updateWeights);
    int gap = m_interleave ? 1 : m_temporalDifference;
    int start = m_updateRoot ? 0 : 1;

    for (int i = 0; i < m_numReplays; ++i)
    {
        int offset = 0;
        if (!m_interleave && m_temporalDifference > 1)
            offset = SgRandom::Global().Int(m_temporalDifference);

        int episode = SelectEpisode(i);
        m_learningRule->Start(m_history, episode);
        Sweep(episode, start, offset, gap);
        m_learningRule->End();
    }
}

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlForwardTrainer);

RlForwardTrainer::RlForwardTrainer(GoBoard& board, RlLearningRule* rule, 
    RlHistory* history, RlEvaluator* evaluator)
:   RlEpisodicTrainer(board, rule, history, evaluator)
{
}

void RlForwardTrainer::Sweep(int episode, int start, int offset, int gap)
{
    // Replay the specified game from the history in a forwards pass
    for (int t1 = start + offset; t1 < m_history->GetLength(episode); t1 += gap)
    {
        if (m_history->GetState(t1, episode).Terminal())
            continue;
            
        int t2 = t1 + m_temporalDifference;
        if (m_refreshValues)
        {
            RefreshValue(m_history->GetState(t1, episode));
            RefreshValue(m_history->GetState(t2, episode));
        }
        
        m_learningRule->DoLearn(m_history, episode, t1, t2);
    }
}

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlBackwardTrainer);

RlBackwardTrainer::RlBackwardTrainer(GoBoard& board, RlLearningRule* rule, 
    RlHistory* history, RlEvaluator* evaluator)
:   RlEpisodicTrainer(board, rule, history, evaluator)
{
}

void RlBackwardTrainer::Sweep(int episode, int start, int offset, int gap)
{
    // Replay the specified game from the history in a backwards pass
    for (int t1 = m_history->GetLength(episode) - 1 - offset; 
        t1 >= start; t1 -= gap)
    {
        if (m_history->GetState(t1).Terminal())
            continue;

        int t2 = t1 + m_temporalDifference;
        if (m_refreshValues)
        {
            RefreshValue(m_history->GetState(t1, episode));
            RefreshValue(m_history->GetState(t2, episode));
        }
        m_learningRule->DoLearn(m_history, episode, t1, t2);
    }
}

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlRandomTrainer);

RlRandomTrainer::RlRandomTrainer(GoBoard& board, RlLearningRule* rule, 
    RlHistory* history, RlEvaluator* evaluator)
:   RlTrainer(board, rule, history, evaluator)
{
}

void RlRandomTrainer::Train()
{
    // Replay randomly selected transitions from the history
    m_learningRule->SetUpdateWeights(m_updateWeights);
    int start = m_updateRoot ? 0 : 1;
    for (int i = 0; i < m_numReplays; ++i)
    {
        // Select a random transition from the history
        int episode = SelectEpisode(i);
        int t1 = SgRandom::Global().Range(start,
            m_history->GetLength(episode));
        int t2 = t1 + m_temporalDifference;
        if (m_history->GetState(t1).Terminal())
            continue;
 
        if (m_refreshValues)
        {
            RefreshValue(m_history->GetState(t1, episode));
            RefreshValue(m_history->GetState(t2, episode));
        }

        m_learningRule->DoLearn(m_history, episode, t1, t2);
    }
}

//----------------------------------------------------------------------------

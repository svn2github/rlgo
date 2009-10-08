//----------------------------------------------------------------------------
/** @file RlTrainer.h
    Classes to train from historic experience
*/
//----------------------------------------------------------------------------

#ifndef RLTRAINER_H
#define RLTRAINER_H

#include "RlUtils.h"

class RlEvaluator;
class RlHistory;
class RlLearningRule;
class RlState;

//----------------------------------------------------------------------------
/** Classes to train from historic experience (experience replay) */
class RlTrainer : public RlAutoObject
{
public:

    RlTrainer(GoBoard& board, RlLearningRule* rule = 0, 
        RlHistory* history = 0, RlEvaluator* evaluator = 0);

    virtual void LoadSettings(std::istream& settings);
    virtual void Train() = 0;
    
    const RlHistory* GetHistory() const { return m_history; }
    RlLearningRule* GetLearningRule() const { return m_learningRule; }
    
protected:

    void RefreshValue(RlState& state);
    int SelectEpisode(int replay);
    
    enum
    {
        EP_CURRENT,   // train on current episode only
        EP_LAST,      // train on most recent episodes in history
        EP_RANDOM    // train on randomly selected episodes in history
    };
    
    /** Learning rule to apply to the experience */
    RlLearningRule* m_learningRule;

    /** History containing training experience */
    RlHistory* m_history;
    
    /** Evaluator used to refresh value of experience */
    RlEvaluator* m_evaluator;

    /** Which episodes to train on */
    int m_episodes;

    /** How many experience replays to perform */
    int m_numReplays;
    
    /** Whether to train from root position */
    bool m_updateRoot;
    
    /** How many time-steps between pairs of states that are updated:
        1=Update from black to white
        2=Update from black to black
        n=Update from n steps into the future */
    int m_temporalDifference;
    
    /** Whether to refresh values online, or use historic values */
    bool m_refreshValues;
    
    /** Whether to interleave updates, or to do a single trajectory
        (Only relevant for episodic updates with temporal difference > 1) */
    bool m_interleave;
};

//----------------------------------------------------------------------------
/** Train in sweeps through episodes of experience */
class RlEpisodicTrainer : public RlTrainer
{
public:

    RlEpisodicTrainer(GoBoard& board, RlLearningRule* rule = 0, 
        RlHistory* history = 0, RlEvaluator* evaluator = 0);

    virtual void Train();
    virtual void Sweep(int episode, int start, int offset, int gap) = 0;
};


//----------------------------------------------------------------------------
/** Train in forward sweeps through episodes of experience */
class RlForwardTrainer : public RlEpisodicTrainer
{
public:

    DECLARE_OBJECT(RlForwardTrainer);

    RlForwardTrainer(GoBoard& board, RlLearningRule* rule = 0, 
        RlHistory* history = 0, RlEvaluator* evaluator = 0);

    virtual void Sweep(int episode, int start, int offset, int gap);
};

//----------------------------------------------------------------------------
/** Train in backward sweeps through episodes of experience */
class RlBackwardTrainer : public RlEpisodicTrainer
{
public:

    DECLARE_OBJECT(RlBackwardTrainer);

    RlBackwardTrainer(GoBoard& board, RlLearningRule* rule = 0, 
        RlHistory* history = 0, RlEvaluator* evaluator = 0);

    virtual void Sweep(int episode, int start, int offset, int gap);
};

//----------------------------------------------------------------------------
/** Train on randomly selected transitions from the history */
class RlRandomTrainer : public RlTrainer
{
public:

    DECLARE_OBJECT(RlRandomTrainer);

    RlRandomTrainer(GoBoard& board, RlLearningRule* rule = 0, 
        RlHistory* history = 0, RlEvaluator* evaluator = 0);

    virtual void Train();
};
//----------------------------------------------------------------------------

#endif // RLTRAINER_H

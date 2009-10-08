//----------------------------------------------------------------------------
/** @file RlGibbs.h
    Policy based on Gibbs distribution
*/
//----------------------------------------------------------------------------

#ifndef RLGIBBS_H
#define RLGIBBS_H

#include "RlPolicy.h"

//----------------------------------------------------------------------------
/** Select move according to Gibbs distribution */
class RlGibbs : public RlPolicy
{
public:

    DECLARE_OBJECT(RlGibbs);

    RlGibbs(GoBoard& board, RlEvaluator* evaluator = 0, 
         RlAgentLog* log = 0, RlFloat temperature = 1.0);

    /** Load settings from specified file */
    virtual void LoadSettings(std::istream& settings);

    /** Initialise from settings */
    virtual void Initialise();

    /** Select a move in the current position for the specified colour */
    virtual SgMove SelectMove(RlState& state);
        
    /** Debug display of probability distribution (after calculation) */
    void DebugDisplay(std::ostream& ostr);

    /** Probability of specified move */
    RlFloat GetProbability(SgMove move) const;
    
protected:
    
    /** Make Gibbs distribution from specified preferences */
    void MakeGibbs(bool negate);
    
    /** Evaluate candidate moves */
    void EvaluateMoves(RlState& state);

    /** Initialise output log for this policy */
    virtual void InitLogs();

    /** Log any info for this policy */
    virtual void LogPolicy(const RlState& state, SgBlackWhite move);
    
private:

    RlFloat m_temperature;
    bool m_centre;
    bool m_redraw;
    
    /** Evaluation data for current state only */
    std::vector<SgMove> m_moves;    
    std::vector<RlFloat> m_probs;
    std::vector<RlFloat> m_evals;
    std::vector<RlFloat> m_values;
    std::vector<RlChangeList> m_changeLists;
    std::vector<int> m_moveIndices;
    
    /** Debugging statistics */
    RlStat m_statMean;
    RlStat m_statEntropy;
    RlStat m_statDeterministic;
};

//----------------------------------------------------------------------------

#endif // RLGIBBS_H

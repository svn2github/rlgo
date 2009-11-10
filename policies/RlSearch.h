//----------------------------------------------------------------------------
/** @file RlSearch.h
    Full width alpha-beta search using RLGO evaluation
*/
//----------------------------------------------------------------------------

#ifndef RLSEARCH_H
#define RLSEARCH_H

#include "RlAlphaBeta.h"
#include "RlPolicy.h"

class RlConvert;
class RlTimeControl;

//----------------------------------------------------------------------------
/** Search class using RL evaluator */
class RlSearchPolicy : public RlPolicy
{
public:

    DECLARE_OBJECT(RlSearchPolicy);

    RlSearchPolicy(GoBoard& board, RlEvaluator* searchevaluator = 0, 
        RlAlphaBeta* alphabeta = 0);

    virtual void LoadSettings(std::istream& settings);
    virtual SgMove SelectMove(RlState& state);
    virtual bool SearchValue(RlFloat& value) const;

private:

    RlAlphaBeta* m_alphaBeta;

    /** If m_convert != 0
        m_convertEvaluator is converted into m_evaluator at start of search */
    RlEvaluator* m_convertEvaluator;
    RlConvert* m_convert;

    enum
    {
        RL_DEPTH,
        RL_ELAPSED,
        RL_PREDICTED
    };

    /** Time management */
    int m_controlMode;
    RlTimeControl* m_timeControl;

    RlFloat m_searchValue;
    std::vector<SgMove> m_principalVariation;
};

//----------------------------------------------------------------------------

#endif // RLSEARCH_H


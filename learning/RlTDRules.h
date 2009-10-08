//----------------------------------------------------------------------------
/** @file RlTDRules.h
    Temporal difference learning rules
*/
//----------------------------------------------------------------------------

#ifndef RLTDRULES_H
#define RLTDRULES_H

#include "RlLearningRule.h"

//----------------------------------------------------------------------------
/** Update rule based on TD(0) */
class RlTD0 : public RlLearningRule
{
public:

    DECLARE_OBJECT(RlTD0);

    RlTD0(GoBoard& board, RlWeightSet* wset = 0, RlAgentLog* log = 0);

protected:

    /** TD-error */
    virtual void CalcDelta();
};

//----------------------------------------------------------------------------
/** Lambda-return algorithm (for backwards execution only) */
class RlLambdaReturn : public RlLearningRule
{
public:

    DECLARE_OBJECT(RlLambdaReturn);

    RlLambdaReturn(GoBoard& board, RlWeightSet* wset = 0, 
        RlAgentLog* log = 0, RlFloat lambda = 0.9f);

    /** Load settings from specified file */
    virtual void LoadSettings(std::istream& settings);
        
    /** Lambda-return can only operate with backwards execution */
    virtual bool IsForwards() const { return false; }
    virtual bool IsBackwards() const { return true; }

protected:

    /** Lambda-return */
    virtual void CalcDelta();    
    
private:

    RlFloat m_lambda;
    RlFloat m_lambdaReturn;
};

//----------------------------------------------------------------------------
/** TD(lambda) algorithm */
class RlTDLambda : public RlTD0
{
public:

    DECLARE_OBJECT(RlTDLambda);

    RlTDLambda(GoBoard& board, RlWeightSet* wset = 0, RlAgentLog* log = 0, 
        RlFloat lambda = 1.0, bool replacing = false);

    /** Load settings from specified file */
    virtual void LoadSettings(std::istream& settings);

    /** Start a new episode */
    virtual void Start(RlHistory* history, int episode);
    
    /** Update all weights (must set data first) */
    virtual void Learn();
    
    /** TD(lambda) can only operate with forwards execution */
    virtual bool IsForwards() const { return true; }
    virtual bool IsBackwards() const { return false; }

protected:

    /** Clear all eligibility traces */
    void ClearEligibility();

    /** Decay eligibility before changing weights */
    void DecayEligibility();

    /** Update eligibility for an active feature */
    void UpdateActive(RlWeight& weight, RlOccur occurrences);

    /** Update weights for all features with non-zero eligibilities */
    void UpdateEligible();
    
    /** Update a single weight */
    void UpdateWeight(RlWeight& weight);

private:

    RlFloat m_lambda;
    bool m_replacing;
    RlFloat m_zeroThreshold;
    std::list<RlWeight*> m_nonZero;
};

//----------------------------------------------------------------------------

#endif // RLTDRULES_H

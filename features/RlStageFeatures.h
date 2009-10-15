//----------------------------------------------------------------------------
/** @file RlStageFeatures.h
    Features for the stage of the game
*/
//----------------------------------------------------------------------------

#ifndef RLSTAGEFEATURES_H
#define RLSTAGEFEATURES_H

#include "RlBinaryFeatures.h"
#include "RlTracker.h"

//----------------------------------------------------------------------------
/** Binary features for stage of the game (based on current time-step) */
class RlStageFeatures : public RlBinaryFeatures
{
public:

    DECLARE_OBJECT(RlStageFeatures);

    RlStageFeatures(GoBoard& board, int timescale = 10);
    
    /** Load settings */
    virtual void LoadSettings(std::istream& settings);
    
    /** Create corresponding object for incremental tracking */
    virtual RlTracker* CreateTracker(
        std::map<RlBinaryFeatures*, RlTracker*>& trackermap);

    /** Get the total number of features currently in this set */
    virtual int GetNumFeatures() const;

    /** Read a feature from stream (see implementation for spec) */
    virtual int ReadFeature(std::istream& desc) const;

    /** Describe a feature in text form */
    virtual void DescribeFeature(int featureindex, std::ostream& str) const;

    /** Single word description of feature set */
    virtual void DescribeSet(std::ostream& str) const;
    
    int GetFeatureIndex(int timestep) const
    {
        SG_ASSERT(timestep >= 0 && timestep < RL_MAX_TIME);
        if (timestep >= m_maxStep)
            return m_maxStep / m_timeScale;
        else
            return timestep / m_timeScale;
    }
    
private:

    /** Number of timesteps in each stage */
    int m_timeScale;
    
    /** Whether timestep is relative to last reset, or is absolute move num */
    bool m_relative;
    
    /** Maximum number of time-steps that are divided into stages
        (all subsequent time-steps belong to final stage) */
    int m_maxStep;

friend class RlStageTracker;
};

class RlStageTracker : public RlTracker
{
public:

    RlStageTracker(GoBoard& board, const RlStageFeatures* stagefeatures = 0);

    /** Reset to current board position */
    virtual void Reset();
    
    /** Incremental execute */
    virtual void Execute(SgMove move, SgBlackWhite colour, 
        bool execute, bool store);

    /** Incremental undo */
    virtual void Undo();

    /** Size of active set */
    virtual int GetActiveSize() const;    

private:

    const RlStageFeatures* m_stageFeatures;
    int m_timeStep;
};

//----------------------------------------------------------------------------

#endif // RLSTAGEFEATURES_H

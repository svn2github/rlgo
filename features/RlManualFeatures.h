//----------------------------------------------------------------------------
/** @file RlManualFeatures.h
    Simple feature set for testing, with active features assigned manually
*/
//----------------------------------------------------------------------------

#ifndef RLMANUALFEATURES_H
#define RLMANUALFEATURES_H

#include "RlActiveSet.h"
#include "RlBinaryFeatures.h"
#include "RlTracker.h"

//----------------------------------------------------------------------------
/** Manually assigned features for testing purposes */
class RlManualFeatureSet : public RlBinaryFeatures
{
public:

    DECLARE_OBJECT(RlManualFeatureSet);

    RlManualFeatureSet(GoBoard& board, int numfeatures = 0);

    void Initialise();
    
    void LoadSettings(std::istream& settings);

    /** Create corresponding object for incremental tracking */
    virtual RlTracker* CreateTracker(
        std::map<RlBinaryFeatures*, RlTracker*>& trackermap);

    /** Get the total number of features currently in this set */
    virtual int GetNumFeatures() const;

    /** Get a feature by description (see implementation for spec) */
    virtual int ReadFeature(std::istream& desc) const;

    /** Describe a feature in text form */
    virtual void DescribeFeature(int featureindex, std::ostream& str) const;
    
    /** Single word description of feature set */
    virtual void DescribeSet(std::ostream& name) const;

    void Clear();
    void Set(int feature, int count) { m_active[feature] = count; }

private:

    int m_numFeatures;
    std::vector<int> m_active;
    
friend class RlManualTracker;
};

//----------------------------------------------------------------------------
/** Tracker for manually assigned features */
class RlManualTracker : public RlTracker
{
public:

    RlManualTracker(GoBoard& board, RlManualFeatureSet* manual = 0);

    /** Incremental reset */
    virtual void Reset();

    /** Incremental execute */
    virtual void Execute(SgMove move, SgBlackWhite colour, bool execute);

    /** Incremental undo */
    virtual void Undo();

    /** Size of active set */
    virtual int GetActiveSize() const;

protected:

    void Update();

private:

    RlManualFeatureSet* m_manual;
};

//----------------------------------------------------------------------------

#endif // RLMANUALFEATURES_H

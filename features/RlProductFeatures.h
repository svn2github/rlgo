//----------------------------------------------------------------------------
/** @file RlProductFeatures.h
    Conjunctions of two feature sets
*/
//----------------------------------------------------------------------------

#ifndef RLPRODUCTFEATURES_H
#define RLPRODUCTFEATURES_H

#include "RlCompoundFeatures.h"

//----------------------------------------------------------------------------
/** Conjunctions of two feature sets */
class RlProductFeatures : public RlCompoundFeatures
{
public:
    
    DECLARE_OBJECT(RlProductFeatures);
    
    RlProductFeatures(GoBoard& board, 
        RlBinaryFeatures* set1 = 0, RlBinaryFeatures* set2 = 0);

    virtual void LoadSettings(std::istream& settings);
    virtual void Initialise();

    /** Create corresponding object for incremental tracking */
    virtual RlTracker* CreateTracker(
        std::map<RlBinaryFeatures*, RlTracker*>& trackermap);

    /** Get the total number of features currently in this set */
    virtual int GetNumFeatures() const;

    /** Read a feature from stream (see implementation for spec) */
    virtual int ReadFeature(std::istream& desc) const;

    /** Describe a feature in text form */
    virtual void DescribeFeature(int featureindex, std::ostream& str) const;

    /** Describe a feature in LaTex form */
    virtual void DescribeTex(int featureindex, std::ostream& tex, 
        bool invert) const;
    
    /** Display a feature in GoGui format */
    virtual void DisplayFeature(int featureindex, std::ostream& cmd) const;

    /** Single word description of feature set */
    virtual void DescribeSet(std::ostream& str) const;

    /** Get a page of features */
    virtual void GetPage(int pagenum, std::vector<int>& indices, 
        std::ostream& pagename) const;

    /** Get number of feature pages */
    virtual int GetNumPages() const;

    /** Get the position of a feature */
    virtual SgPoint GetPosition(int featureindex) const;

    /** Collect all points touching this feature */
    virtual void CollectPoints(int featureindex, 
        std::vector<SgPoint>& points) const;

    /** Check whether a feature touches the specified point */
    virtual bool Touches(int featureindex, SgPoint point) const;

    const RlBinaryFeatures* GetSet1() const { return m_set1; }
    const RlBinaryFeatures* GetSet2() const { return m_set2; }

protected:

    int GetFeatureIndex(int index1, int index2) const;

private:

    RlBinaryFeatures* m_set1;
    RlBinaryFeatures* m_set2;
    
friend class RlProductTracker;
};

//----------------------------------------------------------------------------
/** Tracker for product features */
class RlProductTracker : public RlCompoundTracker
{
public:

    RlProductTracker(GoBoard& board, RlProductFeatures* features,
        RlTracker* tracker1, RlTracker* tracker2);

    /** Propagate changes from child trackers */
    virtual void PropagateChanges();

    /** Size of active set */
    virtual int GetActiveSize() const;

protected:

    void JoinChanges();

    int GetSlotIndex(int slot1, int slot2);
        
private:

    const RlProductFeatures* m_productFeatures;
    RlTracker* m_tracker1;
    RlTracker* m_tracker2;
};

//----------------------------------------------------------------------------

#endif // RLPRODUCTFEATURES_H

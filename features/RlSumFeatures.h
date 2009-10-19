//----------------------------------------------------------------------------
/** @file RlSumFeatures.h
    Sum of several feature sets
*/
//----------------------------------------------------------------------------

#ifndef RLSUMFEATURES_H
#define RLSUMFEATURES_H

#include "RlCompoundFeatures.h"

//----------------------------------------------------------------------------
/** Sum of several feature sets */
class RlSumFeatures : public RlCompoundFeatures
{
public:
    
    DECLARE_OBJECT(RlSumFeatures);

    RlSumFeatures(GoBoard& board);
    
    void Initialise();
    void LoadSettings(std::istream& settings);

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

    /** Output a latex formatted table of the top weighted features */
    virtual void TopTex(std::ostream& tex, const RlWeightSet* wset, 
        int rows, int cols) const;

    int GetFeatureIndex(int set, int localindex) const;
    int GetFeatureSet(int featureindex) const;

private:

    std::vector<int> m_offset;
    int m_totalFeatures;
    
friend class RlSumTracker;
};

//----------------------------------------------------------------------------
/** Tracker for sum features */
class RlSumTracker : public RlCompoundTracker
{
public:

    RlSumTracker(GoBoard& board, const RlSumFeatures* features);

    /** Add a new tracker */
    virtual void AddTracker(RlTracker* tracker);

    /** Propagate changes from child trackers */
    virtual void PropagateChanges();

    /** Size of active set */
    virtual int GetActiveSize() const;
        
protected:

    void SumChanges();
        
private:

    const RlSumFeatures* m_sumFeatures;
    std::vector<int> m_slotOffset;
    int m_totalSlots;
};

//----------------------------------------------------------------------------

#endif // RLSUMFEATURES_H

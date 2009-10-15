//----------------------------------------------------------------------------
/** @file RlCompoundFeatures.h
    Base class for feature sets built from other feature sets
*/
//----------------------------------------------------------------------------

#ifndef RLCOMPOUNDFEATURES_H
#define RLCOMPOUNDFEATURES_H

#include "RlBinaryFeatures.h"
#include "RlTracker.h"

//----------------------------------------------------------------------------
/** Base class for feature sets built from other feature sets */
class RlCompoundFeatures : public RlBinaryFeatures
{
public:
    
    RlCompoundFeatures(GoBoard& board);

    /** Ensure that all child feature sets are initialised */
    virtual void Initialise();

    /** Feature discovery */
    virtual void Start();
    virtual void End();
    virtual void Clear();
    virtual void Prune();
    virtual void SaveData(std::ostream& data);
    virtual void LoadData(std::istream& data);    

    void AddFeatureSet(RlBinaryFeatures* features);
    int GetNumSets() const { return m_featureSets.size(); }
    const RlBinaryFeatures* GetSet(int set) const;

protected:

    int GetFeatureSet(int featureindex) const;

    /** Create trackers for all children and store in the trackermap.
        Won't create trackers for classes already in the trackermap.
        To be called by child classes during CreateTracker. */
    void CreateChildTrackers(
        std::map<RlBinaryFeatures*, RlTracker*>& trackermap);

    std::vector<RlBinaryFeatures*> m_featureSets;
};

inline const RlBinaryFeatures* RlCompoundFeatures::GetSet(int set) const 
{ 
    SG_ASSERT(set >= 0 && set < ssize(m_featureSets));
    return m_featureSets[set]; 
}

//----------------------------------------------------------------------------
/** Base class for trackers that call other trackers */
class RlCompoundTracker : public RlTracker
{
public:

    RlCompoundTracker(GoBoard& board);

    virtual void AddTracker(RlTracker* tracker);

    /** Initialise all trackers */
    virtual void Initialise();

    /** Reset to current board position */
    virtual void Reset();
    
    /** Incremental execute */
    virtual void Execute(SgMove move, SgBlackWhite colour, 
        bool execute, bool store);

    /** Incremental undo */
    virtual void Undo();
    
    /** Add/subtract changelist from active set */
    virtual void AddChanges(bool store);
    virtual void SubChanges();

    /** Update dirty moves */
    virtual void UpdateDirty(SgMove move, SgBlackWhite colour,
        RlDirtySet& dirty);

    /** Clear all changes including child trackers */
    virtual void ClearChanges();

    /** Propagate changes from child trackers */
    virtual void PropagateChanges();

    /** Remember current position for fast resets */
    virtual void SetMark();
    virtual void ClearMark();
                
protected:

    std::vector<RlTracker*> m_trackers;
};

//----------------------------------------------------------------------------

#endif // RLCOMPOUNDFEATURES_H

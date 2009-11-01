//----------------------------------------------------------------------------
/** @file RlSharedFeatures.h
    Share features between equivalent sets
*/
//----------------------------------------------------------------------------

#ifndef RLSHAREDFEATURES_H
#define RLSHAREDFEATURES_H

#include "RlCompoundFeatures.h"
#include "RlUtils.h"
#include <vector>

//----------------------------------------------------------------------------
/** Shared features divide a set of input features into equivalence classes,
    with each equivalence class represented by a canonical feature,
    and corresponding to a single output feature.    
*/

class RlSharedFeatures : public RlCompoundFeatures
{
public:

    DECLARE_OBJECT(RlSharedFeatures);

    RlSharedFeatures(GoBoard& board, RlBinaryFeatures* featureset = 0);

    virtual ~RlSharedFeatures();

    /** Main lookup function */
    int GetOutputFeature(int inputfeature) const
    { 
        return m_lookup[inputfeature].m_index; 
    }

    /** Get the sign of the lookup (for antisymmetric weight sharing) */
    int GetSign(int inputfeature) const
    { 
        return m_lookup[inputfeature].m_sign; 
    }
    
    /** Inverse lookup: retrieve canonical feature from output feature */
    int GetCanonicalFeature(int outputfeature) const
    {
        return m_inverseMap[outputfeature];
    }
    
    /** Calculate the lowest index amongst equivalent set of features */
    int CalcCanonical(int inputfeature, int& sign) const;

    /** By default each input feature is in its own equivalence class */
    virtual void GetEquivalent(int inputfeature, 
        std::vector<int>& equivalent,
        std::vector<int>& signs) const;

    /** Some features can be ignored and given zero sign */
    virtual bool IgnoreFeature(int featureindex) const;

    /** Display a feature in GoGui format */
    virtual void DisplayFeature(int featureindex, std::ostream& cmd) const;

    /** Debug display of lookup tables */
    void Display(std::ostream& disp) const;

    /** Get number of input features */
    int GetNumInputFeatures() const { return m_numInputFeatures; }

    //------------------------------------------------------------------------
    // RlBinaryFeatures virtual functions

    /** Setup lookup table to canonical features, based on main feature set */
    virtual void Initialise();

    /** Load settings file */
    virtual void LoadSettings(std::istream& settings);
    
    /** Create corresponding object for incremental tracking */
    virtual RlTracker* CreateTracker(
        std::map<RlBinaryFeatures*, RlTracker*>& trackermap);

    /** Number of output features */
    virtual int GetNumFeatures() const;
    
    /** Get an output feature by description */
    virtual int ReadFeature(std::istream& desc) const;

    /** Describe an output feature in text form */
    virtual void DescribeFeature(int outputfeature, std::ostream& str) const;

    /** Describe an output feature in LaTex form */
    virtual void DescribeTex(int outputfeature, std::ostream& tex, 
        bool invert) const;

    /** Describe set in single word */
    virtual void DescribeSet(std::ostream& str) const;

    /** Get a page of features */
    virtual void GetPage(int pagenum, std::vector<int>& indices, 
        std::ostream& pagename) const;

    /** Get number of feature pages */
    virtual int GetNumPages() const;

    /** Get an output feature's sign by description */
    virtual int GetSign(const std::string& desc) const;

    /** Get an output feature's sign by description */
    virtual int ReadSign(std::istream& desc) const;

    /** Get the position of a feature */
    virtual SgPoint GetPosition(int featureindex) const;

    /** Get the centre of a feature */
    virtual void CollectPoints(int featureindex, 
        std::vector<SgPoint>& points) const;

    /** Check whether a feature touches the specified point */
    virtual bool Touches(int featureindex, SgPoint point) const;

    /** Accessor for underlying feature set */
    const RlBinaryFeatures* FeatureSet() const { return m_featureSet; } 
    RlBinaryFeatures* FeatureSet() { return m_featureSet; }
    
    void UseTableFile(bool val) { m_tableFile = val; }
    void IgnoreSelfInverse(bool selfinverse) { m_selfInverse = selfinverse; }

protected:

    /** Make lookup tables */
    void MakeTables();

    /** Load in precalculated lookup tables */
    bool LoadTables();

    /** Save out precalculated lookup tables */
    bool SaveTables();

    /** Get filename to use for lookup tables */
    bfs::path TablePath() const;

private:

    struct Lookup
    {
        int m_index; // Lookup from input features to output features
        int m_sign; // Sign of input->output feature map
    };

    RlBinaryFeatures* m_featureSet;
    int m_numInputFeatures;
    int m_numOutputFeatures;
    Lookup* m_lookup;
    int* m_inverseMap; // Inverse lookup from output to canonical features

    bool m_selfInverse;
    bool m_tableFile;
};

//----------------------------------------------------------------------------
/** Tracker for shared features */
class RlSharedTracker : public RlCompoundTracker
{
public:

    RlSharedTracker(GoBoard& board, const RlSharedFeatures* features,
        RlTracker* tracker);

    /** Reset to current board position */
    virtual void Reset();
    
    /** Incremental execute */
    virtual void Execute(SgMove move, SgBlackWhite colour, 
        bool execute, bool store);

    /** Incremental undo */
    virtual void Undo();

    /** Size of active set */
    virtual int GetActiveSize() const;

protected:

    void ShareChanges();

private:

    const RlSharedFeatures* m_sharedFeatures;
    RlTracker* m_tracker;
};

//----------------------------------------------------------------------------

#endif // RLSHAREDFEATURES_H


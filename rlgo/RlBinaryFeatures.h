//----------------------------------------------------------------------------
/** @file RlBinaryFeatures.h
    Base class for a binary set of features
*/
//----------------------------------------------------------------------------

#ifndef RLGO_BINARY_FEATURES
#define RLGO_BINARY_FEATURES

#include "GoBoard.h"
#include "RlUtils.h"

#include <fstream>
#include <vector>
#include <map>

class RlTracker;
class RlWeightSet;

//----------------------------------------------------------------------------
/** An abstract class for storing a set of binary features */
class RlBinaryFeatures : public RlAutoObject
{
public:
    
    RlBinaryFeatures(GoBoard& board);

    /** Each binary feature set creates a corresponding tracker object.
        The trackermap ensures that this is only created once for each set.
        This is used to incrementally track the currently active features */
    virtual RlTracker* CreateTracker(
        std::map<RlBinaryFeatures*, RlTracker*>& trackermap) = 0;

    /** Get the total number of features in this set.
        If feature discovery is used, this is the total capacity of the set. */
    virtual int GetNumFeatures() const = 0;

    //------------------------------------------------------------------------
    /** Feature discovery.
        Most feature sets represent a fixed set of features.
        Some feature sets "discover" the features over time, from data. */

    /** Clear any discovered data */
    virtual void Clear() { }

    /** Start new round of discovery */
    virtual void Start() { }
    
    /** End round of discovery */
    virtual void End() { }

    /** Prune discovered data */
    virtual void Prune() { }

    /** Save any discovered data */
    virtual void SaveData(std::ostream& data);

    /** Load any discovered data */
    virtual void LoadData(std::istream& data);
    
    //------------------------------------------------------------------------
    /** Debug and display */

    /** Read a feature from stream (see implementation for spec) */
    virtual int ReadFeature(std::istream& desc) const = 0;

    /** Get a feature by description (see implementation for spec) */
    int GetFeature(const std::string& desc) const;

    /** Describe a feature in text form */
    virtual void DescribeFeature(int featureindex, 
        std::ostream& str) const = 0;

    /** Describe a feature in LaTex form */
    virtual void DescribeTex(int featureindex, std::ostream& tex,
        bool invert) const;
    
    /** Display a feature in GoGui format */
    virtual void DisplayFeature(int featureindex, std::ostream& cmd) const;

    /** Single word description of feature set */
    virtual void DescribeSet(std::ostream& str) const = 0;

    /** Single word description of feature set */
    std::string SetName() const;

    /** Get a page of features */
    virtual void GetPage(int pagenum, std::vector<int>& indices, 
        std::ostream& pagename) const;

    /** Get number of feature pages */
    virtual int GetNumPages() const;

    /** Point associated with a feature (e.g. its anchor) */
    virtual SgPoint GetPosition(int featureindex) const;

    /** Check whether a feature touches the specified point */
    virtual bool Touches(int featureindex, SgPoint point) const;
    
    /** Collect all points touching this feature */
    virtual void CollectPoints(int featureindex, 
        std::vector<SgPoint>& points) const;

    /** Output a latex formatted table of the top weighted features */
    virtual void TopTex(std::ostream& tex, const RlWeightSet* wset, 
        int rows, int cols) const;
};

#endif


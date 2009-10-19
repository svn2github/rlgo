//----------------------------------------------------------------------------
/** @file RlLocalShapeConvert.h
    Class to convert weights for local shape features into equivalent form */
//----------------------------------------------------------------------------

#ifndef RL_LOCALSHAPECONVERT_H
#define RL_LOCALSHAPECONVERT_H

#include "RlConvert.h"

class RlLocalShapeFeatures;
class RlLocalShapeSet;

//-----------------------------------------------------------------------------
/** Convert a local shape set into a single size
    e.g. 1x1, 2x2, 3x3 -> 3x3 only */
class RlLocalShapeFusion : public RlConvert
{
public:

    DECLARE_OBJECT(RlLocalShapeFusion);

    RlLocalShapeFusion(GoBoard& board, RlLocalShapeSet* localshapeset = 0,
        RlLocalShapeFeatures* fusedfeatures = 0);

    virtual void LoadSettings(std::istream& settings);
    virtual void Initialise();

    /** Use fuse table to convert weight set */
    virtual void Convert(const RlWeightSet* allweights,
        RlWeightSet* fusedweights) const;

protected:

    /** Make a table to fuse a local shape set into a single size
        e.g. 1x1, 2x2, 3x3 -> 3x3 only */
    void MakeFuseTable();
        
private:

    RlLocalShapeSet* m_localShapeSet; // source
    RlLocalShapeFeatures* m_fusedFeatures; // target
    std::vector< std::vector<int> > m_fuseTable;
};

//-----------------------------------------------------------------------------
/** Convert local shape set with weight sharing into an equivalent
    local shape set with no weight sharing
    e.g. 1x1 LI/LD, 2x2 LI/LD, 3x3 LI/LD -> 1x1, 2x2, 3x3 */
class RlLocalShapeUnshare : public RlConvert
{
public:

    DECLARE_OBJECT(RlLocalShapeUnshare);

    RlLocalShapeUnshare(GoBoard& board, 
        RlLocalShapeSet* sharedshapeset = 0,
        RlLocalShapeSet* unsharedshapeset = 0);

    virtual void LoadSettings(std::istream& settings);
    virtual void Initialise();

    void Convert(const RlWeightSet* sharedweights,
        RlWeightSet* unsharedweights) const;

private:

    RlLocalShapeSet* m_sharedShapeSet; // source
    RlLocalShapeSet* m_unsharedShapeSet; // target
};

#endif // RL_LOCALSHAPECONVERT_H

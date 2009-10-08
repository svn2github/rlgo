//----------------------------------------------------------------------------
/** @file RlWeightSet.h
    A complete set of weights corresponding to a set of input features
*/
//----------------------------------------------------------------------------

#ifndef RLWEIGHTSET_H
#define RLWEIGHTSET_H

#include "RlWeight.h"
#include "RlUtils.h"
#include <vector>

class RlSharedMemory;
class RlBinaryFeatures;

//----------------------------------------------------------------------------
/** A complete set of weights corresponding to a set of input features */
class RlWeightSet : public RlAutoObject
{
public:

    DECLARE_OBJECT(RlWeightSet);

    RlWeightSet(GoBoard& board, RlBinaryFeatures* featureset = 0);
    ~RlWeightSet();
    
    virtual void Initialise();
    virtual void LoadSettings(std::istream& settings);
    
    /** Set all weights to zero */
    void ZeroWeights();

    /** Set all weights to uniform random range */
    void RandomiseWeights(RlFloat min, RlFloat max);

    /** Add weights from another weight set */
    void AddWeights(RlWeightSet* source);

    /** Subtract weights from another weight set */
    void SubWeights(RlWeightSet* source);

    /** Get a weight (non-const access) */
    RlWeight& Get(int featureindex)
    { 
        return m_weights[featureindex];
    }

    /** Get a weight (const access) */
    const RlWeight& Get(int featureindex) const
    { 
        return m_weights[featureindex];
    }

    /** Total number of input features */
    int GetNumFeatures() const { return m_numFeatures; }
    
    /** Load weights */
    void Load(std::istream& wstream);
    
    /** Save weights */
    void Save(std::ostream& wstream);
    
    /** Get feature index of specified weight */
    int GetFeatureIndex(const RlWeight* weight) const
    {
        int index = weight - m_weights;
        SG_ASSERT(index >= 0 && index < m_numFeatures);
        return index;
    }

private:

    RlBinaryFeatures* m_featureSet;
    RlWeight* m_weights;
    int m_numFeatures;
    int m_numWeights;
    std::string m_shareName;
    RlSharedMemory* m_sharedMemory;
    bool m_strict;
    int m_streamMode; // deprecated
};

//----------------------------------------------------------------------------

#endif // RLCONNECTIONSET_H


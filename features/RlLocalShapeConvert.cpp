//----------------------------------------------------------------------------
/** @file RlLocalShapeConvert.cpp
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlLocalShapeConvert.h"

#include "RlLocalShape.h"
#include "RlLocalShapeFeatures.h"
#include "RlLocalShapeSet.h"
#include "RlLocalShapeShare.h"
#include "RlShapeUtil.h"
#include "RlWeightSet.h"

using namespace RlShapeUtil;
using namespace std;

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlLocalShapeFusion);

RlLocalShapeFusion::RlLocalShapeFusion(GoBoard& board, 
    RlLocalShapeSet* localshapeset,
    RlLocalShapeFeatures* fusedfeatures)
:   RlConvert(board),
    m_localShapeSet(localshapeset),
    m_fusedFeatures(fusedfeatures)
{
}

void RlLocalShapeFusion::LoadSettings(istream& settings)
{
    settings >> RlSetting<RlLocalShapeSet*>
        ("LocalShapeSet", m_localShapeSet);
    settings >> RlSetting<RlLocalShapeFeatures*>
        ("FusedFeatures", m_fusedFeatures);
}

void RlLocalShapeFusion::Initialise()
{
    if (!m_localShapeSet || !m_fusedFeatures)
        throw SgException("Fusing requires unshared shape set and fused features");

    m_localShapeSet->EnsureInitialised();
    m_fusedFeatures->EnsureInitialised();

    if (m_fusedFeatures->GetXSize() != m_localShapeSet->GetMaxSize()
        || m_fusedFeatures->GetYSize() != m_localShapeSet->GetMaxSize())
        throw SgException("Fused set must match largest size of local shape set");
    if (m_localShapeSet->GetShapeSpec() != eSquare)
        throw SgException("Fusion only supports square shapes currently");
    if (m_localShapeSet->GetShareTypes() != (1 << eNone))
        throw SgException("Fusion can't use weight sharing, apply unshare conversion first");

    MakeFuseTable();
}

void RlLocalShapeFusion::MakeFuseTable()
{    
    // Currently assumes square shapes only
    int minsize = m_localShapeSet->GetMinSize();
    int maxsize = m_localShapeSet->GetMaxSize();
    
    m_fuseTable.resize(m_fusedFeatures->GetNumFeatures());
    for (int featureindex = 0; 
        featureindex < m_fusedFeatures->GetNumFeatures(); 
        featureindex++)
    {
        RlLocalShape localshape(maxsize, maxsize);
        int anchorindex, shapeindex, x, y;
        m_fusedFeatures->DecodeIndex(
            featureindex, shapeindex, anchorindex, x, y);
        localshape.SetShapeIndex(shapeindex);

        // Find set of all subshapes to fuse into this feature index
        m_fuseTable[featureindex].clear();
        for (int size = minsize; size <= maxsize; ++size) 
        {
            int setindex = size - minsize;
            RlLocalShapeFeatures* subfeatures = 
                m_localShapeSet->GetShapes(setindex);
            RlLocalShape subshape(size, size);
            
            // Always include anchor subshape (bottom-left subshape, dx=0 dy=0)
            // Also include all subshapes of topmost and rightmost features
            // because these are not anchor subshapes of any shape
            int maxdx = 0, maxdy = 0;
            if (x == m_fusedFeatures->GetXNum() - 1)
                maxdx = maxsize - size;
            if (y == m_fusedFeatures->GetYNum() - 1)
                maxdy = maxsize - size;
            for (int dx = 0; dx <= maxdx; dx++)
            {
                for (int dy = 0; dy <= maxdy; dy++)
                {
                    localshape.GetSubShape(subshape, dx, dy);
                    int subshapeindex = subshape.GetShapeIndex();
                    int subanchorindex = subfeatures->GetAnchorIndex(
                        x + dx, y + dy);
                    int subfeatureindex = subfeatures->EncodeIndex(
                        subshapeindex, subanchorindex);
                    int globalindex = m_localShapeSet->GetFeatureIndex(
                            setindex, subfeatureindex);
                    m_fuseTable[featureindex].push_back(globalindex);
                }
            }
        }
    }
}

void RlLocalShapeFusion::Convert(const RlWeightSet* allweights,
    RlWeightSet* fusedweights) const
{
    if (m_fuseTable.empty())
        throw SgException("Trying to fuse weights without making fuse table");

    for (int featureindex = 0; 
        featureindex < fusedweights->GetNumFeatures(); 
        featureindex++)
    {
        RlWeight& w = fusedweights->Get(featureindex);
        w.Weight() = 0;
        for (vector<int>::const_iterator i_index = 
                m_fuseTable[featureindex].begin();
            i_index != m_fuseTable[featureindex].end(); 
            i_index++)
        {
            int index = *i_index;
            w.Weight() += allweights->Get(index).Weight();
        }
    }
}

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlLocalShapeUnshare);

RlLocalShapeUnshare::RlLocalShapeUnshare(GoBoard& board, 
    RlLocalShapeSet* sharedshapeset,
    RlLocalShapeSet* unsharedshapeset)
:   RlConvert(board),
    m_sharedShapeSet(sharedshapeset),
    m_unsharedShapeSet(unsharedshapeset)
{
}

void RlLocalShapeUnshare::LoadSettings(istream& settings)
{
    settings >> RlSetting<RlLocalShapeSet*>
        ("SharedShapeSet", m_sharedShapeSet);
    settings >> RlSetting<RlLocalShapeSet*>
        ("UnsharedShapeSet", m_unsharedShapeSet);
}

void RlLocalShapeUnshare::Initialise()
{
    if (!m_sharedShapeSet || !m_unsharedShapeSet)
        throw SgException
            ("Unsharing requires unshared shape set and shared shape set");

    m_sharedShapeSet->EnsureInitialised();
    m_unsharedShapeSet->EnsureInitialised();

    if (m_sharedShapeSet->GetNumShapeSets() != 
        m_unsharedShapeSet->GetNumShapeSets())
        throw SgException("Shared and unshared shape sets must match");
    if (m_unsharedShapeSet->GetShareTypes() != (1 << eNone))
        throw SgException("Unshared shape set using weight sharing");
}

void RlLocalShapeUnshare::Convert(const RlWeightSet* sharedweights,
    RlWeightSet* unsharedweights) const
{
    // Convert shared weights to unshared weights
    unsharedweights->ZeroWeights();
    int numsharedsets = m_sharedShapeSet->GetNumShapeSets();
    int numunsharedsets = m_unsharedShapeSet->GetNumShapeSets();
    for (int unsharedset = 0; unsharedset < numunsharedsets; ++unsharedset)
    {
        for (int sharedset = 0; sharedset < numsharedsets; ++sharedset)
        {
            if (m_sharedShapeSet->GetShapes(sharedset)->GetXSize()
                != m_unsharedShapeSet->GetShapes(unsharedset)->GetXSize()
                || m_sharedShapeSet->GetShapes(sharedset)->GetYSize()
                != m_unsharedShapeSet->GetShapes(unsharedset)->GetYSize())
                continue;
            
            for (int localunsharedindex = 0; 
                localunsharedindex < m_sharedShapeSet->GetShapes(sharedset)
                    ->GetNumFeatures(); 
                ++localunsharedindex)
            {
                int localsharedindex = m_sharedShapeSet->GetShare(sharedset)
                    ->GetOutputFeature(localunsharedindex);
                int sign = m_sharedShapeSet->GetShare(sharedset)
                    ->GetSign(localunsharedindex);
                if (localsharedindex == -1 || sign == 0)
                    continue;
                int globalunsharedindex = m_unsharedShapeSet->GetFeatureIndex(
                    unsharedset, localunsharedindex);
                int globalsharedindex = m_sharedShapeSet->GetFeatureIndex(
                    sharedset, localsharedindex);
                unsharedweights->Get(globalunsharedindex).Weight() 
                    += sharedweights->Get(globalsharedindex).Weight() * sign;
            }
        }
    }
}

//----------------------------------------------------------------------------

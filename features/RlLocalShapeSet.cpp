//----------------------------------------------------------------------------
/** @file RlLocalShapeSet.cpp
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlLocalShapeSet.h"

#include "RlLocalShapeFeatures.h"
#include "RlLocalShapeShare.h"
#include "RlShapeUtil.h"
#include "RlWeightSet.h"

#include <boost/filesystem/fstream.hpp>

using namespace std;
using namespace RlShapeUtil;

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlLocalShapeSet);

RlLocalShapeSet::RlLocalShapeSet(GoBoard& board)
:   RlSumFeatures(board),
    m_minSize(1),
    m_maxSize(3),
    m_symmetry(false),
    m_ignoreEmpty(true),
    m_ignoreSelfInverse(true)
{
}

RlLocalShapeSet::~RlLocalShapeSet()
{
    for (vector<ShapeSet>::iterator i_shapeSet = m_shapeSets.begin();
        i_shapeSet != m_shapeSets.end(); ++i_shapeSet)
    {
        delete i_shapeSet->m_shapes;
        for (vector<RlLocalShapeShare*>::iterator i_shares = i_shapeSet->m_shares.begin();
            i_shares != i_shapeSet->m_shares.end(); ++i_shares)
            delete *i_shares;
    }    
}

void RlLocalShapeSet::LoadSettings(istream& settings)
{
    int version;
    settings >> RlVersion(version, 14, 14);
    
    string shapespec;
    vector<string> sharetypes;
    settings >> RlSetting<string>("ShapeSpec", shapespec);
    settings >> RlSetting<int>("MinSize", m_minSize);
    settings >> RlSetting<int>("MaxSize", m_maxSize);
    settings >> RlSetting<bool>("Symmetry", m_symmetry);
    settings >> RlSetting< vector<string> >("ShareTypes", sharetypes);
    settings >> RlSetting<bool>("IgnoreEmpty", m_ignoreEmpty);
    settings >> RlSetting<bool>("IgnoreSelfInverse", m_ignoreSelfInverse);
    
    int sharetype = GetShareType(sharetypes);
    AddAnyShapes(shapespec, sharetype);
}

int RlLocalShapeSet::GetShareType(const std::vector<std::string>& types)
{
    int sharetype = 0;
    for (vector<string>::const_iterator i_types = types.begin(); 
        i_types != types.end(); ++ i_types)
    {
        string typestring = *i_types;
        int type = GetSymmetryType(typestring);
        sharetype = sharetype | (1 << type);
    }
    
    return sharetype;
}

void RlLocalShapeSet::AddAnyShapes(const string& shapespec, int sharetypes)
{
    m_shapeSpec = GetShapeSpec(shapespec);
    switch (m_shapeSpec)
    {
        case eAll:
            AddAllShapes(sharetypes);
            break;
        case eSquare:
            AddSquareShapes(sharetypes);
            break;
        case eRect:
            AddRectShapes(1, sharetypes);
            break;
        default:
            SG_ASSERT(false);
    }
}

void RlLocalShapeSet::AddAllShapes(int sharetypes)
{
    for (int xsize = m_minSize; xsize <= m_maxSize; ++xsize)
        for (int ysize = m_minSize; ysize <= m_maxSize; ++ysize)
            AddShapes(xsize, ysize, sharetypes);
}

void RlLocalShapeSet::AddSquareShapes(int sharetypes)
{
    for (int size = m_minSize; size <= m_maxSize; ++size)
        AddShapes(size, size, sharetypes);
}

void RlLocalShapeSet::AddRectShapes(int maxoffset, int sharetypes)
{
    for (int xsize = m_minSize; xsize <= m_maxSize; ++xsize)
        for (int ysize = m_minSize; ysize <= m_maxSize; ++ysize)
            if (abs(xsize - ysize) <= maxoffset)
                AddShapes(xsize, ysize, sharetypes);
}

void RlLocalShapeSet::AddShapes(int xsize, int ysize, int sharetypes)
{
    RlLocalShapeFeatures* shapes = 
        new RlLocalShapeFeatures(m_board, xsize, ysize);
    ShapeSet shapeset(shapes);
        
    if (sharetypes & (1 << eNone))
        AddFeatureSet(shapes);
    if (sharetypes & (1 << eNLI))
        AddShares(new RlLIFeatureShare(
            m_board, shapes, false), shapeset);
    if (sharetypes & (1 << eNLD))
        AddShares(new RlLDFeatureShare(
            m_board, shapes, false), shapeset);
    if (sharetypes & (1 << eLI))
        AddShares(new RlLIFeatureShare(
            m_board, shapes, true), shapeset);
    if (sharetypes & (1 << eLD))
        AddShares(new RlLDFeatureShare(
            m_board, shapes, true), shapeset);
    if (sharetypes & (1 << eCI))
        AddShares(new RlCIFeatureShare(
            m_board, shapes), shapeset);

    m_shapeSets.push_back(shapeset);
}

void RlLocalShapeSet::AddShares(RlLocalShapeShare* shares,
    ShapeSet& shapeset)
{
    AddFeatureSet(shares);
    shares->IgnoreEmpty(m_ignoreEmpty);
    shares->IgnoreSelfInverse(m_ignoreSelfInverse);
    shapeset.m_shares.push_back(shares);
}

void RlLocalShapeSet::SaveUnsharedWeights(const RlWeightSet& sharedweights,
    const bfs::path& filename)
{
    // Create unshared features corresponding to shared features
    RlLocalShapeSet unsharedfeatureset(m_board);
    for (int shapeset = 0; shapeset < ssize(m_shapeSets); ++shapeset)
        unsharedfeatureset.AddFeatureSet(
            new RlLocalShapeFeatures(
                m_board,
                m_shapeSets[shapeset].m_shapes->GetXSize(), 
                m_shapeSets[shapeset].m_shapes->GetYSize()));
    unsharedfeatureset.EnsureInitialised();

    // Create weight set for unshared features
    RlWeightSet unsharedweights(m_board, &unsharedfeatureset);
    unsharedweights.ZeroWeights();
    unsharedweights.EnsureInitialised();
    
    // Convert shared weights to unshared weights
    int sharecount = 0;
    for (int shapeset = 0; shapeset < ssize(m_shapeSets); ++shapeset)
    {
        for (int sh = 0; sh < ssize(m_shapeSets[shapeset].m_shares); sh++)
        {
            for (int localunsharedindex = 0; 
                localunsharedindex < m_shapeSets[shapeset].m_shapes->GetNumFeatures(); 
                ++localunsharedindex)
            {
                int localsharedindex = m_shapeSets[shapeset].m_shares[sh]
                    ->GetOutputFeature(localunsharedindex);
                int sign = m_shapeSets[shapeset].m_shares[sh]
                    ->GetSign(localunsharedindex);
                if (localsharedindex == -1 || sign == 0)
                    continue;
                int globalunsharedindex = unsharedfeatureset.GetFeatureIndex(
                    shapeset, localunsharedindex);
                int globalsharedindex = GetFeatureIndex(
                    sharecount, localsharedindex);
                unsharedweights.Get(globalunsharedindex).Weight() 
                    += sharedweights.Get(globalsharedindex).Weight() * sign;
            }
            sharecount++;
        }
    }

    // Save data
    bfs::ofstream weightfile(filename);
    unsharedweights.Save(weightfile);
}

//----------------------------------------------------------------------------

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

using namespace std;
using namespace RlShapeUtil;

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlLocalShapeSet);

RlLocalShapeSet::RlLocalShapeSet(GoBoard& board, int minsize, int maxsize,
    int shapespec, int sharetypes)
:   RlSumFeatures(board),
    m_shapeSpec(shapespec),
    m_shareTypes(sharetypes),
    m_minSize(minsize),
    m_maxSize(maxsize),
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
    settings >> RlSetting< vector<string> >("ShareTypes", sharetypes);
    settings >> RlSetting<bool>("IgnoreEmpty", m_ignoreEmpty);
    settings >> RlSetting<bool>("IgnoreSelfInverse", m_ignoreSelfInverse);
    
    m_shapeSpec = RlShapeUtil::GetShapeSpec(shapespec);
    m_shareTypes = ReadShareTypes(sharetypes);
}

void RlLocalShapeSet::Initialise()
{
    AddAnyShapes(m_shapeSpec, m_shareTypes);
    RlSumFeatures::Initialise();
}

int RlLocalShapeSet::ReadShareTypes(const std::vector<std::string>& types)
{
    int sharetypes = 0;
    for (vector<string>::const_iterator i_types = types.begin(); 
        i_types != types.end(); ++ i_types)
    {
        string typestring = *i_types;
        int type = GetSymmetryType(typestring);
        sharetypes = sharetypes | (1 << type);
    }
    
    return sharetypes;
}

void RlLocalShapeSet::AddAnyShapes(int shapespec, int sharetypes)
{
    switch (shapespec)
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

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
/** @file RlLocalShapeShare.cpp
    See RlLocalShapeShare.h
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlLocalShapeShare.h"

#include "RlLocalShapeFeatures.h"

using namespace std;
using namespace RlShapeUtil;

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlLocalShapeShare);

RlLocalShapeShare::RlLocalShapeShare(GoBoard& board,
    RlLocalShapeFeatures* localshapes, bool invert)
:   RlSharedFeatures(board, localshapes),
    m_localShapes(localshapes),
    m_invert(invert),
    m_ignoreEmpty(true)
{
}

void RlLocalShapeShare::LoadSettings(istream& settings)
{
    RlSharedFeatures::LoadSettings(settings);

    settings >> RlSetting<bool>("Invert", m_invert);
    settings >> RlSetting<bool>("IgnoreEmpty", m_ignoreEmpty);
}

void RlLocalShapeShare::Initialise()
{
    m_localShapes = dynamic_cast<RlLocalShapeFeatures*>(FeatureSet());
    m_localShapes->EnsureInitialised();

    m_flipX = true;
    m_flipY = true;
    m_transpose = m_localShapes->Square(); // Square shapes only

    RlSharedFeatures::Initialise();
}

bool RlLocalShapeShare::IgnoreFeature(int featureindex) const
{
    return m_ignoreEmpty && m_localShapes->IsEmpty(featureindex);
}

void RlLocalShapeShare::DescribeSet(ostream& name) const
{    
    m_localShapes->DescribeSet(name);
    name << "-None";
    if (m_ignoreEmpty)
        name << "-IE";
}

void RlLocalShapeShare::GetPage(int pagenum, vector<int>& indices, 
    ostream& pagename) const
{
    if (m_ignoreEmpty) // skip over page 0 (empty features)
        RlSharedFeatures::GetPage(pagenum + 1, indices, pagename);
    else
        RlSharedFeatures::GetPage(pagenum, indices, pagename);
}

int RlLocalShapeShare::GetNumPages() const
{
    if (m_ignoreEmpty)
        return RlSharedFeatures::GetNumPages() - 1;
    else
        return RlSharedFeatures::GetNumPages();    
}

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlLDFeatureShare);

void RlLDFeatureShare::GetEquivalent(int featureindex, 
    vector<int>& equivalent,
    vector<int>& signs) const
{
    for (int flipx = 0; flipx <= m_flipX; ++flipx)
    {
        for (int flipy = 0; flipy <= m_flipY; ++flipy)
        {
            for (int transpose = 0; transpose <= m_transpose; ++transpose)
            {
                for (int invert = 0; invert <= m_invert; ++invert)
                {
                    int eindex = m_localShapes->Transform(
                        featureindex, flipx, flipy, transpose);
                    if (invert)
                        eindex = m_localShapes->Invert(eindex);
                    equivalent.push_back(eindex);
                    signs.push_back(invert ? -1 : +1);
                }
            }
        }
    }
}

void RlLDFeatureShare::DescribeSet(ostream& name) const
{    
    m_localShapes->DescribeSet(name);
    name << "-LD";
}

void RlLDFeatureShare::DescribeTex(
    int featureindex, ostream& tex, bool invert) const
{
    m_localShapes->SetDisplayMode(eLD);
    RlSharedFeatures::DescribeTex(featureindex, tex, invert);
}

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlLIFeatureShare);

void RlLIFeatureShare::GetEquivalent(int featureindex, 
    vector<int>& equivalent,
    vector<int>& signs) const
{
    int eindex;
    for (int flipx = 0; flipx <= m_flipX; ++flipx)
    {
        for (int flipy = 0; flipy <= m_flipY; ++flipy)
        {
            for (int transpose = 0; transpose <= m_transpose; ++transpose)
            {
                for (int invert = 0; invert <= m_invert; ++invert)
                {
                    eindex = m_localShapes->Transform(
                        featureindex, flipx, flipy, transpose);
                    eindex = m_localShapes->Translate(eindex, 0, 0);
                    if (invert)
                        eindex = m_localShapes->Invert(eindex);
                    equivalent.push_back(eindex);
                    signs.push_back(invert ? -1 : +1);
                }
            }
        }
    }
}

void RlLIFeatureShare::DescribeSet(ostream& name) const
{
    m_localShapes->DescribeSet(name);
    name << "-LI";
}

void RlLIFeatureShare::DescribeTex(
    int featureindex, ostream& tex, bool invert) const
{
    m_localShapes->SetDisplayMode(eLI);
    RlSharedFeatures::DescribeTex(featureindex, tex, invert);
}

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlCIFeatureShare);

void RlCIFeatureShare::GetEquivalent(int featureindex, 
    vector<int>& equivalent,
    vector<int>& signs) const
{
    equivalent.push_back(featureindex);
    signs.push_back(+1);
    equivalent.push_back(m_localShapes->Invert(featureindex));
    signs.push_back(-1);
}

void RlCIFeatureShare::DescribeSet(ostream& name) const
{
    m_localShapes->DescribeSet(name);
    name << "-CI";
}

void RlCIFeatureShare::DescribeTex(
    int featureindex, ostream& tex, bool invert) const
{
    m_localShapes->SetDisplayMode(eNone);
    RlSharedFeatures::DescribeTex(featureindex, tex, invert);
}

//----------------------------------------------------------------------------

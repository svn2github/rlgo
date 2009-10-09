//----------------------------------------------------------------------------
/** @file RlBinaryFeatures.cpp
    @see RlBinaryFeatures.h
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlBinaryFeatures.h"

#include "RlTex.h"
#include "RlWeightSet.h"
#include "RlUtils.h"
#include <sstream>

using namespace std;

//----------------------------------------------------------------------------
// Binary feature set

RlBinaryFeatures::RlBinaryFeatures(GoBoard& board)
:   RlAutoObject(board)
{
}

void RlBinaryFeatures::SaveData(ostream& data)
{
    SG_UNUSED(data);
}

void RlBinaryFeatures::LoadData(istream& data)
{
    SG_UNUSED(data);
}

int RlBinaryFeatures::GetFeature(const string& desc) const
{
    istringstream iss;
    iss.str(desc);
    return ReadFeature(iss);
}

void RlBinaryFeatures::DisplayFeature(
    int featureindex, ostream& cmd) const
{
    SG_UNUSED(featureindex);
    SG_UNUSED(cmd);
}

void RlBinaryFeatures::DescribeTex(int featureindex, ostream& tex,
    bool invert) const
{
    // By default display the standard textual format
    SG_UNUSED(invert);
    DescribeFeature(featureindex, tex);
}

string RlBinaryFeatures::SetName() const
{
    ostringstream oss;
    DescribeSet(oss);
    return oss.str();
}

SgPoint RlBinaryFeatures::GetPosition(int featureindex) const
{
    SG_UNUSED(featureindex);
    return SG_NULLPOINT;
}

void RlBinaryFeatures::GetPage(int pagenum, vector<int>& indices, 
    ostream& pagename) const
{
    SG_UNUSED(pagenum);
    for (int j = 0; j < m_board.Size(); ++j)
        for (int i = 0; i < m_board.Size(); ++i)
            indices.push_back(-1);
    pagename << "NullPage";
}

int RlBinaryFeatures::GetNumPages() const
{
    return 0;
}

bool RlBinaryFeatures::Touches(int featureindex, SgPoint point) const
{
    SG_UNUSED(featureindex);
    SG_UNUSED(point);
    return false;
}

void RlBinaryFeatures::CollectPoints(int featureindex, 
    vector<SgPoint>& points) const
{
    SG_UNUSED(featureindex);
    SG_UNUSED(points);
}

void RlBinaryFeatures::TopTex(ostream& tex, 
    const RlWeightSet* wset, int rows, int cols) const
{
    RlTexTable textable(tex, cols);
    textable.StartTable(false);
    textable.TopFeatures(wset, this, rows, false);
    textable.EndTable();
}

//----------------------------------------------------------------------------


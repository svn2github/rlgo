//----------------------------------------------------------------------------
/** @file RlSumFeatures.cpp
    See RlSumFeatures.h
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlSumFeatures.h"
#include "RlTex.h"

using namespace std;

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlSumFeatures);

RlSumFeatures::RlSumFeatures(GoBoard& board) 
:   RlCompoundFeatures(board),
    m_totalFeatures(0)
{
}

void RlSumFeatures::LoadSettings(istream& settings)
{
    settings >> RlSetting<vector<RlBinaryFeatures*> >(
        "FeatureSets", m_featureSets);
}

void RlSumFeatures::Initialise()
{
    RlCompoundFeatures::Initialise();
    m_totalFeatures = 0;
    m_offset.clear();
    for (int i = 0; i < ssize(m_featureSets); ++i)
    {
        m_offset.push_back(m_totalFeatures);
        m_totalFeatures += m_featureSets[i]->GetNumFeatures();
    }
}

RlTracker* RlSumFeatures::CreateTracker(
    map<RlBinaryFeatures*, RlTracker*>& trackermap)
{
    CreateChildTrackers(trackermap);
    RlSumTracker* sumtracker = new RlSumTracker(m_board, this);
    for (int i = 0; i < ssize(m_featureSets); ++i)
        sumtracker->AddTracker(trackermap[m_featureSets[i]]);
    return sumtracker;
}

int RlSumFeatures::GetNumFeatures() const
{
    return m_totalFeatures;
}

int RlSumFeatures::ReadFeature(std::istream& desc) const
{
    string setname, featurename;
    desc >> setname >> ws;

    for (int i = 0; i < ssize(m_featureSets); ++i)
    {
        ostringstream oss;
        m_featureSets[i]->DescribeSet(oss);
        if (oss.str() == setname)
            return m_featureSets[i]->ReadFeature(desc) + m_offset[i];
    }

    return -1;
}

void RlSumFeatures::DescribeFeature(int featureindex, ostream& str) const
{
    int set = GetFeatureSet(featureindex);
    int newindex = featureindex - m_offset[set];
    m_featureSets[set]->DescribeFeature(newindex, str);
}

void RlSumFeatures::DescribeTex(int featureindex, ostream& tex, 
    bool invert) const
{
    int set = GetFeatureSet(featureindex);
    int newindex = featureindex - m_offset[set];
    m_featureSets[set]->DescribeTex(newindex, tex, invert);
}

void RlSumFeatures::DisplayFeature(int featureindex, ostream& cmd) const
{
    int set = GetFeatureSet(featureindex);
    int newindex = featureindex - m_offset[set];
    m_featureSets[set]->DisplayFeature(newindex, cmd);
}

void RlSumFeatures::DescribeSet(std::ostream& str) const
{
    str << "(";
    for (int i = 0; i < ssize(m_featureSets); ++i)
    {
        if (i > 0)
            str << "+";
        m_featureSets[i]->DescribeSet(str);
    }
    str << ")";
}

void RlSumFeatures::GetPage(int pagenum, vector<int>& indices, 
    ostream& pagename) const
{
    SG_ASSERT(pagenum >= 0 && pagenum < GetNumPages());
    int localpagenum = 0, localset = 0, numpages = 0;
    for (int i = 0; i < ssize(m_featureSets); ++i)
    {
        localset = i;
        localpagenum = pagenum - numpages;
        numpages += m_featureSets[i]->GetNumPages();
        if (numpages > pagenum)
            break;
    }
    SG_ASSERT(localset < ssize(m_featureSets));
    m_featureSets[localset]->GetPage(localpagenum, indices, pagename);
    for (int i = 0; i < m_board.Size() * m_board.Size(); ++i)
        if (indices[i] != -1)
            indices[i] += m_offset[localset];
}

int RlSumFeatures::GetNumPages() const
{
    int numpages = 0;
    for (int i = 0; i < ssize(m_featureSets); ++i)
        numpages += m_featureSets[i]->GetNumPages();
    return numpages;
}

int RlSumFeatures::GetFeatureSet(int featureindex) const
{
    for (int i = 1; i < ssize(m_featureSets); ++i)
        if (featureindex < m_offset[i])
            return i - 1;
    return ssize(m_featureSets) - 1;
}

int RlSumFeatures::GetFeatureIndex(int set, int localindex) const
{ 
    SG_ASSERT(set >= 0 && set < ssize(m_featureSets));
    return m_offset[set] + localindex; 
}

SgPoint RlSumFeatures::GetPosition(int featureindex) const
{
    int set = GetFeatureSet(featureindex);
    int newindex = featureindex - m_offset[set];
    return m_featureSets[set]->GetPosition(newindex);
}

void RlSumFeatures::CollectPoints(int featureindex, 
    vector<SgPoint>& points) const
{
    int set = GetFeatureSet(featureindex);
    int newindex = featureindex - m_offset[set];
    m_featureSets[set]->CollectPoints(newindex, points);
}

bool RlSumFeatures::Touches(int featureindex, SgPoint pt) const
{
    int set = GetFeatureSet(featureindex);
    int newindex = featureindex - m_offset[set];
    return m_featureSets[set]->Touches(newindex, pt);
}

void RlSumFeatures::TopTex(ostream& tex, 
    const RlWeightSet* wset, int rows, int cols) const
{
    RlTexTable textable(tex, cols);
    textable.StartTable(true);
    for (int i = 1; i < ssize(m_featureSets); ++i)
    {
        if (i > 0)
            textable.Line();
        textable.TopFeatures(wset, m_featureSets[i], 
            rows, true, GetFeatureIndex(i, 0));
    }
    textable.EndTable();
}

//----------------------------------------------------------------------------

RlSumTracker::RlSumTracker(GoBoard& board, const RlSumFeatures* features)
:   RlCompoundTracker(board),
    m_sumFeatures(features),
    m_totalSlots(0)
{
}

void RlSumTracker::AddTracker(RlTracker* tracker)
{
    RlCompoundTracker::AddTracker(tracker);
    m_slotOffset.push_back(m_totalSlots);
    m_totalSlots += tracker->GetActiveSize();
}

void RlSumTracker::SumChanges()
{
    for (int i = 0; i < ssize(m_trackers); ++i)
    {
        for (RlChangeList::Iterator i_changes(m_trackers[i]->ChangeList());
            i_changes; ++i_changes)
        {
            NewChange(
                i_changes->m_slot + m_slotOffset[i],
                i_changes->m_featureIndex + m_sumFeatures->m_offset[i],
                i_changes->m_occurrences);
        }
    }
}

void RlSumTracker::PropagateChanges()
{
    RlCompoundTracker::PropagateChanges();
    SumChanges();
}

int RlSumTracker::GetActiveSize() const
{
    return m_totalSlots;
}

//----------------------------------------------------------------------------

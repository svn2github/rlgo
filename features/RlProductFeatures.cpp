//----------------------------------------------------------------------------
/** @file RlProductFeatures.cpp
    See RlProductFeatures.h
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlProductFeatures.h"

using namespace std;

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlProductFeatures);

RlProductFeatures::RlProductFeatures(GoBoard& board, 
    RlBinaryFeatures* set1, RlBinaryFeatures* set2)
:   RlCompoundFeatures(board),
    m_set1(set1),
    m_set2(set2)
{
}

void RlProductFeatures::LoadSettings(istream& settings)
{
    settings >> RlSetting<RlBinaryFeatures*>("Set1", m_set1);
    settings >> RlSetting<RlBinaryFeatures*>("Set2", m_set2);
}

void RlProductFeatures::Initialise()
{
    AddFeatureSet(m_set1);
    AddFeatureSet(m_set2);
    RlCompoundFeatures::Initialise();
}

RlTracker* RlProductFeatures::CreateTracker(
    map<RlBinaryFeatures*, RlTracker*>& trackermap)
{
    CreateChildTrackers(trackermap);
    return new RlProductTracker(m_board, this,
        trackermap[m_set1], trackermap[m_set2]);
}

int RlProductFeatures::GetNumFeatures() const
{
    return m_set1->GetNumFeatures() * m_set2->GetNumFeatures();
}

int RlProductFeatures::ReadFeature(std::istream& desc) const
{
    int index1 = m_set1->ReadFeature(desc);
    char c;
    desc >> c;
    SG_ASSERT(c == '*');
    int index2 = m_set2->ReadFeature(desc);
    return GetFeatureIndex(index1, index2);
}

void RlProductFeatures::DescribeFeature(int featureindex, ostream& str) const
{
    int index1 = featureindex % m_set1->GetNumFeatures();
    int index2 = featureindex / m_set1->GetNumFeatures();
    m_set1->DescribeFeature(index1, str);
    str << "*";
    m_set2->DescribeFeature(index2, str);
}

void RlProductFeatures::DescribeTex(int featureindex, 
    ostream& tex, bool invert) const
{
    int index1 = featureindex % m_set1->GetNumFeatures();
    int index2 = featureindex / m_set1->GetNumFeatures();
    m_set1->DescribeTex(index1, tex, invert);
    tex << "*";
    m_set2->DescribeTex(index2, tex, invert);
}

void RlProductFeatures::DisplayFeature(int featureindex, ostream& cmd) const
{
    int index1 = featureindex % m_set1->GetNumFeatures();
    int index2 = featureindex / m_set1->GetNumFeatures();
    m_set1->DisplayFeature(index1, cmd);
    m_set2->DisplayFeature(index2, cmd);
}

void RlProductFeatures::DescribeSet(std::ostream& str) const
{
    m_set1->DescribeSet(str);
    str << "*";
    m_set2->DescribeSet(str);
}

void RlProductFeatures::GetPage(int pagenum, vector<int>& indices, 
    ostream& pagename) const
{
    SG_ASSERT(pagenum >= 0 && pagenum < GetNumPages());
    int index2 = pagenum / m_set1->GetNumPages();
    int page1 = pagenum % m_set1->GetNumPages();
    m_set2->DescribeFeature(index2, pagename);
    pagename << "*";
    m_set1->GetPage(page1, indices, pagename);
    for (int i = 0; i < m_board.Size() * m_board.Size(); ++i)
        indices[i] = GetFeatureIndex(indices[i], index2);
}

int RlProductFeatures::GetNumPages() const
{
    // Each feature in set 2 has its own pages from set 1.
    return m_set2->GetNumFeatures() * m_set1->GetNumPages();
}

inline int RlProductFeatures::GetFeatureIndex(int index1, int index2) const
{
    return index2 * m_set1->GetNumFeatures() + index1;
}

SgPoint RlProductFeatures::GetPosition(int featureindex) const
{
    //@todo: better definition of get position
    // For now use position of primary feature set
    return m_set1->GetPosition(featureindex);
}

bool RlProductFeatures::Touches(int featureindex, SgPoint pt) const
{
    int index1 = featureindex % m_set1->GetNumFeatures();
    int index2 = featureindex / m_set1->GetNumFeatures();
    return m_set1->Touches(index1, pt)
        || m_set2->Touches(index2, pt);
}

void RlProductFeatures::CollectPoints(int featureindex, 
    vector<SgPoint>& points) const
{
    int index1 = featureindex % m_set1->GetNumFeatures();
    int index2 = featureindex / m_set1->GetNumFeatures();
    m_set1->CollectPoints(index1, points);
    m_set2->CollectPoints(index2, points);
}

//----------------------------------------------------------------------------

RlProductTracker::RlProductTracker(GoBoard& board, RlProductFeatures* features,
    RlTracker* tracker1, RlTracker* tracker2)
:   RlCompoundTracker(board),
    m_productFeatures(features),
    m_tracker1(tracker1),
    m_tracker2(tracker2)
{
    AddTracker(m_tracker1);
    AddTracker(m_tracker2);
}

void RlProductTracker::JoinChanges()
{
    // For Cartesian products of sets A, B and changes dA, dB:
    //     (A+dA) * (B+dB) = A*B + A*dB + B*dA + dA*dB
    //              d(A*B) = A*dB + B*dA + dA*dB
    
    for (RlActiveSet::Iterator i_active1(m_tracker1->Active()); 
        i_active1; ++i_active1)
    {
        for (RlChangeList::Iterator i_change2(m_tracker2->ChangeList()); 
            i_change2; ++i_change2)
        {
            NewChange(
                GetSlotIndex(i_active1.Slot(), i_change2->m_slot),
                m_productFeatures->GetFeatureIndex(
                    i_active1->m_featureIndex, 
                    i_change2->m_featureIndex),
                i_active1->m_occurrences * i_change2->m_occurrences);
        }
    }
    
    for (RlChangeList::Iterator i_change1(m_tracker1->ChangeList()); 
        i_change1; ++i_change1)
    {
        for (RlActiveSet::Iterator i_active2(m_tracker2->Active()); 
            i_active2; ++i_active2)
        {
            NewChange(
                GetSlotIndex(i_change1->m_slot, i_active2.Slot()),
                m_productFeatures->GetFeatureIndex(
                    i_change1->m_featureIndex,
                    i_active2->m_featureIndex), 
                i_change1->m_occurrences * i_active2->m_occurrences);
        }
    }
    
    for (RlChangeList::Iterator i_change1(m_tracker1->ChangeList()); 
        i_change1; ++i_change1)
    {
        for (RlChangeList::Iterator i_change2(m_tracker2->ChangeList()); 
            i_change2; ++i_change2)
        {
            NewChange(
                GetSlotIndex(i_change1->m_slot, i_change2->m_slot),
                m_productFeatures->GetFeatureIndex(
                    i_change1->m_featureIndex,
                    i_change2->m_featureIndex), 
                i_change1->m_occurrences * i_change2->m_occurrences);
        }
    }
}

void RlProductTracker::PropagateChanges()
{
    RlCompoundTracker::PropagateChanges();
    JoinChanges();
}

int RlProductTracker::GetActiveSize() const
{
    return m_tracker1->GetActiveSize() * m_tracker2->GetActiveSize();
}

inline int RlProductTracker::GetSlotIndex(int slot1, int slot2)
{
    return slot2 * m_tracker1->GetActiveSize() + slot1;
}

//----------------------------------------------------------------------------

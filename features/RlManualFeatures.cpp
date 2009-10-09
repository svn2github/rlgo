//----------------------------------------------------------------------------
/** @file RlManualFeatures.cpp
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlManualFeatures.h"

using namespace std;

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlManualFeatureSet);

RlManualFeatureSet::RlManualFeatureSet(GoBoard& board, int numfeatures)
:   RlBinaryFeatures(board),
    m_numFeatures(numfeatures)
{
}

void RlManualFeatureSet::LoadSettings(istream& settings)
{
    int version;
    settings >> RlVersion(version, 0, 0);
    settings >> RlSetting<int>("NumFeatures", m_numFeatures);
}   

void RlManualFeatureSet::Initialise()
{
    SG_ASSERT(m_numFeatures);
    for (int i = 0; i < m_numFeatures; ++i)
        m_active.push_back(0);
    RlBinaryFeatures::Initialise();
}

RlTracker* RlManualFeatureSet::CreateTracker(
    map<RlBinaryFeatures*, RlTracker*>& trackermap)
{
    SG_UNUSED(trackermap);
    SG_ASSERT(IsInitialised());
    return new RlManualTracker(m_board, this);
}

void RlManualFeatureSet::Clear()
{
    for (int i = 0; i < m_numFeatures; ++i)
        m_active[i] = 0;
}

int RlManualFeatureSet::GetNumFeatures() const
{
    return m_numFeatures;
}

int RlManualFeatureSet::ReadFeature(istream& desc) const
{    
    int featureindex;
    desc >> featureindex;
    return featureindex;
}

void RlManualFeatureSet::DescribeFeature(int featureindex, ostream& str) const
{
    str << featureindex;
}

void RlManualFeatureSet::DescribeSet(ostream& name) const
{
    name << "Manual";
}

//----------------------------------------------------------------------------

RlManualTracker::RlManualTracker(GoBoard& board, RlManualFeatureSet* manual)
:   RlTracker(board),
    m_manual(manual)
{ 
}

void RlManualTracker::Reset()
{
    RlTracker::Reset();
    for (int f = 0; f < m_manual->m_numFeatures; ++f)
        if (m_manual->m_active[f])
            NewChange(f, f, m_manual->m_active[f]);
}

void RlManualTracker::Execute(SgMove move, SgBlackWhite colour, bool execute)
{
    RlTracker::Execute(move, colour, execute);
    Update();
}

void RlManualTracker::Undo()
{
    RlTracker::Undo();
}

int RlManualTracker::GetActiveSize() const
{
    return m_manual->m_numFeatures;
}

void RlManualTracker::Update()
{
    for (int f = 0; f < m_manual->m_numFeatures; ++f)
    {
        int occur = Active().GetOccurrences(f);
        if (occur != m_manual->m_active[f])
            NewChange(f, f, m_manual->m_active[f] - occur);
    }
}

//----------------------------------------------------------------------------


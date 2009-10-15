//----------------------------------------------------------------------------
/** @file RlCompoundFeatures.cpp
    See RlCompoundFeatures.h
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlCompoundFeatures.h"

using namespace std;

//----------------------------------------------------------------------------

RlCompoundFeatures::RlCompoundFeatures(GoBoard& board) 
:   RlBinaryFeatures(board)
{
}

void RlCompoundFeatures::Initialise()
{
    RlBinaryFeatures::Initialise();
    for (int i = 0; i < ssize(m_featureSets); ++i)
        m_featureSets[i]->EnsureInitialised();
}

void RlCompoundFeatures::CreateChildTrackers(
    map<RlBinaryFeatures*, RlTracker*>& trackermap)
{
    for (int i = 0; i < ssize(m_featureSets); ++i)
    {
        RlBinaryFeatures* child = m_featureSets[i];
		if (!trackermap[child])
		{
			RlTracker* childtracker = child->CreateTracker(trackermap);
            trackermap[child] = childtracker;
		}
    }
}

void RlCompoundFeatures::AddFeatureSet(RlBinaryFeatures* features)
{
    m_featureSets.push_back(features);
}

void RlCompoundFeatures::Start()
{
    for (int i = 0; i < ssize(m_featureSets); ++i)
        m_featureSets[i]->Start();
}

void RlCompoundFeatures::End()
{
    for (int i = 0; i < ssize(m_featureSets); ++i)
        m_featureSets[i]->End();
}

void RlCompoundFeatures::Clear()
{
    for (int i = 0; i < ssize(m_featureSets); ++i)
        m_featureSets[i]->Clear();
}

void RlCompoundFeatures::Prune()
{
    for (int i = 0; i < ssize(m_featureSets); ++i)
        m_featureSets[i]->Prune();
}

void RlCompoundFeatures::SaveData(ostream& data)
{
    for (int i = 0; i < ssize(m_featureSets); ++i)
        m_featureSets[i]->SaveData(data);
}

void RlCompoundFeatures::LoadData(istream& data)
{
    for (int i = 0; i < ssize(m_featureSets); ++i)
        m_featureSets[i]->LoadData(data);
}

//----------------------------------------------------------------------------

RlCompoundTracker::RlCompoundTracker(GoBoard& board)
:   RlTracker(board)
{
}

void RlCompoundTracker::AddTracker(RlTracker* tracker) 
{ 
    SG_ASSERT(tracker);
    m_trackers.push_back(tracker); 
}

void RlCompoundTracker::Initialise()
{
    RlTracker::Initialise();
    for (int i = 0; i < ssize(m_trackers); ++i)
        if (m_trackers[i]->Tock() != Tock())
            m_trackers[i]->Initialise();
}

void RlCompoundTracker::Reset()
{
    RlTracker::Reset();
    for (int i = 0; i < ssize(m_trackers); ++i)
        if (m_trackers[i]->Tock() != Tock())
            m_trackers[i]->Reset();
}

void RlCompoundTracker::Execute(SgMove move, SgBlackWhite colour, 
    bool execute, bool store)
{
    RlTracker::Execute(move, colour, execute, store);
    for (int i = 0; i < ssize(m_trackers); ++i)
        if (m_trackers[i]->Tock() != Tock())
            m_trackers[i]->Execute(move, colour, execute, store);
}

void RlCompoundTracker::Undo()
{
    RlTracker::Undo();
    for (int i = 0; i < ssize(m_trackers); ++i)
        if (m_trackers[i]->Tock() != Tock())
            m_trackers[i]->Undo();
}

void RlCompoundTracker::AddChanges(bool store)
{
    RlTracker::AddChanges(store);
    for (int i = 0; i < ssize(m_trackers); ++i)
        if (m_trackers[i]->Tock() != Tock())
            m_trackers[i]->AddChanges(store);
}

void RlCompoundTracker::SubChanges()
{
    RlTracker::SubChanges();
    for (int i = 0; i < ssize(m_trackers); ++i)
        if (m_trackers[i]->Tock() != Tock())
            m_trackers[i]->SubChanges();
}

void RlCompoundTracker::UpdateDirty(SgMove move, SgBlackWhite colour, 
    RlDirtySet& dirty)
{
    RlTracker::UpdateDirty(move, colour, dirty);
    for (int i = 0; i < ssize(m_trackers); ++i)
        if (m_trackers[i]->Tock() != Tock())
            m_trackers[i]->UpdateDirty(move, colour, dirty);
}

void RlCompoundTracker::SetMark()
{
    RlTracker::SetMark();
    for (int i = 0; i < ssize(m_trackers); ++i)
        if (m_trackers[i]->Tock() != Tock())
            m_trackers[i]->SetMark();
}

void RlCompoundTracker::ClearMark()
{
    RlTracker::ClearMark();
    for (int i = 0; i < ssize(m_trackers); ++i)
        if (m_trackers[i]->Tock() != Tock())
            m_trackers[i]->ClearMark();
}

void RlCompoundTracker::ClearChanges()
{
    RlTracker::ClearChanges();
    for (int i = 0; i < ssize(m_trackers); ++i)
        if (m_trackers[i]->Tock() != Tock())
            m_trackers[i]->ClearChanges();
}

void RlCompoundTracker::PropagateChanges()
{
    RlTracker::PropagateChanges();
    for (int i = 0; i < ssize(m_trackers); ++i)
        if (m_trackers[i]->Tock() != Tock())
            m_trackers[i]->PropagateChanges();
}

//----------------------------------------------------------------------------

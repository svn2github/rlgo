//----------------------------------------------------------------------------
/** @file RlStageFeatures.cpp
    See RlStageFeatures.h
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlStageFeatures.h"

using namespace std;

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlStageFeatures);

RlStageFeatures::RlStageFeatures(GoBoard& board, int timescale)
:   RlBinaryFeatures(board),
    m_timeScale(timescale),
    m_relative(false),
    m_maxStep(RL_MAX_TIME)
{
}

void RlStageFeatures::LoadSettings(istream& settings)
{
    settings >> RlSetting<int>("TimeScale", m_timeScale);
    settings >> RlSetting<bool>("Relative", m_relative);
    settings >> RlSetting<int>("MaxStep", m_maxStep);
}

RlTracker* RlStageFeatures::CreateTracker(
    map<RlBinaryFeatures*, RlTracker*>& trackermap)
{
    SG_UNUSED(trackermap);
    SG_ASSERT(IsInitialised());
    return new RlStageTracker(m_board, this);
}

int RlStageFeatures::GetNumFeatures() const
{
    return m_maxStep / m_timeScale + 1;
}

int RlStageFeatures::ReadFeature(std::istream& desc) const
{
    char s[7], d1, d2;
    desc.get(s, 7);
    if (string(s) != "STAGE-")
        throw SgException("Expecting stage feature");
    desc >> d1 >> d2;
    int stage = RlShapeUtil::MakeNumber(d1, d2);
    if (stage < 0 || stage > GetNumFeatures())
        throw SgException("Stage out of range");
    return stage;
}

void RlStageFeatures::DescribeFeature(int featureindex, 
    std::ostream& str) const
{
    str << "STAGE-" << m_timeScale << "-" << featureindex;
}

void RlStageFeatures::DescribeSet(std::ostream& str) const
{
    // Single word description (no whitespace)
    str << "StageFeatures-" << m_timeScale;
}

//----------------------------------------------------------------------------

RlStageTracker::RlStageTracker(GoBoard& board, const RlStageFeatures* features)
:   RlTracker(board),
    m_stageFeatures(features)
{
}

void RlStageTracker::Reset()
{
    RlTracker::Reset();
    if (m_stageFeatures->m_relative)
        m_timeStep = 0;
    else
        m_timeStep = m_board.MoveNumber();
    NewChange(0, m_stageFeatures->GetFeatureIndex(m_timeStep), 1);
}

void RlStageTracker::Execute(SgMove move, SgBlackWhite colour, 
    bool execute, bool store)
{
    RlTracker::Execute(move, colour, execute, store);
    int oldstage = m_stageFeatures->GetFeatureIndex(m_timeStep);
    int newstage = m_stageFeatures->GetFeatureIndex(m_timeStep + 1);
    if (newstage != oldstage)
    {
        NewChange(0, oldstage, -1);
        NewChange(0, newstage, +1);
    }
    if (execute)
        m_timeStep++;
}

void RlStageTracker::Undo()
{
    RlTracker::Undo();
    int oldstage = m_stageFeatures->GetFeatureIndex(m_timeStep);
    int newstage = m_stageFeatures->GetFeatureIndex(m_timeStep - 1);
    if (newstage != oldstage)
    {
        NewChange(0, oldstage, -1);
        NewChange(0, newstage, +1);
    }
    m_timeStep--;
}

int RlStageTracker::GetActiveSize() const
 {
    return 1;
}

//----------------------------------------------------------------------------

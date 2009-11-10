//----------------------------------------------------------------------------
/** @file RlHistory.cpp
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlHistory.h"
#include "RlSetup.h"

using namespace std;

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlHistory);

RlHistory::RlHistory(GoBoard& board, int capacity)
:   RlAutoObject(board),
    m_capacity(capacity),
    m_numEpisodes(0),
    m_cursor(0)
{
}

void RlHistory::LoadSettings(istream& settings)
{
    settings >> RlSetting<int>("Capacity", m_capacity);
}

void RlHistory::Initialise()
{
    SG_ASSERT(m_capacity > 0);
}

void RlHistory::Clear()
{
    for (int i = 0; i < m_capacity; ++i)
        m_history[i].Clear();
    m_numEpisodes = 0;
    m_cursor = m_capacity - 1;
}

void RlHistory::Truncate(int length, int n)
{
    GetEpisode(n).Truncate(length);
}

void RlHistory::Resize(int activesize)
{
    // Only resize when empty
    SG_ASSERT(m_numEpisodes == 0);
    RlDebug(RlSetup::VOCAL) << "Creating history... ";
    m_history.resize(m_capacity);
    for (int i = 0; i < m_capacity; ++i)
        m_history[i].Resize(activesize);
    RlDebug(RlSetup::VOCAL) << "done\n" ;
}

void RlHistory::NewEpisode()
{
    m_cursor = (m_cursor + 1) % m_capacity;
    m_history[m_cursor].Clear();
    if (m_numEpisodes < m_capacity)
        m_numEpisodes++;
}

void RlHistory::TerminateEpisode(RlFloat score)
{
    // Create one terminal state for each colour
    int length = GetLength();
    RlState& laststate = GetState(length - 1);
    laststate.SetTerminal(score);
    AddState(length, SgOppBW(laststate.Colour()));
    GetState(length).SetTerminal(score);

    // After a game ending with two passes, history contains data of this form
    // t=0: (s, a, r) = (Empty board, first move, 0)
    // t=1: (s, a, r) = (Position, move, 0)
    // ...
    // t=T-2: (s, a, r) = (Final position, pass 1, 0)
    // t=T-1: (s, a, r) = (Final position, pass 2, 0)
    // t=T:   (s, a, r) = (Terminal, null, score)
    // t=T+1: (s, a, r) = (Terminal, null, score)
}

RlFloat RlHistory::GetReturn(int n) const
{
    RlFloat totalreward = 0;
    for (int t = 0; t < GetLength(n); ++t)
    {
        totalreward += GetState(t, n).Reward();
        // Only include first terminal state
        if (GetState(t, n).Terminal()) 
            break;
    }
    return totalreward;
}

//----------------------------------------------------------------------------


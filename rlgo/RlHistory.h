//----------------------------------------------------------------------------
/** @file RlHistory.h
    Stored record of learning data from previous games
*/
//----------------------------------------------------------------------------

#ifndef RLHISTORY_H
#define RLHISTORY_H

#include "RlActiveSet.h"
#include "RlFactory.h"
#include "RlState.h"
#include "RlUtils.h"

//----------------------------------------------------------------------------
/** Simple sequence of states for each time-step. 
    Memory is allocated in fixed array of size RL_MAX_TIME, 
    so that pointers to old states will remain valid (no reallocation). */
class RlEpisode
{
public:

    RlEpisode()
    :   m_length(0)
    {
    }
    
    RlState& operator[](int index)
    {
        SG_ASSERT(index < m_length);
        SG_ASSERT(index < RL_MAX_TIME);
        return m_data[index];
    }

    const RlState& operator[](int index) const
    {
        SG_ASSERT(index < m_length);
        SG_ASSERT(index < RL_MAX_TIME);
        return m_data[index];
    }
    
    int Size() const
    {
        return m_length;
    }
        
    void Clear()
    {
        for (int i = 0; i < m_length; ++i)
            m_data[i].Uninitialise();
        m_length = 0;
    }

    void Truncate(int length)
    {
        for (int i = length; i < m_length; ++i)
            m_data[i].Uninitialise();
        m_length = length;
    }
    
    void Resize(int activesize)
    {
        for (int i = 0; i < RL_MAX_TIME; ++i)
            m_data[i].Resize(activesize);
    }

    void AddState(int timestep, SgBlackWhite colour)
    {
        SG_ASSERT(m_length == timestep);
        SG_ASSERT(timestep < RL_MAX_TIME);
        m_data[timestep].Initialise(timestep, colour);
        m_length++;
    }

private:

    int m_length;
    RlState m_data[RL_MAX_TIME];
};

//----------------------------------------------------------------------------
/** Keep track of a set of historic learning data, that can be accessed to 
    find similar items */
class RlHistory : public RlAutoObject
{
public:

    DECLARE_OBJECT(RlHistory);

    RlHistory(GoBoard& board, int capacity = 1);
    
    /** Load from settings */
    virtual void LoadSettings(std::istream& settings);

    /** Initialise history to appropriate capacity */
    virtual void Initialise();

    /** Clear the history without deallocating memory */
    void Clear();

    /** Clear the history after given length, without deallocating memory */
    void Truncate(int length, int n = 0);

    /** Specify size for active sets */
    void Resize(int activesize);
    
    /** Add a new state into the current episode */
    void AddState(int timestep, SgBlackWhite colour);
    
    /** Get a specific item from the history
        n specifies how many games back in the past, 0 means current game. */
    RlState& GetState(int timestep, int n = 0);
    const RlState& GetState(int timestep, int n = 0) const;

    /** Update cursor to new episode, clear states in new episode */
    void NewEpisode();

    /** Terminate episode by adding terminal states containing final score */
    void TerminateEpisode(RlFloat score);

    /** Total number of episodes stored in the history. */
    int GetCapacity() const { return m_capacity; }

    /** Total number of episodes stored since last Clear() */
    int GetNumEpisodes() const { return m_numEpisodes; }

    /** Get length of game, n games into the past */
    int GetLength(int n = 0) const;
    
    /** Get the return (sum of rewards over all timesteps) */
    RlFloat GetReturn(int n = 0) const;
    
protected:

    RlEpisode& GetEpisode(int n);
    const RlEpisode& GetEpisode(int n) const;
    
private:

    std::vector<RlEpisode> m_history;
    int m_capacity;
    int m_numEpisodes;
    int m_cursor;
};

inline RlState& RlHistory::GetState(int timestep, int n) 
{ 
    return GetEpisode(n)[timestep];
}

inline const RlState& RlHistory::GetState(int timestep, int n) const
{ 
    return GetEpisode(n)[timestep];
}

inline int RlHistory::GetLength(int n) const
{
    return GetEpisode(n).Size();
}

inline RlEpisode& RlHistory::GetEpisode(int n)
{ 
    SG_ASSERT(n >= 0 && n < m_capacity);
    int index = (m_cursor - n + m_capacity) % m_capacity;
    return m_history[index];
}

inline const RlEpisode& RlHistory::GetEpisode(int n) const
{ 
    SG_ASSERT(n >= 0 && n < m_capacity);
    int index = (m_cursor - n + m_capacity) % m_capacity;
    return m_history[index];
}

inline void RlHistory::AddState(int timestep, SgBlackWhite colour)
{
    GetEpisode(0).AddState(timestep, colour);
}

//----------------------------------------------------------------------------

#endif // RLHISTORY_H


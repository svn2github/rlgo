//----------------------------------------------------------------------------
/** @file RlMoveFilter.h
    Filters specifying which moves to consider
*/
//----------------------------------------------------------------------------

#ifndef RLMOVEFILTER_H
#define RLMOVEFILTER_H

#include "RlFactory.h"
#include "SgBlackWhite.h"
#include "SgPointSet.h"
#include <vector>
#include <list>

//----------------------------------------------------------------------------
/** Class hierarchy for filtering moves to consider during move selection */
class RlMoveFilter : public RlAutoObject
{
public:

    DECLARE_OBJECT(RlMoveFilter);

    RlMoveFilter(GoBoard& board, bool considerpass = false);

    virtual void LoadSettings(std::istream& settings);

    /** Test whether move is allowed by filter */
    virtual bool ConsiderMove(SgMove move, SgBlackWhite colour,
        bool checklegal = true) const;

    /** Reset allowed moves at the beginning of a game */
    virtual void Reset();
    
    /** Update allowed moves after a move is played */
    virtual void Execute(SgMove move, SgBlackWhite colour);
    
    /** Reset allowed moves after a move is undone */
    virtual void Undo();
    
    /** Select a random allowed move */
    SgMove GetRandomMove(SgBlackWhite colour) const;
    
    /** Make a vector of all allowed moves.
        Calls ConsiderMove on all vacant points. */
    void GetMoveVector(SgBlackWhite colour, std::vector<SgMove>& moves) const;
    
    /** Make an SgList of all vacant moves (even if disallowed or illegal) */
    void GetVacantList(SgList<SgMove>& moves) const;

    /** Make an SgVector of all vacant moves (even if disallowed or illegal) */
    void GetVacantVector(SgVector<SgMove>& moves) const;
    
    /** No allowed moves */
    bool Empty(SgBlackWhite colour) const;
    
    /** Whether the pass move is allowed by the move filter */
    bool ConsiderPass() const { return m_considerPass; }
    
    /** Number of vacant points */
    int NumVacant() const { return m_vacant.size(); }
    
    /** Number of allowed points. Checks every vacant point */
    int NumAllowed(SgBlackWhite colour) const;
    
    /** Iterate through a set of allowed moves.
        @param CheckMove  Use move filter to test whether move is allowed */
    class Iterator
    {
    public:
    
        Iterator(const RlMoveFilter& filter, SgBlackWhite toplay,
            bool checkmove = true)
        :   m_filter(filter),
            m_toPlay(toplay),
            m_checkMove(checkmove),
            m_cursor(m_filter.m_vacant.begin())
        {
            Skip();
        }
        
        SgMove operator*()
        {
            return *m_cursor;
        }

        void operator++()
        {
            m_cursor++;
            Skip();
        }
        
        operator bool() const
        {
            return m_cursor != m_filter.m_vacant.end();
        }

    private:

        void Skip()
        {
            if (m_checkMove)
            {
                while (m_cursor != m_filter.m_vacant.end()
                    && !m_filter.ConsiderMove(*m_cursor, m_toPlay))
                    ++m_cursor;
            }
        }
        
        const RlMoveFilter& m_filter;
        SgBlackWhite m_toPlay;
        bool m_checkMove;
        std::list<SgMove>::const_iterator m_cursor;
    };

private:
    
    void AddVacant(SgPoint point);
    void ClearVacant(SgPoint point);
    void Store(SgPoint point, bool addvacant);

private:

    /** Whether the pass move is allowed by the move filter */
    bool m_considerPass;

    /** All vacant moves */
    // @todo: would like to use slist here
    std::list<SgMove> m_vacant;
    std::list<SgMove>::iterator m_iVacant[SG_PASS + 1];

    /** Undo information */
    struct Change
    {
        Change(SgPoint point, bool addvacant, int step)
        :   m_point(point),
            m_addVacant(addvacant),
            m_step(step)
        { }
        
        SgPoint m_point;
        bool m_addVacant;
        int m_step;
    };
    
    std::vector<Change> m_changes;
    int m_step;
    
    friend class Iterator;
};

//----------------------------------------------------------------------------
/** Don't play moves in single point eyes */
class RlSingleEyeFilter : public RlMoveFilter
{
public:

    DECLARE_OBJECT(RlSingleEyeFilter);

    RlSingleEyeFilter(GoBoard& board, bool considerpass = false);
    
    /** Test whether move is allowed by filter */
    virtual bool ConsiderMove(SgMove move, SgBlackWhite colour,
        bool checklegal = true) const;
};

//----------------------------------------------------------------------------
/** Don't play moves in simple eyes (checks for false eyes) */
class RlSimpleEyeFilter : public RlMoveFilter
{
public:

    DECLARE_OBJECT(RlSimpleEyeFilter);

    RlSimpleEyeFilter(GoBoard& board,
        bool considerpass = false);
    
    /** Test whether move is allowed by filter */
    virtual bool ConsiderMove(SgMove move, SgBlackWhite colour,
        bool checklegal = true) const;
    
    static bool CheckEye(GoBoard& board, SgMove move, SgBlackWhite colour);
};

//----------------------------------------------------------------------------
/** Only play moves within specified Manhattan distance of last move */
class RlProximityFilter : public RlMoveFilter
{
public:

    DECLARE_OBJECT(RlProximityFilter);

    RlProximityFilter(GoBoard& board, 
        bool considerpass = false, int maxdistance = 2);
    
    virtual void LoadSettings(std::istream& settings);
    
    /** Test whether move is allowed by filter */
    virtual bool ConsiderMove(SgMove move, SgBlackWhite colour,
        bool checklegal = true) const;
    
private:

    int m_maxDistance;
};

//----------------------------------------------------------------------------
/** Only play moves within specified point set */
class RlPointSetFilter : public RlMoveFilter
{
public:

    DECLARE_OBJECT(RlPointSetFilter);

    RlPointSetFilter(GoBoard& board, bool considerpass = false);
        
    void SetPoints(const SgPointSet& points) { m_points = points; }

    virtual void LoadSettings(std::istream& settings);
    
    /** Test whether move is allowed by filter */
    virtual bool ConsiderMove(SgMove move, SgBlackWhite colour,
        bool checklegal = true) const;
    
private:

    SgPointSet m_points;
};

//----------------------------------------------------------------------------
/** Union of other filters */
class RlUnionFilter : public RlMoveFilter
{
public:

    DECLARE_OBJECT(RlUnionFilter);

    RlUnionFilter(GoBoard& board, bool considerpass = false);

    void AddFilter(RlMoveFilter* filter) { m_filters.push_back(filter); }

    virtual void LoadSettings(std::istream& settings);

    /** Test whether move is allowed by filter */
    virtual bool ConsiderMove(SgMove move, SgBlackWhite colour,
        bool checklegal = true) const;
    
private:

    std::vector<RlMoveFilter*> m_filters;
};

//----------------------------------------------------------------------------
/** Intersection of other filters */
class RlIntersectionFilter : public RlMoveFilter
{
public:

    DECLARE_OBJECT(RlIntersectionFilter);

    RlIntersectionFilter(GoBoard& board, bool considerpass = false);

    void AddFilter(RlMoveFilter* filter) { m_filters.push_back(filter); }

    virtual void LoadSettings(std::istream& settings);

    /** Test whether move is allowed by filter */
    virtual bool ConsiderMove(SgMove move, SgBlackWhite colour,
        bool checklegal = true) const;
    
private:

    std::vector<RlMoveFilter*> m_filters;
};

//----------------------------------------------------------------------------


#endif // RLMOVEFILTER_H


//----------------------------------------------------------------------------
/** @file RlMiscUtil.h
    Miscellaneous utility functions used by the RLGO module
*/
//----------------------------------------------------------------------------

#ifndef RLMISCUTIL_H
#define RLMISCUTIL_H

#include "GoBoard.h"
#include "SgMove.h"
#include "SgStatistics.h"

//----------------------------------------------------------------------------

typedef double RlFloat;
typedef SgStatistics<RlFloat, int> RlStat;

//----------------------------------------------------------------------------

// Number of players in the game
#define RL_NUM_COLOURS 2
// Maximum number of moves on any board size
#define RL_MAX_MOVES SG_PASS+1
// Maximum time-step in any game before an automatic draw is declared
#define RL_MAX_TIME 1002

const RlFloat RlInfinity = 1e8;

//----------------------------------------------------------------------------

#define ssize(X) (static_cast<signed>(X.size()))

template <class T>
inline bool Contains(const std::vector<T>& vec, const T& element)
{
    return find(vec.begin(), vec.end(), element) != vec.end();
}

//----------------------------------------------------------------------------
/** Sanity checking */

// By default sanity checking only occurs in debug. 
#ifdef _DEBUG
#define SANITY_CHECKING
#endif

#define IS_NAN(x) ((x) != (x))

#ifdef SANITY_CHECKING
#define SANITY_CHECK(x,from,to) \
    if (IS_NAN((x)) || (x) < from || (x) > to) \
    { \
        cerr << "Insane number " \
            << __FILE__ << ':' << __LINE__ << ": " << x << '\n'; \
        abort(); \
    }
#else
#define SANITY_CHECK(x,from,to) (static_cast<void>(0))
#endif

//----------------------------------------------------------------------------
/** Iterate through board, marking final move
*/
class RlBoardIterator
{
public:
    RlBoardIterator(const GoBoard& board)
    :   m_point(board.BoardConst().BoardIterAddress()),
        m_nextPoint(board.BoardConst().BoardIterAddress() + 1)
    { 
    }

    bool Final() const
    {
        return *m_nextPoint == SG_ENDPOINT;
    }

    /** Advance the state of the iteration to the next element. */
    void operator++()
    {
        ++m_point;
        ++m_nextPoint;
    }

    /** Return the value of the current element. */
    SgPoint operator*() const
    {
        return *m_point;
    }

    /** Return true if iteration is valid, otherwise false. */
    operator bool() const
    {
        return *m_point != SG_ENDPOINT;
    }

private:
    const SgPoint* m_point;
    const SgPoint* m_nextPoint;
};

//----------------------------------------------------------------------------
/** Split string function */
inline void SplitString(const std::string &text, const std::string &seps, 
    std::list<std::string>& words)
{
    int n = text.length();
    int start, stop;
    start = text.find_first_not_of(seps);
    while ((start >= 0) && (start < n))
    {
        stop = text.find_first_of(seps, start);
        if ((stop < 0) || (stop > n)) 
            stop = n;
        words.push_back(text.substr(start, stop - start));
        start = text.find_first_not_of(seps, stop + 1);
    }
}

//----------------------------------------------------------------------------

#endif // RLMISCUTIL_H

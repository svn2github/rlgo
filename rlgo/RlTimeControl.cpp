//----------------------------------------------------------------------------
/** @file RlTimeControl.cpp
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlTimeControl.h"
#include "SgTimeRecord.h"

using namespace std;

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlTimeControl);

RlTimeControl::RlTimeControl(GoBoard& board)
:   GoTimeControl(board),
    RlAutoObject(board),
    m_fraction(1.0)
{
}

void RlTimeControl::LoadSettings(istream& settings)
{
    double remain, finalspace, fastfactor;
    int fastopen;
    settings >> RlSetting<int>("FastOpen", fastopen);
    settings >> RlSetting<double>("FastFactor", fastfactor);
    settings >> RlSetting<double>("RemainingConstant", remain);
    settings >> RlSetting<double>("MinTime", m_quickTime);
    settings >> RlSetting<double>("FinalSpace", finalspace);
    settings >> RlSetting<double>("Fraction", m_fraction);
    settings >> RlSetting<double>("SafetyTime", m_safetyTime);
    SetFastOpenMoves(fastopen);
    SetFastOpenFactor(fastfactor);
    SetRemainingConstant(remain);
    SetMinTime(m_quickTime);
    SetFinalSpace(finalspace);
}

double RlTimeControl::TimeForCurrentMove(const SgTimeRecord& timeRecord,
    bool quiet)
{
    double timeForMove =  m_fraction * GoTimeControl::TimeForCurrentMove(
        timeRecord, quiet);
        
    double timeLeft = timeRecord.TimeLeft(m_board.ToPlay());
    if (timeLeft < m_safetyTime 
        && timeRecord.MovesLeft(m_board.ToPlay()) != 1)
        return m_quickTime;
        
    if (m_fastAfterPass && m_board.GetLastMove() == SG_PASS)
        return m_quickTime;
        
    return timeForMove;
}

//----------------------------------------------------------------------------

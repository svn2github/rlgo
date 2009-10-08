//----------------------------------------------------------------------------
/** @file RlFuegoPlayout.cpp
    See RlFuegoPlayout.h
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlFuegoPlayout.h"

#include "GoUctPlayoutPolicy.h"

using namespace std;

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlFuegoPlayout);

RlFuegoPlayout::RlFuegoPlayout(GoBoard& board)
:   RlAutoObject(board),
    m_allSafe(false),
    m_uctPlayout(0)
{
}

RlFuegoPlayout::~RlFuegoPlayout()
{
    if (m_uctPlayout)
        delete m_uctPlayout;
}

void RlFuegoPlayout::LoadSettings(istream& settings)
{
    // Could specify some settings for the default policy
    SG_UNUSED(settings);
}

void RlFuegoPlayout::Initialise()
{
    m_uctPlayout = new GoUctPlayoutPolicy<GoBoard>(m_board, m_param);
}

SgMove RlFuegoPlayout::GenerateMove()
{
    return m_uctPlayout->GenerateMove();
}

void RlFuegoPlayout::OnStart()
{
    m_uctPlayout->StartPlayout();
}

void RlFuegoPlayout::OnEnd()
{
    m_uctPlayout->EndPlayout();
}

void RlFuegoPlayout::OnPlay()
{
    m_uctPlayout->OnPlay();
}

//----------------------------------------------------------------------------

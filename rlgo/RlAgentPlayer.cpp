//----------------------------------------------------------------------------
/** @file RlAgentPlayer.cpp
    @see RlAgentPlayer.h
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlAgentPlayer.h"
#include "RlEvaluator.h"

using namespace std;

//----------------------------------------------------------------------------

RlAgentPlayer::RlAgentPlayer(GoBoard& board)
:   GoPlayer(board)
{
}

void RlAgentPlayer::SetAgent(RlAgent* agent)
{
    m_agent = agent;
}

SgMove RlAgentPlayer::GenMove(const SgTimeRecord& time, SgBlackWhite toPlay)
{
    // Ignore time altogether for now   
    SG_UNUSED(time);
    SG_UNUSED(toPlay);
    
    // @todo: allow non-alternating play: may be consequences in learning rules
    if (toPlay != m_agent->Board().ToPlay())
        throw SgException("RLGO requires alternating play"); 
    
    // Select move and update the agent accordingly
    RlSetup::Get()->SetTimeRecord(time);
    return m_agent->SelectMove();
}

void RlAgentPlayer::OnNewGame()
{
    if (m_agent)
        m_agent->NewGame();
}

void RlAgentPlayer::OnPlay(GoPlayerMove move)
{
    if (m_agent)
        m_agent->Execute(move.Point(), move.Color(), false);
}

void RlAgentPlayer::OnUndo()
{
    if (m_agent)
        m_agent->Undo(false);
}

void RlAgentPlayer::OnBoardChange()
{
    // Update state to current board position.
    if (m_agent)
        m_agent->NewGame();
}

std::string RlAgentPlayer::Name() const
{
    return "RLGO";
}

void RlAgentPlayer::Ponder()
{
    if (m_agent)
        m_agent->Ponder();
}

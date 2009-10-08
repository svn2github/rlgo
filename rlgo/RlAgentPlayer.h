//----------------------------------------------------------------------------
/** @file RlAgentPlayer.h
    A basic player based upon a learning agent
*/
//----------------------------------------------------------------------------

#ifndef RLGO_AGENT_PLAYER_H
#define RLGO_AGENT_PLAYER_H

#include <vector>
#include "GoPlayer.h"
#include "RlAgent.h"

class GoBoard;
class GoGame;

//----------------------------------------------------------------------------
/** A basic player that plays using an agent */
class RlAgentPlayer : public GoPlayer
{
public:

    RlAgentPlayer(GoBoard& board);

    /** Select move, given current board status */
    virtual SgMove GenMove(const SgTimeRecord& time, SgBlackWhite toPlay);
    
    /** Start a new game. */
    virtual void OnNewGame();

    /** Update internal state after played move */
    virtual void OnPlay(GoPlayerMove move);

    /** Update internal state after undo */
    virtual void OnUndo();

    /** Update internal state with a board change (e.g. loadsgf) */
    virtual void OnBoardChange();
    
    /** Ponder */
    virtual void Ponder();
    
    virtual std::string Name() const;
    
    void SetAgent(RlAgent* agent);
    RlAgent* GetAgent() { return m_agent; }
    
private:

    RlAgent* m_agent;
};

#endif


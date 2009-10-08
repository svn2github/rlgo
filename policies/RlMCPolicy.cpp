//----------------------------------------------------------------------------
/** @file RlMCPolicy.cpp
    See RlMCPolicy.h
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlMCPolicy.h"

#include "RlEvaluator.h"
#include "RlMoveFilter.h"
#include "RlSimulator.h"

using namespace std;
using namespace GoBoardUtil;

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlMCPolicy);

RlMCPolicy::RlMCPolicy(GoBoard& board, RlEvaluator* evaluator, 
    RlSimulator* simulator)
:   RlPolicy(board, evaluator),
    m_simulator(simulator)
{
}

void RlMCPolicy::LoadSettings(istream& settings)
{
    RlPolicy::LoadSettings(settings);
    settings >> RlSetting<RlSimulator*>("Simulator", m_simulator);
}

SgMove RlMCPolicy::SelectMove(RlState& state)
{
    //@todo: could be more efficient by ordering moves (e.g. killer first)
    // and then cutting off when max wins can't be exceeded.

    static vector<SgMove> moves;
    m_evaluator->GetMoveFilter()->GetMoveVector(state.Colour(), moves);
    SgBlackWhite toplay = m_board.ToPlay();
    RlFloat bestvalue = toplay == SG_BLACK ? -RlInfinity : +RlInfinity;
    SgMove bestmove = SG_PASS;
    
    for (int m = 0; m < ssize(moves); ++m)
    {
        SgMove move = moves[m];
        SG_ASSERT(m_board.IsLegal(move));
        m_board.Play(move, toplay);
        m_simulator->Simulate();
        m_board.Undo();
        RlFloat value = m_simulator->AverageScore();
        if (toplay == SG_BLACK && value > bestvalue
            || toplay == SG_WHITE && value < bestvalue)
        {
            bestmove = move;
            bestvalue = value;
        }
    }

    return bestmove;
}

//----------------------------------------------------------------------------

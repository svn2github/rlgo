//----------------------------------------------------------------------------
/** @file RlSearchPolicy.cpp
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlSearch.h"

#include "RlAlphaBeta.h"
#include "RlConvert.h"
#include "RlEvaluator.h"
#include "RlSetup.h"
#include "RlTimeControl.h"

using namespace std;

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlSearchPolicy);

RlSearchPolicy::RlSearchPolicy(GoBoard& board, RlEvaluator* searchEvaluator,
    RlAlphaBeta* alphabeta)
:   RlPolicy(board, searchEvaluator),
    m_alphaBeta(alphabeta),
    m_convertEvaluator(0),
    m_convert(0),
    m_timeControl(0)
{
}

void RlSearchPolicy::LoadSettings(istream& settings)
{
    settings >> RlSetting<RlAlphaBeta*>("AlphaBeta", m_alphaBeta);
    settings >> RlSetting<RlEvaluator*>("SearchEvaluator", m_evaluator);
    settings >> RlSetting<RlEvaluator*>("ConvertEvaluator", m_convertEvaluator);
    settings >> RlSetting<RlConvert*>("Convert", m_convert);
    settings >> RlSetting<int>("ControlMode", m_controlMode);
    settings >> RlSetting<RlTimeControl*>("TimeControl", m_timeControl);
}

SgMove RlSearchPolicy::SelectMove(RlState& state)
{
    SG_UNUSED(state);
    
    // Convert evaluator
    if (m_convertEvaluator && m_convert)
    {
        RlDebug(RlSetup::VOCAL) << "Converting weights...";
        m_convert->Convert(
            m_convertEvaluator->GetWeightSet(),
            m_evaluator->GetWeightSet());
        RlDebug(RlSetup::VOCAL) << " done\n";
    }

    // Search does not currently support difference computations
    // (and it's faster this way too)
    m_evaluator->EnsureSimple();
    m_evaluator->EnsureSupportUndo();
    m_evaluator->Reset();

    // Temporarily switch ko rule to SIMPLEKO to avoid slow full board
    // repetition test in GoBoard::Play()
    //GoRestoreKoRule restoreKoRule(m_board);
    //m_board.Rules().SetKoRule(GoRules::SIMPLEKO);

    switch (m_controlMode)
    {
        case RL_DEPTH:
        {
            m_alphaBeta->SetMaxTime(+RlInfinity);
            m_alphaBeta->SetMaxPredictedTime(+RlInfinity);
            break;
        }

        case RL_ELAPSED:
        {
            double t = m_timeControl->TimeForCurrentMove(
                RlSetup::Get()->GetTimeRecord());
            m_alphaBeta->SetMaxTime(t);
            m_alphaBeta->SetMaxPredictedTime(+RlInfinity);
            break;
        }
    
        case RL_PREDICTED:
        {
            double t = m_timeControl->TimeForCurrentMove(
                RlSetup::Get()->GetTimeRecord());
            m_alphaBeta->SetMaxTime(+RlInfinity);
            m_alphaBeta->SetMaxPredictedTime(t);
            break;
        }
        
        default:
            throw SgException("Unknown control mode");
    }
    
    // Run main search
    m_alphaBeta->Clear(); // evaluation function may have changed
    m_searchValue = m_alphaBeta->Search(m_principalVariation);
    
    if (m_principalVariation.empty())
        return SG_PASS;
    else
        return m_principalVariation.front();
}

bool RlSearchPolicy::SearchValue(RlFloat& value) const 
{ 
    value = m_searchValue; 
    return true;
}

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
/** @file RlMCRules.cpp
    See RlMCRules.h
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlMCRules.h"

using namespace std;

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlMonteCarlo);

RlMonteCarlo::RlMonteCarlo(GoBoard& board, RlWeightSet* wset, RlLogger* log)
:   RlLearningRule(board, wset, log)
{
}

void RlMonteCarlo::CalcDelta()
{
    m_target = m_return;
    m_delta = m_return - m_oldValue;
}

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
/** @file RlFuegoPlayout.h
    Policy wrapper for Fuego playouts
*/
//----------------------------------------------------------------------------

#ifndef RLFUEGOPLAYOUT_H
#define RLFUEGOPLAYOUT_H

#include "RlUtils.h"
#include "GoUctPlayoutPolicy.h"

//----------------------------------------------------------------------------
/** Fuego playout wrapper class */
class RlFuegoPlayout : public RlAutoObject
{
public:

    DECLARE_OBJECT(RlFuegoPlayout);

    RlFuegoPlayout(GoBoard& board);
    ~RlFuegoPlayout();

    virtual void LoadSettings(std::istream& settings);
    virtual void Initialise();
        
    SgMove GenerateMove();
    void OnPlay();
    void OnStart();
    void OnEnd();

private:

    GoUctPlayoutPolicyParam m_param;
    SgBWSet m_safe;
    SgPointArray<bool> m_allSafe;

    GoUctPlayoutPolicy<GoBoard>* m_uctPlayout;
};

//----------------------------------------------------------------------------

#endif // RLFUEGOPLAYOUT_H

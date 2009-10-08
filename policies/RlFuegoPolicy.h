//----------------------------------------------------------------------------
/** @file RlFuegoPolicy.h
    Policy wrapper for Fuego playouts
*/
//----------------------------------------------------------------------------

#ifndef RLFUEGOPOLICY_H
#define RLFUEGOPOLICY_H

#include "RlPolicy.h"
#include "RlFuegoPlayout.h"

//----------------------------------------------------------------------------
/** Fuego playout policy class */
class RlFuegoPlayoutPolicy : public RlPolicy
{
public:

    DECLARE_OBJECT(RlFuegoPlayoutPolicy);

    RlFuegoPlayoutPolicy(GoBoard& board, RlFuegoPlayout* playout = 0);

    /** Select a move in the current position for the specified colour */
    virtual SgMove SelectMove(RlState& state);

    /** Load settings */
    virtual void LoadSettings(std::istream& settings);

    /** Sample probability distribution over all moves */
    virtual void SampleProbabilities(int timestep, 
        int numsamples, std::vector<RlFloat>& probs);

private:

    RlFuegoPlayout* m_fuegoPlayout;
    bool m_incremental;
};

//----------------------------------------------------------------------------

#endif // RLFUEGOPOLICY_H

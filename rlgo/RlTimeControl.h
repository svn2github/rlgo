//----------------------------------------------------------------------------
/** @file RlTimeControl.h
    Simple class to control time in RLGO, derived from SgTimeControl
*/
//----------------------------------------------------------------------------

#ifndef RL_TIMECONTROL_H
#define RL_TIMECONTROL_H

#include "RlUtils.h"
#include "GoTimeControl.h"

//----------------------------------------------------------------------------
/** RLGO time control */
class RlTimeControl : public GoTimeControl, public RlAutoObject
{
public:

    DECLARE_OBJECT(RlTimeControl);

    RlTimeControl(GoBoard& board);
    
    virtual void LoadSettings(std::istream& settings);

    virtual double TimeForCurrentMove(const SgTimeRecord& timeRecord,
        bool quiet = false);
    
private:

    /** Proportion of time for this move to allocate to this object */
    double m_fraction;
    
    /** Time to use if playing "quickly" (e.g. within safety threshold) */
    double m_quickTime;
    
    /** If remaining time is below safety threshold, use m_quickTime */
    double m_safetyTime;
    
    /** Use m_quickTime after opponent pass (helps end games faster) */
    bool m_fastAfterPass;
};

//----------------------------------------------------------------------------

#endif // RL_TIMECONTROL_H
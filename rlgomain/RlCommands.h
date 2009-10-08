//----------------------------------------------------------------------------
/** @file RlCommands.h
    GTP extension commands for RLGO
*/
//----------------------------------------------------------------------------

#ifndef RLCOMMANDS_H
#define RLCOMMANDS_H

#include "GtpEngine.h"
#include "RlActiveSet.h"
#include "RlUtils.h"
#include <vector>

class GtpCommand;
class GtpEngine;
class RlRealAgent;
class RlSetup;
class RlTracker;
class RlWeightSet;

//----------------------------------------------------------------------------
/** GTP extension commands for RLGO */

class RlCommands
{
public:

    RlCommands();
    
    void Setup(RlSetup* setup);
    void Register(GtpEngine& e);
    void AddGoGuiAnalyzeCommands(GtpCommand&);
    
    /** @name Command Callbacks */
    // @{
    void CmdActiveAll(GtpCommand&);
    void CmdActiveNext(GtpCommand&);
    void CmdActivePrev(GtpCommand&);
    void CmdActiveWeights(GtpCommand&);
    void CmdDirty(GtpCommand&);
    void CmdEvalParams(GtpCommand& cmd);
    void CmdEvaluate(GtpCommand&);
    void CmdFeatureNext(GtpCommand&);
    void CmdFeaturePrev(GtpCommand&);
    void CmdLicense(GtpCommand&);
    void CmdLoadWeights(GtpCommand&);
    void CmdMoveFilter(GtpCommand&);
    void CmdMoveFreqs(GtpCommand&);
    void CmdNewGame(GtpCommand&);
    void CmdOutputParams(GtpCommand& cmd);
    void CmdPageNext(GtpCommand&);
    void CmdPagePrev(GtpCommand&);
    void CmdPolicy(GtpCommand&);
    void CmdSaveWeights(GtpCommand&);
    void CmdSimulate(GtpCommand&);
    void CmdTopTex(GtpCommand&);
    void CmdVariation(GtpCommand&);
    // @} // @name

    void WriteFloatBoard(GtpCommand& cmd, const std::vector<RlFloat>& v);
    void WriteStringBoard(GtpCommand& cmd, const std::vector<std::string>& v);

protected:
    
    void Register(GtpEngine& engine, const std::string& command,
        GtpCallback<RlCommands>::Method method);

    void AllActive(std::vector<RlChange>& entries);
    void DescribeActive(GtpCommand& cmd, RlChange& entry, int size);
    void DescribeFeature(GtpCommand& cmd, int m_featureIndex);
    void DescribePage(GtpCommand& cmd);

    class Influence
    {
    public:
    
        Influence();
        RlFloat Value(SgPoint pt) const { return m_values[pt]; }
        bool IsSet(SgPoint pt) { return m_set[pt]; }
        void Set(SgPoint pt, RlFloat value);
        void Add(SgPoint pt, RlFloat value);
        void DBoard(RlCommands* commands, GtpCommand& cmd);
        void Gfx(RlCommands* commands, GtpCommand& cmd);

    private:
        
        RlFloat m_values[SG_MAXPOINT];
        bool m_set[SG_MAXPOINT];
    };

    struct Scale
    {
        Scale() 
        :   m_mode(RL_NONE) { }
    
        RlFloat ScaleFloat(RlFloat eval) const;
        void ScaleInfluence(RlCommands::Influence& influence) const;
        std::string ModeString(int mode) const;

        enum
        {
            RL_NONE,
            RL_CENTRED,
            RL_LOGISTIC,
            RL_LINEAR,
            RL_STDDEV,
            RL_MAXDEV,
            RL_NUM_MODES
        };

        int m_mode;
        RlFloat m_min, m_max;
    };

    //@todo: make these const access
    GoBoard& Board() { return *m_board; }
    RlRealAgent* Agent() { return m_agent; }

private:

    class EntryCmp
    {
    public:
        EntryCmp(const RlWeightSet* weightset)
        :   m_weightSet(weightset)
        { }
        
        bool operator()(const RlChange& lhs, const RlChange& rhs) const;
        
    private:
        const RlWeightSet* m_weightSet;
    };

    GoBoard* m_board;
    RlRealAgent* m_agent;

    // State required by GTP commands    
    bool m_stepSize;
    bool m_sort;
    int m_numPV;
    int m_featureIndex;
    int m_activeIndex;
    int m_pageIndex;
    Scale m_scale;
};

//----------------------------------------------------------------------------

#endif // RLCOMMANDS_H

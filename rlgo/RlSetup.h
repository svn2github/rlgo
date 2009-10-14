//----------------------------------------------------------------------------
/** @file RlSetup.h
    Class managing global settings for RLGO
*/
//----------------------------------------------------------------------------

#ifndef RLSETUP_H
#define RLSETUP_H

#include "RlFactory.h"
#include "SgDebug.h"
#include "SgTimeRecord.h"
#include <boost/filesystem/path.hpp>
#include <fstream>
#include <vector>

namespace bfs = boost::filesystem;

class GoGame;
class RlPointIndex;
class RlRealAgent;
class RlSimAgent;
class RlSimulator;

//----------------------------------------------------------------------------
/** Global settings used to set up RLGO infrastructure */
class RlSetup : public RlAutoObject
{
public:

    DECLARE_OBJECT(RlSetup);

    RlSetup(GoBoard& board, RlRealAgent* agent = 0);
    ~RlSetup();

    virtual void LoadSettings(std::istream& settings);
    virtual void Initialise();

    /** Set root path of RLGO distribution */
    void SetMainPath(const bfs::path& mainpath);

    /** Get path to input subdirectory */
    bfs::path GetInputPath() const;

    /** Get path to output subdirectory */
    bfs::path GetOutputPath() const;

    int GetBoardSize() const { return m_boardSize; }
    std::string GetRules() const { return m_rules; }
    int GetRandomSeed() const { return m_randomSeed; }
    bool GetDebugOutput() const { return m_debugOutput; }
    bool DebugLevel(int level) const { return m_debugLevel >= level; }
    void SetDebugLevel(int level) { m_debugLevel = level; }
    bool GetVerification() const { return m_verification; }
    
    RlRealAgent* GetMainAgent() { return m_mainAgent; }
    RlSimAgent* GetSimAgent() { return m_simAgent; }
    void SetMainAgent(RlRealAgent* agent) { m_mainAgent = agent; }
    void SetSimAgent(RlSimAgent* agent) { m_simAgent = agent; }
    
    void SetGame(GoGame* game) { m_game = game; }
    GoGame* GetGame() const { return m_game; }
    
    const SgTimeRecord& GetTimeRecord() { return m_timeRecord; }
    void SetTimeRecord(const SgTimeRecord& timerecord) { m_timeRecord = timerecord; }
    
    RlSimulator* GetSelfPlay() { return m_selfPlay; }
    void SetSelfPlay(RlSimulator* selfplay) { m_selfPlay = selfplay; }
    
    int GetNumGtpCommands() const
    {
        return m_gtpCommands.size();
    }
    
    const std::string& GetGtpCommand(int index) const
    {
        return m_gtpCommands[index];
    }

    /** Accessor to main setup (set during construction) */
    static RlSetup* Get()
    {
        return s_mainSetup;
    }
    
    /** Process number during IPC (-1 for single-process) */
    int GetProcess() const { return m_process; }

    /** DebugOutput options */
    enum
    {
        NONE, // No debug output
        STD,  // Output to stderr (UNIX) or stdout
        FILE  // Output to "DebugStr" file
    };

    /** DebugLevel options */
    enum
    {
        SILENT,  // No debug output
        QUIET,   // Minimal debug output
        VOCAL,   // Moderate debug output
        VERBOSE  // All debug output
    };

private:

    /** Path to root directory of RLGO distribution */
    bfs::path m_mainPath;

    /** Path to input files used by RLGO, relative to m_mainPath
        e.g. weights, share files, book files, etc. */
    bfs::path m_inputPath;
    
    /** Path to temporary output files created by RLGO, 
        relative to m_mainPath */
    bfs::path m_outputPath;
    
    /** Path to book file, or empty if no book file,
        relative to m_inputPath */
    bfs::path m_bookFile;
    
    /** Board size with which to initialise board */
    int m_boardSize;
    
    /** Name of rule set to use */
    std::string m_rules;
    
    /** Random seed for this run */
    int m_randomSeed;
    
    /** Process number during IPC (-1 for single-process) */
    int m_process;    
        
    /** Top-level simulator used to run in self-play mode */
    RlSimulator* m_selfPlay;

    /** Debug output (see enum for options) */
    int m_debugOutput;
    
    /** Debug level (0 to 3) */
    int m_debugLevel;
    
    /** Whether to run slow verification code */
    bool m_verification;
    
    /** Top-level agent used by player */
    RlRealAgent* m_mainAgent;

    /** Agent used by player to execute simulations (may be NULL) */
    RlSimAgent* m_simAgent;
        
    /** The main game record @TODO: remove this altogether */
    GoGame* m_game;
    
    /** The current time record (set during GenMove) */
    SgTimeRecord m_timeRecord;
    
    /** Any GTP commands used during setup (e.g. time controls) */
    std::vector<std::string> m_gtpCommands;
    
    /** Point index utility class */
    RlPointIndex* m_pointIndex;
    
    /** Pointer to main setup created during initialisation */
    static RlSetup* s_mainSetup;
};

namespace RlPathUtil
{
    /** Convenience function for accessing global input path */
    inline bfs::path GetInputPath()
    {
        SG_ASSERT(RlSetup::Get());
        return RlSetup::Get()->GetInputPath();
    }

    /** Convenience function for accessing global output path */
    inline bfs::path GetOutputPath()
    {
        SG_ASSERT(RlSetup::Get());
        return RlSetup::Get()->GetOutputPath();
    }
    
    /** Convenience function for getting the process number */
    inline int GetProcess()
    {
        if (RlSetup::Get())
            return RlSetup::Get()->GetProcess();
        else
            return -1;
    }
}

inline std::ostream& RlDebug(int debuglevel = RlSetup::VERBOSE)
{
    static std::ofstream s_rlNullStream;
    if (!RlSetup::Get() || RlSetup::Get()->DebugLevel(debuglevel))
        return SgDebug();
    else
        return s_rlNullStream;
}

//----------------------------------------------------------------------------

#endif // RLSETUP_H


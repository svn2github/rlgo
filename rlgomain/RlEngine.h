//----------------------------------------------------------------------------
/** @file RlEngine.h
    GTP interface for RLGO player
*/
//----------------------------------------------------------------------------

#ifndef RLENGINE_H
#define RLENGINE_H

#include "GoGtpEngine.h"
#include "RlCommands.h"
#include "RlActiveSet.h"

class RlActiveConnections;
class RlAgentPlayer;
class RlConnection;
class RlRealAgent;
class RlSetup;
class RlTracker;

//----------------------------------------------------------------------------
/** GtpEngine for RLGO */

class RlEngine
    : public GoGtpEngine
{
public:

    /** Constructor.
        @param in Input GTP stream
        @param out Output GTP stream
        @param settingsfile Settings file used to setup RLGO objects
        @param randomSeed As in SgGtpCommands::SetRandomSeed()
    */
    RlEngine(std::istream& in, std::ostream& out, 
        const bfs::path& programPath, 
        const bfs::path& rlgoPath, 
        const bfs::path& settingsFile,
        const std::vector<std::pair<std::string, std::string> >& overrides);

    ~RlEngine();

    /** Override analyze commands to add RLGO extension commands */
    virtual void CmdAnalyzeCommands(GtpCommand& cmd);

    /** Name of the program */
    virtual void CmdName(GtpCommand& cmd);
    
    /** Version of the program */
    virtual void CmdVersion(GtpCommand& cmd);

    /** Load and initialise RLGO from settings file */
    RlSetup* InitSettings(const bfs::path& settingsFile, 
        const bfs::path& rlgoPath, GoBoard& bd);

    /** Set overrides from command line and environment variables */
    void SetOverrides(
        const std::vector<std::pair<std::string, std::string> >& overrides);

private:

    void ForceLink();

    void SetPlayer(const std::string& playerId);    

private:

    RlAgentPlayer* m_player;
    RlSetup* m_setup;
    RlCommands m_commands;
};

//----------------------------------------------------------------------------

#endif // RLGOENGINE_H


//----------------------------------------------------------------------------
/** @file RlEngine.cpp
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlEngine.h"

#include <iostream>
#include <time.h>

#include "GoGtpCommandUtil.h"
#include "RlAgentPlayer.h"
#include "RlForceLink.h"
#include "RlSetup.h"
#include "RlSimulator.h"
#include "RlUtils.h"
#include "SgDebug.h"
#include "SgTimer.h"

#undef PACKAGE
#undef PACKAGE_VERSION
#undef VERSION
#include "RlConfig.h"

using namespace boost;
using namespace std;

//----------------------------------------------------------------------------

RlEngine::RlEngine(istream& in, ostream& out,
    const bfs::path& programPath, 
    const bfs::path& rlgoDir, 
    const bfs::path& settingsfile,
    const vector<pair<string, string> >& overrides)
:   GoGtpEngine(in, out, 0, programPath.native_file_string().c_str())
{
    m_commands.Register(*this);
    RlForceLink();

    // Create a single player that is used by both sides
    RlAgentPlayer* agentplayer = new RlAgentPlayer(Board());
    m_player = agentplayer;
    GoGtpEngine::SetPlayer(agentplayer);

    SetOverrides(overrides);
    m_setup = InitSettings(settingsfile, rlgoDir, m_player->Board());

    // Set player to use newly created agent
    m_player->SetAgent(m_setup->GetMainAgent());
    m_setup->SetGame(&GetGame());
    m_commands.Setup(m_setup);
}

RlEngine::~RlEngine()
{
    RlGetFactory().Clear();
}

RlSetup* RlEngine::InitSettings(const bfs::path& settingsfile, 
    const bfs::path& rlgoPath, GoBoard& bd)
{
    // Delete all old objects in the factory (agent, feature sets, etc.)
    RlGetFactory().Clear();

    RlGetFactory().SetDefaultPath(rlgoPath / "settings");
    RlGetFactory().Load(bd, settingsfile);
    RlSetup* setup = dynamic_cast<RlSetup*>
        (RlGetFactory().GetObject("Setup"));

    setup->SetMainPath(rlgoPath);
    SgRandom::SetSeed(setup->GetRandomSeed());
    SetTimeLimit(setup->GetDefaultTime());

    // Initialise board to specified board size and rules
    Init(setup->GetBoardSize());
    SetNamedRules(setup->GetRules());
    
    // Initialise all objects in factory
    RlGetFactory().Initialise();

    // Execute any GTP commands specified in the setup file
    for (int i = 0; i < setup->GetNumGtpCommands(); ++i)
        ExecuteCommand(setup->GetGtpCommand(i));
    
    return setup;
}

void RlEngine::SetOverrides(const vector<pair<string, string> >& overrides)
{
    // Settings get specified with following priority:
    //  1. Command line
    //  2. Environment variable
    //  3. Settings file

    #if UNIX
    extern char** environ;
    for (int i = 0; environ[i] != NULL; i++)
    {
        string env(environ[i]);

        if (env.substr(0, 2) == "Rl")
        {
            int i = env.find('=');
            string setting = env.substr(2, i - 2);
            string value = env.substr(i + 1);
            RlGetFactory().SetOverride(setting, value);
        }
    }
    #endif

    for (vector<pair<string, string> >::const_iterator 
            i_overrides = overrides.begin(); 
            i_overrides != overrides.end(); ++i_overrides)
    {
        RlGetFactory().SetOverride(i_overrides->first, i_overrides->second);
    }
}

void RlEngine::CmdAnalyzeCommands(GtpCommand& cmd)
{
    GoGtpEngine::CmdAnalyzeCommands(cmd);
    m_commands.AddGoGuiAnalyzeCommands(cmd);
    string response = cmd.Response();
    cmd.SetResponse(GoGtpCommandUtil::SortResponseAnalyzeCommands(response));
}

void RlEngine::CmdName(GtpCommand& cmd)
{
    cmd << "RLGO";
}

void RlEngine::CmdVersion(GtpCommand& cmd)
{
    cmd << VERSION;
}

//----------------------------------------------------------------------------

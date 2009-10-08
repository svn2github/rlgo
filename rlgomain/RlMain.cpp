//----------------------------------------------------------------------------
/** @file RlMain.cpp
    Main function for Rl.
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"

#include <iostream>
#include <dirent.h>
#include "GoInit.h"
#include "RlEngine.h"
#include "RlAgent.h"
#include "RlSetup.h"
#include "RlSimulator.h"
#include "SgDebug.h"
#include "SgException.h"
#include "SgInit.h"

#undef PACKAGE
#undef PACKAGE_VERSION
#undef VERSION
#include "RlConfig.h"

using namespace std;
using namespace boost::filesystem;

//----------------------------------------------------------------------------

namespace {

/** @name Settings from command line options */
// @{

vector<pair<string, string> > g_overrides;
string g_config;
bfs::path g_execFile; // Executable filename
bfs::path g_execDir; // Executable directory
bfs::path g_rlgoDir; // Root directory of RLGO distribution
bfs::path g_settingsFile("dyna2-settings.set"); // Default settings file

// @} // @name

void MainLoop()
{
    RlEngine engine(cin, cout, 
        g_execDir, 
        g_rlgoDir, 
        g_settingsFile, 
        g_overrides);
    GoGtpAssertionHandler assertionHandler(engine);
    if (g_config != "")
        engine.ExecuteFile(g_config);
    RlSimulator* simulator = RlSetup::Get()->GetSelfPlay();
    if (simulator)
    {
        SgDebug() << "Running in self-play mode\n";
        simulator->Simulate();
    }
    else
    {
        // Call new game once on startup, 
        // so that GTP genmove will work immediately
        RlSetup::Get()->GetMainAgent()->NewGame();    
        SgDebug() << "Ready\n";
        engine.MainLoop();
    }
}

void ParseOptions(int argc, char** argv)
{
    // Arguments must be specified in pairs:
    //    -setting value

    // There is one special pair specifying the settings file
    // All other pairs specify overrides of settings

    if (argc % 2 != 1)
        throw SgException("Settings must be specified in pairs");

    for (int i = 1; i < argc; i += 2)
    {
        if (argv[i][0] != '-')
            throw SgException("Expected setting to start with '-'");

        string setting = argv[i] + 1;
        string value = argv[i + 1];

        if (setting == "settings" || setting == "Settings")
            g_settingsFile = value;
        else
            g_overrides.push_back(pair<string, string>(setting, value));

        if (setting == "config")
            g_config = value;
    }
}

void PrintStartupMessage()
{
    SgDebug() << "RLGO version " << VERSION << '\n' <<
        "Copyright (C) 2009 David Silver.\n"
        "Built on top of the Fuego 0.4 library (see http://fuego.sf.net)\n"
        "This program comes with ABSOLUTELY NO WARRANTY. This is\n"
        "free software and you are welcome to redistribute it under\n"
        "certain conditions. Type `rlgo-license' for details.\n\n";
}

bool SearchPath(const bfs::path& rlgoPath)
{
    return exists(rlgoPath / "README")
        && exists(rlgoPath / "INSTALL")
        && exists(rlgoPath / "rlgo")
        && exists(rlgoPath / "rlgomain");        
}

void GetProgramDir(const char* programPath)
{
    if (!programPath)
        throw SgException("Could not find program path");

    bfs::path execpath = bfs::path(programPath, boost::filesystem::native);
    g_execDir = execpath.branch_path();
    g_execFile = execpath.leaf();
        
    g_rlgoDir = g_execDir / "..";
    if (!SearchPath(g_rlgoDir))
    {
        cout << "Executable directory = " << g_execDir << "\n:";
        cout << "Executable filename = " << g_execFile << "\n:";
        cout << "RLGO directory = " << g_rlgoDir << "\n:";
        throw SgException("Could not find root directory of RLGO distribution");
    }
}

void FindSettingsFile()
{
    string settings = g_settingsFile.file_string();
    if (exists(g_rlgoDir / "settings" / settings))
        return;
    if (exists(g_rlgoDir / "settings" / (settings + ".set")))
    {
        g_settingsFile = bfs::path(settings + ".set");
        return;
    }
    if (exists(g_rlgoDir / "settings" / (settings + "-settings.set")))
    {
        g_settingsFile = bfs::path(settings + "-settings.set");
        return;
    }
    throw SgException("Could not find settings file");
}

} // namespace

//----------------------------------------------------------------------------

int main(int argc, char** argv)
{
    if (argc > 0 && argv != 0)
    {
        GetProgramDir(argv[0]);
        try
        {
            ParseOptions(argc, argv);
        }
        catch (const SgException& e)
        {
            SgDebug() << e.what() << "\n";
            return 1;
        }
    }
    FindSettingsFile();

    try
    {
        SgInit();
        GoInit();
        PrintStartupMessage();
        MainLoop();
        GoFini();
        SgFini();
    }
    catch (const GtpFailure& e)
    {
        SgDebug() << e.Response() << '\n';
        return 1;
    }
    catch (const std::exception& e)
    {
        SgDebug() << e.what() << '\n';
        return 1;
    }
    return 0;
}

//----------------------------------------------------------------------------


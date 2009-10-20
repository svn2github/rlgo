//----------------------------------------------------------------------------
/** @file RlSetup.cpp
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlSetup.h"

#include "RlAgent.h"
#include "RlSimulator.h"
#include "RlUtils.h"
#include <boost/filesystem/convenience.hpp>
#include <boost/lexical_cast.hpp>

//@TODO
//#include <boost/algorithm/string.hpp>

using namespace boost;
using namespace std;

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlSetup);

RlSetup* RlSetup::s_mainSetup = 0;

RlSetup::RlSetup(GoBoard& board, RlRealAgent* agent)
:   RlAutoObject(board),
    m_inputPath(""),
    m_outputPath(""),
    m_boardSize(9),
    m_rules("cgos"),
    m_randomSeed(-1),
    m_process(-1),
    m_selfPlay(0),
    m_debugOutput(STD),
    m_debugLevel(VOCAL),
    m_verification(false),
    m_defaultTime(10),
    m_mainAgent(agent),
    m_simAgent(0),
    m_game(0),
    m_timeRecord(true, 10),
    m_pointIndex(0)
{
    s_mainSetup = this;
}

RlSetup::~RlSetup()
{
    if (m_pointIndex)
        delete m_pointIndex;
}

void RlSetup::LoadSettings(std::istream& settings)
{
    int version;
    string inputpath, outputpath, bookfile;
    int numgtp;
    
    settings >> RlVersion(version, 7, 7);
    settings >> RlSetting<string>("InputPath", inputpath);
    settings >> RlSetting<string>("OutputPath", outputpath);
    settings >> RlSetting<int>("BoardSize", m_boardSize);    
    settings >> RlSetting<string>("Rules", m_rules);
    settings >> RlSetting<int>("RandomSeed", m_randomSeed);
    settings >> RlSetting<int>("Process", m_process);
    settings >> RlSetting<RlSimulator*>("SelfPlay", m_selfPlay);
    settings >> RlSetting<int>("DebugOutput", m_debugOutput);
    settings >> RlSetting<int>("DebugLevel", m_debugLevel);
    settings >> RlSetting<bool>("Verification", m_verification);
    settings >> RlSetting<int>("DefaultTime", m_defaultTime);
    settings >> RlSetting<RlRealAgent*>("MainAgent", m_mainAgent);
    settings >> RlSetting<RlSimAgent*>("SimAgent", m_simAgent);
    settings >> RlSetting<string>("BookFile", bookfile);
    
    settings >> RlSetting<int>("NumGtp", numgtp);    
    for (int i = 0; i < numgtp; ++i)
    {
        string gtp;
        settings >> RlLongSetting("Gtp", gtp);
        m_gtpCommands.push_back(gtp);
    }

    // Relative to main path, which is not yet known.
    m_inputPath = bfs::path(inputpath);
    m_outputPath = bfs::path(outputpath);
    
    // Relative to input path
    m_bookFile = bfs::path(bookfile);
}

void RlSetup::SetMainPath(const bfs::path& mainpath)
{
    m_mainPath = mainpath;
}

bfs::path RlSetup::GetInputPath() const
{
    return m_mainPath / m_inputPath;
}

bfs::path RlSetup::GetOutputPath() const
{
    return m_mainPath / m_outputPath;
}

void RlSetup::Initialise()
{
    create_directories(m_mainPath / m_outputPath);
    m_pointIndex = new RlPointIndex(m_boardSize);

    if (m_debugOutput == NONE)
    {
        SgDebugToNull();
    }
    if (m_debugOutput == FILE
        || (m_debugOutput == STD && m_process > 0))
    {
        ostringstream name;
        name << "DebugStr";
        if (m_process >= 0)
            name << "P" << m_process;
        bfs::path pathname = m_mainPath / m_outputPath / name.str();
        SgDebugToFile(pathname.native_file_string().c_str());
    }

    if (!m_bookFile.empty() && m_bookFile != "NULL")
        m_gtpCommands.push_back(string("book_load ") 
            + bfs::complete(m_bookFile, m_mainPath / m_inputPath).
            native_file_string());

    m_timeRecord = SgTimeRecord(true, m_defaultTime);
}

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
/** @file RlLogger.cpp
    See RlLogger.h
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlLogger.h"

#include "RlBinaryFeatures.h"
#include "RlTrace.h"

#include <boost/timer.hpp>

using namespace boost;
using namespace std;
using namespace RlPathUtil; 

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlLogger);

RlLogger::RlLogger(GoBoard& board)
:   RlAutoObject(board),
    m_active(false),
    m_debugLevel(RlSetup::VOCAL),
    m_logMode(LOG_LINEAR),
    m_logNext(0),
    m_interval(1),
    m_intervalMul(2),
    m_logStartOnly(false),
    m_numGames(-1)
{
}

void RlLogger::LoadSettings(istream& settings)
{
    settings >> RlSetting<int>("DebugLevel", m_debugLevel);
    settings >> RlSetting<int>("LogMode", m_logMode);
    settings >> RlSetting<RlFloat>("Interval", m_interval);
    settings >> RlSetting<RlFloat>("IntervalMul", m_intervalMul);
    settings >> RlSetting<bool>("LogStartOnly", m_logStartOnly);
    settings >> RlSetting<string>("TraceFeatures", m_traceFile);
}

void RlLogger::Initialise()
{
    m_numGames = -1;
}

void RlLogger::StartGame()
{
    m_numGames++;
    m_numMoves = 0;
    
    m_active = m_logMode != LOG_NEVER && m_numGames == m_logNext;
    if (!m_active)
        return;

    m_logNext = m_numGames + m_interval;
    switch (m_logMode)
    {
    case LOG_LINEAR: // e.g. 0, 100, 200, 300, ...
        break;
    case LOG_EXP: // e.g. 0, 100, 200, 400, 800, ...
        if (m_numGames > 0)
            m_interval *= m_intervalMul;
        break;
    case LOG_EXPINTERVAL: // e.g. 0, 100, 300, 700, 1500, ...
        m_interval *= m_intervalMul;
        break;
    case LOG_NICE: // e.g. 0, 1..9, 10..90, 100..900, ...
        if (m_numGames > 0 && 
            m_numGames + m_interval >= m_interval * m_intervalMul)
            m_interval *= m_intervalMul;
        break;
    }
}

void RlLogger::EndGame()
{  
}

void RlLogger::StepMove()
{
    m_numMoves++;
}

void RlLogger::TraceFeatures(RlBinaryFeatures* featureset)
{
    // Read features to trace from input file
    bfs::path trpath = bfs::complete(m_traceFile, GetInputPath());
    bfs::ifstream tracefeatures(trpath);
    tracefeatures >> ws;
    while (!tracefeatures.eof())
    {
        int featureindex = featureset->ReadFeature(tracefeatures);
        if (featureindex >= 0)
        {
            ostringstream oss;
            featureset->DescribeFeature(featureindex, oss);
            string featurename = oss.str();
            m_traceIndices[featureindex] = m_traceFeatures.size();
            m_traceFeatures.push_back(
                pair<string, int>(featurename, featureindex));
        }
        tracefeatures >> ws;
    }
}


//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
/** @file RlWeightSet.cpp
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlWeightSet.h"

#include "RlWeight.h"
#include "RlBinaryFeatures.h"
#include "RlProcessUtil.h"
#include "RlSetup.h"
#include "RlUtils.h"
#include "SgException.h"
#include <boost/filesystem/operations.hpp>

using namespace RlPathUtil;
using namespace std;

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlWeightSet);

RlWeightSet::RlWeightSet(GoBoard& board, 
    RlBinaryFeatures* featureset)
:   RlAutoObject(board),
    m_featureSet(featureset),
    m_weights(0),
    m_numFeatures(0),
    m_numWeights(0),
    m_sharedMemory(0),
    m_strict(true),
    m_streamMode(0)
{
}

void RlWeightSet::LoadSettings(istream& settings)
{
    settings >> RlSetting<RlBinaryFeatures*>("FeatureSet", m_featureSet);
    settings >> RlSetting<string>("ShareName", m_shareName);
    settings >> RlSetting<bool>("Strict", m_strict);
    settings >> RlSetting<int>("StreamMode", m_streamMode);
}

void RlWeightSet::Initialise()
{
    m_featureSet->EnsureInitialised();
    m_numFeatures = m_featureSet->GetNumFeatures();
    m_numWeights = m_numFeatures;

    if (m_shareName == "" || m_shareName == "NULL")
    {
        m_weights = new RlWeight[m_numWeights];
        m_sharedMemory = 0;    
    }
    else
    {
        int bytes = m_numWeights * sizeof(RlWeight);
        bfs::path pathname = bfs::complete(m_shareName, GetInputPath());
        m_sharedMemory = new RlSharedMemory(pathname, 0, bytes);
        m_weights = (RlWeight*) m_sharedMemory->GetData();    
    }
}

RlWeightSet::~RlWeightSet()
{
    if (m_sharedMemory)
        delete m_sharedMemory;
    else if (m_weights)
        delete [] m_weights;
}

void RlWeightSet::ZeroWeights()
{
    for (int i = 0; i < m_numFeatures; ++i)
        Get(i).Weight() = 0;
}

void RlWeightSet::RandomiseWeights(RlFloat min, RlFloat max)
{
    for (int i = 0; i < m_numFeatures; ++i)
        Get(i).Weight() = SgRandomFloat(min, max);
}

void RlWeightSet::AddWeights(RlWeightSet* source)
{
    SG_ASSERT(source->m_numWeights == m_numWeights);
    for (int i = 0; i < m_numFeatures; ++i)
        Get(i).Weight() += source->Get(i).Weight();
}

void RlWeightSet::SubWeights(RlWeightSet* source)
{
    SG_ASSERT(source->m_numWeights == m_numWeights);
    for (int i = 0; i < m_numFeatures; ++i)
        Get(i).Weight() -= source->Get(i).Weight();
}

void RlWeightSet::Save(ostream& wstream)
{
    RlGetFactory().EnableOverrides(false);
    string setname = m_featureSet->SetName();    
    int version = 2;
    wstream << RlVersion(version);
    wstream << RlSetting<string>("FeatureSet", setname);
    wstream << RlSetting<int>("NumSaved", m_numFeatures);

    for (int i = 0; i < m_numFeatures; ++i)
        Get(i).Save(wstream);

    RlGetFactory().EnableOverrides(true);
}

void RlWeightSet::Load(istream& wstream)
{
    RlGetFactory().EnableOverrides(false);
    string loadname, setname = m_featureSet->SetName();    
    int numsaved, version;
    wstream >> RlVersion(version, 2, 1);
    wstream >> RlSetting<string>("FeatureSet", loadname);
    wstream >> RlSetting<int>("NumSaved", numsaved);
    
    if (m_strict && loadname != setname)
        throw SgException("Loading weights for incorrect feature set");
    if (m_strict && m_numFeatures != numsaved)
        throw SgException("Loading weight set of incorrect size");

    // Load as many features as possible
    int featurestoload;
    if (m_numFeatures < numsaved)
        featurestoload = m_numFeatures;
    else
        featurestoload = numsaved;

    for (int i = 0; i < m_numFeatures; ++i)
        Get(i).Load(wstream);

    RlGetFactory().EnableOverrides(true);
}

//----------------------------------------------------------------------------

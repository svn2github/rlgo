//----------------------------------------------------------------------------
/** @file RlSharedFeatures.cpp
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlSharedFeatures.h"

#include "SgException.h"
#include "RlLocalShape.h"
#include "RlSetup.h"
#include "SgDebug.h"
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>

using namespace std;
using namespace RlPathUtil;

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlSharedFeatures);

RlSharedFeatures::RlSharedFeatures(GoBoard& board, 
    RlBinaryFeatures* featureset)
:   RlCompoundFeatures(board),
    m_featureSet(featureset),
    m_numInputFeatures(0),
    m_numOutputFeatures(0), // calculated during Initialise
    m_lookup(0),
    m_inverseMap(0),
    m_selfInverse(true),
    m_tableFile(true)
{
}

RlSharedFeatures::~RlSharedFeatures()
{
    if (m_lookup)
        delete [] m_lookup;
    if (m_inverseMap)
        delete [] m_inverseMap;
}

void RlSharedFeatures::LoadSettings(std::istream& settings)
{
    RlBinaryFeatures::LoadSettings(settings);

    int version;
    settings >> RlVersion(version, 2, 2);
    settings >> RlSetting<RlBinaryFeatures*>("FeatureSet", m_featureSet);
    settings >> RlSetting<bool>("SelfInverse", m_selfInverse);
    settings >> RlSetting<bool>("TableFile", m_tableFile);
}

void RlSharedFeatures::Initialise()
{
    AddFeatureSet(m_featureSet);
    RlCompoundFeatures::Initialise();

    if (!LoadTables())
    {
        MakeTables();
        SaveTables();
    }
}

RlTracker* RlSharedFeatures::CreateTracker(
    map<RlBinaryFeatures*, RlTracker*>& trackermap)
{
    SG_UNUSED(trackermap);
    SG_ASSERT(IsInitialised());
    CreateChildTrackers(trackermap);
    return new RlSharedTracker(m_board, this, trackermap[FeatureSet()]);
}

void RlSharedFeatures::MakeTables()
{
    RlDebug(RlSetup::VOCAL) << "Making share table for " 
        << SetName() << "...";
    m_numInputFeatures = FeatureSet()->GetNumFeatures();
    m_numOutputFeatures = 0;
    m_lookup = new Lookup[m_numInputFeatures];
    m_inverseMap = new int[m_numInputFeatures]; // Only need outputs

    for (int i = 0; i < m_numInputFeatures; ++i)
    {
        // Find canonical version of feature i
        int sign;
        int canonical = CalcCanonical(i, sign);

        // If canonical, create a new output feature
        if (canonical == i)
        {
            if (sign)
            {
                m_lookup[i].m_index = m_numOutputFeatures;
                m_lookup[i].m_sign = sign;
                m_inverseMap[m_numOutputFeatures] = canonical;
                m_numOutputFeatures++;
            }
            
            // Ignore features with sign zero
            else
            {
                m_lookup[i].m_index = -1;
                m_lookup[i].m_sign = 0;
            }
        }
        
        // Otherwise point to existing output feature 
        // corresponding to canonical feature
        // NB: this must already exist because canonical feature
        // is defined to have the lowest input feature index
        // Specify sign for antisymmetric weight sharing
        else
        {
            SG_ASSERT(canonical < i);
            m_lookup[i].m_index = m_lookup[canonical].m_index;
            m_lookup[i].m_sign = sign;
        }        
    }    

    RlDebug(RlSetup::VOCAL) << " done\n" 
        << SetName() << ": " << m_numInputFeatures << "->"
        << m_numOutputFeatures << " features \n";
}

int RlSharedFeatures::CalcCanonical(int featureindex, int& sign) const
{
    if (IgnoreFeature(featureindex))
    {
        sign = 0;
        return featureindex;
    }

    vector<int> equivalent;
    vector<int> signs;
    GetEquivalent(featureindex, equivalent, signs);
        
    SG_ASSERT(! equivalent.empty());
    
    int minindex = equivalent[0];
    sign = signs[0];
    
    for (int i = 1; i < ssize(equivalent); ++i)
    {
        int eindex = equivalent[i];
        int esign = signs[i];
        
        if (eindex < minindex)
        {
            minindex = eindex;
            sign = esign;
        }
    }

    // If canonical feature index occurs with both +ve and -ve signs
    // then it is a self-inverted feature.
    if (m_selfInverse)
    {
        bool pos = false, neg = false;
        for (int i = 0; i < ssize(equivalent); ++i)
        {
            if (equivalent[i] == minindex)
            {
                if (signs[i] == +1)
                    pos = true;
                if (signs[i] == -1)
                    neg = true;                
            }
        }

        // Ignore self-inverted features by setting sign to zero
        if (pos && neg)
            sign = 0;
    }
    
    return minindex;
}

void RlSharedFeatures::GetEquivalent(int featureindex, 
    vector<int>& equivalent,
    vector<int>& signs) const
{
    // By default, each feature is its own equivalence class
    equivalent.push_back(featureindex);
    signs.push_back(1);
}

void RlSharedFeatures::Display(ostream& disp) const
{
    // Display corresponding canonical feature for each feature index
    for (int i = 0; i < m_numInputFeatures; ++i)
    {
        FeatureSet()->DescribeFeature(i, disp);
        disp << "->\n";
        FeatureSet()->DescribeFeature(
            m_inverseMap[m_lookup[i].m_index], disp);
        disp << "\n";
    }
}

void RlSharedFeatures::DisplayFeature(int featureindex, ostream& cmd) const
{
    // Display the canonical example of this feature
    FeatureSet()->DisplayFeature(GetCanonicalFeature(featureindex), cmd);
}

bfs::path RlSharedFeatures::TablePath() const
{
    ostringstream oss;
    oss << "Share-";
    DescribeSet(oss);
    if (!m_selfInverse) // this tag is added if self inverse ISN'T ignored
        oss << "-SI";
    oss << "-Size-" << m_board.Size();
    oss << ".share";
    bfs::path filename = GetInputPath() / "shares" / oss.str();
    return filename;
}

bool RlSharedFeatures::LoadTables()
{
    if (!m_tableFile)
        return false;

    bfs::ifstream tables(TablePath(), ios::binary | ios::in);
    if (!tables)
        return false;
    
    RlDebug(RlSetup::VOCAL) << "Loading share table for " 
        << SetName() << "...";

    int version;
    tables >> RlVersion(version, 1, 1);
    tables >> RlSetting<int>("NumInputFeatures", m_numInputFeatures);
    tables >> RlSetting<int>("NumOutputFeatures", m_numOutputFeatures);

    m_lookup = new Lookup[m_numInputFeatures];
    m_inverseMap = new int[m_numOutputFeatures];

    tables.read((char*) m_lookup, m_numInputFeatures * sizeof(Lookup));
    tables.read((char*) m_inverseMap, m_numOutputFeatures * sizeof(int));
    RlDebug(RlSetup::VOCAL) << " done\n";
    return true;
}

bool RlSharedFeatures::SaveTables()
{
    if (!m_tableFile)
        return false;

    RlDebug(RlSetup::VOCAL) << "Saving share table for " 
        << SetName() << "...";
    
    bfs::create_directories(TablePath().branch_path());
    bfs::ofstream tables(TablePath(), ios::binary | ios::out);
    if (!tables)
        return false;
    
    tables << RlVersion(1);
    tables << RlSetting<int>("NumInputFeatures", m_numInputFeatures);
    tables << RlSetting<int>("NumOutputFeatures", m_numOutputFeatures);

    tables.write((char*) m_lookup, m_numInputFeatures * sizeof(Lookup));
    tables.write((char*) m_inverseMap, m_numOutputFeatures * sizeof(int));
    RlDebug(RlSetup::VOCAL) << " done\n";
    return true;
}

int RlSharedFeatures::GetNumFeatures() const 
{ 
    return m_numOutputFeatures; 
}

int RlSharedFeatures::ReadFeature(istream& desc) const
{
    int inputfeature = FeatureSet()->ReadFeature(desc);
    return GetOutputFeature(inputfeature);
}

int RlSharedFeatures::GetSign(const string& desc) const
{
    istringstream iss;
    iss.str(desc);
    return ReadSign(iss);
}

int RlSharedFeatures::ReadSign(istream& desc) const
{
    int inputfeature = FeatureSet()->ReadFeature(desc);
    return GetSign(inputfeature);
}

void RlSharedFeatures::DescribeFeature(int outputfeature, ostream& str) const
{
    int canonicalfeature = GetCanonicalFeature(outputfeature);
    FeatureSet()->DescribeFeature(canonicalfeature, str);
}

void RlSharedFeatures::DescribeTex(int outputfeature, ostream& tex,
    bool invert) const
{
    int canonicalfeature = GetCanonicalFeature(outputfeature);
    FeatureSet()->DescribeTex(canonicalfeature, tex, invert);
}

void RlSharedFeatures::DescribeSet(ostream& str) const
{
    str << "SharedFeatures";
}

void RlSharedFeatures::GetPage(int pagenum, vector<int>& indices, 
    ostream& pagename) const
{
    FeatureSet()->GetPage(pagenum, indices, pagename);
    for (int i = 0; i < m_board.Size() * m_board.Size(); ++i)
    {
        if (indices[i] != -1)
            indices[i] = GetOutputFeature(indices[i]);
    }
}

int RlSharedFeatures::GetNumPages() const
{
    return FeatureSet()->GetNumPages();
}

bool RlSharedFeatures::IgnoreFeature(int featureindex) const
{
    SG_UNUSED(featureindex);
    return false;
}

SgPoint RlSharedFeatures::GetPosition(int featureindex) const
{
    int newindex = GetCanonicalFeature(featureindex);
    return FeatureSet()->GetPosition(newindex);
}

void RlSharedFeatures::CollectPoints(int featureindex, 
    vector<SgPoint>& points) const
{
    int newindex = GetCanonicalFeature(featureindex);
    return FeatureSet()->CollectPoints(newindex, points);
}

bool RlSharedFeatures::Touches(int featureindex, SgPoint pt) const
{
    //@todo: could consider touching if any equivalent feature touches
    int newindex = GetCanonicalFeature(featureindex);
    return FeatureSet()->Touches(newindex, pt);
}

//----------------------------------------------------------------------------

RlSharedTracker::RlSharedTracker(GoBoard& board, 
    const RlSharedFeatures* features, RlTracker* tracker)
:   RlCompoundTracker(board),
    m_sharedFeatures(features),
    m_tracker(tracker)
{
    AddTracker(m_tracker);
}

void RlSharedTracker::ShareChanges()
{
    for (RlChangeList::Iterator i_changes(m_tracker->ChangeList());
        i_changes; ++i_changes)
    {
        int sign = m_sharedFeatures->GetSign(i_changes->m_featureIndex);
        if (sign != 0)
            NewChange(
                i_changes->m_slot, 
                m_sharedFeatures->GetOutputFeature(i_changes->m_featureIndex),
                i_changes->m_occurrences * sign);
    }
}

void RlSharedTracker::PropagateChanges()
{
    RlCompoundTracker::PropagateChanges();
    ShareChanges();
}

int RlSharedTracker::GetActiveSize() const
{
    return m_tracker->GetActiveSize();
}

//----------------------------------------------------------------------------

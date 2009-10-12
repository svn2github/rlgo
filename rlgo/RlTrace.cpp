//----------------------------------------------------------------------------
/** @file RlTrace.cpp
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlTrace.h"

#include "RlFactory.h"
#include "RlSetup.h"
#include <boost/filesystem/convenience.hpp>

using namespace boost;
using namespace std;
using namespace RlPathUtil;

//----------------------------------------------------------------------------

RlLog::RlLog(const bfs::path& logpath, int every, char sep)
:   m_logFile(logpath),
    m_separator(sep),
    m_started(false),
    m_flushLines(true),
    m_steps(0),
    m_every(every)
{
    m_logFile << "Index";
}

RlLog::RlLog(RlAutoObject* object, 
        const std::string& tag,
        const std::string& ext,
        int every, char sep)
:   m_logFile(GenLogName(object, tag, ext)),
    m_separator(sep),
    m_started(false),
    m_flushLines(true),
    m_steps(0),
    m_every(every)
{
    m_logFile << "Index";
}        

void RlLog::AddItem(const string& item, const boost::any& defaultvalue)
{
    if (m_started)
        throw SgException("Can't add log item after log has started");
        
    m_logFile << m_separator << item;
    m_stringColumns[item] = m_values.size();
    m_defaults.push_back(defaultvalue);
    m_values.push_back(defaultvalue);
}

void RlLog::AddItem(const string& item, int itemid,
                    const boost::any& defaultvalue)
{
    m_columns[itemid] = m_values.size();
    AddItem(item, defaultvalue);
}

void RlLog::Step()
{
    if (Active())
    {
        m_logFile << "\n" << m_steps;
        for (vector<any>::iterator iValues = m_values.begin(); 
            iValues != m_values.end(); ++iValues)
        {
            // This is ugly, but boost::any doesn't support operator<<
            if (iValues->type() == typeid(char))
                m_logFile << m_separator << any_cast<char>(*iValues);
            if (iValues->type() == typeid(int))
                m_logFile << m_separator << any_cast<int>(*iValues);
            if (iValues->type() == typeid(float))
                m_logFile << m_separator << any_cast<float>(*iValues);
            if (iValues->type() == typeid(double))
                m_logFile << m_separator << any_cast<double>(*iValues);
            if (iValues->type() == typeid(bool))
                m_logFile << m_separator << any_cast<bool>(*iValues);
            if (iValues->type() == typeid(string))
                m_logFile << m_separator << any_cast<string>(*iValues);
        }
        
        // Make sure that log file is always up to date, to help debugging
        if (m_flushLines)
            m_logFile << flush;
        
        m_values = m_defaults;
        m_started = true;
    }
    
    m_steps++;
}

void RlLog::Log(int itemid, const boost::any& value)
{
    if (!Active())
        return;

    // Don't add any new columns, just set value for existing column.
    map<int, int>::iterator iColumns = m_columns.find(itemid);
    if (iColumns != m_columns.end())
    {
        int col = iColumns->second;
        SG_ASSERT(col >= 0 && col < ssize(m_values));
        m_values[col] = value;
    }
}

void RlLog::Log(const string& item, const boost::any& value)
{
    if (!Active())
        return;

    // Don't add any new columns, just set value for existing column.
    map<string, int>::iterator iStringColumns = m_stringColumns.find(item);
    if (iStringColumns != m_stringColumns.end())
    {
        int col = iStringColumns->second;
        SG_ASSERT(col >= 0 && col < ssize(m_values));
        m_values[col] = value;
    }
}

bool RlLog::Exists(const string& item) const
{
    return m_stringColumns.find(item) != m_stringColumns.end();
}

bool RlLog::Exists(int itemid) const
{
    return m_columns.find(itemid) != m_columns.end();
}

bfs::path RlLog::GenLogName(RlAutoObject* object, 
    const std::string& tag, const string& ext, int counter)
{
    string tracename = RlTrace::GenTraceName(object, tag, counter);
    return GetOutputPath() / (tracename + ext);
}

//----------------------------------------------------------------------------

RlTrace::RlTrace(const bfs::path& dir, 
    const string& prefix,
    const string& postfix,
    int every, char sep)
:   m_dir(dir),
    m_prefix(prefix),
    m_postfix(postfix),
    m_every(every),
    m_separator(sep)
{
    create_directories(m_dir);
}

RlTrace::RlTrace(RlAutoObject* object, 
    const string& tag, 
    const std::string& postfix, 
    int every, char sep)
:   m_dir(GetOutputPath()),
    m_prefix(GenTraceName(object, tag)),
    m_postfix(postfix),
    m_every(every),
    m_separator(sep)
{
    create_directories(m_dir);
}

RlTrace::~RlTrace()
{
    for (map<string, RlLog*>::iterator iLogs = m_stringLogs.begin(); 
        iLogs != m_stringLogs.end(); ++iLogs)
    {
        delete iLogs->second;
    }
}

RlLog* RlTrace::operator[](const std::string& logname)
{
    map<string, RlLog*>::iterator iLogs = m_stringLogs.find(logname);
    if (iLogs == m_stringLogs.end())
        throw SgException("Referencing log that doesn't exist");
        
    return iLogs->second;
}

const RlLog* RlTrace::operator[](const std::string& logname) const
{
    map<string, RlLog*>::const_iterator iLogs = m_stringLogs.find(logname);
    if (iLogs == m_stringLogs.end())
        throw SgException("Referencing log that doesn't exist");
        
    return iLogs->second;
}

RlLog* RlTrace::operator[](int logid)
{
    map<int, RlLog*>::iterator iLogs = m_integerLogs.find(logid);
    if (iLogs == m_integerLogs.end())
        throw SgException("Referencing log that doesn't exist");
        
    return iLogs->second;
}

const RlLog* RlTrace::operator[](int logid) const
{
    map<int, RlLog*>::const_iterator iLogs = m_integerLogs.find(logid);
    if (iLogs == m_integerLogs.end())
        throw SgException("Referencing log that doesn't exist");
        
    return iLogs->second;
}

void RlTrace::AddLog(const string& logname)
{
    bfs::path fullpath = m_dir / (m_prefix + logname + m_postfix);
    m_stringLogs[logname] = new RlLog(fullpath, m_every, m_separator);
}

void RlTrace::AddLog(const string& logname, int logid)
{
    AddLog(logname);
    m_integerLogs[logid] = m_stringLogs[logname];
}

void RlTrace::AddItemToAll(const string& item, const boost::any& defaultvalue)
{
    for (map<string, RlLog*>::iterator iLogs = m_stringLogs.begin(); 
        iLogs != m_stringLogs.end(); ++iLogs)
    {
        iLogs->second->AddItem(item, defaultvalue);
    }
}

void RlTrace::AddItemToAll(const string& item, int itemid,
    const boost::any& defaultvalue)
{
    for (map<string, RlLog*>::iterator iLogs = m_stringLogs.begin(); 
        iLogs != m_stringLogs.end(); ++iLogs)
    {
        iLogs->second->AddItem(item, itemid, defaultvalue);
    }
}

bool RlTrace::ExistsLog(const string& logname) const
{
    for (map<string, RlLog*>::const_iterator iLogs = m_stringLogs.begin(); 
        iLogs != m_stringLogs.end(); ++iLogs)
    {
        if (iLogs->first == logname)
            return true;
    }
            
    return false;
}

bool RlTrace::ExistsLog(int logid) const
{
    for (map<int, RlLog*>::const_iterator iLogs = m_integerLogs.begin(); 
        iLogs != m_integerLogs.end(); ++iLogs)
    {
        if (iLogs->first == logid)
            return true;
    }
            
    return false;
}

bool RlTrace::ExistsItem(const string& item) const
{
    for (map<string, RlLog*>::const_iterator iLogs = m_stringLogs.begin(); 
        iLogs != m_stringLogs.end(); ++iLogs)
    {
        if (iLogs->second->Exists(item))
            return true;
    }
            
    return false;
}

bool RlTrace::ExistsItem(int itemid) const
{
    for (map<string, RlLog*>::const_iterator iLogs = m_stringLogs.begin(); 
        iLogs != m_stringLogs.end(); ++iLogs)
    {
        if (iLogs->second->Exists(itemid))
            return true;
    }
            
    return false;
}

void RlTrace::StepAll()
{
    for (map<string, RlLog*>::iterator iLogs = m_stringLogs.begin(); 
        iLogs != m_stringLogs.end(); ++iLogs)
    {
        iLogs->second->Step();
    }
}

bool RlTrace::Active()
{
    for (map<string, RlLog*>::iterator iLogs = m_stringLogs.begin(); 
        iLogs != m_stringLogs.end(); ++iLogs)
    {
        if (iLogs->second->Active())
            return true;
    }
            
    return false;
}

string RlTrace::GenTraceName(RlAutoObject* object, 
    const string& tag, int counter)
{
    ostringstream namestr;
    
    if (object)
    {
        string name = RlGetFactory().GetID(object);
        if (name.empty())
            namestr << "Default" << object->GetName();
        else
            namestr << name;
    }
    
    namestr << tag;

    int process = GetProcess();
    if (process >= 0)
        namestr << "P" << process;
    if (counter >= 0)
        namestr << "N" << counter;
        
    return namestr.str();
}

//----------------------------------------------------------------------------


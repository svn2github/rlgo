//----------------------------------------------------------------------------
/** @file RlTrace.h
    Class for tracing evolution of specified sets of variables.
*/
//----------------------------------------------------------------------------

#ifndef RLTRACE_H
#define RLTRACE_H

#include "RlUtils.h"

#include <map>
#include <boost/any.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/path.hpp>

class RlAutoObject;

namespace bfs = boost::filesystem;

//----------------------------------------------------------------------------
/** Class logging the evolution of several items, referenced by name or ID */
class RlLog
{
public:

    RlLog(const bfs::path& logpath, int every = 1, char sep = '\t');
    RlLog(RlAutoObject* object, 
        const std::string& tag = "",
        const std::string& ext = ".txt",
        int every = 1, char sep = '\t');

    /** Create a new column for specified item, referenced by name */
    void AddItem(const std::string& item, const boost::any& defaultval = 0);

    /** Create a new column for specified item, referenced by ID */
    void AddItem(const std::string& item, int itemid, 
        const boost::any& defaultvalue = 0);

    /** Output current values and move log onto a new time step */
    void Step();

    /** Log doesn't have to be active every time step (avoids huge files) */
    bool Active() const { return m_every && (m_steps % m_every == 0); }    

    /** Log the current value for an item specified by name */
    void Log(const std::string& item, const boost::any& value);

    /** Log the current value for an item specified by ID */
    void Log(int itemid, const boost::any& value);

    /** Check whether the specified item name exists */
    bool Exists(const std::string& item) const;

    /** Check whether the specified item ID exists */
    bool Exists(int itemid) const;

    /** Generate a filename of the form OutputPath/ObjectTag[-Process].ext */
    static bfs::path GenLogName(
        RlAutoObject* object, 
        const std::string& tag = "",
        const std::string& ext = ".txt",
        int counter = -1);

private:

    bfs::ofstream m_logFile;
    char m_separator;
    bool m_started;
    bool m_flushLines;
    int m_steps;
    int m_every;

    std::map<int, int> m_columns;
    std::map<std::string, int> m_stringColumns;
    std::vector<boost::any> m_defaults;
    std::vector<boost::any> m_values;
};

//----------------------------------------------------------------------------
/** Class managing a related set of log files.
    For example, the items could be features, and the log files could be
    different properties of those features (weight, eligibility, etc.) */
class RlTrace
{
public:

    RlTrace(const bfs::path& dir, 
        const std::string& prefix = "",
        const std::string& postfix = ".txt",
        int every = 1, char sep = '\t');
    RlTrace(RlAutoObject* object, 
        const std::string& tag = "", 
        const std::string& postfix = ".txt",
        int every = 1, char sep = '\t');
    ~RlTrace();
        
    /** Create a new log */
    void AddLog(const std::string& logname);

    /** Create a new log, referenced by integer identifier */
    void AddLog(const std::string& logname, int logid);

    /** Get log by name */
    RlLog* operator[](const std::string& logname);
    const RlLog* operator[](const std::string& logname) const;

    /** Get log by integer identifier */
    RlLog* operator[](int logid);
    const RlLog* operator[](int logid) const;

    /** Add an item specified by name to all logs */
    void AddItemToAll(const std::string& item,
        const boost::any& defaultvalue = 0);

    /** Add an item specified by ID to all logs */
    void AddItemToAll(const std::string& item, int itemid,
        const boost::any& defaultvalue = 0);

    /** Does log with this name exist? */
    bool ExistsLog(const std::string& logname) const;

    /** Does log with this integer identifier exist? */
    bool ExistsLog(int logid) const;
    
    /** Does item specified by name exist in any log? */
    bool ExistsItem(const std::string& item) const;

    /** Does item specified by ID exist in any log? */
    bool ExistsItem(int itemid) const;

    /** When active all traces are stepped */
    void StepAll();
        
    /** Are any of the logs active? */
    bool Active();    
    
    /** Generate a name of the form ObjectTag[PProcess][NCounter] */
    static std::string GenTraceName(
        RlAutoObject* object, 
        const std::string& tag = "",
        int counter = -1);
    
private:

    bfs::path m_dir;
    std::string m_prefix;
    std::string m_postfix;
    int m_every;
    char m_separator;

    std::map<int, RlLog*> m_integerLogs;
    std::map<std::string, RlLog*> m_stringLogs;
};

//----------------------------------------------------------------------------

#endif // RLTRACE_H


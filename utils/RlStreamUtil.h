//---------------------------------------------------------------------------
/** @file RlStreamUtil.h
    Streaming utility functions
*/
//---------------------------------------------------------------------------

#ifndef RLSTREAMUTIL_H
#define RLSTREAMUTIL_H

#include "RlFactory.h"
#include "RlMiscUtil.h"
#include "SgException.h"
#include "SgSystem.h"
#include <boost/filesystem/path.hpp>
#include <boost/lexical_cast.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

namespace bfs = boost::filesystem;

//---------------------------------------------------------------------------
/** Rl streaming commands:

@verbatim
COMMAND       ASCII         BINARY

RlSetting<T>  foo = X\n     X
RlLongSetting foo = <X>\n   X
RlVersion     m_version = V\n V
RlToken       foo           foo
RlNewLine     \n            
RlIndent      \t\t...
RlComment     #X

RlSkipTo: skip to the end of the next matching token
RlSkipToBracket: skip to corresponding closing bracket
RlIndentor: increases indent size for duration of scope
RlMode: selects binary or ascii for duration of scope
@endverbatim

*/

//---------------------------------------------------------------------------
/** stream I/O for reading/writing a variable in form: "Variable = value\n"*/
template <class T>
struct RlSetting
{
    RlSetting(const std::string& token, T& object)
    :   m_token(token),
        m_object(object)
    {
    }
    
    std::string m_token;
    mutable T& m_object; 
        // allow temporary RlSetting objects to modify m_object
};

//---------------------------------------------------------------------------
/** stream I/O for reading/writing a variable in form: 
    Variable = "a long string"\n 
*/
struct RlLongSetting
{
    RlLongSetting(const std::string& token, std::string& object)
    :   m_token(token),
        m_object(object)
    {
    }
    
    std::string m_token;
    mutable std::string& m_object; 
        // allow temporary RlSetting objects to modify m_object
};

//---------------------------------------------------------------------------
/** Struct that can be used in stream I/O for version checking */
struct RlVersion
{
    /** Constructor to read a version and check its range */
    RlVersion(int& version, int current, int oldest = 0)
    :   m_version(version), m_current(current), m_oldest(oldest) 
    {
    }

    /** Constructor to write current version */
    RlVersion(int current)
    :   m_version(m_current), m_current(current)
    {
    }
    
    mutable int& m_version;
    int m_current, m_oldest;
};

//---------------------------------------------------------------------------
/** Read and write a single token string */
class RlToken
{
public:
    RlToken(const std::string& token)
    :   m_token(token) {}
    
    static void ReadString(std::istream& i, std::string& s);
    static void WriteString(std::ostream& o, const std::string& s);
    static void ReadStringVector(std::istream& i, 
        std::vector<std::string>& ids);

    std::string m_token;
    
    enum { MaxStr = 256 };
};

//---------------------------------------------------------------------------
/** Newline object */
class RlNewLine
{
};

//---------------------------------------------------------------------------
/** Indent object */
class RlIndent
{
};

//---------------------------------------------------------------------------
/** Indent settings output for the duration of object existence */
class RlIndentor
{
public:
    RlIndentor();
    ~RlIndentor();

    static std::string& GetPrefix() { return s_prefix; }

private:
    std::string m_oldPrefix;
    static std::string s_prefix;
};

//---------------------------------------------------------------------------
/** Select output mode for the duration of object existence */
class RlMode
{
public:
    RlMode(int mode);
    ~RlMode();

    /** Set mode from extension:
        If .txt use Ascii, if .bin use Binary */
    RlMode(const bfs::path& filename);
    
    static int GetMode() { return s_mode; }
    
    static std::ios::openmode OpenMode(std::ios::openmode mode)
    {
        if (s_mode == Ascii)
            return mode;
        else
            return mode | std::ios::binary;
    }
    
    enum 
    { 
        Ascii,
        Binary 
    };
    
private:
    int m_oldMode;
    static int s_mode;
};

//---------------------------------------------------------------------------
/** Struct that can be used in stream input to skip until specified token */
struct RlSkipTo
{
public:

    RlSkipTo(const std::string& token);

    bool Match() const;
    void Append(char c) const;

private:

    const std::string& m_token;
    int m_size;

    // allow temporary RlSkipTo objects to modify buffer and cursor
    mutable char m_buffer[RlToken::MaxStr];
    mutable int m_cursor;
};

//---------------------------------------------------------------------------
/** Struct that can be used in stream input to skip until matching bracket */
struct RlSkipToBracket
{
public:

    RlSkipToBracket(char open, char close, int parity = 0);

    /** Use this version to store contents up until close bracket */
    RlSkipToBracket(char open, char close, int parity, 
        std::string& contents);
    
    bool Match(char c) const;

    std::string& GetContents() const { return m_contentRef; }

private:

    char m_open, m_close;
    mutable bool m_opened;
    mutable int m_parity;
    mutable std::string m_contents;
    mutable std::string& m_contentRef;
};

//---------------------------------------------------------------------------
/** Struct to read/write comments from a stream */
struct RlComment
{
public:

    RlComment(char delimiter = '#')
    :  m_delimiter(delimiter)
    { }
    
    RlComment(const std::string& comment, char delimiter = '#')
    :   m_comment(comment),
        m_delimiter(delimiter)
    { }
    
    mutable std::string m_comment;
    char m_delimiter;
};

//---------------------------------------------------------------------------

std::istream& operator>>(std::istream& i, const RlLongSetting& setting);
std::ostream& operator<<(std::ostream& o, const RlLongSetting& setting);
std::istream& operator>>(std::istream& i, const RlVersion& version);
std::ostream& operator<<(std::ostream& o, const RlVersion& version);
std::istream& operator>>(std::istream& i, const RlToken& token);
std::ostream& operator<<(std::ostream& o, const RlToken& token);
std::istream& operator>>(std::istream& i, const RlNewLine& newline);
std::ostream& operator<<(std::ostream& o, const RlNewLine& newline);
std::istream& operator>>(std::istream& i, const RlIndent& indent);
std::ostream& operator<<(std::ostream& o, const RlIndent& indent);
std::istream& operator>>(std::istream& i, const RlSkipTo& skipto);
std::istream& operator>>(std::istream& i, const RlSkipToBracket& skiptob);
std::istream& operator>>(std::istream& i, const RlComment& comment);
std::ostream& operator<<(std::ostream& o, const RlComment& comment);

//---------------------------------------------------------------------------
// Basic setting

template <class T>
inline std::istream& operator>>(std::istream& i, const RlSetting<T>& setting)
{
    if (RlMode::GetMode() == RlMode::Ascii)
    {
        i >> RlToken(setting.m_token) >> RlToken("=") >> setting.m_object;
    }
    else
    {
        i.read(
            reinterpret_cast<char*>(&setting.m_object), 
            sizeof(setting.m_object));
    }
    
    RlGetFactory().GetOverride(setting.m_token, setting.m_object);
    i >> RlComment();
    return i;
}

template <class T>
inline std::ostream& operator<<(std::ostream& o, const RlSetting<T>& setting)
{
    if (RlMode::GetMode() == RlMode::Ascii)
    {
        o << RlIndent() << setting.m_token << " = " 
            << setting.m_object << "\n";
    }
    else
    {
        o.write(
            reinterpret_cast<char*>(&setting.m_object), 
            sizeof(setting.m_object));
    }
    
    return o;
}

//---------------------------------------------------------------------------
// String setting

inline std::istream& operator>>(std::istream& i,
                                const RlSetting<std::string>& setting)
{
    if (RlMode::GetMode() == RlMode::Ascii)
    {
        i >> RlToken(setting.m_token) >> RlToken("=") >> setting.m_object;
    }
    else
    {
        RlToken::ReadString(i, setting.m_object);
    }
                  
    RlGetFactory().GetOverride(setting.m_token, setting.m_object);
    i >> RlComment();
    return i;
}

inline std::ostream& operator<<(std::ostream& o, 
                                const RlSetting<std::string>& setting)
{
    if (RlMode::GetMode() == RlMode::Ascii)
    {
        o << RlIndent() << setting.m_token << " = " 
            << setting.m_object << "\n";
    }
    else
    {
        RlToken::WriteString(o, setting.m_object);
    }
    
    return o;
}

//---------------------------------------------------------------------------
// Pointer setting

template <class T>
inline std::istream& operator>>
    (std::istream& i, 
     const RlSetting<T*>& setting)
{
    std::string id;
    if (RlMode::GetMode() == RlMode::Ascii)
    {
        i >> RlToken(setting.m_token) >> RlToken("=") >> id;
    }
    else
    {
        RlToken::ReadString(i, id);
    }

    RlGetFactory().GetOverride(setting.m_token, id);

    if (id == "NULL")
    {
        setting.m_object = 0;
    }
    else 
    {
        setting.m_object = dynamic_cast<T*>(RlGetFactory().GetObject(id));
        if (!setting.m_object)
            throw SgException("Bad cast");
    }

    i >> RlComment();
    return i;
}

template <class T>
inline std::ostream& operator<<
    (std::ostream& o, 
     const RlSetting<T*>& setting)
{
    std::string id;
    if (setting.m_object)
        id = RlGetFactory().GetID(setting.m_object);
    else
        id = "NULL";

    if (RlMode::GetMode() == RlMode::Ascii)
    {
        o << RlIndent() << setting.m_token << " = " << id << "\n";
    }
    else
    {
        RlToken::WriteString(o, id);
    }
    
    return o;
}

//---------------------------------------------------------------------------
// Basic vector setting

template <class T>
inline std::istream& operator>>
    (std::istream& i, 
     const RlSetting<std::vector<T> >& setting)
{
    std::string id, rhs;
    std::vector<std::string> stringvec;

    if (RlMode::GetMode() == RlMode::Ascii)
    {
        i >> RlToken(setting.m_token) >> RlToken("=");
        RlToken::ReadStringVector(i, stringvec);
    }
    else
    { 
        int numstr;
        i.read(reinterpret_cast<char*>(&numstr), sizeof(int));
        for (int j = 0; j < numstr; ++j)
        {
            std::string str;
            RlToken::ReadString(i, str);
            stringvec.push_back(str);
        }
    }    

    if (RlGetFactory().GetOverride(setting.m_token, rhs))
    {
        std::istringstream iss(rhs);
        RlToken::ReadStringVector(iss, stringvec);
    }
    
    setting.m_object.clear();
    for (int j = 0; j < ssize(stringvec); ++j)
    {
        T object = boost::lexical_cast<T>(stringvec[j]);
        setting.m_object.push_back(object);
    }

    i >> RlComment();
    return i;
}

template <class T>
inline std::ostream& operator<<
    (std::ostream& o, 
     const RlSetting<std::vector<T> >& setting)
{
    std::vector<std::string> stringvec;
    for (unsigned int j = 0; j < setting.m_object.size(); ++j)
    {
        std::string str = boost::lexical_cast<std::string>(setting.m_object[j]);
        stringvec.push_back(str);
    }

    std::vector<std::string> ids;
    int numobjects = ssize(stringvec);

    if (RlMode::GetMode() == RlMode::Ascii)
    {
        o << RlIndent() << setting.m_token << " = " << numobjects << " [ ";
        for (int j = 0; j < numobjects; ++j)
            o << stringvec[j] << " ";
        o << "]\n";
    }
    else
    {
        o.write((char*) (&numobjects), sizeof(int));
        for (int j = 0; j < numobjects; ++j)
            RlToken::WriteString(o,stringvec[j]);
    }
    
    return o;
}

//---------------------------------------------------------------------------
// Pointer vector setting

template <class T>
inline std::istream& operator>>
    (std::istream& i, 
     const RlSetting<std::vector<T*> >& setting)
{
    std::string id, rhs;
    std::vector<std::string> ids;

    if (RlMode::GetMode() == RlMode::Ascii)
    {
        i >> RlToken(setting.m_token) >> RlToken("=");
        RlToken::ReadStringVector(i, ids);
    }
    else
    { 
        int numobjects;
        i.read(reinterpret_cast<char*>(&numobjects), sizeof(int));
        for (int j = 0; j < numobjects; ++j)
        {
            RlToken::ReadString(i, id);
            ids.push_back(id);
        }
    }

    if (RlGetFactory().GetOverride(setting.m_token, rhs))
    {
        std::istringstream iss(rhs);
        RlToken::ReadStringVector(iss, ids);
    }

    setting.m_object.clear();
    for (int j = 0; j < ssize(ids); ++j)
    {
        if (ids[j] == "NULL")
        {
            setting.m_object.push_back(0);
            continue;
        }
        
        T* object = dynamic_cast<T*>(RlGetFactory().GetObject(ids[j]));
        if (!object)
            throw SgException("Bad cast");
        setting.m_object.push_back(object);
    }

    i >> RlComment();
    return i;
}

template <class T>
inline std::ostream& operator<<
    (std::ostream& o, 
     const RlSetting<std::vector<T*> >& setting)
{
    std::vector<std::string> ids;
    int numobjects = ssize(setting.m_object);
    for (int j = 0; j < numobjects; ++j)
    {
        if (setting.m_object[j])
            ids.push_back(RlGetFactory().GetID(setting.m_object[j]));
        else
            ids.push_back("NULL");
    }

    if (RlMode::GetMode() == RlMode::Ascii)
    {
        o << RlIndent() << setting.m_token << " = " << numobjects << " [ ";
        for (int j = 0; j < numobjects; ++j)
            o << ids[j] << " ";
        o << "]\n";
    }
    else
    {
        o.write(reinterpret_cast<char*>(&numobjects), sizeof(int));
        for (int j = 0; j < numobjects; ++j)
            RlToken::WriteString(o, ids[j]);
    }
    
    return o;
}

//---------------------------------------------------------------------------

#endif // RLSTREAMUTILS_H


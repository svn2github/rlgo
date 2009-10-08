//---------------------------------------------------------------------------
/** @file RlStreamUtil.cpp
    @see RlStreamUtil.h
*/
//---------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgException.h"
#include "RlStreamUtil.h"
#include <boost/filesystem/convenience.hpp>
#include <sstream>

using namespace std;

//---------------------------------------------------------------------------

std::istream& operator>>(std::istream& i, const RlLongSetting& setting)
{
    if (RlMode::GetMode() == RlMode::Ascii)
    {
        i >> RlToken(setting.m_token) >> RlToken("=");
        i >> RlSkipTo("<") >> RlSkipToBracket('<', '>', 1, setting.m_object);
    }
    else
    {
        i.read(
            reinterpret_cast<char*>(&setting.m_object), 
            sizeof(setting.m_object));
    }
    
    i >> RlComment();
    return i;
}

std::ostream& operator<<(std::ostream& o, const RlLongSetting& setting)
{
    if (RlMode::GetMode() == RlMode::Ascii)
    {
        o << RlIndent() << setting.m_token << " = ";
        o << '<' << setting.m_object << '>' << "\n";
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

istream& operator>>(istream& i, const RlVersion& version)
{
    i >> RlSetting<int>("Version", version.m_version);
    if (version.m_version < version.m_oldest 
        || version.m_version > version.m_current)
        throw SgException("Unsupported version");
        
    return i;
}

ostream& operator<<(ostream& o, const RlVersion& version)
{
    o << RlSetting<int>("Version", version.m_version);
    
    return o;
}

//---------------------------------------------------------------------------

void RlToken::ReadString(istream& i, string& s)
{
    char buf[MaxStr];
    int size = i.get();
    i.read(buf, size);
    buf[size] = '\0';
    s = std::string(buf);
}

void RlToken::WriteString(ostream& o, const string& s)
{
    if (ssize(s) >= MaxStr)
        throw SgException("String exceeds maximum length");
    char size = static_cast<char>(s.size());
    o.put(size);
    o.write(s.c_str(), size);
}

void RlToken::ReadStringVector(std::istream& i,std::vector<std::string>& ids)
{
    std::string id;
    int numobjects;
    ids.clear();
    i >> numobjects >> RlSkipTo("[");
    int j = 0;
    i >> id;
    while (id.compare("]") != 0 && (j <= (numobjects + 1)))
    {
        ids.push_back(id);
        ++j;
        i >> id;
    }
    if (numobjects != j)
    {
        ostringstream ostr;
        ostr << "Expecting " << numobjects << " in vector, seeing " 
            << j << " elements\n";
        throw SgException("The Settings File has an"
            "incompatible number of elements in a vector\n");
     } 
    if (id.compare("]") != 0)
        i >> RlSkipToBracket('[', ']', 1);
}

istream& operator>>(istream& i, const RlToken& token)
{
    std::string intoken;
    
    if (!i)
        throw SgException("Failed to read token");
    
    if (RlMode::GetMode() == RlMode::Ascii)
    {
        i >> intoken;
    }
    else
    {
        RlToken::ReadString(i, intoken);
    }

    if (intoken != token.m_token)
    {
        ostringstream ostr;
        ostr << "Unmatched token: expected \"" << token.m_token
             << "\" and encountered \"" << intoken << "\"";
        throw SgException(ostr.str());
    }
    
    return i;
}

ostream& operator<<(ostream& o, const RlToken& token)
{
    if (RlMode::GetMode() == RlMode::Ascii)
    {
        o << token.m_token;
    }
    else
    {
        RlToken::WriteString(o, token.m_token);
    }

    return o;
}

//---------------------------------------------------------------------------

RlSkipTo::RlSkipTo(const string& token)
:   m_token(token),
    m_size(ssize(m_token)),
    m_cursor(0)
{
    SG_ASSERT(m_size < RlToken::MaxStr);
    for (int i = 0; i < m_size; ++i)
        m_buffer[i] = -1;
}

bool RlSkipTo::Match() const
{
    for (int i = 0; i < m_size; ++i)
        if (m_buffer[(m_cursor + i) % m_size] != m_token[i])
            return false;
     
    return true;
}
    
void RlSkipTo::Append(char c) const
{
    m_buffer[m_cursor] = c;
    m_cursor++;
    if (m_cursor >= m_size)
        m_cursor = 0;
}

istream& operator>>(istream& i, const RlSkipTo& skip)
{
    while (!skip.Match())
    {
        char c = i.get();
        if (i.eof())
            throw SgException("Match not found during RlSkipTo");
        skip.Append(c);
    }

    return i;
}

//---------------------------------------------------------------------------

RlSkipToBracket::RlSkipToBracket(char open, char close, int parity)
:   m_open(open), 
    m_close(close), 
    m_opened(parity >= 0), 
    m_parity(parity),
    m_contentRef(m_contents)
{
}

RlSkipToBracket::RlSkipToBracket(char open, char close, int parity,
    string& contents)
:   m_open(open), 
    m_close(close), 
    m_opened(parity >= 0), 
    m_parity(parity),
    m_contentRef(contents)
{
}

bool RlSkipToBracket::Match(char c) const
{
    if (c == m_open)
    {
        m_parity++;
        m_opened = true;
    }
    
    if (c == m_close)
        m_parity--;
        
    if (m_parity < 0)
        throw SgException("More closing than opening brackets");

    return m_opened && m_parity == 0;
}

istream& operator>>(istream& i, const RlSkipToBracket& skip)
{
    bool match = false;
    while (!match)
    {
        char c = i.get();
        if (i.eof())
            throw SgException("Match not found during RlSkipToBracket");
        match = skip.Match(c);
        if (!match)
            skip.GetContents().push_back(c);
    }

    return i;
}

//---------------------------------------------------------------------------

istream& operator>>(istream& i, const RlIndent& indent)
{
    SG_UNUSED(i);
    SG_UNUSED(indent);
    
    return i;
}

ostream& operator<<(ostream& o, const RlIndent& indent)
{
    SG_UNUSED(indent);

    if (RlMode::GetMode() == RlMode::Ascii)
        o << RlIndentor::GetPrefix();
        
    return o;
}

//---------------------------------------------------------------------------

istream& operator>>(istream& i, const RlNewLine& newline)
{
    SG_UNUSED(i);
    SG_UNUSED(newline);
    
    return i;
}

ostream& operator<<(ostream& o, const RlNewLine& newline)
{
    SG_UNUSED(newline);
    
    if (RlMode::GetMode() == RlMode::Ascii)
        o << "\n";
        
    return o;
}

//---------------------------------------------------------------------------

string RlIndentor::s_prefix = "";

RlIndentor::RlIndentor()
:   m_oldPrefix(s_prefix)
{
    s_prefix += "\t";
}

RlIndentor::~RlIndentor()
{
    s_prefix = m_oldPrefix;
}

//---------------------------------------------------------------------------

int RlMode::s_mode = RlMode::Ascii;

RlMode::RlMode(int mode)
:   m_oldMode(s_mode)
{
    s_mode = mode;
}

RlMode::~RlMode()
{
    s_mode = m_oldMode;
}

RlMode::RlMode(const bfs::path& filename)
:   m_oldMode(s_mode)
{
    if (bfs::extension(filename) == ".txt")
        s_mode = Ascii;
    else if (bfs::extension(filename) == ".bin")
        s_mode = Binary;
    else
        throw SgException("Extension is neither .txt nor .bin");
}

//---------------------------------------------------------------------------

istream& operator>>(istream& i, const RlComment& comment)
{
    std::string incomment;
    
    if (!i)
        throw SgException("Failed to read comment");
    
    // Comments are no-ops in binary mode
    if (RlMode::GetMode() == RlMode::Ascii)
    {
        char buffer[RlToken::MaxStr];
        i.getline(buffer, RlToken::MaxStr);
        string s = string(buffer);
        string::size_type start = s.find(comment.m_delimiter);
        if (start == s.npos)
            comment.m_comment = string("");
        else
            comment.m_comment = s.substr(start + 1);
    }
    
    return i;
}

ostream& operator<<(ostream& o, const RlComment& comment)
{
    if (RlMode::GetMode() == RlMode::Ascii)
    {
        o << RlIndent() << comment.m_delimiter << comment.m_comment << "\n";
    }

    return o;
}

//---------------------------------------------------------------------------

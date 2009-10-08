//----------------------------------------------------------------------------
/** @file RlFactory.cpp
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlFactory.h"

#include "SgDebug.h"
#include "SgException.h"
#include "RlUtils.h"
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>

using namespace std;

//----------------------------------------------------------------------------

RlAutoObject::RlAutoObject(GoBoard& board)
:   m_board(board),
    m_initialised(false)
{
}

RlAutoObject::~RlAutoObject()
{
}

void RlAutoObject::Initialise()
{
    SG_ASSERT(! m_initialised);
}

void RlAutoObject::EnsureInitialised()
{
    static int tabs = 0;
    
    if (!m_initialised)
    {
        string myID = RlGetFactory().GetID(this);
        tabs++;
        Initialise();
        tabs--;
    }
    m_initialised = true;
} 

void RlAutoObject::LoadSettings(istream& settings)
{
    SG_UNUSED(settings);
}

void RlAutoObject::SaveSettings(ostream& settings)
{
    SG_UNUSED(settings);
}

//----------------------------------------------------------------------------

RlRegister::RlRegister(const string& name, RlFactoryFn factoryfn)
{
    RlGetFactory().Register(name, factoryfn);
}

//----------------------------------------------------------------------------

RlFactory::RlFactory()
:   m_enabled(true)
{
}

RlFactory::~RlFactory()
{
    Clear();
}

void RlFactory::Clear()
{
    for (vector<RlAutoObject*>::iterator i_object = m_objectVector.begin();
        i_object != m_objectVector.end(); ++i_object)
    {
        delete *i_object;
    }
    
    m_objects.clear();
    m_objectVector.clear();
    m_IDs.clear();
}

void RlFactory::Register(const string& name, RlFactoryFn factoryfn)
{
    SG_ASSERT(m_registry.find(name) == m_registry.end());
    m_registry[name] = factoryfn;
}

RlAutoObject* RlFactory::GetObject(const string& id)
{
    if (m_objects.find(id) == m_objects.end())
    {
        ostringstream ss;
        ss << "No object in factory with id: " << id;
        throw SgException(ss.str());
    }

    return m_objects[id];
}

string RlFactory::GetID(RlAutoObject* object)
{
    if (m_IDs.find(object) == m_IDs.end())
        return "";
    else
        return m_IDs[object];
}

void RlFactory::AddObject(RlAutoObject* object, const string& id)
{
    if (!object)
        throw SgException("Adding null object");

    if (m_objects.find(id) != m_objects.end())
    {
        ostringstream ss;
        ss << "Duplicate object ID: " << id;
        throw SgException(ss.str());
    }

    m_objects[id] = object;
    m_objectVector.push_back(object);
    m_IDs[object] = id;
}

RlAutoObject* RlFactory::CreateObject(GoBoard& board, const string& name)
{
    map<string, RlFactoryFn>::iterator iRegistry = m_registry.find(name);
    if (iRegistry == m_registry.end())
    {
        SgDebug() << "Class " << name << " not in registry.\n" 
            << "Registry contents:\n";
        for (std::map<string, RlFactoryFn>::iterator 
             iRegistry2 = m_registry.begin();
             iRegistry2 != m_registry.end();
             iRegistry2++)
        {
            string classname = iRegistry2->first;
            SgDebug() << classname << endl;
        }
        
        ostringstream ss;
        ss << "Class " << name << " not found in registry";
        throw SgException(ss.str());
    }

    RlFactoryFn factoryfn = iRegistry->second;
    return factoryfn(board);
}

void RlFactory::Save(const bfs::path& filename)
{
    bfs::ofstream settings(filename);
    if (!settings)
    {
        ostringstream ss;
        ss << "Couldn't save " << filename.string();
        throw SgException(ss.str());
    }

    for (map<string, RlAutoObject*>::iterator i_object = m_objects.begin();
        i_object != m_objects.end();
        ++i_object)
    {
        string id = i_object->first;
        RlAutoObject* object = i_object->second;
        string name = object->GetName();
        settings << RlSetting<string>("Object", name);
        settings << "{\n";
        RlIndentor indentor;
        settings << RlSetting<string>("ID", id);
        object->SaveSettings(settings);
        settings << "}\n\n";
        settings << flush;
    }
}

void RlFactory::Load(GoBoard& board, const bfs::path& filename)
{
    AllocateAllObjects(board, filename);
    LoadAllObjects(board, filename);
}

bfs::path RlFactory::GetFullPath(const bfs::path& filename)
{
    bfs::path fullpath;
    if (exists(filename))
        fullpath = filename;
    else if (exists(m_defaultPath / filename))
        fullpath = m_defaultPath / filename;
    else
    {
        ostringstream ss;
        ss << "Settings file not found: " << filename.string();
        throw SgException(ss.str());
    }
    
    return fullpath;
}

void RlFactory::AllocateAllObjects(GoBoard& board, const bfs::path& filename)
{
    bfs::path fullpath = GetFullPath(filename);
    bfs::ifstream settings(fullpath);
    if (!settings)
    {
        ostringstream ss;
        ss << "Couldn't open " << filename.string();
        throw SgException(ss.str());
    }
    
    SgDebug() << "Loading objects from settings file: " 
        << fullpath.string() << " ...";
    AllocateObjects(board, settings);
}

void RlFactory::LoadAllObjects(GoBoard& board, const bfs::path& filename)
{
    SG_UNUSED(board);
    bfs::path fullpath = GetFullPath(filename);
    bfs::ifstream settings(GetFullPath(fullpath));
    if (!settings)
    {
        ostringstream ss;
        ss << "Couldn't open " << filename.string();
        throw SgException(ss.str());
    }

    LoadObjects(settings);
    SgDebug() << " done\n";
}

void RlFactory::AllocateObjects(GoBoard& board, istream& settings)
{
    // First pass through file to allocate all objects
    while (NextObject(settings))
    {
        string name;
        settings >> RlSetting<string>("Object", name);
        RlAutoObject* object = CreateObject(board, name);
        settings >> RlSkipTo("{") >> ws;
        settings >> RlSetting<string>("ID", m_currentID);
        AddObject(object, m_currentID);
        RlInclude* include = dynamic_cast<RlInclude*>(object);
        if (include)
            include->Allocate(settings);
        settings >> RlSkipToBracket('{', '}', 1) >> ws;        
    }
}

void RlFactory::LoadObjects(istream& settings)
{
    // Second pass through file to load all object settings
    while (NextObject(settings))
    {
        string name;
        settings >> RlSetting<string>("Object", name);
        settings >> RlSkipTo("{") >> ws;
        settings >> RlSetting<string>("ID", m_currentID);
        RlAutoObject* object = GetObject(m_currentID);
        object->LoadSettings(settings);
        RlInclude* include = dynamic_cast<RlInclude*>(object);
        if (include)
            include->Load(settings);
        settings >> RlSkipToBracket('{', '}', 1) >> ws;
    }
}

void RlFactory::Initialise()
{
    SgDebug() << "Initialising objects from settings file...\n";

    // Initialise Setup object first
    RlAutoObject* setup = GetObject("Setup");
    setup->EnsureInitialised();

    for (vector<RlAutoObject*>::iterator i_object = m_objectVector.begin();
        i_object != m_objectVector.end(); ++i_object)
    {
        RlAutoObject* object = *i_object;
        object->EnsureInitialised();
    }

    SgDebug() << "Initialisation done\n";
}

void RlFactory::SetOverride(const std::string& token, 
    const std::string& value)
{
    // Split token into settings using "+" and override each setting
    string seps("+");
    list<string> settings;
    SplitString(token, seps, settings);
    for (list<string>::iterator i_settings = settings.begin();
        i_settings != settings.end(); ++i_settings)
    {
        SetOneOverride(*i_settings, value);
    }
}

void RlFactory::SetOneOverride(const std::string& token, 
    const std::string& value)
{
    // Replace all instances of "~" in value with spaces
    string replacedvalue = value;
    for (int i = 0; i < ssize(value); ++i)
        if (replacedvalue[i] == '~')
            replacedvalue[i] = ' ';
            
    string::size_type dotpos = token.find('.', 0);
    if (dotpos == string::npos) // Global override
    {
        m_globalOverrides[token] = replacedvalue;
    }
    else // Local override
    {
        string localobject = string(token, 0, dotpos);
        string localtoken = string(token, dotpos + 1, string::npos);
        m_localOverrides[localobject][localtoken] = replacedvalue;
    }
}

bool RlFactory::OverrideExists(const std::string& token, string& override)
{
    // Split token into settings using "+" and override each setting
    string seps("+");
    list<string> settings;
    SplitString(token, seps, settings);
    for (list<string>::iterator i_settings = settings.begin();
        i_settings != settings.end(); ++i_settings)
    {
        if (OneOverrideExists(*i_settings, override))
            return true;
    }
    return false;
}

bool RlFactory::OneOverrideExists(const std::string& token, string& override)
{
    string::size_type dotpos = token.find('.', 0);
    if (dotpos == string::npos) // Global override
    {
        override = m_globalOverrides[token];
        return !override.empty();
    }
    else // Local override
    {
        string localobject = string(token, 0, dotpos);
        string localtoken = string(token, dotpos + 1, string::npos);
        override = m_localOverrides[localobject][localtoken];
        return !override.empty();
    }
}

bool RlFactory::NextObject(istream& settings)
{
    // Eat comments
    string line;
    while (settings.peek() == '#') 
    {
        getline(settings, line);
        settings >> ws;
    }
    
    return !settings.eof();
}

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlInclude);

RlInclude::RlInclude(GoBoard& board)
:   RlAutoObject(board)
{
}

void RlInclude::Allocate(istream& settings)
{
    string include;
    settings >> RlSetting<string>("Include", include);
    if (include != "NULL")
        RlGetFactory().AllocateAllObjects(m_board, include);
}

void RlInclude::Load(istream& settings)
{
    string include;
    settings >> RlSetting<string>("Include", include);
    if (include != "NULL")
        RlGetFactory().LoadAllObjects(m_board, include);
}

//----------------------------------------------------------------------------

IMPLEMENT_OBJECT(RlOverride);

RlOverride::RlOverride(GoBoard& board)
:   RlAutoObject(board)
{
}

void RlOverride::LoadSettings(istream& settings)
{
    int numoverrides;
    string token, value, oldvalue;
    settings >> RlSetting<int>("NumOverrides", numoverrides);
    for (int i = 0; i < numoverrides; ++i)
    {
        settings >> RlSetting<string>("Token", token);
        settings >> RlSetting<string>("Value", value);
        
        if (RlGetFactory().OverrideExists(token, oldvalue))
        {
            SgDebug() << "Keeping previous override: " 
                << token << " = " << oldvalue << "\n";
        }
        else
        {
            RlGetFactory().SetOverride(token, value);
        }
    }
}

//----------------------------------------------------------------------------


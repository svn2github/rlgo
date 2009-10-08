//----------------------------------------------------------------------------
/** @file RlFactory.h
    Automatic class creation functionality
*/
//----------------------------------------------------------------------------

#ifndef RLFACTORY_H
#define RLFACTORY_H

#include <boost/lexical_cast.hpp>
#include <boost/filesystem/path.hpp>
#include <iostream>
#include <map>
#include <vector>

namespace bfs = boost::filesystem;

class GoBoard;

//----------------------------------------------------------------------------
/** Base class for all classes that can be created automatically by name */
class RlAutoObject
{
public:

    RlAutoObject(GoBoard& board);

    virtual ~RlAutoObject();
    virtual std::string GetName() const = 0;

    /** Use this function to call initialise, 
        e.g. to make sure that any objects on which
        initialisation depends are already initialised */
    void EnsureInitialised();

    virtual void LoadSettings(std::istream& settings);
    virtual void SaveSettings(std::ostream& settings);
    
    GoBoard& GetBoard() { return m_board; }
    bool IsInitialised() const { return m_initialised; }
    
protected:

    /** Initialise is called after construction and/or LoadSettings */
    virtual void Initialise();

    GoBoard& m_board;
    bool m_initialised;
};

typedef RlAutoObject* (*RlFactoryFn)(GoBoard& board);

//----------------------------------------------------------------------------
/** Mechanism for automatically registering AutoObject classes */
class RlRegister
{
public:

    RlRegister(const std::string& name, RlFactoryFn fn);
};

//----------------------------------------------------------------------------
/** Class for loading and saving AutoObject classes automatically */
class RlFactory
{
public:

    RlFactory();
    ~RlFactory();

    /** Get an object from the factory by id */
    RlAutoObject* GetObject(const std::string& id);

    /** Get an ID for an object in the factory */
    std::string GetID(RlAutoObject* object);

    /** Add an object manually to the factory */
    void AddObject(RlAutoObject* object, const std::string& id);

    /** Create an object of named type */
    RlAutoObject* CreateObject(GoBoard& board, const std::string& name);

    /** Specify default path for settings files */
    void SetDefaultPath(const bfs::path& path) { m_defaultPath = path; }

    /** Load objects from settings file into factory */
    void Load(GoBoard& board, const bfs::path& filename);

    /** Save objects from factory into settings file */
    void Save(const bfs::path& filename);

    /** Register a class into the factory */
    void Register(const std::string& name, RlFactoryFn fn);

    /** Initialise all objects.
        This should be called after construction and settings are loaded */
    void Initialise();

    /** Delete all objects */
    void Clear();

    void EnableOverrides(bool enabled) { m_enabled = enabled; }

    /** Set setting override. 
        Plus '+' in token will be split into separate settings,
            each overridden with the same value
        Tilde '~' in value will be replaced by spaces.
        A dot '.' in token specifies a local override: "object.parameter"
        If there is no dot in token then override is global. */        
    void SetOverride(const std::string& token, const std::string& value);

    /** Check if specified override already exists
        (old overrides are not replaced) */
    bool OverrideExists(const std::string& token, std::string& override);
    
    /** Get global setting override (returns false if no override) */
    template <class T> bool GetOverride(const std::string& token, 
        T& value);

    /** Get all objects of specified type */
    template <class T> void GetAll(std::vector<T*>& objects);

protected:

    bfs::path GetFullPath(const bfs::path& filename);
    void AllocateAllObjects(GoBoard& board, const bfs::path& filename);
    void LoadAllObjects(GoBoard& board, const bfs::path& filename);
    void AllocateObjects(GoBoard& board, std::istream& settings);
    void LoadObjects(std::istream& settings);
    bool NextObject(std::istream& settings);
    void SetOneOverride(const std::string& token, const std::string& value);
    bool OneOverrideExists(const std::string& token, std::string& override);

private:

    bfs::path m_defaultPath;
    std::map<std::string, RlFactoryFn> m_registry;
    std::map<std::string, RlAutoObject*> m_objects;
    std::vector<RlAutoObject*> m_objectVector;
    std::map<RlAutoObject*, std::string> m_IDs;
    std::string m_currentID;
    std::map<std::string, std::string> m_globalOverrides;
    std::map<std::string, std::map<std::string, std::string> >
        m_localOverrides;
    bool m_enabled;

friend class RlInclude;
};

/** Factory singleton */
inline RlFactory& RlGetFactory()
{
    static RlFactory factory;
    return factory;
}

/** Get global setting override (returns false if no override) */
template <class T> bool RlFactory::GetOverride(
    const std::string& token, T& value)
{
    if (!m_enabled)
        return false;

    std::string override = m_globalOverrides[token];
    if (override.empty())
        override = m_localOverrides[m_currentID][token];        
    if (override.empty())
        return false;

    value = boost::lexical_cast<T>(override);
    return true;
}

template <class T> void RlFactory::GetAll(std::vector<T*>& objects)
{
    objects.clear();
    for (std::vector<RlAutoObject*>::iterator i_object 
            = m_objectVector.begin();
        i_object != m_objectVector.end(); ++i_object)
    {
        RlAutoObject* object = *i_object;
        T* objectT = dynamic_cast<T*>(object);
        if (objectT)
            objects.push_back(objectT);
    }
}


//----------------------------------------------------------------------------
/** These macros should be used in class declaration and implementation
    respectively */

#define DECLARE_OBJECT(classname) \
    static RlAutoObject* Create##classname \
        (GoBoard& board); \
    virtual std::string GetName() const; \
    static void ForceLink(); \
    static RlRegister Register##classname
// Semi-colon purposefully omitted, to be used after macro.

#define IMPLEMENT_OBJECT(classname) \
    RlAutoObject* classname::Create##classname \
        (GoBoard& board) \
    { \
        return new classname(board); \
    } \
    \
    std::string classname::GetName() const \
    { \
        return #classname; \
    } \
    \
    void classname::ForceLink() \
    { \
        RlAutoObject* autoobject = 0; \
        classname* object = dynamic_cast<classname*>(autoobject); \
        SG_UNUSED(object); \
    } \
    RlRegister classname::Register##classname \
        (#classname, &classname::Create##classname)
// Semi-colon purposefully omitted, to be used after macro.

// The ForceLink function can be called for all classes that aren't
// automatically linked. Would prefer to find an automatic solution... 

//----------------------------------------------------------------------------
/** Special auto object to include other settings files.
    Includes must occur first in the settings file. */
class RlInclude : public RlAutoObject
{
public:

    DECLARE_OBJECT(RlInclude);
    
    RlInclude(GoBoard& board);

    /** Allocate objects from included file */
    virtual void Allocate(std::istream& settings);

    /** Load settings from included file */
    virtual void Load(std::istream& settings);
};

//----------------------------------------------------------------------------
/** Special auto object to override later settings.
    Overrides must occur first in the settings file. */
class RlOverride : public RlAutoObject
{
public:

    DECLARE_OBJECT(RlOverride);
    
    RlOverride(GoBoard& board);

    /** Load settings from file.
        This causes overrides to be set. */
    virtual void LoadSettings(std::istream& settings);
};

//----------------------------------------------------------------------------

#endif // RLFACTORY_H


//----------------------------------------------------------------------------
/** @file RlProcessUtil.h
    Multi-processing utility functions
*/
//----------------------------------------------------------------------------

#ifndef RLPROCESSUTIL_H
#define RLPROCESSUTIL_H

#include <string>
#include <boost/filesystem/path.hpp>

namespace bfs = boost::filesystem;

//----------------------------------------------------------------------------
/** Shared memory object. */
class RlSharedMemory
{
public:

    RlSharedMemory(const bfs::path& sharename, int index, int bytes);
    ~RlSharedMemory();
    
    char* GetData() { return m_data; }

    void Error();
    
private:

    int m_id;
    char* m_data;
};

//----------------------------------------------------------------------------
/** Semaphore for controlling access between processes */
class RlSemaphore
{
public:

    RlSemaphore(const bfs::path& semname, int index);
    ~RlSemaphore();
    
    void Lock();
    void Unlock();

private:

    int m_id;
};

//----------------------------------------------------------------------------

#endif // RLPROCESSUTIL_H

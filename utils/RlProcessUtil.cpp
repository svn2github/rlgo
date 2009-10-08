//----------------------------------------------------------------------------
/** @file RlProcessUtil.cpp
    See RlProcessUtil.h
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "RlProcessUtil.h"

#include "SgException.h"

// Define this for multi-processing capability
#define RL_MULTI

#ifdef RL_MULTI
#include <sys/types.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <sys/sem.h> 
#include <sys/errno.h>
#endif // RL_MULTI

using namespace std;

//----------------------------------------------------------------------------

#ifdef RL_MULTI

RlSharedMemory::RlSharedMemory(
    const bfs::path& sharename, int index, int bytes)
{
    std::string filename = sharename.native_file_string();
    key_t key = ftok(filename.c_str(), index);
    if (key == -1)
        throw SgException("Shared file " + filename + " doesn't exist");
    m_id = shmget(key, bytes, 0644 | IPC_CREAT);
    if (m_id == -1)
        Error();
    m_data = (char*) shmat(m_id, 0, 0);
    if (m_data == (char*) -1)
        Error();
}

RlSharedMemory::~RlSharedMemory()
{
    if (shmdt(m_data) == -1)
        throw SgException("Failed to detach shared memory");

    // Will only delete shared memory once everyone is detached.
    shmctl(m_id, IPC_RMID, 0);
}

void RlSharedMemory::Error()
{
    // Note: must ensure that system allows large shared memory segments
    // On Mac OS X, specify in /etc/rc: sysctl -w kern.sysv.shmmax=blah
    switch(errno)
    {
    case EACCES:
        throw SgException("Don't have permission for existing shared memory");
    case ENOSPC:
        throw SgException("Too many shared memory identifiers");
    case ENOMEM:
        throw SgException("Not enough memory for shared memory object");
    case EINVAL:
        throw SgException("Invalid shared memory arg (size exceeds shmmax?)");
    case EEXIST:
    case ENOENT:
        throw SgException("This error should not occur");
    default:
        throw SgException("Failed to share memory: unknown error");
    }
}

RlSemaphore::RlSemaphore(const bfs::path& semname, int index)
{
    std::string filename = semname.native_file_string();
    key_t key = ftok(filename.c_str(), index);
    if (key == -1)
        throw SgException("Semaphore " + filename + " doesn't exist");
    m_id = semget(key, 1, 0644 | IPC_CREAT);
    if (semctl(m_id, 0, SETVAL, 1) == -1)
        throw SgException("Failed to initialise semaphore");
}

RlSemaphore::~RlSemaphore()
{
    semctl(m_id, 0, IPC_RMID);
}

void RlSemaphore::Lock()
{
    struct sembuf op;
    op.sem_num = 0;
    op.sem_op = -1;
    op.sem_flg = 0;
    semop(m_id, &op, 1);
}

void RlSemaphore::Unlock()
{
    struct sembuf op;
    op.sem_num = 0;
    op.sem_op = 1;
    op.sem_flg = 0;
    semop(m_id, &op, 1);
}

#else

RlSharedMemory::RlSharedMemory(
    const std::string& sharename, int index, int bytes)
:   m_id(0),
    m_data(0)
{
    SG_UNUSED(sharename);
    SG_UNUSED(index);
    SG_UNUSED(bytes);
    throw SgException("Tried to share memory without RL_MULTI defined");
}

RlSharedMemory::~RlSharedMemory()
{
    throw SgException("Tried to share memory without RL_MULTI defined");
}

RlSemaphore::RlSemaphore(const std::string& semname, int index)
:   m_id(0)
{
    SG_UNUSED(semname);
    SG_UNUSED(index);
}

RlSemaphore::~RlSemaphore()
{
}

void RlSemaphore::Lock()
{
}

void RlSemaphore::Unlock()
{
}

#endif // RL_MULTI

//----------------------------------------------------------------------------

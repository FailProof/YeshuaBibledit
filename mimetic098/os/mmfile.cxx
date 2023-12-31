#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"

#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wswitch-default"
#pragma GCC diagnostic ignored "-Wuseless-cast"

#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wswitch-default"
#pragma GCC diagnostic ignored "-Wuseless-cast"

#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wswitch-default"
#pragma GCC diagnostic ignored "-Wuseless-cast"

#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wswitch-default"

#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wsuggest-override"

#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Wold-style-cast"

#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"

#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#pragma clang diagnostic ignored "-Wsign-conversion"

#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wundef"

/***************************************************************************
    copyright            : (C) 2002-2008 by Stefano Barbato
    email                : stefano@codesink.org

    $Id: mmfile.cxx,v 1.6 2008-10-07 11:06:26 tat Exp $
 ***************************************************************************/
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#pragma GCC diagnostic ignored "-Wsign-conversion"
#include <mimetic098/libconfig.h>
#include <mimetic098/os/mmfile.h>
#include <cstring>

using namespace std;

namespace mimetic
{

MMFile::MMFile()
: m_stated(false), m_fd(-1), m_beg(0), m_end(0)
{
}

MMFile::MMFile(const string& fqn, int mode)
: m_fqn(fqn), m_stated(false), m_fd(-1), m_beg(0), m_end(0)
{
    memset(&m_st, 0, sizeof(m_st));
    if(!stat())
        return;
    open(mode);
}

bool MMFile::open(const std::string& fqn, int mode /*= O_RDONLY*/)
{
    m_fqn = fqn;
    if(!stat() || !S_ISREG(m_st.st_mode))
        return false;
    return open(mode);
}

bool MMFile::open(int mode)
{
    if(!stat() || !S_ISREG(m_st.st_mode))
        return false;
    m_fd = ::open(m_fqn.c_str(), mode);
    if(m_fd > 0)
        return map();
    else
        return false;
}

bool MMFile::map()
{
    m_beg = (char*) mmap(0, m_st.st_size, PROT_READ, MAP_SHARED,m_fd,0);
    if(m_beg > (char *)0)
    {
        m_end = m_beg + m_st.st_size;
        //#ifdef HAVE_MADVISE
        //madvise(m_beg, m_st.st_size, MADV_SEQUENTIAL);
        //#endif
        return true;
    }
    return false;
}

MMFile::~MMFile()
{
    if(m_beg)
        munmap(m_beg, m_st.st_size);
    if(m_fd)
        close();
}

MMFile::iterator MMFile::begin()
{
    return m_beg;
}

MMFile::iterator MMFile::end()
{
    return m_end;
}

MMFile::const_iterator MMFile::begin() const
{
    return m_beg;
}


MMFile::const_iterator MMFile::end() const
{
    return m_end;
}

uint MMFile::read(char* buf, int bufsz)
{
    long r;
    do
    {
        r = ::read(m_fd, buf, bufsz);
    } while(r < 0 && errno == EINTR);
    return (uint)r;
}

MMFile::operator bool() const
{
    return m_fd > 0;
}

bool MMFile::stat()
{
    return m_stated || (m_stated = (::stat(m_fqn.c_str(), &m_st) == 0));
}

void MMFile::close() 
{
    while(::close(m_fd) < 0 && errno == EINTR)
        ;
    m_fd = -1;
}

}


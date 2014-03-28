//@
//@ Dklab Realplexor: Comet server which handles 1000000+ parallel browser connections
//@ Author: Dmitry Koterov, dkLab (C)
//@ GitHub: http://github.com/DmitryKoterov/
//@ Homepage: http://dklab.ru/lib/dklab_realplexor/
//@
//@ ATTENTION: Java-style C++ programming below. :-)
//@
//@ This is a line-by-line C++ rewrite of Perl prototype code with obvious speed
//@ optimizations (like avoiding excess copies, config pre-parsing etc.).
//@
//@ The code is so compact (2600 lines) and so simple, that I decided not to
//@ split it into *.hpp & *.cpp files nor create Makefiles, but place
//@ everything into included *.h files (like Perl, Java, C# and most of other
//@ languages do). It is not quite common for C++, but it surely simple
//@ when a program is small (especially when it is rewritten line by line
//@ from another language).
//@
//@ Also the code has global variables within the top namespace: one variable
//@ per Storage and one CONFIG, they are like singletons.
//@

//
// Fiber Handle: an opened socket (connection) with event-driven write buffering support.
//
// The task is to allow unlimited amount of data to be written to a client
// connection, even if this amount is larger than Unix socket buffers
// (by default in Perl this buffer is near 160K, in C++ it's near 2K).
//

#ifndef REALPLEXOR_EVENT_FH_H
#define REALPLEXOR_EVENT_FH_H

namespace Realplexor { namespace Event {
using std::shared_ptr;
using std::exception;

class FH
{
    shared_ptr<Socket> _sock;

public:
    FH(shared_ptr<Socket> sock): _sock(sock)
    {
        _sock->blocking(false);
    }

    virtual ~FH() {}

    size_t recv_and_append_to(string& s)
    {
        return _sock->recv_and_append_to(s);
    }

    int send(const string& s)
    {
        return _sock->send(s);
    }

    int shutdown(int how)
    {
        return _sock->shutdown(how);
    }

    string peeraddr()
    {
        return _sock->peeraddr();
    }

    int fileno()
    {
        return _sock->fileno();
    }
};

}}
#endif

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
// Connection abstraction.
//
// Object of this class is created when a connection is accepted
// and destroyed when its connection is closed. Each object
// represents a separated connection and may hold intermediate
// data collected while its connection is processed.
//

#ifndef REALPLEXOR_CONNECTION_H
#define REALPLEXOR_CONNECTION_H

namespace Realplexor { namespace Event {
using std::shared_ptr;
using std::exception;

class Connection
{
private:
    fh_t _fh;
    ServerBase* _server;

protected:
    string rdata;

public:

    // Called on new connection.
    // DO NOT save event object here to avoid cyclic references!
    // Note that server is needed here for logging purposes only (e.g. server->error()).
    Connection(const fh_t& fh, ServerBase* server): _fh(fh), _server(server)
    {
        DEBUG("connection opened");
    }

    // Reads available data chunk from fh and returns number of read bytes.
    size_t read_available_data()
    {
        return _fh->recv_and_append_to(rdata);
    }

    // Called on close.
    virtual void onclose() =0;

    // Called on connection close.
    // Unfortunately destructor cannot be pure in C++.
    virtual ~Connection() {}

    // MUST be called from derived destructors!
    // Unfortunately we cannot call onclose() directly from this class destructor,
    // because in C++ we cannot call virtual functions of derived classes from
    // destructors (at this time derived class members do not exist already).
    void ondestruct()
    {
        try {
            onclose();
        } catch (exception& e) {
            _server->error(_fh, e.what());
        }
        DEBUG("connection closed");
    }

    // Called on timeout.
    virtual void ontimeout()
    {
        DEBUG("timeout");
    }

    // Called on event exception.
    virtual void onerror(const string& msg)
    {
        DEBUG("error: " + msg);
    }

    // Called on data read.
    virtual void onread(size_t nread)
    {
        DEBUG("read " + lexical_cast<string>(nread) + " bytes");
    }

    // Returns the socket.
    fh_t fh()
    {
        return _fh;
    }

    // Returns this connection name.
    virtual string name()
    {
        return "";
    }

    // Prints a debug message.
    void debug_(const string& msg)
    {
        const string& name = this->name();
        _server->debug_(_fh, (name.length()? "[" + name + "] " : "") + msg);
    }

};

}}
#endif

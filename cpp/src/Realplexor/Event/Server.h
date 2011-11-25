#ifndef REALPLEXOR_EVENT_SERVER_H
#define REALPLEXOR_EVENT_SERVER_H

// Speedup macro.
#define DEBUG(s) if (CONFIG.verbosity > 0) debug_((s))

namespace Realplexor { namespace Event {
using std::shared_ptr;
using std::exception;

class ServerBase
{
protected:
    typedef void (*logger_t)(const string& s);
    string name;
    logger_t logger;

public:
    ServerBase(string name, logger_t logger): name(name), logger(logger) {}

    // Destrictor.
    virtual ~ServerBase() {}

    // Controls error messages.
    virtual void debug_(const filehandle_t& fh, const string& msg)
    {
        message(fh, "DEBUG: " + msg);
    }

    // Controls error messages.
    virtual void error(const filehandle_t& fh, const string& msg)
    {
        message(fh, "ERROR: " + msg);
    }

    // Controls info messages.
    virtual void message(const filehandle_t& fh, const string& msg0)
    {
        string msg = trim_right_copy(msg0);
        if (fh) {
            string addr = fh->peeraddr();
            if (!addr.length()) addr = "?:?";
            msg = addr + ": " + msg;
        }
        msg = name + ": " + msg;
        if (logger) {
            logger(msg);
        }
    }
};


template<class ConnClass>
class Server: public ServerBase
{
    vector<shared_ptr<ev::io>> events;
    string listen;
    int timeout;
    string connClass;

public:

    // Creates a new server pool.
    Server(string name, string listen, int timeout, logger_t logger): ServerBase(name, logger), listen(listen), timeout(timeout)
    {
        string lastAddr;
        try {
            for (auto& addr: split(" ", listen)) {
                lastAddr = addr;
                events.push_back(add_listen(addr));
            }
        } catch (runtime_error e) {
            events.clear();
            die(lastAddr + ": " + e.what());
        } catch (exception e) {
            events.clear();
            die(lastAddr + ": unknown exception");
        }
    }

    // Called on data read.
    // Returns false if the connection must be closed immediately.
    bool handle_read(shared_ptr<ConnClass> connection, int type)
    {
        filehandle_t fh = connection->fh();
        try {
            // Timeout?
            if (type & EV_TIMEOUT) {
                connection->ontimeout();
                return false;
            }
            // An error?
            if (type & EV_ERROR) {
                connection->onerror("An error returned to event handler");
                return false;
            }
            // Read the next data chunk.
            size_t nread = fh->read_and_append_to(connection->get_data_buf());
            // End of the request reached (must never reach because of eof() check above?).
            if (!nread) {
                return false;
            }
            // Run data handler.
            connection->onread(nread);
            return true;
        } catch (runtime_error& e) {
            error(fh, e.what());
            return false;
        } catch (exception& e) {
            error(fh, e.what());
            return false;
        }
        return true;
    }

    // Called on a new connect.
    void handle_connect(filehandle_t fh)
    {
        filehandle_t socket(fh->accept());
        socket->blocking(false);
        shared_ptr<ConnClass> connection(new ConnClass(socket, this));

        // This holds all objects needed within event handlers.
        // Unfortunately we cannot use C++0x closures, because captured vars 
        // are const, but we really need to destroy io and timer event objects 
        // from within callbacks. So we use plain old structures, plain pointers 
        // and new/delete operators instead of shared_ptr.
        struct IoTimerClosure
        {
            shared_ptr<ev::io>     io;
            shared_ptr<ev::timer>  timer;
            shared_ptr<ConnClass>  connection;
            Server<ConnClass>*     server;

            void handle(int revents)
            {
                if (server->handle_read(connection, revents)) {
                    timer->again();
                } else {
                    delete this;
                }
            }

            void handle(ev::io& w, int revents)
            {
                handle(revents);
            }

            void handle(ev::timer& w, int revents)
            {
                handle(revents);
            }
        };

        // This object is deleted from within event handler callback.
        IoTimerClosure *closure = new IoTimerClosure();
        closure->server = this;
        closure->connection = connection;

        // Initialize IO event. When happens, this event restart Timer & IO events.
        closure->io.reset(new ev::io());
        closure->io->ev::io::set<IoTimerClosure, &IoTimerClosure::handle>(closure);
        closure->io->ev::io::set(socket->fileno(), EV_READ);
        closure->io->start();

        // Initialize Timer event. When happens, this event destroys Timer & IO event.
        closure->timer.reset(new ev::timer());
        closure->timer->ev::timer::set<IoTimerClosure, &IoTimerClosure::handle>(closure);
        closure->timer->ev::timer::set(timeout, timeout);
        closure->timer->start();

        // Leave closure along. It will be destroyed either by IO callback
        // (when the data is finished) or by Timer callback.
    }

    // Adds a new listen address to the pool.
    // Croaks in case of error.
    shared_ptr<ev::io> add_listen(string addr)
    {
        filehandle_t fh(new Socket(addr));
        fh->blocking(false);
        
        // This holds all objects needed within event handlers.
        struct IoClosure
        {
            filehandle_t        fh;
            Server<ConnClass>*  server;
            void handle(ev::io& w, int revents)
            {
                server->handle_connect(fh);
            }
        };

        // This object is NEVER deleted (we assume that Server object lives forever).
        IoClosure* closure = new IoClosure();
        closure->server = this;
        closure->fh = fh;

        // Create an event and return it.
        shared_ptr<ev::io> evt(new ev::io());
        evt->ev::io::set<IoClosure, &IoClosure::handle>(closure);
        evt->ev::io::set(fh->fileno(), EV_READ);
        evt->start();

        message(0, "listening " + addr);
        return evt;
    }

};

void mainloop()
{
    ev_loop(EV_DEFAULT_ 0);
}

}}
#endif
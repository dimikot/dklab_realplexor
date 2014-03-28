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
// WAIT connection.
//
#ifndef REALPLEXOR_CONNECTION_WAIT_H
#define REALPLEXOR_CONNECTION_WAIT_H

namespace Connection {
using namespace Realplexor;
using std::shared_ptr;

class Wait: public Realplexor::Event::Connection
{
    shared_ptr<DataPairChain> pairs;
    string _name;

public:
    Wait(fh_t fh, Realplexor::Event::ServerBase* server): Connection(fh, server)
    {
        pairs.reset(new DataPairChain());
    }

    // Hack: unfortunately C++ cannot call overriden virtual functions from base class destructors.
    virtual ~Wait()
    {
        ondestruct();
    }

    // Called when a data is available to read.
    virtual void onread(size_t nread)
    {
        Realplexor::Event::Connection::onread(nread);

        // Data must be ignored, identifier is already extracted.
        if (pairs->size()) {
            return;
        }

        // Try to extract IDs from the new data chunk.
        Realplexor::LimitIdsSet limit_ids;
        Realplexor::CredPair cred;
        if (Realplexor::Common::extract_pairs(rdata, *pairs, limit_ids, cred)) {
            if (!pairs->size()) throw runtime_error("Empty identifier passed");

            // Check if we have special marker: IFRAME.
            if (pairs->begin()->id == CONFIG.iframe_id) {
                pairs->clear();
                DEBUG("IFRAME marker received, sending content");
                Realplexor::Common::send_static(fh(), CONFIG.static_iframe.content, CONFIG.static_iframe.time, "text/html; charset=" + CONFIG.charset);
                return;
            }
            // Check if we have special marker: SCRIPT.
            if (pairs->begin()->id == CONFIG.script_id) {
                pairs->clear();
                DEBUG("SCRIPT marker received, sending content");
                Realplexor::Common::send_static(fh(), CONFIG.static_script.content, CONFIG.static_script.time, "text/javascript; charset=" + CONFIG.charset);
                return;
            }

            // IDs are extracted. Send response headers immediately.
            // We send response AFTER reading IDs, because before
            // this reading we don't know if a static page or
            // a data was requested.
            fh()->send(
                "HTTP/1.1 200 OK\r\n"
                "Connection: close\r\n"
                "Cache-Control: no-store, no-cache, must-revalidate\r\n"
                "Expires: Mon, 26 Jul 1997 05:00:00 GMT\r\n"
                "Content-Type: text/javascript; charset=" + CONFIG.charset + "\r\n\r\n" +
                " \r\n" // this immediate space plus text/javascript hides XMLHttpRequest in FireBug console
            );

            // Ignore all other input from IN and register identifiers.
            rdata = "";
            pairs_by_fhs.set_pairs_for_fh(fh(), pairs);
            IdsToSendSet ids_to_process;
            for (auto& pair: *pairs) {
                connected_fhs.add_to_id(pair.id, pair.cursor, fh());
                // Create new online timer, but do not start it - it is
                // started at LAST connection close, later.
                string id = pair.id;
                auto callback = [id]() {
                    LOGGER("[" + id + "] is now offline");
                    events.notify(DataEventType::OFFLINE, id);
                    // It is better to change the order of upper two lines for more clear logging,
                    // but it is already covered by auto-tests, so...
                };
                bool firstTime = online_timers.assign_stopped_timer_for_id<decltype(callback)>(id, callback);
                if (firstTime) {
                    // If above returned true, this ID was offline, but become online.
                    events.notify(DataEventType::ONLINE, id);
                }
                ids_to_process.insert(pair.id);
            }
            DEBUG("registered"); // ids are already in the debug line prefix
            // Try to send pendings.
            Realplexor::Common::send_pendings(ids_to_process);
            return;
        }

        // Check for the data overflow.
        if (rdata.length() > CONFIG.wait_maxlen) {
            throw runtime_error("overflow (received " + lexical_cast<string>(rdata.length()) + " bytes total)");
        }
    }

    // Called on timeout (send error message).
    virtual void ontimeout()
    {
        if (fh()) {
            fh()->shutdown(2);
        }
        Realplexor::Event::Connection::ontimeout();
    }

    // Called on client disconnect.
    virtual void onclose()
    {
        if (pairs->size()) {
            for (auto& pair: *pairs) {
                // Remove the client from all lists.
                connected_fhs.del_from_id_by_fh(pair.id, fh());
                // Turn on offline timer if it was THE LAST connection.
                if (!connected_fhs.get_num_fhs_by_id(pair.id)) {
                    online_timers.start_timer_by_id(pair.id, CONFIG.offline_timeout);
                }
            }
        }
        pairs_by_fhs.remove_by_fh(fh());
    }

    // Connection name is its ID.
    virtual string name()
    {
        if (!_name.length() && pairs->size()) {
            _name = lexical_cast<string>(pairs->begin()->cursor) + ":" + pairs->begin()->id +
                (pairs->size() > 1? "(and " + lexical_cast<string>(pairs->size() - 1) + " more)" : "");
        }
        return _name;
    }

};

}
#endif
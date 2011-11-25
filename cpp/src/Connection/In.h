#ifndef REALPLEXOR_CONNECTION_IN_H
#define REALPLEXOR_CONNECTION_IN_H

namespace Connection {
using namespace Realplexor;
using std::shared_ptr;
using std::exception;

class In: public Realplexor::Event::Connection
{
    shared_ptr<DataPairChain> pairs;
    shared_ptr<LimitIdsSet> limit_ids;
    CredPair cred;

public:
    
    // Called on a new connection.
    In(filehandle_t fh, Realplexor::Event::ServerBase* server): Connection(fh, server) 
    {
        pairs.reset(new DataPairChain());
        limit_ids.reset(new LimitIdsSet());
    }

    // Hack: unfortunately C++ cannot call overriden virtual functions from base class destructors.
    virtual ~In()
    {
        ondestruct();
    }

    // Called on timeout.
    void ontimeout()
    {
        Realplexor::Event::Connection::ontimeout();
        pairs->clear();
        data = "";
    }

    // Called on error. 
    void onerror(const string& msg)
    {
        Realplexor::Event::Connection::onerror(msg);
        pairs->clear();
        data = "";
    }   

    // Called when a data is available to read.
    void onread(size_t nread)
    {
        Realplexor::Event::Connection::onread(nread);

        // Try to extract ID from the new data chunk.
        if (!pairs->size()) {
            if (Realplexor::Common::extract_pairs(data, *pairs, *limit_ids, cred)) {
                DEBUG(
                    "parsed IDs" 
                    + (limit_ids->size()? "; limiters are (" + join(sort_keys(*limit_ids), ", ") + ")" : "")
                    + (cred.login.length()? "; login is \"" + cred.login + "\"" : "")
                );
                _assert_auth();
            }
        }

        // Try to process cmd.
        if (_try_process_cmd(false)) return;

        // Check for the data overflow.
        if (data.length() > CONFIG.in_maxlen) {
            die("overflow (received " + lexical_cast<string>(data.length()) + " bytes total)");
        }
    }

    // Called on client side disconnect.
    virtual void onclose() 
    {
        // First, try to process cmd.
        if (_try_process_cmd(true)) return;
        // Then, try to send messages.
        if (_try_process_pairs()) return;
    }

private:

    // Assert that authentication is OK.
    void _assert_auth()
    {
        try {
            if (cred.login.length()) {
                // Login + password are passed. Check credentials.
                if (!CONFIG.users.count(cred.login)) {
                    die("unknown login: " + cred.login);
                }
                string pwd_hash = CONFIG.users.get(cred.login);
                if (crypt(cred.password.c_str(), pwd_hash.c_str()) != pwd_hash) {
                    die("invalid password for login: " + cred.login);
                }
            } else if (!CONFIG.users.count("")) {
                // Guest access, but no guest account is found.
                die("access denied for guest user");
            }
        } catch (exception& e) {
            pairs->clear();
            data = "";
            _send_response(string(e.what()) + "\n", "403 Access Deined");
            throw;
        }
    }

    // Process aux commands (may be started from the beginning
    // of the data of from the first \r\n\r\n part and finished
    // always by \n).
    bool _try_process_cmd(bool finished_reading)
    {
        if (!data.length()) return false;
        // Try to extract cmd.
        string tail_re = finished_reading? "\r?\n\r?\n|$" : "\r?\n\r?\n";
        regex re_in_cmd("(?:^|\r?\n\r?\n)(ONLINE|STATS|WATCH)(?:\\s+([^\r\n]*))?(?:" + tail_re + ")", regex::icase);
        boost::smatch m;
        if (!regex_search(data, m, re_in_cmd)) return false;
        string cmd = to_upper_copy(string(m[1]));
        string arg = m[2];
        // Cmd extracted, process it.
        pairs->clear();
        data = "";
        // Assert authorization.
        _assert_auth();
        DEBUG("received aux command: " + cmd + (arg.length()? " " + arg : ""));
        fh()->shutdown(0); // stop reading
        if (cmd == "ONLINE") {
            _cmd_online(arg);
        } else if (cmd == "STATS") {
            _cmd_stats(arg);
        } else if (cmd == "WATCH") {
            _cmd_watch(arg);
        }
        return true;
    }

    // Try to process pairs.
    bool _try_process_pairs()
    {
        if (!data.length()) return false;
        if (pairs->size()) {
            // Clear headers from the data.
            size_t pos_body;
            if (!get_http_body(data, pos_body)) {
                DEBUG("passed empty HTTP body, ignored");
                data = "";
                return false;
            }
            vector<ident_t> ids_to_process;
            auto checker = _id_prefixes_to_checker("");
            auto rdata = shared_ptr<string>(new string(data, pos_body));
            for (auto& pair: *pairs) {
                cursor_t cursor = pair.cursor;
                ident_t id = pair.id;
                // Check if it is not own pair.
                if (!checker->matched(id)) {
                    DEBUG("skipping not owned [" + id + "] for login " + cred.login);//
                    continue;
                }
                // Add data to queue and set lifetime.
                ids_to_process.push_back(id);
                data_to_send.add_dataref_to_id(id, cursor, rdata, limit_ids);
                int timeout = CONFIG.clean_id_after;
                auto callback = [id, timeout]() {
                    data_to_send.clear_id(id); 
                    LOGGER("[" + id + "] cleaned, because no data is pushed within last " + lexical_cast<string>(timeout) + " seconds");
                };
                cleanup_timers.start_timer_for_id<decltype(callback)>(id, timeout, callback);
            }
            // One debug message per connection.
            if (ids_to_process.size()) {
                DEBUG("added data for [" + join(ids_to_process, ",") + "]");
            }
            // Send pending data.
            Realplexor::Common::send_pendings(ids_to_process);
        }
        return false;
    }

    // Convert space-delimited ID prefixes list to prefix checker.
    shared_ptr<prefix_checker> _id_prefixes_to_checker(const string& id_prefixes)
    {
        vector<string> list;
        if (id_prefixes.length()) list = split(regex("\\s+"), id_prefixes);
        return shared_ptr<prefix_checker>(new prefix_checker(list, cred.login.length()? cred.login + "_" : ""));
    }

    // Command: fetch all online IDs.
    void _cmd_online(const string& id_prefixes)
    {
        vector<ident_t> ids;
        online_timers.get_ids_ref(_id_prefixes_to_checker(id_prefixes), ids);
        DEBUG("sending " + lexical_cast<string>(ids.size()) + " online identifiers");
        _send_response(join(apply(ids, [](string& id) { return id + " " + lexical_cast<string>(connected_fhs.get_num_fhs_by_id(id)) + "\n"; }), ""));
    }

    // Command: watch for clients online/offline status changes.
    void _cmd_watch(const string& arg) 
    {
        smatch m;
        cursor_t cursor = 0;
        string id_prefixes = "";
        try {
            if (regex_search(arg, m, regex("^(\\S+)\\s+(.*)$"))) {
                cursor = lexical_cast<cursor_t>(m[1]);
                id_prefixes = m[2];
            } else {
                cursor = lexical_cast<cursor_t>(arg);
            }
        } catch (bad_lexical_cast& e) {
            cursor = 0;
        }
        DataEventChain list;
        events.get_recent_events(cursor, _id_prefixes_to_checker(id_prefixes), list);
        DEBUG("sending " + lexical_cast<string>(list.size()) + " events");
        _send_response(join(apply(list, [](DataEvent& e) { return e.getType() + " " + lexical_cast<string>(e.cursor) + ":" + e.id + "\n"; }), ""));
    }

    // Command: dump debug statistics.
    // This command is for internal debugging only.
    void _cmd_stats(const string& arg)
    {
        if (cred.login.length()) return;
        DEBUG("sending stats");
        _send_response(
            "[data_to_send]\n" +
            data_to_send.get_stats() +
            "\n[connected_fhs]\n" +
            connected_fhs.get_stats() +
            "\n[online_timers]\n" +
            online_timers.get_stats() +
            "\n[cleanup_timers]\n" +
            cleanup_timers.get_stats() +
            "\n[pairs_by_fhs]\n" +
            pairs_by_fhs.get_stats()
        );
    }

    // Send response anc close the connection.
    void _send_response(const string& d, const string& code = "")
    {
        fh()->write(
            "HTTP/1.0 " + (code.length()? code : "200 OK") + "\r\n" +
            "Content-Type: text/plain\r\n" +
            "Content-Length: " + lexical_cast<string>(d.length()) + "\r\n\r\n" +
            d
        );
        fh()->flush(); // MUST be executed! else SIGPIPE may be issued
        fh()->shutdown(2);
        pairs->clear();
        data = "";
    }
};

}
#endif
#ifndef REALPLEXOR_COMMON_H
#define REALPLEXOR_COMMON_H

// Speedup macro.
#define LOGGER(s) if (CONFIG.verbosity > 0) Realplexor::Common::logger((s))


namespace Realplexor {

class Common 
{
    // This is to execute a piece of code automatically.
    static Common instance;
    Common()
    {
        CONFIG.set_logger(&logger);
    }

public:

    // Logger routine.
    // This function MUST be declared in Realplexor::Common, after all of 
    // Storages are already defined.
    static void logger(const string& s)
    {
        int verb = CONFIG.verbosity;
        if (verb == 0) return;
        string msg = s;
        if (verb > 2) {
            msg = msg + "\n  " + 
                "[pairs_by_fhs=" + lexical_cast<string>(pairs_by_fhs.get_num_items()) +
                " data_to_send=" + lexical_cast<string>(data_to_send.get_num_items()) +
                " connected_fhs=" + lexical_cast<string>(connected_fhs.get_num_items()) +
                " online_timers=" + lexical_cast<string>(online_timers.get_num_items()) +
                " cleanup_timers=" + lexical_cast<string>(cleanup_timers.get_num_items()) +
                " events=" + lexical_cast<string>(events.get_num_items()) +
                "]";
        }
        if (verb >= 2) {
            cout << "[" << strftime_std(from_time_t(ev::now())) << "] " << msg << endl;
        } else {
            cout << msg << endl;
        }
    }

    // Extract pairs [cursor, ID] from the client data. 
    // Return [ [cursor1, id1], [cursor2, id2], ... ] or undef if 
    // no "identifier" marker is found yet.
    //
    // If you call this sub in a list context, the second return
    // value is the list of IDs marked by "*" at identifier=...
    // (this means that these IDs must be listened by a client
    // too to receive the data).
    //
    // If login and password are specified, third return value
    // is [login, password] pair.
    // 
    // Format: 
    // - identifier=abc                         [single identifier]
    // - identifier=abc,def,ghi                 [multiple identifiers]
    // - identifier=12345.23:abc,12345:def      [where 12345... is the cursor]
    // - identifier=abc,def,*ghi,*jkl           [multiple ids, and (ghi, jkl) is returned as second list element]
    // - identifier=LOGIN:PASS@abc,10:def,*ghi  [same as above, but login and password are specified]
    //
    // Returns true if the extraction is succeeded.
    static bool extract_pairs(string& data, DataPairChain& pairs, LimitIdsSet& limit_ids, Realplexor::CredPair& cred)
    {
        //
        // TODO: this and derived functions must be strongly optimized.
        //
        // Return fast if no identifier marker is presented yet.
        if (data.find(CONFIG.IDENTIFIER_PLUS_EQ) == data.npos) return false;
        
        // Identifier marker seems to be presented. Remove referrer headers.
        const char* start = data.c_str();
        const char* p = strcasestr(start, "\nReferer:");
        if (p) {
            size_t pos = p - start + 1;
            while (pos < data.length() && data[pos] != '\n') data[pos++] = ' ';
        }

        // Now check for identifier freely.
        string ids;
        if (!_extract_login_password_ids(data, cred, ids)) return false;
        if (!_split_ids(ids, pairs, limit_ids)) return false;
        return true;
    }

    // Send IFRAME content.
    static void send_static(filehandle_t fh, const string& content, const string& last_modified, const string& type)
    {
        fh->write("HTTP/1.1 200 OK\r\n");
        fh->write("Connection: close\r\n");
        fh->write("Content-Type: " + type + "\r\n");
        fh->write("Last-Modified: " + last_modified + "\r\n");
        fh->write("Expires: Wed, 08 Jul 2037 22:53:52 GMT\r\n");
        fh->write("Cache-Control: public\r\n");
        fh->write("\r\n");
        fh->write(content);
        fh->flush(); // MUST be executed! shutdown() does not issue flush()!
        fh->shutdown(2); // don't use close, it breaks event machine!
    }

    // Send first pending data to clients with specified IDs.
    // Remove sent data from the queue and close connections to clients.
    template <class Cont>
    static void send_pendings(const Cont& ids)
    {
        // Remove old data; do it BEFORE data processing/sending. Why?
        // Because if we receive 1000 new data rows for the same ID,
        // they will all be sent to all connected clients and slow down
        // the performance. When we clean the data before sending, we
        // guarantee that the inner loop will have less than MAX_DATA_FOR_ID
        // iterations for each ID.
        for (auto& id: ids) {
            data_to_send.clean_old_data_for_id(id, CONFIG.max_data_for_id);
        }

        // Collect data to be sent to each connection at %data_by_fh.
        // For each connection also collect matched IDs, so each client
        // receives only the list of IDs which is matched by his request
        // (client does not see IDs of other clients).
        DataToSendByFh data_by_fh;
        set<ident_t> seen_ids; // ordered - for logging

        // Iterate over all IDs to be checked.
        for (auto& id: ids) {
            // All data items for this ID.
            const DataChunkChain& data = data_to_send.get_data_by_id(id);
            if (!data.size()) continue;
            
            // Who listens this ID.
            const DataCursorFhByFh& fhs_hash = connected_fhs.get_hash_by_id(id);
            if (!fhs_hash.size()) continue;

            // Iterate over all connections which listen this ID.
            for (const DataCursorFhByFh::value_type& cursor_and_fh: fhs_hash) {
                // Process a single FH which listens this ID at listen_cursor.
                cursor_t listen_cursor = cursor_and_fh.second.cursor;
                filehandle_t fh = cursor_and_fh.second.fh;

                // What other IDs are listened by this FH.
                const DataPairChain& what_listens_this_fh = pairs_by_fhs.get_pairs_by_fh(fh);
                
                // Iterate over data items.
                for (const DataChunk& item: data) {
                    // If we found an element with smaller cursor, abort iteration,
                    // because all elements are sorted by cursor (bigger cursor first).
                    if (item.cursor <= listen_cursor) break;
                    
                    // Process a single data item in context of this FH.
                    const cursor_t&                cursor    = item.cursor;
                    const shared_ptr<string>&      rdata     = item.rdata;
                    const unordered_set<ident_t>&  limit_ids = *item.rlimit_ids;
                    
                    // Filter data invisible to this client.
                    if (limit_ids.size()) {
                        bool matched = false;
                        for (auto& id_which_is_listened: what_listens_this_fh) {
                            if (limit_ids.count(id_which_is_listened.id)) {
                                matched = true;
                                break;
                            }
                        }
                        if (!matched) continue;
                    }

                    // Hash by dataref to avoid to send the same data 
                    // twice if it is appeared in multiple IDs.
                    if (!data_by_fh.count(fh.get()) || !data_by_fh[fh.get()].count(rdata.get())) {
                        DataToSendChunk& dts = data_by_fh[fh.get()][rdata.get()]; // it also creates this element
                        dts.fh      = fh;
                        dts.cursor  = cursor;
                        dts.rdata   = rdata;
                        dts.ids[id] = cursor;
                    } else {
                        // Add new ID to the list of IDs for this data.
                        data_by_fh[fh.get()][rdata.get()].ids[id] = cursor;
                    }

                    // This is mostly for logging purposes.
                    seen_ids.insert(id);
                }
            }
        }

        // Perform sending operation.
        _do_send(data_by_fh, seen_ids);
    }

private:

    // Shutdown a connection and remove all references to it.
    static int _shutdown_fh(filehandle_t fh)
    {
        // Remove all references to $fh from everywhere.
        for (auto& pair: pairs_by_fhs.get_pairs_by_fh(fh)) {
            connected_fhs.del_from_id_by_fh(pair.id, fh);
        }
        pairs_by_fhs.remove_by_fh(fh);
        fh->flush(); // MUST be executed! shutdown() does not issue flush()!
        return fh->shutdown(2);
    }

    // Send data to each connection (json array format).
    // Response format is:
    // [
    //   {
    //     "ids": { "id1": cursor1, "id2": cursor2, ... },
    //     "data": <data from server without headers>
    //   },
    //   {
    //     "ids": { "id3": cursor3, "id4": cursor4, ... },
    //     "data": <data from server without headers>
    //   },
    //   ...
    // }
    static void _do_send(DataToSendByFh& data_by_fh, set<ident_t>& seen_ids)
    {
        for (DataToSendByFh::value_type &pair: data_by_fh) {
            vector<string> out_vec;

            // Additional ordering by raw data is for better determinism in tests.
            vector<DataToSendChunk*> triple_ptrs;
            transform(
                pair.second.begin(), pair.second.end(), 
                std::back_inserter(triple_ptrs), 
                [](DataToSendByDataRef::value_type& p) { return &p.second; }
            );
            sort(
                triple_ptrs.begin(), triple_ptrs.end(), 
                [](DataToSendChunk* a, DataToSendChunk* b) { 
                    return a->cursor < b->cursor? true : (a->cursor > b->cursor? false : (*a->rdata < *b->rdata)); 
                }
            );

            // Build JSON result.
            for (DataToSendChunk* triple: triple_ptrs) {
                // Build one response block.
                // It's very to send cursors as string to avoid rounding.
                vector<string> ids = apply(triple->ids, [](const std::pair<ident_t, cursor_t>& pair) { return "\"" + pair.first + "\": \"" + lexical_cast<string>(pair.second) + "\""; });
                out_vec.push_back(
                    "  {\n"
                    "    \"ids\": { " + join(ids, ", ") + " },\n"
                    "    \"data\":" + (triple->rdata->find("\n") != string::npos? "\n" : " ") + *triple->rdata + "\n"
                    "  }"
                );
            }

            // Join response blocks into one "multipart".
            string out = "[\n" + join(out_vec, ",\n") + "\n]";
            filehandle_t fh = pair.second.begin()->second.fh;
            // Attention! We MUST use print, not syswrite, because print correctly
            // continues broken transmits for large data packets.
            int r1 = fh->write(out);
            int r2 = _shutdown_fh(fh);
            logger(
                "<- sending " + lexical_cast<string>(out_vec.size()) + " responses " +
                "(" + lexical_cast<string>(out.length()) + " bytes) from " +
                "[" + join(seen_ids, ", ") + "] (print=" + lexical_cast<string>(r1) + ", shutdown=" + lexical_cast<string>(r2) + ")"
            );
        }
    }


    // Parses the string:
    //   identifier=login:pass@aaaa:4,bbb:5,...
    //              ^^^^^ ^^^^ ^^^^^^^^^^^^^^^^
    // and also
    //   identifier=aaaa:4,bbb:5,...
    static bool _extract_login_password_ids(const string& data, Realplexor::CredPair& cred, string& ids)
    {
        boost::smatch m;
        if (!regex_search(data, m, CONFIG.RE_LOGIN_PASSWORD_ID)) {
            return false;
        }
        cred.login = m[1];
        cred.password = m[2];
        ids = m[3];
        return true;
    }

    // Splits a comma-separated list of IDs.
    static bool _split_ids(const string& ids, DataPairChain& pairs, LimitIdsSet& limit_ids)
    {
        cursor_t time = 0;
        size_t pos = 0;
        while (pos < ids.length()) {
            size_t comma = ids.find(',', pos);
            if (comma == ids.npos) comma = ids.length();
            boost::smatch m;
            if (regex_search((ids.begin() + pos), (ids.begin() + comma), m, CONFIG.RE_CURSOR_ID)) {
                if (m[1].length()) {
                    // ID with limiter.
                    limit_ids.insert(m[3]);
                } else {
                    // Not limiter or limiter, but in WAIT line.
                    if (m[2].length()) {
                        // with cursor
                        pairs.push_back(Realplexor::DataPair(lexical_cast<cursor_t>(m[2]), m[3]));
                    } else {
                        if (!time) time = Realplexor::Tools::time_hi_res();
                        pairs.push_back(Realplexor::DataPair(time, m[3]));
                    }
                }
            }
            pos = comma + 1;
        }
        return true;
    }
};

Common Common::instance;

}
#endif

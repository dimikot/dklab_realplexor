//
// Storage::ConnectedFhs: connected clients.
//
// Structure: { ID => { fh1 => [cursor1, fh1], fh2 => [cursor2, fh2], ... } }
// Each ID may be listened in a number of connections. So, when a data
// for $id is arrived, it is pushed to all $connected_fds{$id} clients.
// We store stringified FH in keys for faster access.
//

#ifndef REALPLEXOR_CONNECTEDFHS_H
#define REALPLEXOR_CONNECTEDFHS_H

namespace Storage {
using namespace Realplexor;
using std::shared_ptr;

class ConnectedFhs
{
    map<ident_t, DataCursorFhByFh> storage;

public:

    ConnectedFhs() {}

    void add_to_id(const ident_t& id, cursor_t cursor, filehandle_t fh)
    {
        DataCursorFh &e = storage[id][fh.get()];
        e.cursor = cursor;
        e.fh = fh;
    }

    void del_from_id_by_fh(const ident_t& id, filehandle_t fh)
    {
        if (storage.count(id)) {
            storage[id].erase(fh.get());
            if (!storage[id].size()) storage.erase(id);
        }
    }

    const DataCursorFhByFh& get_hash_by_id(const ident_t& id)
    {
        static DataCursorFhByFh empty;
        return storage.count(id)? storage[id] : empty;
    }

    int get_num_items()
    {
        return storage.size();
    }

    int get_num_fhs_by_id(const ident_t& id)
    {
        return storage.count(id)? storage[id].size() : 0;
    }

    string get_stats()
    {
        vector<string> result;
        for (auto& pairs: storage) {
            result.push_back(
                string(pairs.first) + " => " + 
                join(apply(pairs.second, [](const DataCursorFhByFh::value_type& e) { return "(" + lexical_cast<string>(e.second.fh) + ")"; }), ", ") +
                "\n"
            );
        }
        return join(result, "");
    }
};

}

Storage::ConnectedFhs connected_fhs;

#endif

//
// Storage::DataToSend: data to send by ID.
//
// Structure: { ID => [ [cursor1, \$data1, \%limit_ids], [$cursor2, \$data2, \%limit_ids], ...] }
// When a client with $id is connected, and $data_to_send{$id} is not 
// empty, the data is sent to that client IF this client requested
// a data with "cursor" marker less than presented at $data_to_send{$id}.
// The hash holds REFERENCES to arrays, so we could distinguish the same 
// data block on different IDs. References are very handy: Perl takes 
// care about garbage collecting and ref count, and when the data block 
// is removed from the last ID, it is removed from the memory automatically.
//
// Third element, %limit_ids, shows that this data could be sent to
// only those who also listens IDs from %limit_ids keys. This is used
// to control data visibility.
//

#ifndef REALPLEXOR_STORAGE_DATATOSEND_H
#define REALPLEXOR_STORAGE_DATATOSEND_H

namespace Storage {
using namespace Realplexor;
using std::shared_ptr;

class DataToSend
{
    map<ident_t, DataChunkChain> storage;

public:

    DataToSend() {}

    void clear_id(const ident_t& id)
    {
        storage.erase(id);
    }

    void add_dataref_to_id(const ident_t& id, cursor_t cursor, shared_ptr<string> rdata, shared_ptr<unordered_set<ident_t>> rlimit_ids)
    {
        auto& list = storage[id];
        // In most cases new cursor is greater than the first array
        // element, so we may unshif it without re-sorting to speedup.
        DataChunkChain newList;
        newList.push_back(DataChunk(cursor, rdata, rlimit_ids));
        list.merge(newList, [](const DataChunk& e1, const DataChunk& e2) { return e2.cursor <= e1.cursor; });
        // "<=" is significant here, because we need to insert new element at the head 
        // of the list if it is less than all other elements (or - if not at the head -
        // then "as left as it could be done").
    }

    const DataChunkChain& get_data_by_id(const ident_t& id)
    {
        static DataChunkChain empty;
        return storage.count(id)? storage[id] : empty;
    }

    int get_num_items()
    {
        return storage.size();
    }

    void clean_old_data_for_id(const ident_t& id, size_t max_num)
    {
        if (!storage.count(id) || storage[id].size() < max_num) return;
        auto& list = storage[id];
        while (list.size() > max_num) {
            list.pop_back();
        }
    }

    string get_stats()
    {
        vector<string> result;
        for (auto& idlist: storage) { // sorted
            const ident_t& id = idlist.first;
            vector<string> pairs;
            for (auto& elt: idlist.second) {
                pairs.push_back(
                    "[" + cursor_to_string(elt.cursor) + ": " +
                    lexical_cast<string>(elt.rdata->length()) + "b" +
                    (elt.rlimit_ids->size()? ", limited by (" + join(sort_keys(*elt.rlimit_ids), ", ") + ")" : "") + 
                    "]"
                );
            }
            result.push_back(id + " => " + join(pairs, ", ") + "\n");
        }
        return join(result, "");
    }

};

}

Storage::DataToSend data_to_send;

#endif

//
// Storage::PairsByFhs: list of IDs by FHs.
//
// Structure: { FH => [ [cursor1, id1], [cursor2, id2], ...] }
// Which IDs are registered in which FH's. This information is used to 
// implement listening on multiple IDs during a single connection.
//

#ifndef REALPLEXOR_PAIRSBYFHS_H
#define REALPLEXOR_PAIRSBYFHS_H

namespace Storage {
using namespace Realplexor;
using std::shared_ptr;

class PairsByFhs
{
    map<void*, shared_ptr<DataPairChain>> storage;

public:

    PairsByFhs() {}

    void set_pairs_for_fh(filehandle_t fh, shared_ptr<DataPairChain> list)
    {
        storage[fh.get()] = list;
    }

    void remove_by_fh(filehandle_t fh)
    {
        storage.erase(fh.get());
    }

    const DataPairChain& get_pairs_by_fh(filehandle_t fh)
    {
        static DataPairChain empty;
        return storage.count(fh.get())? *storage[fh.get()] : empty;
    }

    int get_num_items()
    {
        return storage.size();
    }

    string get_stats()
    {
        vector<string> result;
        for (auto& pairs: storage) {
            result.push_back(
                "(" + lexical_cast<string>(pairs.first) + ") => " + 
                join(
                    apply(*pairs.second, [](const DataPair& e) -> string { return cursor_to_string(e.cursor) + ":" + e.id; }), 
                    ", "
                ) +
                "\n"
            );
        }
        return join(result, "");
    }

};

}

Storage::PairsByFhs pairs_by_fhs;

#endif

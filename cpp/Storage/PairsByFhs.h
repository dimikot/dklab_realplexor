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
// Storage::PairsByFhs: list of IDs by FHs.
//
// Structure: { FH => [ [cursor1, id1], [cursor2, id2], ...] }
// Which IDs are registered in which FHs. This information is used to
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

    void set_pairs_for_fh(fh_t fh, shared_ptr<DataPairChain> list)
    {
        storage[fh.get()] = list;
    }

    void remove_by_fh(fh_t fh)
    {
        storage.erase(fh.get());
    }

    const DataPairChain& get_pairs_by_fh(fh_t fh)
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

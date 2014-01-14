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
// Storage::Events: list of events.
//
// Structure: [ [ time, type, id ], event2, ...] }
// Holds list of events. First event is newer than second event etc.
//

#ifndef REALPLEXOR_STORAGE_EVENTS_H
#define REALPLEXOR_STORAGE_EVENTS_H

namespace Storage {
using namespace Realplexor;

class Events
{
    DataEventChain chain;
    cursor_t cur_pos;

public:

    Events(): cur_pos(10) {}

    void notify(DataEventType type, const ident_t& id)
    {
        // Keep no more than EVENT_CHAIN_LEN items.
        if (chain.size() > CONFIG.event_chain_len) {
            chain.pop_back();
        }
        // Add item.
        chain.push_front(DataEvent(cur_pos++, type, id));
    }

    // Return events newer than from_cursor in order of their creation.
    void get_recent_events(cursor_t from_cursor, shared_ptr<prefix_checker> checker, DataEventChain& events)
    {
        // Initial request. Return fake event with its cursor.
        if (!from_cursor) {
            events.push_back(DataEvent(cur_pos, FAKE, "FAKE"));
            return;
        }
        unordered_set<ident_t> seen;
        // Iterate most recent events first.
        for (auto& ev: chain) {
            if (ev.cursor <= from_cursor) break;
            if (!seen.count(ev.id) && checker->matched(ev.id)) {
                events.push_front(ev);
                seen.insert(ev.id);
            }
        }
    }

    int get_num_items()
    {
        return chain.size();
    }
};

}

Storage::Events events;

#endif

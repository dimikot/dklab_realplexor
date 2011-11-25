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

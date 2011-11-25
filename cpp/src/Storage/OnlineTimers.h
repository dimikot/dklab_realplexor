//
// Storage::OnlineTimers: timers for each ID to track online users.
//
// Structure: { ID => TimerEvent }
// Each $id has associated a timeout function. The $online_timers{$id} 
// is assigned with a new TimerEvent object on a client connect, but 
// this timer object is turned to count-down only when the client is 
// disconnected. If the client is still connected, timer object is assigned, 
// but is not activated.
//

#ifndef REALPLEXOR_STORAGE_ONLINETIMERS_H
#define REALPLEXOR_STORAGE_ONLINETIMERS_H

namespace Storage {
using std::shared_ptr;

class OnlineTimers
{
    map<ident_t, shared_ptr<Realplexor::Event::ITimer>> storage;

public:

    OnlineTimers() {}

    // Return true if we just assigned this timer, false if it was
    // already assigned.
    template<typename Cb>
    bool assign_stopped_timer_for_id(const ident_t& id, Cb callback)
    {
        bool firstTime = true;
        // Remove current timer if present.
        if (storage.count(id)) {
            storage[id]->remove();
            storage.erase(id);
            firstTime = false;
        }
        // Create new stopped timer.
        auto wrapper = [&storage, callback, id](int) {
            auto guard = storage[id];
            storage.erase(id); // thanks to guard, the timer is deleted only when we exit this closure
            callback(); // it is important for logs to call erase() before the callback
        };
        storage[id].reset(new Realplexor::Event::Timer<decltype(wrapper)>(wrapper));
        return firstTime;
    }

    void start_timer_by_id(const ident_t& id, int timeout)
    {
        if (storage.count(id)) {
            storage[id]->remove(); // needed to avoid multiple addition of the same timer
            storage[id]->start(timeout);
        }
    }

    int get_num_items()
    {
        return storage.size();
    }

    void get_ids_ref(shared_ptr<prefix_checker> checker, vector<ident_t>& result)
    {
        for (auto& pair: storage) {
            if (checker->matched(pair.first)) result.push_back(pair.first);
        }
    }

    string get_stats()
    {
        vector<string> result;
        for (auto& item: storage) {
            result.push_back(item.first + " => assigned\n");
        }
        return join(result, "");
    }

};

}

Storage::OnlineTimers online_timers;

#endif

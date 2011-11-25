//
// Storage::CleanupTimers: timers for each ID to cleanup queue
// which is not updated for a long time.
//
// Structure: { ID => TimerEvent }
//

#ifndef REALPLEXOR_STORAGE_CLEANUPTIMERS_H
#define REALPLEXOR_STORAGE_CLEANUPTIMERS_H

namespace Storage {
using std::shared_ptr;

class CleanupTimers
{
    map<ident_t, shared_ptr<Realplexor::Event::ITimer>> storage;

public:

    CleanupTimers() {}

    template<typename Cb>
    void start_timer_for_id(ident_t id, int timeout, Cb callback)
    {
        // Remove current timer if present.
        if (storage.count(id)) {
            storage[id]->remove();
            storage.erase(id);
        }
        // Create new timer.
        auto wrapper = [&storage, callback, id](int) { 
            auto guard = storage[id];
            storage.erase(id); // thanks to guard, the timer is deleted only when we exit this closure
            callback(); // it is important for logs to call erase() before the callback
        };
        storage[id].reset(new Realplexor::Event::Timer<decltype(wrapper)>(wrapper));
        storage[id]->start(timeout);
    }

    int get_num_items()
    {
        return storage.size();
    }
    
    string get_stats()
    {
        vector<string> result;
        for (auto &item: storage) {
            result.push_back(item.first + " => assigned\n");
        }
        return join(result, "");
    }

};

}

Storage::CleanupTimers cleanup_timers;

#endif

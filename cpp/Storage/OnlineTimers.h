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
        auto wrapper = [this, callback, id](int) {
            auto guard = this->storage[id];
            this->storage.erase(id); // thanks to guard, the timer is deleted only when we exit this closure
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

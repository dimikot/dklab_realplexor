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
        auto wrapper = [this, callback, id](int) {
            auto guard = this->storage[id];
            this->storage.erase(id); // thanks to guard, the timer is deleted only when we exit this closure
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

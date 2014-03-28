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

#ifndef REALPLEXOR_EVENT_TIMER_H
#define REALPLEXOR_EVENT_TIMER_H

namespace Realplexor { namespace Event {

class ITimer
{
public:
    virtual void start(int timeout) =0;
    virtual void remove() =0;
    virtual ~ITimer() {}
};


template<typename Cb>
class Timer: public ITimer
{
    ev0x::timer<Cb> w;

public:

    Timer(Cb handler): w(handler)
    {
        w.set(0, 0);
    }

    void start(int timeout)
    {
        w.set(timeout, 0);
        w.start();
    }

    void remove()
    {
        // Nothing: removed by destructor.
    }

    virtual ~Timer() {}
};

}}
#endif

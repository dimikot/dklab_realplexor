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
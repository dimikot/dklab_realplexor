#ifndef REALPLEXOR_EVENT_SIGNAL_H
#define REALPLEXOR_EVENT_SIGNAL_H

namespace Realplexor { namespace Event {

template<typename Cb>
class Signal 
{
    ev0x::sig<Cb> w;

public:
    
    Signal(int signal, Cb handler): w(handler)
    {
        w.set(signal);
        w.start();
    }

    virtual ~Signal() {}
};

}}
#endif
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

#ifndef EVPP0X_H
#define EVPP0X_H

#include <ev++.h>
namespace ev0x {

// Unfortunately we do not yet have typedef templates, so use plain old macros.
#define CLASS_WRAPPER(EV)                                  \
    struct EV##_base: public ev::EV                        \
    {                                                      \
        virtual ~EV##_base() {}                            \
    };                                                     \
                                                           \
    typedef std::shared_ptr<EV##_base> EV##_ptr;           \
                                                           \
    template<typename Cb>                                  \
    class EV: public EV##_base                             \
    {                                                      \
        Cb handler;                                        \
        void invoke(ev::EV &w, int revents)                \
        {                                                  \
            handler(revents);                              \
        }                                                  \
    public:                                                \
        EV(Cb handler): handler(handler)                   \
        {                                                  \
            set<EV<Cb>, &EV<Cb>::invoke>(this);            \
        }                                                  \
        virtual ~EV() {}                                   \
    }

CLASS_WRAPPER(sig);
CLASS_WRAPPER(timer);
CLASS_WRAPPER(io);

}
#endif
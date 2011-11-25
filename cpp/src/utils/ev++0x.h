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
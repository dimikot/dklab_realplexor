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

#ifndef UTILS_CHECKED_MAP_H
#define UTILS_CHECKED_MAP_H

//
// Map with get() method which throws an exception if no key is found.
//
template <typename K, typename V>
class checked_map: public map<K, V>
{
    string name;
public:
    checked_map(string name = "map"): name(name) {}

    V get(const K& k)
    {
        if (!((const map<K, V>*)this)->count(k)) {
            throw out_of_range("No key " + k + " presented in " + name);
        }
        return (*this)[k];
    }

    template<class T>
    T get(const K& k)
    {
        try {
            return lexical_cast<T>(get(k));
        } catch (bad_lexical_cast e) {
            throw out_of_range("Error while casting of the key " + k + " presented in " + name);
        }
    }

    checked_map& operator=(const map<K, V>& m)
    {
        ((map<K, V>*)this)->operator=(m);
        return *this;
    }
};

#endif

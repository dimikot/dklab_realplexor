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

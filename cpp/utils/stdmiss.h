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

#ifndef UTILS_STDMISS_H
#define UTILS_STDMISS_H

string operator+ (const char* s1, const string& s2)
{
    return string(s1) + s2;
}

// Magic!
template<typename V, typename Cb>
auto apply(V vec, Cb&& callback) -> vector<decltype(callback(*vec.begin()))>
{
    vector<decltype(callback(*vec.begin()))> result;
    for (auto &e: vec) result.push_back(callback(e));
    return result;
}

template<typename KT, typename VT>
vector<KT> keys(const map<KT, VT>& m)
{
    vector<KT> result;
    for (auto pair: m) result.push_back(pair.first);
    return result;
}

template<typename KT>
vector<KT> sort_keys(const unordered_set<KT>& s)
{
    vector<KT> result;
    for (auto e: s) result.push_back(e);
    sort(result.begin(), result.end());
    return result;
}

vector<string> split(const char *separators, const string& s)
{
    vector<string> strs;
    boost::split(strs, s, boost::is_any_of(separators));
    return strs;
}

vector<string> split(const regex& re, const string& s)
{
    vector<string> strs;
    boost::algorithm::split_regex(strs, s, re);
    return strs;
}

vector<string> split_trim(const char *separators, const string& s)
{
    vector<string> strs = split(separators, s);
    for (auto i = strs.begin(); i != strs.end(); ++i) {
        *i = trim_copy(*i);
    }
    return strs;
}

#endif

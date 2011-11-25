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

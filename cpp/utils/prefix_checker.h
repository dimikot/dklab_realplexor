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

#ifndef UTILS_PREFIX_CHECKER_H
#define UTILS_PREFIX_CHECKER_H

//
// Allows to check a string over the list of prefixes.
//
class prefix_checker
{
    vector<string> list;
    string common_prefix;
    bool need_matching;

public:
    prefix_checker(const vector<string>& list, const string& common_prefix): common_prefix(common_prefix)
    {
        if (list.size()) {
            need_matching = true;
            for (auto& e: list) {
                // Push only elements with this common_prefix (or all if no common_prefix is specified).
                if (!common_prefix.length() || e.find(common_prefix) == 0) {
                    this->list.push_back(e);
                }
            }
        } else if (common_prefix.length()) {
            // Empty list, but common prefix exists: use it as matcher.
            need_matching = true;
            this->list.push_back(common_prefix);
        } else {
            // Always matching list.
            need_matching = false;
        }
    }

    bool matched(const string& s)
    {
        if (!need_matching) return true;
        for (auto& e: list) {
            if (s.find(e) == 0) return true;
        }
        return false;
    }
};

#endif

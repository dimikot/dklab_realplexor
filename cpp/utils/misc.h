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

#ifndef REALPLEXOR_UTILS_H
#define REALPLEXOR_UTILS_H

string SELF;
vector<string> ARGV;

void init_argv(char** argv)
{
    SELF = system_complete(path(argv[0])).string();
    ARGV.clear();
    for (int i = 1; argv[i]; i++) {
        ARGV.push_back(argv[i]);
    }
}

string extract_option(string opt)
{
    auto old = ARGV;
    ARGV.clear();
    string val;
    for (auto i = old.begin(); i != old.end(); ++i) {
        if (*i == opt) {
            ++i;
            if (i != old.end()) {
                val = *i;
            }
        } else {
            ARGV.push_back(*i);
        }
    }
    return val;
}

string get_root_dir()
{
    return path(SELF).branch_path().string();
}

string vformat(const char *fmt, va_list ap)
{
    // Allocate a buffer on the stack that's big enough for us almost
    // all the time.  Be prepared to allocate dynamically if it doesn't fit.
    size_t size = 1024;
    char stackbuf[1024];
    vector<char> dynamicbuf;
    char *buf = &stackbuf[0];
    while (1) {
        // Try to vsnprintf into our buffer.
        int needed = vsnprintf (buf, size, fmt, ap);
        // NB. C99 (which modern Linux and OS X follow) says vsnprintf
        // failure returns the length it would have needed.  But older
        // glibc and current Windows return -1 for failure, i.e., not
        // telling us how much was needed.
        if (needed <= (int)size && needed >= 0) {
            // It fit fine so we're done.
            return string (buf, (size_t) needed);
        }
        // vsnprintf reported that it wanted to write more characters
        // than we allotted.  So try again using a dynamic buffer.  This
        // doesn't happen very often if we chose our initial size well.
        size = (needed > 0) ? (needed+1) : (size*2);
        dynamicbuf.resize (size);
        buf = &dynamicbuf[0];
    }
}

string format(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    string buf = vformat(fmt, ap);
    va_end(ap);
    return buf;
}

const string strerrno()
{
    return strerror(errno);
}

void die(string s)
{
    s = regex_replace(s, regex("\\$!"), [](smatch s) { return strerrno(); });
    throw runtime_error(s);
}

string backtick(string cmd)
{
    FILE* f = popen(cmd.c_str(), "r");
    if (!f) {
        die(format("popen(%s): $!", cmd.c_str()));
    }
    string str("");
    char c;
    while (EOF != (c = fgetc(f))) {
        str += c;
    }
    pclose(f);
    return str;
}

string strftime(string fmt, ptime now)
{
  auto facet = new time_facet(fmt.c_str());
  std::basic_stringstream<char> ss;
  ss.imbue(std::locale(std::cout.getloc(), facet));
  ss << now;
  return ss.str();
}

string strftime_std(ptime now)
{
    return strftime("%a %b %e %H:%M:%S %Y", now);
}

bool is_file(const string& filename)
{
    ifstream in;
    in.open(filename.c_str());
    if (in.fail()) {
        return false;
    }
    in.close();
    return true;
}

string read_file(string fname)
{
    ifstream f(fname.c_str());
    if (!f) die("Cannot open " + fname + ": $!\n");
    string content;
    while (f) {
        char buf[1024];
        f.getline(buf, sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = 0;
        content += string(buf) + "\n";
    }
    content = regex_replace(content, regex("\r"), "");
    return content;
}

void strip_comments(string& line)
{
    size_t p = line.find("#");
    if (p != string::npos) {
        line.erase(p, string::npos);
    }
    trim(line);
}

bool get_http_body(const string& data, size_t& pos)
{
    size_t d = 2;
    size_t p = data.find("\n\n");
    if (p == data.npos) {
        d = 3;
        p = data.find("\n\r\n");
    }
    if (p != data.npos) {
        pos = p + d;
        return true;
    }
    return false;
}

#endif

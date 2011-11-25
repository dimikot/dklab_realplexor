//
// Dklab Realplexor: Comet server which handles 100000+ parallel browser connections.
// Author: Dmitry Koterov, Jul 2011
//
// This is a line-by-line rewrite of Perl prototype code with obvious speed 
// optimizations (like avoiding of not necessery copies, config pre-parsing 
// etc).
//
// The code is so compact (2600 lines) and so simple, that I decided not to
// split it into *.hpp & *.cpp files and do not create Makefiles, but place 
// everything into included *.h files (like Perl, Java, C# and most of other 
// languages do). It is not quite common for C++, but it surely economize time 
// when a program is small (especially when it is rewritten line by line
// from another language).
//
// Also the code has global variables within the top namespace: one variable 
// per Storage and one CONFIG, they are like singletons.
//
// P.S.
// Use 4-space tab width. To edit the code I used MS Visual Studio, Far 
// editor and Midnight Commander, all with different "tabs to space" 
// settings and on different machines, so tabs and spaces are ugly mixed 
// within the code.
//

#include <vector>
#include <list>
#include <unordered_set>
#include <string>
#include <stdarg.h>
#include <stdexcept>
#include <fstream>
#include <errno.h>
#include <iostream>
#include <map>
#include <set>
#include <exception>
#include <algorithm>
#include <functional>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/numeric/conversion/cast.hpp>

#include <signal.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

using namespace std;
using namespace boost;
using namespace boost::algorithm;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace boost::filesystem;
using std::shared_ptr;
using std::exception;

#include "utils/misc.h"
#include "utils/checked_map.h"
#include "utils/prefix_checker.h"
#include "utils/stdmiss.h"
#include "utils/Socket.h"
#include "utils/ev++0x.h"

#include "dklab_realplexor.h"
#include "Realplexor/Config.h"
#include "Realplexor/Tools.h"
#include "Realplexor/Event/Server.h"
#include "Realplexor/Event/Timer.h"
#include "Realplexor/Event/Signal.h"
#include "Realplexor/Event/Connection.h"
#include "Storage/ConnectedFhs.h"
#include "Storage/CleanupTimers.h"
#include "Storage/OnlineTimers.h"
#include "Storage/Events.h"
#include "Storage/DataToSend.h"
#include "Storage/PairsByFhs.h"
#include "Realplexor/Common.h"
#include "Connection/In.h"
#include "Connection/Wait.h"


void mainloop() 
{
    string ulimit = backtick("/bin/sh -c 'ulimit -n'");
    LOGGER("Starting. Opened files limit (ulimit -n): " + trim_copy(ulimit) + ".");
    
    // Load configs.
    string additional_conf = ARGV.size()? ARGV[0] : "";
    CONFIG.load(additional_conf);

    // Initialize servers.
    Realplexor::Event::Server<Connection::Wait> wait(
        "WAIT", // name
        CONFIG.wait_addr, // listen
        CONFIG.wait_timeout, // timeout
        &Realplexor::Common::logger
    );
    Realplexor::Event::Server<Connection::In> in(
        "IN", // name
        CONFIG.in_addr, // listen
        CONFIG.in_timeout, // timeout
        &Realplexor::Common::logger
    );
    
    // Catch signals.
    auto sigHupCallback = [&additional_conf](int revents) {
        LOGGER("SIGHUP received, reloading the config");
        string low_level_opt = CONFIG.reload(additional_conf);
        if (low_level_opt != "") {
            LOGGER("Low-level option \"" + low_level_opt + "\" is changed, restarting the script from scratch");
            exit(0);
        }
    };
    Realplexor::Event::Signal<decltype(sigHupCallback)> sigHup(SIGHUP, sigHupCallback);

    auto sigIntCallback = [](int revents) {
        LOGGER("SIGINT received, exiting");
        exit(0);
    };
    Realplexor::Event::Signal<decltype(sigIntCallback)> sigInt(SIGINT, sigIntCallback);

    auto sigPipeCallback = [](int revents) {
        LOGGER("SIGPIPE ignored");
    };
    Realplexor::Event::Signal<decltype(sigPipeCallback)> sigPipe(SIGPIPE, sigPipeCallback);

    // If running as root, SU to safe user.
    string user = CONFIG.su_user;
    if (!getuid() && user != "") {
        // Use Perl to be independent of glibc's getpwnam() function
        // (because glibc's getpwnam() requires dynamic glibc linking).
        vector<string> ug = split(
            " ",
            backtick("perl -e '(undef,undef,$uid,$gid)=getpwnam \"" + user + "\"; print \"$uid $gid\" if $uid'")
        );
        if (!ug[0].length()) die("User " + user + " is not found");
        LOGGER("Switching current user to unprivileged \"" + user + "\"");
        uid_t uid = lexical_cast<uid_t>(ug[0]);
        gid_t gid = lexical_cast<gid_t>(ug[1]);
        setegid(gid);
        setgid(gid);
        seteuid(uid);
        setuid(uid);
    }
    
    Realplexor::Event::mainloop();
}


int main(int argc, char **argv) 
{
    init_argv(argv);
    
    // Re-run self with high ulimit.
    Realplexor::Tools::rerun_unlimited();

    // Chdir to script's directory.
    if (0 != chdir(get_root_dir().c_str())) {
        die("Cannot chdir to " + get_root_dir() + ": $!");
    }
    
    // Turn on zombie auto-reaper.
    signal(SIGCHLD, SIG_IGN);

    // Child PID (static - for callback).
    static pid_t pid = 0;

    // Parse command-line args. 
    static string pid_file = extract_option("-p");

    // Load config: it is also needed by parent watchdog.
    string additional_conf = ARGV.size()? ARGV[0] : "";
    CONFIG.load(additional_conf, 1);
    
    // Save PID?
    if (pid_file != "") {
        ofstream f(pid_file);
        if (!f) die("Cannot create " + pid_file + ": $!\n");
        f << getpid();
    }
    
    // Called if process dies.
    // Used variables must be global!
    atexit([]() {
        if (!pid) return; // children
        Realplexor::Tools::graceful_kill(pid, pid_file);
    });

    // Run watchdog loop.
    while (1) {
        signal(SIGHUP, SIG_DFL);
        signal(SIGINT, SIG_DFL);
        signal(SIGTERM, SIG_DFL);
        
        pid = fork();
        if (pid < 0) {
            // Fork failed.
            LOGGER("fork() failed: errno=" + lexical_cast<string>(errno));
        } else if (!pid) {
            // Child process.
            mainloop();
            exit(0);
        }
        
        // Pass SIGHUP to children.
        signal(SIGHUP, [](int) {
            kill(pid, SIGHUP);
        });
        // Catch signals to kill the child too.
        signal(SIGINT, [](int) {
            LOGGER("SIGINT received, exiting");
            exit(1);
        });
        signal(SIGTERM, [](int) {
            LOGGER("SIGTERM received, exiting");
            exit(1);
        });
        
        // Process other signals.
        // Waid for child termination.
        Realplexor::Tools::wait_pid_with_memory_limit(pid, CONFIG.max_mem_mb);
        sleep(1);
    }
    
    return 0;
}

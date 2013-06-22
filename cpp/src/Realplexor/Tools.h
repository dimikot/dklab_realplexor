#ifndef REALPLEXOR_TOOLS_H
#define REALPLEXOR_TOOLS_H

namespace Realplexor { 

class Tools 
{
public:

    // Return HiRes time. It is guaranteed that two sequencial calls
    // of this function always return different time, second > first.
    static cursor_t time_hi_res()
    {
        cursor_t time = ev::now(EV_DEFAULT);
        static int time_counter = 0;
        const int cycle = 1000;
        time_counter++;
        if (time_counter > cycle) time_counter = 0;
        long double add = (1.0 / cycle) * time_counter * 0.000001;
        return time + add;
    }

    // Rerun the script unlimited.
    static void rerun_unlimited()
    {
        struct rlimit lim { 1048576, 1048576 };
        setrlimit(RLIMIT_NOFILE, &lim);
    }

    // Gracefully kills a process.
    static void graceful_kill(pid_t pid, string pid_file = "")
    {
        kill(pid, 2);
        sleep(1);
        if (kill(pid, 0) == 0) {
            kill(pid, 9);
            cerr << "Killed the child using a heavy SIGKILL.\n";
        } else {
            cerr << "Normally terminated.\n";
        }
        if (pid_file != "") {
            unlink(pid_file.c_str());
        }
    }

    // Returns amount of used memory by pid (in megabytes).
    static double get_memory_usage(pid_t pid)
    {
        string mem = backtick(
            "ps -p " + lexical_cast<string>(pid) + " -o rss "
#ifdef __APPLE__
    "| awk 'NR>1'"
#else
    "--no-headers"
#endif
        );
        mem = regex_replace(mem, regex("\\s+"), "");
        if (mem == "") return 0;
        return lexical_cast<double>(mem) / 1024;
    }

    // Wait for a process termination.
    // If the process limits memory usage, kills it.
    static void wait_pid_with_memory_limit(pid_t pid, double limit)
    {
        int status;
        while (waitpid(pid, &status, WNOHANG) != -1) {
            sleep(1);
            double mem = get_memory_usage(pid);
            if (limit && mem > limit) {
                cerr << "Daemon process uses " << mem << " MB of memory which is larger than " << limit << " MB. Killing...\n";
                graceful_kill(pid);
                break;
            }
        }
    }
};

}
#endif

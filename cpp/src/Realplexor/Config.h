#ifndef REALPLEXOR_CONFIG_H
#define REALPLEXOR_CONFIG_H


namespace Realplexor {

class Config 
{
    checked_map<string, string> config;
    logger_t logger;

    struct StaticFile
    {
        string content;
        string time;
    };

public:
    int                          verbosity;
    checked_map<string, string>  users;
    size_t                       max_data_for_id;
    string                       wait_addr;
    int                          wait_timeout;
    string                       in_addr;
    int                          in_timeout;
    string                       su_user;
    double                       max_mem_mb;
    size_t                       event_chain_len;
    size_t                       in_maxlen;
    int                          clean_id_after;
    string                       charset;
    size_t                       wait_maxlen;
    int                          offline_timeout;
    string                       iframe_id;
    string                       script_id;
    StaticFile                   static_iframe;
    StaticFile                   static_script;

    string  IDENTIFIER_PLUS_EQ;
    regex   RE_LOGIN_PASSWORD_ID;
    regex   RE_CURSOR_ID;

    Config(): config("config"), users("users list") 
    {
        logger = [](const string&)->void{}; // default
    }

    // Sets another logger routine for this config.
    void set_logger(logger_t l)
    {
        logger = l;
    }

    // Loads the config from the file.
    void load(string add, bool silent = false)
    {
        // Reset config.
        config.clear();
        // Read default config.
        _load_config_file(get_root_dir() + "/dklab_realplexor.conf");
        // Read custom config.
        if (add.length()) {
            if (is_file(add)) {
                if (!silent) logger("CONFIG: appending configuration from " + add);
                _load_config_file(add);
            } else {
                logger("CONFIG: file " + add + " does not exist, skipping");
            }
        }
        _load_users(config.get("USERS_FILE"));
        _parse();
    }

    // Returns "" if reloading is succeeded, else returns the name of
    // option which could not be reloaded.
    string reload(string add)
    {
        regex lowlevel("^(WAIT_ADDR|WAIT_TIMEOUT|IN_ADDIN_TIMEOUT|SU_.*)$");
        regex ignore("^(HOOK_|.*_CONTENT)$");
        // Load new config.
        auto old = config;
        try {
            load(add);
        } catch (const std::exception &e) {
            logger(string("Error reloading config, continue with old settings: ") + e.what());
            config = old;
            _parse();
            return "";
        }
        for (auto& i: config) {
            string opt = i.first;
            string v_old = old.count(opt)? old[opt] : "";
            string v_new = config.count(opt)? config[opt] : "";
            if (v_old != v_new) {
                if (regex_match(opt, lowlevel)) return opt;
                if (regex_match(opt, ignore)) continue;
                logger("Option " + opt + " is changed: " + v_old + " -> " + v_new);
            }
        }
        return "";
    }


private:

    string _perl_to_text(string fname)
    {
        string data = read_file(fname);
        if (regex_search(data, regex("^[^#]*[%$]CONFIG", regex::icase))) {
            // Perl config: run it & print the result for later parsing.
            data = backtick(
                "perl -e '"
                    "#line 1 \"config-parser\"\n"
                    "do qq{" + fname + "}; die $@ if $@;"
                    "while (my ($k, $v) = each(%CONFIG)) {"
                        "print qq{$k = \"};"
                        "print $v if !ref($v);"
                        "print join(qq{ }, @$v) if ref($v) eq qq{ARRAY};"
                        "print qq{\"\\n};"
                    "}"
                "'"
            );
        }
        return data;
    }

    void _load_config_file(string fname)
    {
        for (string line: split("\n", _perl_to_text(fname))) { 
            strip_comments(line);
            vector<string> pair = split("=", line);
            if (pair.size() < 2) continue;
            string k = trim_copy(pair[0]);
            string v = trim_copy(pair[1]);
            if (v.length() >= 2 && v[0] == '"' && v[v.length() - 1] == '"') {
                v.erase(0, 1);
                v.erase(v.length() - 1);
            }
            config[k] = v;
        }
    }

    void _load_users(string fname)
    {
        users.clear();
        if (fname[0] != '/') fname = get_root_dir() + "/" + fname;
        for (string line: split("\n", read_file(fname))) { 
            strip_comments(line);
            vector<string> pair = split(":", line);
            if (pair.size() < 2) continue;
            string login = trim_copy(pair[0]);
            string pass = trim_copy(pair[1]);
            if (login.length() && !regex_match(login, regex("^[a-zA-Z0-9]+$"))) {
                // Must not contain special characters like "_" and others.
                logger(string("Warning: login \"") + login + "\" is not alphanumeric, skipped.");
                continue;
            }
            users[login] = pass;
        }
    }

    void _parse()
    {
        // Parse config options.
        verbosity = config.count("VERBOSITY")? lexical_cast<int>(config.get("VERBOSITY")) : 100;
        max_data_for_id = lexical_cast<size_t>(config.get("MAX_DATA_FOR_ID"));
        wait_addr = config.get("WAIT_ADDR");
        wait_timeout = lexical_cast<int>(config.get("WAIT_TIMEOUT"));
        in_addr = config.get("IN_ADDR");
        in_timeout = lexical_cast<int>(config.get("IN_TIMEOUT"));
        su_user = config.get("SU_USER");
        max_mem_mb = lexical_cast<double>(config.get("MAX_MEM_MB"));
        event_chain_len = lexical_cast<size_t>(config.get("EVENT_CHAIN_LEN"));
        in_maxlen = lexical_cast<size_t>(config.get("IN_MAXLEN"));
        clean_id_after = lexical_cast<int>(config.get("CLEAN_ID_AFTER"));
        charset = config.get("CHARSET");
        wait_maxlen = config.get<size_t>("WAIT_MAXLEN");
        offline_timeout = lexical_cast<int>(config.get("OFFLINE_TIMEOUT"));
        iframe_id = config.get("IFRAME_ID");
        script_id = config.get("SCRIPT_ID");
        _fill_static_file("IFRAME", static_iframe);
        _fill_static_file("SCRIPT", static_script);

        // Generate combined constant values for faster access.
        IDENTIFIER_PLUS_EQ = config.get("IDENTIFIER") + "=";
        RE_LOGIN_PASSWORD_ID = regex(
            "\\b" +
            IDENTIFIER_PLUS_EQ +
            "(?:(\\w+):([^@\\s]+)@)?" +
            "([*\\w,.:]*)" +
            // At the end, find a character, NOT the end of the string! 
            // Because only a chunk may finish, not the whole data.
            "[^*\\w,.:]"
        );
        RE_CURSOR_ID = regex("^(\\*?)(?:(\\d+(?:\\.\\d+)?):)?(\\w+)$");
    }

    void _fill_static_file(const string& param, StaticFile& f)
    {
        string fname = config.get(param + "_FILE");
        if (fname[0] != '/') fname = get_root_dir() + "/" + fname;
        string content = read_file(fname);
        content = regex_replace(content, regex("\\$([a-zA-Z]\\w*)"), [this](smatch s) {
            return config.count(s[1])? config.get(s[1]) : string("undefined-") + s[1];
        });
        f.content = content;
        f.time = strftime("%a, %e %b %Y %H:%M:%S GMT", from_time_t(boost::filesystem::last_write_time(fname.c_str())));
    }
};

}

Realplexor::Config CONFIG;

#endif

#ifndef UTILS_SOCKET_H
#define UTILS_SOCKET_H


//
// Simple socket wrapper class.
//
class Socket
{
    int fh;
    string addr;

    Socket(int fh): fh(fh) {}
    Socket(const Socket& s);
    Socket& operator=(const Socket& s);

public:

    // Creates a listening socket.
    Socket(string localAddr): addr(localAddr)
    {
        auto parts = split(":", localAddr);
        if (parts.size() < 2) die("Address may be in form of \"host:port\", \"" + localAddr + "\" given");
        fh = socket(AF_INET, SOCK_STREAM, 0);
        if (fh < 0) die("ERROR calling socket(): $!");

        // Avoid "address already in use" message at bind() stage.
        int opt = 1;
        setsockopt(fh, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        
        struct sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = inet_addr(parts[0].c_str());
        serv_addr.sin_port = htons(lexical_cast<int>(parts[1]));
        if (::bind(fh, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
            die("ERROR calling bind(): $!");
        }
        listen(fh, 50000);
    }

    // Creates an accepted socket.
    Socket(int fh, const string& addr): fh(fh), addr(addr) 
    {
    }

    virtual ~Socket()
    {
        close(fh);
    }

    int fileno()
    {
        return fh;
    }

    string peeraddr()
    {
        return addr;
    }

    void blocking(bool block)
    {
        int flags = fcntl(fh, F_GETFL, 0);
        fcntl(fh, F_SETFL, block? (flags & (~O_NONBLOCK)) : (flags | O_NONBLOCK));
    }

    std::shared_ptr<Socket> accept()
    {
        struct sockaddr_in cli_addr;
        socklen_t clilen = sizeof(cli_addr);
        int newsockfd = ::accept(fh, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) {
            die("ERROR calling accept(): $!");
        }
        return std::shared_ptr<Socket>(new Socket(newsockfd, string(inet_ntoa(cli_addr.sin_addr)) + ":" + lexical_cast<string>(cli_addr.sin_port)));
    }

    // Appends read data to the end of the string.
    // Returns the number of read bytes.
    size_t read_and_append_to(string& s)
    {
        size_t nread = 0;
        while (true) {
            char buf[1024 * 32];
            int n = ::read(fh, buf, sizeof(buf));
            if (n < 0) {
                die("ERROR calling read(): $!");
            }
            s.append(buf, n);
            nread += n;
            if (n < (int)sizeof(buf)) break;
        }
        return nread;
    }

    // Returns:
    // - 1 if the operation succeeded
    // - 0 if nothing was sent
    // - -1 in case of an error
    int write(const char* buf, size_t len)
    {
        while (true) {
            int n = ::write(fh, buf, len);
            if (n < 0) return -1;
            if (n == (int)len) return 1;
            len -= n;
            buf += n;
        }
    }

    int write(const string& s)
    {
        return write(s.c_str(), s.length());
    }

    void flush()
    {
        // Do nothing - sockets are unbuffered.
    }

    // Returns 0 on error, 1 on success.
    int shutdown(int how)
    {
        return ::shutdown(fh, how) < 0? 0 : 1;
    }
};

// "Scalar" value of a socket with closing on last reference destruction.
typedef std::shared_ptr<Socket> filehandle_t;

#endif

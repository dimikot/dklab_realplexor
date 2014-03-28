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

#ifndef DKLAB_REALPLEXOR_H
#define DKLAB_REALPLEXOR_H

namespace Realplexor {
using std::shared_ptr;

// "Scalar" value of a connection with closing on last reference destruction.
namespace Event { class FH; };
typedef std::shared_ptr<Realplexor::Event::FH> fh_t;

// Reference to a logger procedure.
typedef void (*logger_t)(const string&);

// Cursor (precise).
typedef long double cursor_t;

// Identifier (channel name).
typedef string ident_t;

// Set of IDs to match.
typedef unordered_set<string> LimitIdsSet;

// Set of IDs to send.
typedef unordered_set<string> IdsToSendSet;

// Pair of listening WAIT data.
struct DataPair {
    cursor_t cursor;
    ident_t id;
    DataPair() {}
    DataPair(cursor_t cursor, ident_t id): cursor(cursor), id(id) {}
};
typedef vector<DataPair> DataPairChain;

// Credentials.
struct CredPair {
    string login;
    string password;
    CredPair() {}
private:
    CredPair(const DataPair& p);
    CredPair& operator=(const CredPair& p);
};

// Logged event types.
enum DataEventType {
    ONLINE = 0,
    OFFLINE = 1,
    FAKE = 2,
};

// Logged event.
struct DataEvent {
    cursor_t cursor;
    DataEventType type;
    ident_t id;

    DataEvent() {}
    DataEvent(cursor_t cursor, DataEventType type, ident_t id): cursor(cursor), type(type), id(id) {}

    string getType()
    {
        if (type == DataEventType::ONLINE) return "online";
        if (type == DataEventType::OFFLINE) return "offline";
        if (type == DataEventType::FAKE) return "FAKE";
        return "?";
    }
};
typedef list<DataEvent> DataEventChain;

// Which cursor at which filehandle is current.
struct DataCursorFh {
    cursor_t cursor;
    fh_t fh;
    DataCursorFh() {}
    DataCursorFh(cursor_t cursor, fh_t fh): cursor(cursor), fh(fh) {}
};
typedef map<void*, DataCursorFh> DataCursorFhByFh;

// Pice of data which was received and which must be sent.
struct DataChunk {
    cursor_t cursor;
    shared_ptr<string> rdata;
    shared_ptr<unordered_set<ident_t>> rlimit_ids;
    DataChunk() {}
    DataChunk(cursor_t cursor, shared_ptr<string> rdata, shared_ptr<unordered_set<ident_t>> rlimit_ids): cursor(cursor), rdata(rdata), rlimit_ids(rlimit_ids) {}
};
typedef list<DataChunk> DataChunkChain;

// Piece of data ready to be sent to a fh.
struct DataToSendChunk
{
    fh_t fh;
    cursor_t cursor;
    shared_ptr<string> rdata;
    map<ident_t, cursor_t> ids;
    DataToSendChunk() {}
private:
    // Unfortunately we cannot disable copy constructor, because it is needed by std::map,
    // mut we disable operator=.
    DataToSendChunk& operator=(const DataToSendChunk& p);
};
typedef map<const string*, DataToSendChunk> DataToSendByDataRef;
typedef map<const fh_t::element_type*, DataToSendByDataRef> DataToSendByFh;

string cursor_to_string(cursor_t cur)
{
    // 16 digits is for full phpt-tests compatibility with Perl version
    string v = format("%0.14LF", cur);
    // Make the value 100%-compatible with Perl version.
    auto point = v.find(".");
    if (point != v.npos) {
        for (size_t i = point + 1; i < v.length(); i++) {
            // If we have at least one non-zero after the '.' char,
            // return 14-digit version.
            if (v[i] != '0') return v;
        }
        // We have a true decimal value, skip the point and all digits after it.
        v.erase(point);
    }
    return v;
}

};
#endif

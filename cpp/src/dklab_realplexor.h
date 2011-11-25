#ifndef DKLAB_REALPLEXOR_H
#define DKLAB_REALPLEXOR_H

namespace Realplexor {
using std::shared_ptr;

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
    DataPair(const DataPair& p): cursor(p.cursor), id(p.id) {}
    DataPair& operator=(const DataPair& p) { cursor=p.cursor; id = p.id; return *this; }
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
    DataEvent(const DataEvent& p): cursor(p.cursor), type(p.type), id(p.id) {}
    DataEvent& operator=(const DataEvent& p) { cursor = p.cursor; type = p.type; id = p.id; return *this; }

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
    filehandle_t fh;
    DataCursorFh() {}
    DataCursorFh(cursor_t cursor, filehandle_t fh): cursor(cursor), fh(fh) {}
    DataCursorFh(const DataCursorFh& p): cursor(p.cursor), fh(p.fh) {}
    DataCursorFh& operator=(const DataCursorFh& p) { cursor=p.cursor; fh = p.fh; return *this; }
};
typedef map<void*, DataCursorFh> DataCursorFhByFh;

// Pice of data which was received and which must be sent.
struct DataChunk {
    cursor_t cursor;
    shared_ptr<string> rdata;
    shared_ptr<unordered_set<ident_t>> rlimit_ids;
    DataChunk() {}
    DataChunk(cursor_t cursor, shared_ptr<string> rdata, shared_ptr<unordered_set<ident_t>> rlimit_ids): cursor(cursor), rdata(rdata), rlimit_ids(rlimit_ids) {}
    DataChunk(const DataChunk& p): cursor(p.cursor), rdata(p.rdata), rlimit_ids(p.rlimit_ids) {}
    DataChunk& operator=(const DataChunk& p) { cursor = p.cursor; rdata = p.rdata; rlimit_ids = p.rlimit_ids; return *this; }
};
typedef list<DataChunk> DataChunkChain;

// Piece of data ready to be sent to a fh.
struct DataToSendChunk
{
    filehandle_t fh;
    cursor_t cursor;
    shared_ptr<string> rdata;
    map<ident_t, cursor_t> ids;
    // Unfortunately we cannot disable copy constructor, because it is needed by std::map.
    DataToSendChunk() {}
    DataToSendChunk(const DataToSendChunk& p): fh(p.fh), cursor(p.cursor), rdata(p.rdata), ids(p.ids) {}
private:
    DataToSendChunk& operator=(const DataToSendChunk& p);
};
typedef map<const string*, DataToSendChunk> DataToSendByDataRef;
typedef map<const Socket*, DataToSendByDataRef> DataToSendByFh;

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

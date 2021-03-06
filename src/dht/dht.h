#ifndef _dht_h
#define _dht_h

#include <netinet/in.h>
#include <lcthw/bstrlib.h>

/* Hash */

#define HASH_BYTES 20
#define HASH_BITS (HASH_BYTES * 8)

typedef struct Hash {
    char value[HASH_BYTES];	/* Network byte order */
} Hash;

bstring Dht_HashStr(Hash *hash);

/* Node */

/* Nodes of the DHT network */
typedef struct Node {
    Hash id;
    struct in_addr addr;
    uint16_t port;              /* network byte order */
    time_t reply_time;
    time_t query_time;
    int pending_queries;
    int is_new;                 /* Don't know their id yet */
    unsigned int rfindnode_count;
    unsigned int rgetpeers_count;
    unsigned int rannounce_count;
} Node;

bstring Dht_NodeStr(Node *node);

/* Peer */

/* Peers are BitTorrent clients. We don't talk to them, only search
 * for them and track them. */
typedef struct Peer {
    uint32_t addr;
    uint16_t port;              /* network byte order */
} Peer;

bstring Dht_PeerStr(Peer *peer);

/* Hooks */

typedef enum HookType {         /* HookOp args: */
    HookAddPeer,                /* struct HookPeerData */
    HookFoundPeer,              /* struct HookPeerData */
    HookAnnouncedPeer,          /* struct HookPeerData */
    HookInvalidMessage,         /* Node */
    HookHandleMessage,          /* Message */
    HookSendMessage,            /* Message */
    HookReceiveMessage,         /* Message */
    HookSearchDone,             /* Search */
    HookTypeMax
} HookType;

typedef void (*HookOp)(void *client, void *args);

typedef struct Hook {
    HookType type;
    HookOp fun;
} Hook;

struct HookPeerData {
    Hash *info_hash;
    Peer *peers;
    size_t count;
};

struct HookAnnounceData {
    void *search;
    Node *node;
};

/* Message */

/* Foreign tokens are of unknown length. */
struct FToken {
    char *data;
    size_t len;
};

bstring Dht_FTokenStr(struct FToken ftoken);

typedef enum MessageType {
    MUnknown = 0,
    QPing = 0100, QFindNode = 0101, QGetPeers = 0102, QAnnouncePeer = 0104,
    RPing = 0200, RFindNode = 0201, RGetPeers = 0202, RAnnouncePeer = 0204,
    RError = 0210,
} MessageType;

bstring Dht_MessageTypeStr(MessageType type);

#define MessageType_IsQuery(T) ((T) & 0100)
#define MessageType_IsReply(T) ((T) & 0200)
#define MessageType_AsReply(T) ((T) ^ 0300)

typedef struct QPingData {
} QPingData;

typedef struct QFindNodeData {
    Hash *target;
} QFindNodeData;

typedef struct QGetPeersData {
    Hash *info_hash;
} QGetPeersData;

typedef struct QAnnouncePeerData {
    Hash *info_hash;
    uint16_t port;
    struct FToken token;
} QAnnouncePeerData;

typedef struct RPingData {
} RPingData;

typedef struct RFindNodeData {
    Node **nodes;
    size_t count;
} RFindNodeData;

/* May be cast as RFindNodeData */
typedef struct RGetPeersData {
    Node **nodes;
    size_t count;
    struct FToken token;
    Peer *values;
} RGetPeersData;

typedef struct RAnnouncePeerData {
} RAnnouncePeerData;

typedef struct RErrorData {
    int code;
    bstring message;
} RErrorData;

typedef int32_t merror_t;

typedef struct Message {
    MessageType type;
    merror_t errors;
    Node node;
    char *t;
    size_t t_len;
    Hash id;
    void *context;
    union {
        QPingData qping;
        QFindNodeData qfindnode;
        QGetPeersData qgetpeers;
        QAnnouncePeerData qannouncepeer;
        RPingData rping;
        RFindNodeData rfindnode;
        RGetPeersData rgetpeers;
        RAnnouncePeerData rannouncepeer;
        RErrorData rerror;
    } data;
} Message;

bstring Dht_MessageStr(Message *message);

#define MERROR_UNKNOWN_TYPE       0x0001
#define MERROR_INVALID_QUERY_TYPE 0x0002
#define MERROR_INVALID_TID        0x0004
#define MERROR_INVALID_NODE_ID    0x0008
#define MERROR_INVALID_DATA       0x0010
#define MERROR_PROGRAM            0x0020

bstring Dht_MERROR_Str(merror_t errors);

#define RERROR_GENERIC       201
#define RERROR_SERVER        202
#define RERROR_PROTOCOL      203
#define RERROR_METHODUNKNOWN 204

bstring Dht_RERROR_Str(int code);

/* API */

void *Dht_CreateClient(Hash id, uint32_t addr, uint16_t port, uint16_t peer_port);
void Dht_DestroyClient(void *client);
int Dht_AddNode(void *client, uint32_t addr, uint16_t port);
void *Dht_AddSearch(void *client, Hash info_hash);

bstring Dht_ClientStr(void *client);

int Dht_Start(void *client);
int Dht_Stop(void *client);
int Dht_Process(void *client);

int Dht_AddHook(void *client, Hook *hook);
int Dht_RemoveHook(void *client, Hook *hook);

#endif

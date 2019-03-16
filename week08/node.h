#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <ctype.h>

#define CONNECT_N       10
#define PROT_PING 55
#define PROT_ACK 56
#define PROT_ADD_PEER 50
#define PROT_SYNC_PEERS 60
#define PROT_SYNC_FILES 61
#define PROT_GET_FILE 70
#define PROT_OK 100
#define PROT_NO 200
#define SERVER_PORT     1337
#define MY_IP_ADDRESS "192.168.1.67"
#define TRUE 1
#define FALSE 0
#define PING_INTERVAL 5
#define PEER_BUF 10

#define min(m,n) ((m) < (n) ? (m) : (n))
#define max(m,n) ((m) > (n) ? (m) : (n))
typedef struct Peer {
    char name[25];
    char ip_address[20];
    uint16_t port;
} Peer;
struct LinkedPeerNode {
    struct LinkedPeerNode *next, *previous;
    struct Peer value;
};
struct LinkedPeerList {
    int length;
    struct LinkedPeerNode *self;
};


struct PeerFile {
    char name[25];
};
struct LinkedFileNode {
    struct LinkedFileNode *next, *previous;
    struct PeerFile value;
};
struct LinkedFileList {
    int length;
    struct LinkedFileNode *self;
};
struct Protocol {
    short type;
};

struct greet_client_data {
    int client_socket;
    struct sockaddr_in client_addr;
};
struct PeerNode {
    Peer self;
    struct LinkedPeerList peers;
    struct LinkedFileList files;
};
struct PeerNode this_node;


void *initialise_client(void *);

void *initialise_server(void *);

void *handle_client(void *);

void *ping_clients(void *);

void connect_to_peer(struct Peer);

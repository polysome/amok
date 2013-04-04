#include <arpa/inet.h>
#include <assert.h>
#include <netinet/ip.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <dht/client.h>
#include <dht/network.h>
#include <lcthw/dbg.h>

int NetworkUp(Client *client)
{
    assert(client != NULL && "NULL Client pointer");
    assert(client->socket != -1 && "Invalid Client socket");

    struct sockaddr_in sockaddr = { 0 };

    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = client->node.port;
    sockaddr.sin_addr = client->node.addr;

    int rc = bind(client->socket,
                  (struct sockaddr *) &sockaddr,
                  sizeof(struct sockaddr_in));
    check(rc == 0, "bind failed");

    return 0;
error:
    return -1;
}

int NetworkDown(Client *client)
{
    assert(client != NULL && "NULL Client pointer");
    assert(client->socket != -1 && "Invalid Client socket");

    int rc;

retry:
  
    rc = close(client->socket);

    if (rc == -1 && errno == EINTR)
        goto retry;

    check(rc == 0, "Close on socket fd failed");

    return 0;
error:
    return -1;
}

int Send(Client *client, Node *node, char *buf, size_t len)
{
    assert(client != NULL && "NULL Client pointer");
    assert(node != NULL && "NULL Node pointer");
    assert(buf != NULL && "NULL buf pointer");
    assert(len <= UDPBUFLEN && "buf too large for UDP");

    struct sockaddr_in addr = { 0 };

    addr.sin_family = AF_INET;
    addr.sin_addr = node->addr;
    addr.sin_port = node->port;

    int rc = sendto(client->socket,
                    buf,
                    len,
                    0, (struct sockaddr *) &addr,
                    sizeof(addr));
    check(rc == (int)len, "sendto failed");

    return 0;
error:
    return -1;
}

int Receive(Client *client, Node *node, char *buf, size_t len)
{
    assert(client != NULL && "NULL Client pointer");
    assert(node != NULL && "NULL Node pointer");
    assert(buf != NULL && "NULL buf pointer");
    assert(len >= UDPBUFLEN && "buf too small for UDP");

    struct sockaddr_in srcaddr = { 0 };
    socklen_t addrlen = sizeof(srcaddr);

    int rc = recvfrom(client->socket,
                      buf,
                      len,
                      0,
                      (struct sockaddr *) &srcaddr,
                      &addrlen);

    assert(addrlen == sizeof(srcaddr) && "Unexpected addrlen from recvfrom");

    check(rc >= 0, "Receive failed");

    node->addr = srcaddr.sin_addr;
    node->port = srcaddr.sin_port;

    return rc;
error:
    return -1;
}

int SendMessage(Client *client, Message *msg, Node *node)
{
    assert(client != NULL && "NULL Client pointer");
    assert(msg != NULL && "NULL Message pointer");
    assert(node != NULL && "NULL Node pointer");
    assert(msg->t_len == sizeof(tid_t) && "Wrong outgoing t");

    int len = Message_Encode(msg, client->buf, UDPBUFLEN);
    check(len > 0, "Message_Encode failed");

    int rc = Send(client, node, client->buf, len);
    check(rc == 0, "Send failed");

    if (MessageType_IsQuery(msg->type))
    {
        PendingResponse entry = { msg->type, *(tid_t *)msg->t, node->id, msg->context };

        rc = client->pending->addPendingResponse(client->pending, entry);
        check(rc == 0, "addPendingResponses failed");
    }

    return 0;
error:
    return -1;
}

Message *ReceiveMessage(Client *client, Node *node)
{
    assert(client != NULL && "NULL Client pointer");
    assert(node != NULL && "NULL Node pointer");

    int len = Receive(client, node, client->buf, UDPBUFLEN);
    check(len > 0, "Receive failed");

    Message *message = Message_Decode(client->buf,
                                      len,
                                      (struct PendingResponses *)client->pending);
    check(message != NULL, "Message_Decode failed");

    return message;
error:
    return NULL;
}

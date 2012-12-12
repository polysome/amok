#ifndef _protocol_h
#define _protocol_h

#include <bencode.h>
#include <message.h>

#define OWN_TID_LEN 2

typedef int (*GetResponseType_fp)(void *responses,
				  char *transaction_id,
				  MessageType *type);

struct PendingResponses {
    GetResponseType_fp getResponseType;
};

Message *Message_Decode(char *data, size_t len, struct PendingResponses *pending); 

int Message_Encode(Message *message, char *buf, size_t len);

#endif

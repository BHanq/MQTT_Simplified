//Structure of the header inside the packet
//Enumeration for message type (MST)

#include <arpa/inet.h>
#include <netinet/in.h>

enum MESSAGE_TYPE { HELLO = 0, PUBLISH = 1,PUBACK = 2, SUBSCRIBE=3, SUBACK = 4, DISCONNECT = 5  , CONNECT = 6, CONNACK = 7 ,UNSUB = 8,UNSUBACK = 9, PINGREQ = 10, PINGRESP = 11,PUSH = 12 , PUSHACK = 13};

//Enumeration for both QoS
enum QOS { UNRELIABLE = 0, RELIABLE = 1};

struct Header
{
    unsigned int mst : 4;
    unsigned int rel : 1;
    char headerOption[20];
};

//Structure of a packet
struct Packet
{
    struct Header header;
    char payload[50];
};

struct Topic
{
    char* name;
    struct  sockaddr_in6 ips[2];
};

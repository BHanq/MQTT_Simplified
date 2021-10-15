/* Enumeration for message type (MST) */
enum MESSAGE_TYPE { HELLO = 0, PUBLISH = 1,PUBACK = 2, SUBSCRIBE=3, SUBACK = 4, DISCONNECT = 5  , CONNECT = 6, CONNACK = 7 ,UNSUB = 8,UNSUBACK = 9, PINGREQ = 10, PINGRESP = 11,PUSH = 12 , PUSHACK = 13};

/* Enumeration for both QoS */
enum QOS { UNRELIABLE = 0, RELIABLE = 1};

/* Structure of the header, it contains the mst(message type), rel (reliable) and the headerOption */
struct Header
{
    unsigned int mst : 4;
    unsigned int rel : 1;
    char headerOption[20];
};

/*Structure of a packet, it contains the Header struc and the payload */
struct Packet
{
    struct Header header;
    char payload[50];
};

/* Structure for the ping request with the destination address value */
struct PingReq
{
     const uip_ipaddr_t dest_ipaddr;
};

/* Structure of a topic, it contains the name and ip address valuesgit  */
struct Topic
{
    char* name;
    uip_ipaddr_t ips[2];
};

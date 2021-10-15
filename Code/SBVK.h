#include "contiki.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "structure.h"
#include "sys/log.h"

/* Initiate a connection to a remote device or broker */
void connect(struct simple_udp_connection *udp_conn, const uip_ipaddr_t *destAddr);

/* Recover the message type from the header of the packet added in parameter.*/
int getMessageType(struct Packet packet);

/* Return an acknowledge after a CONNECTION paquet */
void connACK(struct simple_udp_connection *udp_conn,const uip_ipaddr_t *destAddr);

/* Initiate a connection to a remote device or broker */
void hello(struct simple_udp_connection *udp_conn,const uip_ipaddr_t *destAddr,bool init);

/* Finish a transmission (a client give the network) */
void disconnect(struct simple_udp_connection *udp_conn,const uip_ipaddr_t *destAddr);

/* Subscribe to a topic */
void subscribe(struct simple_udp_connection *udp_conn, const uip_ipaddr_t *destAddr, char *topicname);

/* Return an acknowledge after subscription to a topic */
void subACK(struct simple_udp_connection *udp_conn,const uip_ipaddr_t *destAddr);

/* Unsubscribe to a topic */
void unSUB();

/* Return an acknowledge after unsubscription to a topic */
void unSUBACK();

/* Publish information/command for a topic */
void publish(struct simple_udp_connection *udp_conn,const uip_ipaddr_t *destAddr, bool command, char *topicname, char *value);

/* Return an acknowledge after publish of a value */ 
void pubACK(struct simple_udp_connection *udp_conn,const uip_ipaddr_t *destAddr);

/* Transfer an information/command (method for the broker) to the server*/
void push(struct simple_udp_connection *udp_conn, const uip_ipaddr_t *destAddr, bool command, char *topicname, char *value);

/* Return an acknowledge when the information/command is received to the server*/
void pushACK(struct simple_udp_connection *udp_conn, const uip_ipaddr_t *destAddr);

/* Getter of isConnected value. */
bool isConnected();

/* Start the thread wait_ping_process  */
void startPingThread(struct simple_udp_connection *udp_conn,	const uip_ipaddr_t *destAddr);

/* Stop the thread wait_ping_process */
void stopPingThread();

/* Make a ping to check the connection */
void pingreq(struct simple_udp_connection *udp_conn,const uip_ipaddr_t *destAddr);

/* Respond to a ping */
void pingresp(struct simple_udp_connection *udp_conn,const uip_ipaddr_t *destAddr);

/**
 * Handle the message from his type :
 * ( HELLO , PUBLISH, SUBSCRIBE, DISCONNECT, PUBACK, CONNECT, CONNACK, SUBACK, UNSUB, UNSUBACK, PINGREQ, PINGRESP, PUSH, PUSHACK)
 * then it manage the folowing procedure. 
 */
void handleMessage(struct Packet packetRcv,struct simple_udp_connection *udp_conn,const uip_ipaddr_t *destAddr);

/* Recover pushed information packet */
char* getPushedINFO();
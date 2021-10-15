#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "structure.h"
#include "sys/log.h"
#include "random.h"


#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO
bool connected=false;

/* Create a packet in function of the command (push, connect, ...) */
static struct Packet createPacket(unsigned int mst, unsigned int rel, char* headerOption, char* payload){
	struct Header header;
	header.mst = mst;
	header.rel = rel;
	strcpy(header.headerOption, headerOption);
	
	struct Packet packet;
	packet.header = header;
	char modifPayload[50];
	strcat(modifPayload, "  ");
	strcat(modifPayload, payload);
	strcpy(packet.payload, modifPayload);
	
	return packet;
}

char* pushedINFO;
bool fresh = 0;

/* Recover pushed information packet */
char* getPushedINFO(){
	if(fresh){
		fresh = 1;
		return pushedINFO;
	}
	return NULL;
}


bool ackRcv;
int ackTypeWanted;
struct simple_udp_connection *udp_connAck;
uip_ipaddr_t destAddrAck;
struct Packet packetAck;
	
#define SEND_INTERVAL		  (5 * CLOCK_SECOND)
/* Declaration fo process ackThread */
PROCESS(ackThread, "Check ack");

/**
 * This thread  ackThread send an acknowledgement packet until he receive his own acknowledgement.
 */
PROCESS_THREAD(ackThread, ev, data)
{
	static struct etimer periodic_timer;
	//static unsigned count;
	//const uip_ipaddr_t *destAddrCast = data;
	PROCESS_BEGIN();
	etimer_set(&periodic_timer, random_rand() % SEND_INTERVAL);
	while(1) {
	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

	if(!ackRcv){
		simple_udp_sendto(udp_connAck,&packetAck,  sizeof(packetAck),&destAddrAck);
	}else{
		//free(destAddrAck);
		PROCESS_EXIT();
	}

	/* Add some jitter */
	etimer_set(&periodic_timer, SEND_INTERVAL
		- CLOCK_SECOND + (random_rand() % (2 * CLOCK_SECOND)));
	}

	PROCESS_END();
}


/* Start the process ackThread and send the destination address received. */
int qosThread(struct Packet packet, struct simple_udp_connection *udp_conn,const uip_ipaddr_t *destAddr){
	udp_connAck = udp_conn;
	LOG_INFO("Change reliable addresse to : ");
	    	LOG_INFO_6ADDR(destAddr);
      		LOG_INFO_("\n");
      	packetAck = packet;
	destAddrAck = *destAddr;
	process_start(&ackThread, &destAddr);
	
	return 0;
}

/* Send a packet to a device or a broker, called by connect, connACK, push, ... */
static int sendPacket(struct Packet packet, struct simple_udp_connection *udp_conn,const uip_ipaddr_t *destAddr){
	if(packet.header.rel){
		ackRcv = false;
		ackTypeWanted = packet.header.mst+1;
		qosThread(packet, udp_conn, destAddr);
	}
	LOG_INFO("Packet is send ");
	simple_udp_sendto(udp_conn,&packet,  sizeof(packet),destAddr);
	return 0;
}

/* Recover the message type from the header of the packet added in parameter.*/
 int getMessageType(struct Packet packet){
	return packet.header.mst;
}


/* Initiate a connection to a remote device or broker */
void hello(struct simple_udp_connection *udp_conn,const uip_ipaddr_t *destAddr, bool init){
	struct Packet packet;
	if (init){
	LOG_INFO("Send hello packet to ");
	packet = createPacket(HELLO, UNRELIABLE,  "init", "123456789100000000");
	}else{
	LOG_INFO("Send response hello packet to ");
	packet = createPacket(HELLO, UNRELIABLE, "response", "testpayload");
	}
	LOG_INFO_6ADDR(destAddr);
    LOG_INFO_("\n");
	sendPacket(packet, udp_conn,destAddr);
}

/* Initiate a connection to a remote device or broker */
void connect(struct simple_udp_connection *udp_conn,const uip_ipaddr_t *destAddr){
	LOG_INFO("Send connect packet to ");
	LOG_INFO_6ADDR(destAddr);
      	LOG_INFO_("\n");
	struct Packet packet = createPacket(CONNECT, RELIABLE,  "CONNECT", "testpayload");
	sendPacket(packet, udp_conn,destAddr);
}

/* Return an acknowledge after a CONNECTION paquet */
void connACK(struct simple_udp_connection *udp_conn,const uip_ipaddr_t *destAddr){
	LOG_INFO("Send connack packet to ");
	LOG_INFO_6ADDR(destAddr);
      	LOG_INFO_("\n");
	struct Packet packet = createPacket(CONNACK, UNRELIABLE,  "CONNACK", "testpayload");
	sendPacket(packet, udp_conn,destAddr);
}

/* Finish a transmission (a client give the network) */
void disconnect(struct simple_udp_connection *udp_conn,const uip_ipaddr_t *destAddr){
    LOG_INFO("Send disconnect packet to ");
    LOG_INFO_6ADDR(destAddr);
    LOG_INFO("\n");
    struct Packet packet = createPacket(DISCONNECT, UNRELIABLE,  "DISCONNECT", "testpayload");
    sendPacket(packet, udp_conn,destAddr);

}

/* Subscribe to a topic */
void subscribe(struct simple_udp_connection *udp_conn, const uip_ipaddr_t *destAddr, char *topicname){
	struct Packet packet;
	packet = createPacket(SUBSCRIBE, RELIABLE,  topicname ,"");
	sendPacket(packet, udp_conn,destAddr);
}

/* Return an acknowledge after subscription to a topic */
void subACK(struct simple_udp_connection *udp_conn,const uip_ipaddr_t *destAddr){
	struct Packet packet;
	packet = createPacket(SUBACK, UNRELIABLE,  "", "");
	sendPacket(packet, udp_conn,destAddr);
}

/* Unsubscribe to a topic */
void unSUB(struct simple_udp_connection *udp_conn, const uip_ipaddr_t *destAddr, char *topicname){

}

/* Return an acknowledge after unsubscription to a topic */
void unSUBACK(){
	//sendPacket();
}

/* Publish information/command for a topic */
void publish(struct simple_udp_connection *udp_conn, const uip_ipaddr_t *destAddr, bool command, char *topicname, char *value){
	struct Packet packet;
	if(command){
		packet = createPacket(PUBLISH, RELIABLE,  topicname, value);
		sendPacket(packet, udp_conn,destAddr);
	}else{
		packet = createPacket(PUBLISH, UNRELIABLE,  topicname, value);
		sendPacket(packet, udp_conn,destAddr);
	}
	
}
/* Return an acknowledge after publish of a value */ 
void pubACK(struct simple_udp_connection *udp_conn, const uip_ipaddr_t *destAddr){
	struct Packet packet;
	packet = createPacket(PUBACK, UNRELIABLE,  "", "");
	sendPacket(packet, udp_conn,destAddr);
}

/* Transfer an information/command (method for the broker) to the server */
void push(struct simple_udp_connection *udp_conn, const uip_ipaddr_t *destAddr, bool command, char *topicname, char *value){
	struct Packet packet;
	if(command){
		packet = createPacket(PUSH, RELIABLE,  topicname, value);
		sendPacket(packet, udp_conn,destAddr);
	}else{
		packet = createPacket(PUSH, UNRELIABLE,  topicname, value);
		sendPacket(packet, udp_conn,destAddr);
	}
}


/* Return an acknowledge when the information/command is received to the server */
void pushACK(struct simple_udp_connection *udp_conn, const uip_ipaddr_t *destAddr){
	struct Packet packet;
	packet = createPacket(PUSHACK, UNRELIABLE,  "", "");
	sendPacket(packet, udp_conn,destAddr);
}

/* Make a ping to check the connection */
void pingreq(struct simple_udp_connection *udp_conn,const uip_ipaddr_t *destAddr){ 
	LOG_INFO("Send pingreq packet to ");
	LOG_INFO_6ADDR(destAddr);
      	LOG_INFO_("\n");
	struct Packet packet = createPacket(PINGREQ, UNRELIABLE,  "PINGREQ", "testPayload");
	sendPacket(packet, udp_conn,destAddr);
}

	
/* Respond to a ping */
void pingresp(struct simple_udp_connection *udp_conn,const uip_ipaddr_t *destAddr){
	LOG_INFO("Send pingresp packet to ");
	LOG_INFO_6ADDR(destAddr);
      	LOG_INFO_("\n");
	struct Packet packet = createPacket(PINGRESP, UNRELIABLE,  "PINGRESP", "pingRespTestPayload");
	sendPacket(packet, udp_conn,destAddr); 
}

/* declaration of the process wait_ping_process */
PROCESS(wait_ping_process, "WAIT client"); 

struct simple_udp_connection *udp_connPing;
uip_ipaddr_t destAddrPing;
struct Packet packetAck;
int countPing;
/**
 * This thread (wait_ping_process)  ping each 10 second the destination ping address.
 * It stop and disconnect the mote after 5 pings without ping response.
 */
PROCESS_THREAD(wait_ping_process, ev, data){
	PROCESS_BEGIN(); 
	countPing = 0;
	LOG_INFO("PING wait_ping_process " );
	LOG_INFO_6ADDR(&destAddrPing); 
	LOG_INFO("\n"); 	
	static struct etimer timer;

	etimer_set(&timer, CLOCK_SECOND*10); //Ping each 10 seconds 
  	
  	while(1 && countPing < 5){
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer)); 
 		LOG_INFO("Send ping number %i to ", countPing );
		LOG_INFO_6ADDR(&destAddrPing); 
		LOG_INFO("\n"); 
		pingreq(udp_connPing,  &destAddrPing);
		countPing++;
		
    		etimer_reset(&timer);
    		
	}
	disconnect(udp_connPing, &destAddrPing);
  	PROCESS_END();
} 

/* Start the thread wait_ping_process  */
void startPingThread(struct simple_udp_connection *udp_conn, const uip_ipaddr_t *destAddr){
	udp_connPing = udp_conn;
	destAddrPing = *destAddr;
	LOG_INFO("Ping should be to " );
	LOG_INFO_6ADDR(&destAddrPing); 
	LOG_INFO("\n"); 
	process_start(&wait_ping_process,NULL);//START/RESTART PROCESS PING
}

/* Stop the thread wait_ping_process */
void stopPingThread(){
	process_exit(&wait_ping_process); //STOP PROCESS PING
}

/* Getter of isConnected value. */
bool isConnected(){
	return connected;
}


int TOPICSIZE = 1;
struct Topic topics[1]; 
/**
 * Handle the message from his type :
 * ( HELLO , PUBLISH, SUBSCRIBE, DISCONNECT, PUBACK, CONNECT, CONNACK, SUBACK, UNSUB, UNSUBACK, PINGREQ, PINGRESP, PUSH, PUSHACK)
 * then it manage the folowing procedure. 
 */
void handleMessage(struct Packet packetRcv,struct simple_udp_connection *udp_conn,const uip_ipaddr_t *destAddr){
	int msgType = getMessageType(packetRcv);
  	LOG_INFO("Received msg with MessageType = %i\n",msgType);
  	
  	bool exist = 0;
  	bool created = 0;
  	struct Topic tp;

  	int i;
  	int j;

  	if(!ackRcv && msgType == ackTypeWanted){
  		LOG_INFO("ACK Well received\n");
  		ackRcv=true;
  	}
	switch (msgType){
		case HELLO:
			if(strcmp(packetRcv.header.headerOption ,"init")==0) {
				LOG_INFO("Hello received response with Hello\n");
				hello(udp_conn,destAddr,0);
			}else{
				LOG_INFO("Hello response received, response with connect\n");
				connect(udp_conn,destAddr);
			}
			
			break;
		case PUBLISH:
			//If the publish is in the reliable mode, send puback
			if(packetRcv.header.rel == 1) {
				LOG_INFO("SUPUBLISH received \n");
				LOG_INFO("PUBACK is sending... \n");
				pubACK(udp_conn,destAddr);
			}
			else{
				LOG_INFO("PUBLISH received received, NOT ACK \n");
			}
			LOG_INFO("SU I push %s w/ payload : %s \n", packetRcv.header.headerOption, packetRcv.payload);
			//TODO Envoyer aux subscribers avec un push 

			//Loop between all topics
			for (i = 0; i <=1; ++i)
			{
				//If the topic is not null
				if( topics[i].name != NULL ){
					//If the topic correspond to the published topic
					if( ! strcmp(topics[i].name , packetRcv.header.headerOption ) ){
						//Send to all ip a push
						for (j = 0; j <= 1; ++j)
						{
							push(udp_conn , &topics[i].ips[j] , packetRcv.header.rel , packetRcv.header.headerOption , packetRcv.payload );
						}
					}
				}
			}
			break;
		case SUBSCRIBE:

			LOG_INFO("SU00 SUBSCRIBE received... \n");

			LOG_INFO("SU01 le topic est %s \n", packetRcv.header.headerOption);

			//Loop for the topics
			for(i=0 ; i <= TOPICSIZE ; ++i){
				//if the topic is not null
				if( topics[i].name != NULL ){
					//if the topic math the received topic
					if( ! strcmp(topics[i].name , packetRcv.header.headerOption ) ){
						//Mark the topic as existing
						exist = 1;
						LOG_INFO("SU11 le topic existe déja \n");
						//We loop into the ip of this topic
						for(j=0 ; j <= 1 ; ++j){

							//Check if subscriber not already in the list TODO
							
							//if an ip slot is empty
							if( &topics[i].ips[j] == NULL ){
								//We write the ip of the sbscriber
								LOG_INFO("SU12 Subscriber registered \n");

								topics[i].ips[j] = *destAddr;
								LOG_INFO("SU13 Address from the pd ");
  								LOG_INFO_6ADDR(destAddr);
  								LOG_INFO_("\n");

							}
						}
					}
				}
			}
			//If the topic does not exist
			if(!exist){
				LOG_INFO("SU21 le topic n'existe pas encore \n");
				//Loop between the existing project to find an empty slot
				for(i=0 ; i <= TOPICSIZE ; ++i){
					if( topics[i].name == NULL && created == 0){
						created = 1;
						LOG_INFO("SU22 le topic a été enregistré \n");

						tp.name = packetRcv.header.headerOption;
						
						tp.ips[0] = *destAddr;

						topics[i] = tp;
					}
				}
			}

			LOG_INFO("SUBACK is sending \n");
			subACK(udp_conn,destAddr);

			//sendPacket();
			break;
		case DISCONNECT://disconnect();
			connected = false;
			break;
		case PUBACK:
			LOG_INFO("SUPUBACK received \n");
			break;
		case CONNECT://PUBACK();
			
			LOG_INFO("Connect received response with connack\n");
			connACK(udp_conn,destAddr);
			
			break;
		case CONNACK://connect(data, datalen);
			connected = true;
			break;
		case SUBACK:
			LOG_INFO("SUBACK received \n");
			break;
		case UNSUB://UNSUB();
			break;
		case UNSUBACK://UNSUBACK();
			break;
		case PINGREQ://ping();
			LOG_INFO("Ping request received\n");
			LOG_INFO_6ADDR(destAddr);
			LOG_INFO("\n");
			pingresp(udp_conn,destAddr);
			break;
		case PINGRESP://PINGRESP();
			LOG_INFO("PING response received\n");
			stopPingThread();
			break;
		case PUSH:
			LOG_INFO("SU I received topic %s and value %s \n" , packetRcv.header.headerOption , packetRcv.payload);
			pushedINFO = packetRcv.payload;
			fresh = 1;
			pushACK(udp_conn,destAddr);
			LOG_INFO("SU pushed is %s\n",pushedINFO);
			LOG_INFO("SU value is %s\n",packetRcv.payload);
			break;
		case PUSHACK:
			LOG_INFO("SU PUSHACK received \n");
			break;
		default:
			break;
	}

}





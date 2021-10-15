#include "structureapp.h"
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
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

//Print ipv6 address
void ipv6_expander(const struct in6_addr * addr) {
    char str[40];
    sprintf(str, "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
            (int)addr->s6_addr[0], (int)addr->s6_addr[1],
            (int)addr->s6_addr[2], (int)addr->s6_addr[3],
            (int)addr->s6_addr[4], (int)addr->s6_addr[5],
            (int)addr->s6_addr[6], (int)addr->s6_addr[7],
            (int)addr->s6_addr[8], (int)addr->s6_addr[9],
            (int)addr->s6_addr[10], (int)addr->s6_addr[11],
            (int)addr->s6_addr[12], (int)addr->s6_addr[13],
            (int)addr->s6_addr[14], (int)addr->s6_addr[15]);
    printf("Ipv6 addr = %s \n", str);

}

	bool ackRcv;
	int ackTypeWanted;
	int sockAck;
	struct sockaddr_in6 destAddrAck;
	struct Packet packetAck;
//Thread waiting for ack
void* threadAck(void *arg){
	while(1){
		sleep(3); //Wait 3 seconds
		if(!ackRcv){
		sendto(sockAck, &packetAck, sizeof(packetAck),MSG_CONFIRM, ( struct sockaddr *) &destAddrAck, sizeof(destAddrAck));
		}
		else{
			pthread_exit(NULL);
		}
	}
}

pthread_t thread_id;
//Launching reliable thread waiting for ack
int qosThread(int sock,struct Packet packet, struct  sockaddr_in6 dest_addr){
 	packetAck = packet;
	destAddrAck = dest_addr;
	sockAck = sock;
    pthread_create(&thread_id, NULL, threadAck, NULL);
	return 0;
}

/* Send a packet to a device or a broker, called by connect, connACK, push, ... */
int sendPacket(int sock, struct Packet packet, struct  sockaddr_in6 dest_addr){
	 if(packet.header.rel){
	 	ackRcv = false;
	 	ackTypeWanted = packet.header.mst+1;
	 	qosThread(sock,packet, dest_addr);
	 }
	printf("send packet to");
	sendto(sock, &packet, sizeof(packet),MSG_CONFIRM, ( struct sockaddr *) &dest_addr, sizeof(dest_addr));
	return 0;
}

//When we receive a paquet, we split the paquet and we check which is the type of message
 int getMessageType(struct Packet packet){
	return packet.header.mst;
}

/* Initiate a connection to a remote device or broker */
void hello(int sock, struct  sockaddr_in6 dest_addr, bool init){
	
	printf("send hello2\n");
	struct Packet packet;
	if (init){
	packet = createPacket(HELLO, UNRELIABLE, "init", "testpayload");
	}else{
	packet = createPacket(HELLO, UNRELIABLE, "response", "testpayload");
	}
	printf("send hello3\n");
	sendPacket(sock, packet,dest_addr);
	printf("send hello 4\n");
}

/* Initiate a connection to a remote device or broker */
void sendConnect(int sock, struct  sockaddr_in6 dest_addr){
	struct Packet packet = createPacket(CONNECT, RELIABLE, "CONNECT", "testpayload");
	sendPacket(sock, packet,dest_addr);
}

/* Return an acknowledge after a CONNECTION paquet */
void connACK(int sock, struct  sockaddr_in6 dest_addr){
	struct Packet packet = createPacket(CONNACK, UNRELIABLE,  "CONNACK", "testpayload");
	sendPacket(sock, packet,dest_addr);
}

/* Return an acknowledge after subscription to a topic */
void subACK(int sock, struct  sockaddr_in6 dest_addr){
	struct Packet packet;
	packet = createPacket(SUBACK, UNRELIABLE,  "", "");
	sendPacket(sock, packet,dest_addr);
}

/* Return an acknowledge after publish to a topic */
void pubACK(int sock, struct  sockaddr_in6 dest_addr){
	struct Packet packet;
	packet = createPacket(PUBACK, UNRELIABLE,  "", "");
	sendPacket(sock, packet,dest_addr);
}

/* Transfer an information/command (method for the broker) */
void push(int sock, struct  sockaddr_in6 dest_addr, bool command, char *topicname, char *value){
	struct Packet packet;
	if(command){
		packet = createPacket(PUSH, RELIABLE,  topicname, value);
		sendPacket(sock, packet,dest_addr);
	}else{
		packet = createPacket(PUSH, UNRELIABLE,  topicname, value);
		sendPacket(sock, packet,dest_addr);
	}
}

/* Respond to a ping */
void pingresp(int sock, struct  sockaddr_in6 dest_addr){
	struct Packet packet = createPacket(PINGRESP, UNRELIABLE,  "PINGRESP", "pingRespTestPayload");
	sendPacket(sock, packet,dest_addr);
}

 int TOPICSIZE = 10;
struct Topic topics[10]; 

// Publish information into the broker topics
void publishTo(struct Packet packetRcv,int sock,struct  sockaddr_in6 dest_addr){
	for (int i = 0; i <=10; ++i)
			{
				//If the topic is not null
				if( topics[i].name != NULL ){
					//If the topic correspond to the published topic
					if( ! strcmp(topics[i].name , packetRcv.header.headerOption ) ){
						//Send to all ip a push
						for (int j = 0; j <= 1; ++j)
						{

							push(sock, topics[i].ips[j] , packetRcv.header.rel , packetRcv.header.headerOption , packetRcv.payload );
						}
					}
				}
			}
}



static unsigned count =0;
//Handle message based on message type
void handleMessage(struct Packet packetRcv,int sock,struct  sockaddr_in6 dest_addr){
	
	int msgType = getMessageType(packetRcv);
  	printf("handleMessage %i \n", msgType);
  	bool exist = 0;
  	bool created = 0;
  	struct Topic tp;

  	int i;
  	int j;
  	printf("HeaderOption%s\n", packetRcv.header.headerOption );
  	printf("Payload = %s\n",packetRcv.payload );
  	if(!ackRcv && msgType == ackTypeWanted){
  	 	ackRcv=true;
  	 }
  	printf("Before switch \n");
	switch (msgType){
		case HELLO:
			printf("send hello\n");
			if(strcmp(packetRcv.header.headerOption ,"init")==0) {
				hello(sock,dest_addr,0);
			}else{
				sendConnect(sock,dest_addr);
			}
			
			break;
		case PUBLISH:
			//If the publish is in the reliable mode, send puback
			if(packetRcv.header.rel == 1) {
				
				pubACK(sock,dest_addr);
			}
			
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

							push(sock, topics[i].ips[j] , packetRcv.header.rel , packetRcv.header.headerOption , packetRcv.payload );
						}
					}
				}
			}
			break;
		case CONNECT://PUBACK();
			connACK(sock,dest_addr);
			break;
		case SUBSCRIBE:
			//Loop for the topics
			for(i=0 ; i <= TOPICSIZE ; ++i){
				//if the topic is not null
				if( topics[i].name != NULL ){
					//if the topic match the received topic
					if( ! strcmp(topics[i].name , packetRcv.header.headerOption ) ){
						//Mark the topic as existing
						exist = 1;
						//We loop into the ip of this topic
						for(j=0 ; j <= 1 ; ++j){

							//Check if subscriber not already in the list TODO
							
							//if an ip slot is empty
							if( &topics[i].ips[j] == NULL ){
								//We write the ip of the sbscriber
								topics[i].ips[j] = dest_addr;
							}
						}
					}
				}
			}
			//If the topic does not exist
			if(!exist){
				//Loop between the existing project to find an empty slot
				for(i=0 ; i <= TOPICSIZE ; ++i){
					if( topics[i].name == NULL && created == 0){
						created = 1;
						tp.name = packetRcv.header.headerOption;
						tp.ips[0] = dest_addr;
						topics[i] = tp;
					}
				}
			}
			subACK(sock,dest_addr);
			break;
case PINGREQ:
			pingresp(sock,dest_addr);
			break;
		default:
			
			break;
	}

}





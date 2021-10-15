#include "contiki.h"
#include "net/routing/routing.h"
#include "random.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"

#include<stdio.h>
#include<stdlib.h>

#include "../../SBVK.h"
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

#define SEND_INTERVAL2		  (60 * CLOCK_SECOND)

static struct simple_udp_connection udp_conn;

PROCESS(udp_client_process, "UDP client");

AUTOSTART_PROCESSES(&udp_client_process);

static struct etimer periodic_timer;

static void
udp_rx_callback(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{

  startPingThread(&udp_conn,sender_addr);
  struct Packet* received_struct_ptr;
    received_struct_ptr = (struct Packet*) data;
    struct Packet packetRcv;
    packetRcv = *received_struct_ptr;
    handleMessage(packetRcv,&udp_conn,sender_addr);
    char* pushedINFO = getPushedINFO();
    if(pushedINFO != NULL ){

      if(!strcmp(pushedINFO , "ON")){
        LOG_INFO_("SU LIGHT ON ! \n");
      }else if(!strcmp(pushedINFO , "OFF")){
        LOG_INFO_("SU LIGHT OFF ! \n");
      }
    }

}

PROCESS_THREAD(udp_client_process, ev, data)
{
  uip_ipaddr_t dest_ipaddr;

  PROCESS_BEGIN();

  simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
                      UDP_SERVER_PORT, udp_rx_callback);

  etimer_set(&periodic_timer, random_rand() % SEND_INTERVAL2);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {
    uip_ip6addr(&dest_ipaddr, 0xbbbb, 0, 0, 0, 0, 0, 0, 1);
    	if(!isConnected()){

      hello(&udp_conn,&dest_ipaddr,1);
      LOG_INFO("SU I subscribe to Light ");
      subscribe(&udp_conn, &dest_ipaddr, "Light");

      }else{
      	startPingThread(&udp_conn,&dest_ipaddr);
      }
      
    } else {
      LOG_INFO("Not reachable yet\n");
    }

    etimer_set(&periodic_timer, SEND_INTERVAL2
      - CLOCK_SECOND + (random_rand() % (2 * CLOCK_SECOND)));
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

# Border router
From the border router md file:
The embedded border router runs on a node. It allows to transfer all messages from
a 6LoWPAN network to an external classical TCP-IP network using a SLIP tunnel (tunslip6). We use it to perform communication
between devices and the Control Server. 

## How it works

- Create a new Z1 node. Compile the border-router.c on this node. This file is
the initial file present in contiki at /contiki-ng/examples/rpl-border-router/border-router.c
  
- When the border-router is run it allows to convert all 6LoWPAN packets into 
classical TCP/IP packets.
  
- Open the Serial Socket (Server) on the node and start the connection.

- Now nodes can communicate with external hosts through the border-router.

# Utilization

Compiled into contiki on a Z1 mote.

## Pre-configuration

- In the file present in at rpl-border-router/border-router.c, you must comment the following line:
*PLATFORMS_EXCLUDE = nrf52dk z1*. 
  
- You must install ifconfig with this command: *sudo apt install net-tools*

- Enter in the Contiki container (with *contiker*) and go to the file at : 
*contiki-ng/tools/serial-io* and build the tunslip with *make tunslip6*
  
- Now we must open the tunnel between the host and the container of contiki with
this command executed on the host : *sudo ./tunslip6 -a <container-ip> -p <port> bbbb::1/64*.



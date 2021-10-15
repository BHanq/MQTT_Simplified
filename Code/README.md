# LINGI2146 Mobile and Embedded Computing
We decided to develop a protocol similar to MQTT but more lightweight 
and adapted for IoT devices. We also proposed this protocol with a secure implementation.

Firstly, we started with an implementation where we used the broker as a gateway in the RPL
network with devices. We designed our protocol as a library so it is portable and easily adaptive
if we want to use it in another application. 

However, once we must implement the control server we had a problem because
when we implemented the border-router, it was a bit complicated to implement the control server.
It's for this reason that we change all of our implementation. We decided to externalize the broker
putting in the external network (on our host). By this way, the broker and the control server 
run together on the host. It simplifies exchanges between devices and broker through the border-router.
In this configuration, the border-router acts also as the RPL gateway.

Therefore, we have two different implementations. In the first one, the broker was in the 6LoWPAN network and the second
one, it was not in the 6LoWPAN network but it can communicated with it.

## How it works
Here we explain the second implement:

- You must create all Z1 nodes which works as different sensors.

- You have to create the border-router. All configuration are in the rpl-border-router directory. 

- You have to implement the control server and the broker on the host. For the configuration, you can read the README.md
present in the ControlServer directory.
  
- Now the control server and the broker can communicate with devices in the 6LoWPAN network. 

# Utilization
We developed the protocol, the devices, the broker and the control server implementation in C code. All code is available in other directories

## Compilation
To compile the control server and the broker, we have to execute this following command: *gcc -o control controlserv.c -lpthread*.

All structures specific to our protocol are declared in the header file called "structure.h" and "structureapp.h" and the "SBVK_broker.c"
is include into the "BrokerAndControlServer.c" file. 




# Broker of SBVK and Command line interface all in one

The broker receive all packets from the sensors in contiki network handle them and response if it is necessary.
The command line interface allow an easy and beautiful user interaction.
It is developped in c, without using any Contiki library, that is why it is different from the file in root folder.

## How it work 
- Open contiki cooja
- Set your sensor nodes in the simulator (Z1 motes)
- Set the border router in the simulator (Z1 motes)
- On the border router open a serial socket server on port 60001 and start it
- Find ipv6 address of contiki docker instance
- In contiki/tools/serial-io : make tunslip6
- sudo ./tunslip6 -a -<container-ip> -p 60001 bbbb::1/64
- gcc BrokerAndControlServer.c -o BrokerServ -lpthread
- And then launch ./BrokerServ
- Have fun with devices

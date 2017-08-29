# Demo Description

The demo tests the following network function:

for csky_netsetting_example
* set network ip address
* get network ip address

for csky_nettrans_example
* Send TCP Packet
* Receive TCP Packet
* Ping test

# Directory contents
```
+-- bin
|    +--- csky_netsetting_example (binary built)
|    +--- csky_nettrans_example (binary built)
|
+-- csky_netsetting_example.c (source code)
|
+-- csky_nettrans_example.c (source code)
|
+-- csky_net_test.h (source code)
|
+-- Makefile
|
+-- README.md (this file)
```
# How to build

* Before building the demo, please set the toolchain path to PATH global var.
* Use `make` to build the example, the binary will be created in bin/ dir.
* Use `make clean` to clean the example binary.

# How to use

```
Usage: csky_netsetting_example device_name [ipaddr] [netmask] [gateway]
You must check you have the permission to work on the ethernet card
When only device_name is given, it will show information of this card
Otherwise, it will modify ethernet settings.

Usage: csky_nettrans_example sendtcp/tcpserver/ping [ipaddr]
If you want to send a ping test , you can type " csky_nettrans_example ping someip "
If you want test tcp packet tranferring, you can type " csky_nettrans_example tcpserver " to build a tcp receiving server
or you cantype " csky_nettrans_example sendtcp someip" to send tcp packet


```

## Example

### 1. Check usage

$ csky_netsetting_example

$ csky_nettrans_example

### 1. Example Output

```
for csky_netsetting_example
csky_netsetting_example 1.0
Usage: csky_netsetting_example device_name [ipaddr] [netmask] [gateway]
You must check you have the permission to work on the ethernet card
When only device_name is given, it will show information of this card
Otherwise, it will modify ethernet settings.

for csky_nettrans_example
csky_nettrans_example 1.0
Usage: csky_nettrans_example sendtcp/tcpserver/ping [ipaddr]
If you want to send a ping test , you can type " csky_nettrans_example ping someip "
If you want test tcp packet tranferring, you can type " csky_nettrans_example tcpserver " to build a tcp receiving server
or you cantype " csky_nettrans_example sendtcp someip" to send tcp packet

```

### 2. Normal test

#Get IP Address
csky_netsetting_example eth0

#Example Output
IP Address: 172.16.28.225
Netmask: 255.255.255.0
Gateway IP: 172.16.28.254

#Set IP Address
csky_netsetting_example eth0 172.16.28.224 255.255.0.0 172.16.28.254

#Example Output
Set IP Address successfully.
Set Netmask Address successfully.
Set GateWay IP Address successfully.

#Send TCP Packet
csky_nettrans_example sendtcp 172.16.28.224

#Example Output
client: Connecting...
client: Connected
Sent 2048 bytes
Sent 2048 bytes


#Receive TCP Packet
csky_nettrans_example tcpserver

#Example Output
server: Accepting connections on port 2345
server: Connection accepted -- receiving
server: Reading...
server: Received data
server: Received data
server: Received data

#Ping Test
csky_nettrans_example ping 127.0.0.1

#Example Output
ICMP Reply Got from 127.0.0.1
ICMP Reply Got from 127.0.0.1
ICMP Reply Got from 127.0.0.1
ICMP Reply Got from 127.0.0.1

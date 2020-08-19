# Car Platoon

## Packet Structure
ID of Sender | ID of Destination | FSM Payload &

## Finite State Machine
isLeader, Identical, RequestLeader 

* 000: Elect Leader and Generate New ID (new node)
* 001 : Elect Leader
* 010: Generate new ID
* 011: Generate New ID and Elect Leader
* 100: Nothing (Node is leader)
* 101: Generate new ID and Elect new leader
* 110: if Leader, do nothing; if Follower, generate new ID
* 111: Generate new ID and Elect Leader

## Video Demo
https://youtu.be/_Su3Vx-PDqU

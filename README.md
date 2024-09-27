# X-Plane-UDP-communication-with-Arduino-Due
Sketch for communication between X-Plane 11 and Arduino Due by UDP protocol.
No plugins required.

## Connect X-Plane with your Arduino
![](images/GeneralDataOutput.PNG)
1. Under **Network Configuration** check "Send network data output" checkbox :white_check_mark:
2. In drop down menu set "Enter IP Address"
3. Fill in IP address of your Arduino. Make sure it is in your local network range (not restricted by subnet mask).
4. Set port number (for example 49004)
5. In **General Data Output** tab check checkboxes in the last column **Network via UDP** that parameters you want to receive on your Arduino. For example if you want to receive speeds of your aircraft, check checkbox in the row with Index number 3.
6. Set **Accept incoming connections**
7. connect your PC and Arduino to the same wi-fi network. PC could be connected wirelessly or by wire, but Arduino hasto be connected by wire to the Ethernet shield like W5100.
8. In the Arduino IDE go to **Tools > Board > Arduino ARM (32-bits boards) >Arduino Due Programming port**
9. Set the same IP Address that you typed in step No. 3

## Receiving data from X-Plane


  

### Header
5 bytes

Sample fragment of raw data of the first 5 bytes of the message:
0x44 0x41 0x54 0x41 0x2a

First 4 bytes are always for type of packet i.e:
 - DATA
 - DREF
 - CMND
 - MENU
 - FAIL
 - BOAT

The 5th byte is NULL so we can ignore it

### ID
4 bytes, but **ONLY FIRST** byte is important! 


### Data of actual message
32 bytes (8 segments x 4 bytes each)

Segments are Little endian

|Structure      |Header 5 bytes          | ID 4 bytes        | Segment 1         | Segment 2         | Segment 3         | Segment 4| Segment 5| Segment 6| Segment 7| Segment 8|
| :-----------: | :--------------------: | :---------------: | :---------------: | :---------------: | :---------------: | :------: | :------: | :------: | :------: | :------: |
|Raw data       |0x44 0x41 0x54 0x41 0x2a|0x08 0x00 0x00 0x00|                   |                   |0x00 0xc0 0x79 0xc4|0x00 0xc0 0x79 0xc4|0x00 0xc0 0x79 0xc4|0x00 0xc0 0x79 0xc4|0x00 0xc0 0x79 0xc4|0x00 0xc0 0x79 0xc4|0x00 0xc0 0x79 0xc4|
|Decoded        |DATA*                   |8                  |                   |                   |-999               |-999      |-999      |-999      |-999      |-999      |



## Sending data to X-Plane

Supported message types:
- DATA
- DREF


In this example:
DREF+ byte byte byte byte sim/cockpit2/controls/elevator_trim

|Structure      |Header 5 bytes          | Data to send 4 bytes | Name of DREF + NULLS 500 bytes|
| :-----------: | :--------------------: | :------------------: | :---------------------------: |
|Raw data       |0x44 0x52 0x45 0x46 0x2b|                      |0x73 0x69 0x6d 0x2f 0x63 0x6f 0x63 0x6b 0x70 0x69 0x74 0x32 0x2f 0x63 0x6f 0x6e 0x74 0x72 0x6f 0x6c 0x73 0x2f 0x65 0x6c 0x65 0x76 0x61 0x74 0x6f 0x72 0x5f 0x74 0x72 0x69 0x6d 0x00 0x00 0x00...|
|Decoded        |DREF+                   |                      |sim/cockpit2/controls/elevator_trim NUL NUL NUL...|

DREF message has to be exactly 509 bytes long. Header and Data part combined consist od 9 bytes (5+4). So if "sim/cockpit2/controls/elevator_trim" consist of 35 characters its 35 bytes and that means we have to add 465 NULLs (0x00) to keep our message at correct length).

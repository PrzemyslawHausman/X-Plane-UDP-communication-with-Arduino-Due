# X-Plane-UDP-communication-with-Arduino-Due
Sketch for communication between X-Plane 11 and Arduino Due by UDP protocol.



## Receiving data from X-Plane

![](images/GeneralDataOutput.PNG)  

Data 32 bytes (8 segments x 4 bytes each)

Segments are Little endian

|Structure      |Header 5 bytes          | ID 4 bytes        | Segment 1         | Segment 2         | Segment 3         | Segment 4| Segment 5| Segment 6| Segment 7| Segment 8|
| :-----------: | :--------------------: | :---------------: | :---------------: | :---------------: | :---------------: | :------: | :------: | :------: | :------: | :------: |
|Raw data       |0x44 0x41 0x54 0x41 0x2a|0x08 0x00 0x00 0x00|                   |                   |0x00 0xc0 0x79 0xc4|0x00 0xc0 0x79 0xc4|0x00 0xc0 0x79 0xc4|0x00 0xc0 0x79 0xc4|0x00 0xc0 0x79 0xc4|0x00 0xc0 0x79 0xc4|0x00 0xc0 0x79 0xc4|
|Decoded        |DATA*                   |8                  |                   |                   |-999               |-999      |-999      |-999      |-999      |-999      |



## Sending data to X-Plane

Supported message types
1. DATA
2. DREF


In this example:
DREF+ byte byte byte byte sim/cockpit2/controls/elevator_trim

|Structure      |Header 5 bytes          | Data to send 4 bytes | Name of DREF + NULLS 500 bytes|
| :-----------: | :--------------------: | :------------------: | :---------------------------: |
|Raw data       |0x44 0x52 0x45 0x46 0x2b|                      |0x73 0x69 0x6d 0x2f 0x63 0x6f 0x63 0x6b 0x70 0x69 0x74 0x32 0x2f 0x63 0x6f 0x6e 0x74 0x72 0x6f 0x6c 0x73 0x2f 0x65 0x6c 0x65 0x76 0x61 0x74 0x6f 0x72 0x5f 0x74 0x72 0x69 0x6d 0x00 0x00 0x00...|
|Decoded        |DREF+                   |                      |sim/cockpit2/controls/elevator_trim NUL NUL NUL...|

DREF message has to be exactly 509 bytes long. Header and Data part combined consist od 9 bytes (5+4). So if "sim/cockpit2/controls/elevator_trim" consist of 35 characters its 35 bytes and that means we have to add 465 NULLs (0x00) to keep our message at correct length).

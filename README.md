# simpleftp
This is a simple utility for transferring files from server to client. The server is invoked with a specific working directory, from which all files could be transferred (note - currently there is no support for accessing subdirectories). The user using the client application can list all the available files and download any of them. The communication is realized using TCP protocol.

## Build
`make` command produces two executables: `simpleftp-sv` for the server and `simpleftp-cl` for the client

## Command line options
### Server invocation
`simpleftp-sv` has the following command line arguments: <br /><br />
`simpleftp-sv <port> <workdir>`
* `port` - port on which the program listens for the incoming connections. <br />
* `workdir` - path to the working directory <br />

### Client invocation
`simpleftp-cl` has the following command line arguments: <br /><br />
`simpleftp-cl <domain> <port>`
* `domain` - address of the server application (could be localhost)
* `port` - port on which server operates

## Client commands
If the client is successfully connect to the server, the following commands could be issued:
* `ls` - lists available files from the server's `workdir`
* `get <filename>` - downloads a copy of the file from the server to the client

## Protocol of the file transfer
Since the communication happens over TCP, no retransmission requests for parts of the downloaded file is necessary.
The packet that consitutes a part of the transferred file has the form:
|Byte|Description|
|----|-----------|
|0   |Is this the final packet of the file?|
|1, 2 |`len` - length of the file data as a uint16_t|
|3, ..., 2 + len|Raw bytes of file data|

The length of the file data is necessary to properly transfer binary files where the null byte (0x00) can be located in the middle of the file.


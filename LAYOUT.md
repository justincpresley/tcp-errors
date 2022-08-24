# Layout of this Project

The layout is structured from bottom-up, meaning the first files you see here are the lowest-level of
this project. Everything has both a header (hdr) file and a source (src) file unless specified or is a driver.

# Bare-Bones
* 'stdtypes': abbreviation and standardizes variable sizes

# Low-level Modules
* 'byte_help': allocation prints, copies, and general calculations
* 'socket_help': ensures a set amount of bytes are read/write
* 'file_help': helps with getting data from or about files
* 'byte_order': converting network byte order to/from host byte order

# Modules
* 'cobs': a simple module for decoding/verifying/encoding cobs
* 'parity': a simple module for getting/checking/analyzing parity
* 'message': the message structure, a unit to be sent/receive
    - 'socket_msg_help': functions to help sent and receive messages
* 'conn_chan': handling and storing connections vs channels
* 'section': helpful in deciding what byte ranges are missing

# Driver Helpers
* 'server_help': a module that helps the driver server be more simplistic and structured

# Drivers
* 'test'
* 'client'
* 'server'



# Simpleserial Documentation

## Simpleserial v1.1

All communication is initiated by the capture board. The capture board begins
communication by sending a packet:

`[cmd, data_0, data_1, ..., data_n, '\n']`

where `cmd` is an ASCII character indicating the desired command and `data` is
an ASCII representation of the data (i.e. `0xAA` will be `'A'`, `'A'`)
The length of data is fixed by the firmware.

If the target board needs to respond with data, it sends back:

`['r', data_0, data_1, ..., data_n, '\n']`

Again, `data_n` is an ASCII representation of the data.

Whether or not the device sends back data, the device will send back an ack to
tell the capture board
that it is done its operation:

`['z', status, '\n']`

Where status is an 8-bit binary number indicating the result/status of the
operation. For example, most functions use 0x00 to indicate success.

The default baud rate is 38400bps.

### SimpleSerial v1.0

The same as SimpleSerial v1.1, except the ack packet is not included.

### Variable Length Commands

Variable length commands have been added, primarily to support masked AES.
Variable length commands are decided by firmware. In the case
of a variable length command, the send command will instead be:

`[cmd, len, data_0, data_1, ..., data_n, '\n']`

### Reserved Commands

The following commands are reserved by SimpleSerial v1:

* `'v'`: Get Simpleserial version (len=0)
* `'y'`: Get the number of Simpleserial commands on the target
* `'w'`: Get Simpleserial commands

## SimpleSerial v2.0

All communication is initiated by the capture board. The capture board begins
communication by sending a packet:

`[cmd, scmd, dlen, data_0, data_1, ..., data_n, CRC (poly=0xA6)]`

cmd indicates the desired command, `scmd` indicates a sub-command, `dlen` is the
length of the data, data is binary data, and the crc is a CRC of polynomial 0xA6
of the entire packet (from `cmd` to `data_n`). All bytes in the packet may take
any 8-bit binary value, except `dlen`, which is limited to less than 250.
Therefore, only 249 bytes can
be sent per packet.

If the target board needs to respond with data, it sends back:

`[cmd, dlen, data_0, data_1, ..., data_n, CRC (poly=0xA6)]`

Whether or not the device sends back data, the device will send back an error
packet to tell the capture board that
it is done its operation:

`['e', 0x01, error, CRC (poly=0xA6)]`

Before being sent, all packets are [Consistent Overhead Byte Stuffing
(COBS)](https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing) and
null terminated:

`[overhead_byte, [packet], 0x00]`.

The following standard errors are used:

```text
0x00 - OK
0x01 - Invalid command
0x02 - Bad CRC
0x03 - Timeout
0x04 - Invalid length
0x05 - Unexpected frame byte (0x00)
0x06 - Reserved
...

0x0F - Reserved
```

Commands may use additional error codes to indicate the result of an operation.

The default baud rate is 230400bps.

### Reserved Commands

The following commands are reserved by SimpleSerial v2:

* `'v'`: Get simpleserial version (len=0)
* `'w'`: Get simpleserial commands

## Usage from C

You can use the C SimpleSerial library to listen to and send packets containing
commands, data, etc.

There are four exposed methods in the C library.

### `simpleserial_init`

This sets up the SimpleSerial module and prepares any internal commands. It
mostly there for future usage.

Calling it is as simple as:

```c
#include "simpleserial.h"

// ..snip

simpleserial_init();
```

### `simpleserial_addcmd`

Adds a listener on the target for a specific command. Depending on the version
of SimpleSerial you are using, the arguments are different.

> __Note:__ the C implementation of the SimpleSerial can only hold a maximum of
> 16 active command.

#### Arguments

Depending on whether we are using V1.\* or V2, it takes the following ordered
arguments:

##### SimpleSerial V1.\*

* `char` - the __command__ the target needs to listen for.
* `unsigned int` - the __amount of data bytes__ expected. The maximum is 64 bytes.
* `(uint8_t*, uint8_t) -> uint8_t` - the __handler__ for the command taking the
  __data buffer__ and the __actual data buffer length__ in bytes and returning a
  status code.

##### SimpleSerial V2

* `char` - the __command__ the target needs to listen for.
* `unsigned int` - the __amount of data bytes__ expected. The maximum is 192 bytes.
* `(uint8_t, uint8_t, uint8_t, uint8_t*) -> uint8_t` - the __handler__ for the
  command taking the arguments __command__, __sub-command__, __actual data buffer
  length__ and the __data buffer__ in bytes, in that order, and returning a
  status code.

#### Return value

Returns an `int` which is `1` if the maximum expected data buffer length is
exceeded and if the maximum number of commands has been reached. Will return `0`
otherwise.

#### Notes

* The callbacks function return value will be returned to the capture board. In
  SimpleSerial V1.1 this will be through a `'z'` message, as described above.

#### Example

The following is a SimpleSerial V2 example.

```c
#include "simpleserial.h"

uint8_t set_key(uint8_t cmd, uint8_t scmd, uint8_t dlen, uint8_t* data)
{
    // ...snip

    return 0;
}

uint8_t encrypt_plaintext(uint8_t cmd, uint8_t scmd, uint8_t dlen, uint8_t* data)
{
    // ...snip

    return 0;
}

// ... snip

simpleserial_addcmd('k', 16, set_key);
simpleserial_addcmd('p', 16, encrypt_plaintext);
```

### `simpleserial_put`

Write some data to the serial port, which should send a packet from the target
board to the capture board.

#### Arguments

This function takes the following ordered arguments:

* `char` - the __command__ for the capture board (e.g. `'z'` for ack, or `'e'`
  for error).
* `uint8_t` - the __size of the data buffer__.
* `uint8_t*` - the __data buffer__ of the packet send.

#### Example

The following is a SimpleSerial V2 example (although this has no impact on the
usage of the `simpleserial_put` function).

```c
#include "simpleserial.h"

uint8_t encrypt_plaintext(uint8_t cmd, uint8_t scmd, uint8_t dlen, uint8_t* data)
{
    // ...snip (do the actual encryption).

    // Send the result back to the capture board.
    simpleserial_put('r', 16, result_buffer);

    return 0;
}

// ... snip

simpleserial_addcmd('p', 16, encrypt_plaintext);
```

### `simpleserial_get`

Attempt to process a received command. If a packet from the capture board is
found relevant callback function(s) are called.

This is mostly used at the end of binaries to keep checking for commands being
check.

It might return without calling a callback for several reasons:

* There are no handler listening to the command send.
* The send packet is invalid. e.g. in SimpleSerial this could be due to data
  bytes not being in HexASCII format.
* The data buffer has an unexpected length.

#### Example

```c
#include "simpleserial.h"

// ...snip

// Add a listener
simpleserial_addcmd('p', 16, encrypt);

// Keep check if a command was send fitting one of the listeners.
while(1)
    simpleserial_get();
```

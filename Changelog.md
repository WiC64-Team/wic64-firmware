## Version 2.1.0

- Protocols: Fixed bug in parsing extended protocol request header
  - For some reason the payload size was read into a local static variable,
    causing it to be set only on the first extended protocol request

- Queueing: Use separate send and receive buffers for queued transfers
  - We previously used a single buffer for both sending and receiving items
    to/from queue, which caused the sending task to overwritte data that was
    simultaneously being received. This becomes apparent when more data is
    initially send to the queue than is being received on the other side.

- Userport: Increased low period of handshake signal to 5us
  - Increasing the low period prevents sporadic misses of handshakes on the C64
    side that seem to appear on certain mainboard/cia combinations

- Commands: Added new command WIC64_SET_REMOTE_TIMEOUT
  - This allows the user to control the time the WiC64 waits for a remote server
    to serve a request

- HttpClient: Improved retry behaviour
  - Only retry a remote request if the remote request timeout has not been
    exceeded

- HttpClient: Fixed log output regarding number of HTTP request retries

- Commands: Properly terminate string responses for SSID, MAC and IP
  - Reported by Ralf Horstmann (ra1fh)

## Version 2.0.1

This release fixes problems in WMC (Wic64 Media Center) when uploading certain
disk images and also fixes sporadic failures of large uploads (e.g. REU images).

It also fixes the MOSCloud compiler program which was broken due to restrictions
to the maximum URL length introduced by the new firmware.

- Use explicit CLRF sequence as separators in HTTP POST body
  - Fixes an issue where the last byte of an HTTP POST payload was not received
    on the server when it happened to have the value 0x0d

- Fixed subtle timing issue in Userport::onTransferCompleted
  - Fixes sporadic failures when sending response payloads that consist of a
    single byte. The byte may have been read by the C64 as 0xff regardless of
    the actual value since the EPS port was reset to input mode too soon.

- Increased maximum URL length to 8192 characters
  - Maintains backwards compatibility for programs that still use HTTP GET
    requests to transfer binary data encoded in the URL query string.

## Version 2.0.0

- Initial Release
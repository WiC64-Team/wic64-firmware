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
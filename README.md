
![](Docs/logo.svg)

# Cross-Platform C++ Sockets

[![progress](https://img.shields.io/badge/OSX-unkown-yellow.svg)]()&nbsp;
[![progress](https://img.shields.io/badge/Win32-unknown-yellow.svg)]()&nbsp;
[![progress](https://img.shields.io/badge/Debian-pass-green.svg)]()

Tested on:<br>

| System | Info |
| --- | --- |
| Windows 7 64-bit | Compiled as 32-bit VS2013 |
| Windows 10 64-bit | Compiled as 32-bit VS2017 |
| macOS Sierra 10.12.5 64-bit | Compiled with AppleClang 802.0.42 |
| Debian Linux 4.4.9-ti-r25 | Beaglebone Black GCC 4.9.2 |
| Raspbian Linux 4.9.28-v7+ | Raspberry Pi 3 GCC 4.9.2 |

This project has also been tested by others and found to work on Chrome OS and PCDuino.
<br><br>

## Features Comming Soon
An attempt is underway to add full support for the following protocols - though it may take several years to fully implement.
- Websockets (RFC 6455)
- Real-Time Streaming Protocol (RTSP v2, RFC 7826)
- HyperText Transfer Protocol (HTTP v1.1, RFC 7230 - 7235)

## External Libraries
While one of the primary goals of this library is to be self contained, requiring only the C++ standard library, there are some critical features like security that need to be delegated elsewhere.

- _GnuTLS_ for Transparent Layer Security. Optional for all security enabled protocols. As of this writing, GnuTLS uses the LGPLv2.1 software license, which is compatible with the license(s) provided for ImpactSockets.


___
Written with Visual Studio Code markdown previewer.

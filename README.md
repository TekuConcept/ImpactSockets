# Cross-Platform C++ Sockets

[![progress](https://img.shields.io/badge/OSX-unkown-yellow.svg)]()&nbsp;
[![progress](https://img.shields.io/badge/Win32-pass-green.svg)]()&nbsp;
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

### Websockets Comming Soon
An attempt is underway to add full RFC 6455 websocket support wrapping the current TCP sockets.

### Credits
`Sockets.h` and `Sockets.cpp` originally provided by Michael Donahoo and Kenneth Calvert. Authors of _Practical TCP/IP Sockets in C_.

[Nathan Copier](https://github.com/nfcopier) of the Utah State RoboSub Team provided the C++ implementation of iostream wrappers for `TcpClient`.


___
Written with Atom's markdown previewer.

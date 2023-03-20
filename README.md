![frida logo](src/logo/frida-logo-test.png)

## FRee Interactive DisAssembler  
Copyright (C) 2017,2023 by Ivo van Poorten  
Created with QtCreator 4.14.1 and Qt 5.15.2  

##### Runs on:
* Linux, debian 11.6 bullseye (stable), and equivalents  
* Windows 10, and above  
* macOS Catalina 10.15.7, and above  

##### Supported CPUs:
* NMOS 6502  
* NMOS 6502 w/ undefined opcodes  
* CMOS 65C02  
* Intel 8080  

##### File formats:
* Raw  
* Atari 8-bit Binary (.XEX)  
* Atari 8-bit Slight Atari Player (.SAP)  
* Commodore C64 Binary (.PRG)  
* Commodore C64 PSID/RSID (.SID)  
* Atari 2600 2K/4K ROM (.A26)  
* Oric Tape File (.TAP)  
* Apple ][ DOS3.3 4-byte header  
* Apple ][ ProDOS AppleSingle  
* Nintendo NES Song File (.NSF)  
* CP/M Binary at 0100H (.COM)  

##### Build instructions:
qmake frida.pro  
make  

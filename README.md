![frida logo](src/logo/frida-logo-test.png)

## FRee Interactive DisAssembler  
Copyright (C) 2017,2023 by Ivo van Poorten  
Created with QtCreator and Qt 5.15.2  
Also builds with Qt 6.x  

##### Runs on:
* Linux
    * Debian 11.6 Bullseye (stable)  
    * Ubuntu 22.04 LTS Jammy Jellyfish  
    * Linux Mint 21.1 Vera  
    * ...and many others  
* Windows 10, or later  
* macOS Catalina 10.15.7, or later  
* FreeBSD 13.2  
* OpenBSD 7.2  
* NetBSD 9.3  
* OpenIndiana 2022.10 (SunOS/Solaris)  
* Haiku R1/beta4  
* Android 5 (API level 21), or later  

##### Supported CPUs:
* NMOS 6502  
* NMOS 6502 w/ undefined opcodes  
* CMOS 65C02  
* Intel 8080  

##### File formats:
* Raw  
* Atari 8-bit Binary (.XEX)  
* Atari 8-bit Slight Atari Player (.SAP)  
* Atari 8-bit Cartridge (.CAR)  
* Commodore PET/VIC-20/C16/C64/C128 Binary (.PRG)  
* Commodore C64 PSID/RSID (.SID)  
* Atari 2600 2K/4K ROM (.A26)  
* Oric Tape File (.TAP)  
* Apple ][ DOS3.3 4-byte header  
* Apple ][ ProDOS AppleSingle  
* Nintendo NES Song File (.NSF)  
* CP/M Binary at 0100H (.COM)  
* BBC Micro, Electron, Master UEF Tape (.UEF)  

##### Build instructions:
```
git clone https://github.com/ivop/frida  
cd frida  
mkdir build  
cd build  
qmake ../src/frida.pro  
make -j8  
```

###### Notes:  

* Use latest Qt5 5.15.x or Qt6.  
* Linux builds with both clang++ and g++.  
* Windows builds with MinGW-g++, MS compiler is not supported.  
* macOS builds with clang++ that comes with XCode.  
* FreeBSD builds with clang++. frida-nopch.pro builds with gcc.  
* OpenBSD builds with eg++ and clang++.  
* NetBSD builds with clang++. frida-nopch.pro builds with gcc.  
* OpenIndiana builds with g++. clang's type_traits is broken on OpenIndiana.  
* Haiku builds frida-nopch.pro with g++. clang's type_traits is broken on Haiku.  
* Android builds with clang++ that comes with the NDK. You need a big screen tablet, a bluetooth or USB mouse and keyboard.
And memorize all keyboard shortcuts as right-click is hardwired in the Android kernel to the back button. Both armeabi-v7a and arm64-v8a work. Sort of.

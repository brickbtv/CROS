## CROS

CROS - it's a simple OS implementation, which works on APCPU-32 virtual microprocessor. 
APCPU instruction set - is a part of G4 game https://bitbucket.org/ruifig/g4devkit (WIP)

## Main features
* User- and kernel- mode for apps
* TLSF memory allocator (from @mattconte fork: https://github.com/mattconte/tlsf)
* Preemptive multitsaking
* Drivers for:
    * Screen
    * Network
    * Keyboard
    * Disk drive (http://elm-chan.org/fsw/ff/00index_e.html)
    * Clock
    
## Base apps
* Command shell
    * 'ls' - show files and folders in current directory
    * 'mkdir NAME' - make new directory
    * 'mkfile NAME' - make new file
    * 'cd NAME' - change directory
    * 'rm NAME' - remove file or directory
    * 'cat NAME' - print file content to console
    * 'edit NAME' - simple texteditor
* Chat
    * server app
    * client app
    
## APCPU VM requirements :warning:
Current implementation isn't optimized. APCPU VM should be configured with this params to comfortable run: 
> RamKBytes=4096

> TargetedSpeedMhz=32.0

:exclamation: Also, there is main requirement: Disk should be mounted to VM.

## Known troubles
* Memory leaks: 'free(...)' function gives unexpected asserts in TLSF code after process finish. Currently, most of 'free' functions is commented (except libraries code, they works fine with dynamic memory allocation/deallocation).
* Clock callbacks works incorrect in some reason
* Process crash currently crashed the OS 

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
* uBASIC interpreter (from @EtchedPixels fork: https://github.com/EtchedPixels/ubasic)
    
## Base apps
* Command shell
    * 'ls' - show files and folders in current directory
    * 'mkdir DIR_NAME' - make new directory
    * 'mkfile FILE_NAME' - make new file
    * 'cd FILE_NAME' - change directory
    * 'rm NAME' - remove file or directory
    * 'cat FILE_NAME' - print file content to console

* Chat
	* 'edit FILE_NAME' - simple texteditor
	* 'basic FILE_NAME' - uBASIC interpreter
    * server app
    * client app
    
## APCPU VM requirements :warning:
Current implementation isn't optimized. APCPU VM should be configured with this params to comfortable run: 
> RamKBytes=4096

> TargetedSpeedMhz=32.0

:exclamation: Also, there is main requirement: Disk should be mounted to VM.

## Known troubles
* Memory leaks: 'free(...)' function gives unexpected asserts in TLSF code after process finish. Currently, most of 'free' functions is commented.
* Clock callbacks works incorrect in some reason
* Process crash is currently crashed the OS (only for debugging. all you need to avoid crashed - update 'assert' macros with sdk_prc_die() function)

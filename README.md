# m30text
[PS3] Macross 30th Anniversary Game Text tool

Export/Import game text. for main story dialog translation

# For archive purpose, tool is not being update
- Use QuickBMS and script to extract data.dat and get game GOP archive and *.GOP file.
- Extract Japanese text from game *.GOP file  to [UTF-8] text file.
- Import translated  [UTF-8] text file back to game .GOP file.
- Repack .GOP back to GOP archive and then back to data.dat

# How to use
- Two ways to get the data.dat file.
     1. you can run RPCS3 emulator with the game iso image. after game start once, data.dat will be installed in the emulated harddisk under:
 
       dev_hdd0\game\BLJS10184_INSTALL\USRDIR\data\pack
     2. You have a modded PS3 game console. After the game installed, use sftp to copy data.dat from the console harddisk, under similar path as above.

- Get QuickBMS.exe, macros_pidx.bms ,macross_fsts.bms  form https://aluigi.altervista.org/quickbms.htm
- Get macross_fsts_offset2.bms from this repo.
- put QuickBMS.exe macross_fsts_offset2.bms and data.dat in the same directory
- create a output directory 'mkdir output'
-
- run quickbms.exe macross_fsts_offset2.bms data.dat output
- under output directory you will find lot of file with no file extension , one of the is GOP
- mkdir GOP_output
- run  quickbms.exe macross_fsts.bms data.dat GOP_output
- you will find GOP_output\s
   


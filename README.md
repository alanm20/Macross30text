# Macross30text
[PS3] Macross 30th Anniversary: Voices Across the Galaxy game text tool

Export/Import game text. for main story dialog, mission text etc. translation

- Most on screen UI text are actually images. those can only be change using paint application


# For archival purpose, tool is not being update
- Use QuickBMS and script to extract data.dat and get game GOP archive and *.GOP file.
- Extract Japanese text from game *.GOP file  to [UTF-8] text file.
- Import translated  [UTF-8] text file back to game .GOP file.
- Repack .GOP back to GOP archive and then back to data.dat

# acknowledgment
- Luigi Auriemma : the expert of game file extraction. author of QuickBMS.exe, macross_pidx.bms, macross_fsts.bms and many tools.
- I simply discovered that there is a hidden section of data.dat that contain the actual gop text file, made macross_pidx_off2.bms to extract that.

# How to use
 You need a modded PS3 game console to get the data.dat file. After the game installed, use ftp to copy data.dat from the console harddisk, something like this:
  hdd0: game\BLJS10184_INSTALL\USRDIR\data\pack\data.dat

- Get QuickBMS.exe, macross_pidx.bms macross_fsts.bms  form https://aluigi.altervista.org/quickbms.htm
- Get macross_pidx_offset2.bms from this repo.
- put QuickBMS.exe macross_pidx.bms , macross_fsts.bms macross_pidx_offset2.bms and data.dat in the same directory
- create a output directory 'mkdir data_out'
- run quickbms.exe macross_pidx_offset2.bms data.dat data_out
- under data_out directory you will find a gop file. delete everything else, only left gop file in this folder.
- go back  to the parent folder , where quickbms.exe is.
- mkdir GOP_out
- run  quickbms.exe macross_fsts.bms data_out/GOP gop_out
- you will find all gop file under gop_out/data/gop/*.gop
- not all gop file contain text, the important one are gop_battletext.gop , gop_dictionary, gop_scenariotext.gop, gop_subquesttext.gop. Check every gop file.
- you can use utf-8+hex editor to replace japanese text in gop file, provided that new text is shorter than japanese text
- ==
- == [ optional start ] this section is optional. 
- == Export Utf-8 text :  m30text.exe GOP_out/data/gop/gop_battletext.gop >  battletext_jap.txt
- == translate text in exported files. Do not change the special position tag ( start with "_0." ) in the exported text file. Focus on translate just the text itself, use a text editor that support UTF-8 text (i.e. notepad++).
- == Manual line breaks are important and nessessary. Break up a long text line into multiple lines if it wraps incorrectly in game. This is nesessary due to japanese does not use space to separate words, line wrap can happen between any characters. There is no western word warp support in game code.
- ==  There is a limitation on the compressed text size. the translated text will be compressed during repack, game will freeze if compressed size is bigger than original gop. So be concise when translate the text. English translation compressed well and I have not seen any issue.
- == Import translated text back to gop file:  m30text.exe  GOP_out/data/gop/gop_battletext.gop  battletext_eng.txt
- == you may see a warning about gop file is bigger than original. that may be okay as the gop reimport step will compress it.
- == you will get a  GOP_out/data/gop/gop_battletext.gop.trn file. copy it to gop_mod/data/gop/gop_battletext.gop (see import instruction below)
- == [ optional end ]
- ==
- Import just the modified .gop file to gop archive : first create a new empty path contains the modified gop_battletext.gop file
-           mkdir  gop_mod/data/gop/
-           copy    gop_out/data/gop/gop_battletext.gop.trn  gop_mod/data/gop/
-            # the following command give the new path to quickbms to reimport all files in that path back to out/gop file
-           quickbms.exe -r -w  macross_fsts.bms data_out/gop gop_mod
-            # if you get prompt for whether to use experimental reimport feature, anwser 'y'
- Since data_out only contains one gop file. just run quickbms.exe to import that path back to data.dat
-           quickbms.exe -r -w  macross_pidx_off2.bms data.dat data_out
-  FTP this modified data.dat back to PS3 console harddrive and replaced the original data.dat
-  If you have RPCS3 emulator , you can test this data.dat with that. But this game is not fully playable with emulator. 

# other files. There are images, 3D model files in  data*.dat  and fileset*.dat file. they can be extract with 
-     quickbms.exe macross_pidx.bms  pack.idx   output_dir ( this extrace everything from data*.dat, probably also from fileset*.dat)
-     quickbms.exe macross_fsts.bms fileset0.dat  output_dir ( this only extract one fileset0.dat, may be of some use)

# bonus, use google translate to translate text file
     - Can use Google translate to do batch translate Japanese to English (or other languages).
     - Do not use the text box interface of google translate web page. Text box have maximum character limit.
     - start a new Microsoft word document (.docx). copy and paste the content of the export  UTF-8 Japanese text to it.
     - use Google document translate feature to translate the .docx file. Get a english/other language .docx back.
     - copy and paste docx content back to a a plain UTF-8 text file.
     - use m30text.exe tool to import text back to gop file.
     
![alt text](https://github.com/alanm20/m30text/blob/main/images/eng2.png)
![alt text](https://github.com/alanm20/m30text/blob/main/images/eng4.png)
![alt text](https://github.com/alanm20/m30text/blob/main/images/eng6.png)


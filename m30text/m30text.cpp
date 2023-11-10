// m30text.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>

/* alanm 12/13/2021*/

/*
*
*
* Credit to  aluigi who research PIDX and FSTS format and create QuickBMS script to extract  data[1].dat and filelist[0-2].dat archives.
See :https://zenhax.com/viewtopic.php?f=9&t=2212&p=68064&hilit=macross#p67939 for the scripts


Use macross_pidx.bms to  extract files from filelist[0-2].dat , which give you a bunch of FSTS file. use macross_fsts.bms  on FSTS file to extract actual assets (DDS,model etc.)

Use macross_pidx.bms to  extract files from data.dat data1.dat, which give you the actual assets (GOP, DDS, model etc).

There is another set of FSTS files inside data.dat.  I modified macross_pidx.bms and named it macross_pidx_offset2.idx which will extract FSTS from data.dat.

Game text are stored in GOP file. Same GOP file is stored multiple times in the data.dat (i.e. data.dat contains a set of GOPs and also a set of FSTS contain same GOPs).
Changing GOP in the FSTS archive inside data.dat change display text in game. There may be other copies need to be replace but I did not test that far.

Currently cannot change sizes of text string table and gop file size. due to lack of understanding of GOP file struture.
The total size of translated text cannot be bigger than the orignal text.

This tool try to keep translated stings starting at the same offset as original strings, padding unused bytes with 0.
This is due to gop file compressed to a bigger size if too many string offsets are modified.

QuickBMS re-import bigger file back to archive file with workarounds that is not compatible with this game.

if one translated string is bigger than original, this tool will move starting offset of string come after it in the table,
if next string has empty space to cover this shift, the string after it will keep same starting offset as original text.
This scheme  minimizes the number of changes in string offsets.

*/

#define VER_STR "0.0.2"

// Nov-9-2023:  handle last blank line in translate text file. fix section 16 bit alignment 

#define BUF_SZ 4096
char buf[BUF_SZ];

// read big-endian integer from file
#define readBEint(num,fp)  {unsigned char data[4];fread(data, 4, 1, fp);num = (data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3]);}

// convert big-endian bytes to integer
#define BEbytes2int(data,num) {char c[4]; c[3]=data[0]; c[2]=data[1]; c[1]= data[2]; c[0]=data[3]; num= *((int*)c);  }

// covert integer to big-endian bytes
#define int2BEbytes(buf,num) {char*data=(char*)&num;  buf[0]=data[3]; buf[1]=data[2]; buf[2]=data[1]; buf[3]=data[0];}

// skip 4 bytes of input file
#define read4                {    unsigned char data[4]; fread(data, 4, 1, fp); }



void copybytes(FILE* in , FILE*out, int size)
{
    unsigned char buf[BUF_SZ];
    while (size > 0)
    {
        int copy_len;
        if (size >= BUF_SZ)
            copy_len = BUF_SZ;
        else
            copy_len = size;
        fread(buf, copy_len, 1, in);
        fwrite(buf, copy_len, 1, out);
        size -= copy_len;
    }
}

void CreateTranslatedGOP(FILE* in, FILE* trans_in, FILE* out)
{
    fread(buf, 8, 1, in); // section string ENDIBIGE
    fwrite(buf, 8, 1, out);
    if (!strncmp(buf, "ENDIBIGE", 8))
    {
        fread(buf, 8, 1, in);  // advance to next section.
        fwrite(buf, 8, 1, out);
        fread(buf, 8, 1, in);  // section string
        fwrite(buf, 8, 1, out);
        while (!feof(in) && strncmp(buf,"GENESTRT",8)) // just copy data from section that are not GENESTRT
        {
            unsigned int sec_size;
            fread(buf, 4, 1, in);  // section size
            fwrite(buf, 4, 1, out);
            BEbytes2int(buf,sec_size);
            copybytes(in, out, 4 + sec_size);  // skip section
            fread(buf, 8, 1, in);  // next section string
            fwrite(buf, 8, 1, out);
        }

        if (!feof(in))
        {
            // this is the GENESTRT section
            int num_str;
            long sectsize_off,sectsize_off2, data_start,offset_table_start;
            int offsets_offset, values_offset;

            sectsize_off = ftell(out);  // section size field 
            copybytes(in, out, 8);

            data_start = ftell(in);     // marked section data start

            fread(buf, 4, 1, in);
            fwrite(buf, 4, 1, out);
            BEbytes2int(buf, num_str);
            fread(buf, 4, 1, in);
            fwrite(buf, 4, 1, out);
            BEbytes2int(buf, offsets_offset); 
            fread(buf, 4, 1, in);
            fwrite(buf, 4, 1, out);
            BEbytes2int(buf, values_offset);

            sectsize_off2 =ftell(out);  // offset of end of section

            copybytes(in, out, 4);  

            offset_table_start = ftell(out);

            int offset_table_size = values_offset - offsets_offset;
            char* offset_table= new char[offset_table_size];

            if (offset_table)
            {
                unsigned char ch;

                fread(offset_table, offset_table_size, 1, in);
                fwrite(offset_table, offset_table_size, 1, out);

                int output_off = 0;
                bool bCheckValue = false;
                bool EndOfText = false;
                for (int i = 0; i < num_str; i++) // find offsets of each string entry, replace them with string from input text file
                {
                    int offset;
                    int str_len = 0; // length  of text string
                    char* offset_entry = offset_table + (4 * i);   // fetch an offset from table
                    BEbytes2int(offset_entry, offset);

                    fseek(in, data_start + values_offset + offset, SEEK_SET);  // jump to text position

                    // previous string is longer than original , shift this string to bigger offset
                    if (output_off > offset)
                    {
                        int2BEbytes(offset_entry, output_off); // update string offset for this string
                        offset = output_off;
                    }

                    bool utf8 = false;
                    long linepos = ftell(in);

                    if (!EndOfText)  // look ahead and detect utf-8 string
                    {
                        ch = fgetc(in);
                        int bi = 0;
                        while (ch != '\0')
                        {
                            buf[bi++] = ch;
                            if ((ch & 0xc0) == 0xc0) // lazy way to detect utf-8
                            {
                                utf8 = true;
                                break;
                            }
                            ch = fgetc(in);


                        }
                        fseek(in, linepos, SEEK_SET);
                        if (!strncmp(buf, "GOP_TYPE", 8))
                            EndOfText = true;
                    }
                    if (!EndOfText && utf8) // found utf8 string , replace it
                        //if ((ch & 0xc0) == 0xc0) // lazy utf-8 detection
                    {
                        // replace text with strings from text file.
                        char* next_entry = offset_table + (4 * (i + 1));
                        int next_offset;
                        BEbytes2int(next_entry, next_offset);
                        int old_size = next_offset - offset;  //  how much space available for string replacement

                         // get a line from translated text file 
                        fgets(buf, BUF_SZ, trans_in);
                        char* p = buf;
                        if (strncmp(buf, "_0.", 3))   // text must start with _0.<id> tag
                            printf("error: input text file does not match gop, line %d\n", i);
                        else
                        {

                            int trans_idx = atoi(buf + 3);
                            //printf("idx %d %d - \n", i,trans_idx);
                            if (trans_idx != i)
                                printf("error:  GOP and input text index does not match %d != %d\n", i, trans_idx);

                            fgets(buf, BUF_SZ, trans_in); // get the real text line

                            while (*p == ' ') p++; // skip leading space
                        }
                        while (*p && *p != '\n')  // copy text to output
                        {
                            str_len++;
                            fputc(*p++, out);
                        }
                        long lastpos;
                        while (!feof(trans_in)) // check for aditional lines for tranlated text
                        {
                            lastpos = ftell(trans_in); // remember last line end
                            char* ret = fgets(buf, BUF_SZ, trans_in);

                            if (!ret) break; //line is blank with no character, exit 

                            if (strncmp(buf, "_0.", 3)) //does not start with _0. , this is addition lines of previous text 
                            {

                                str_len += 2;
                                fprintf(out, "\\n");  //  newline
                                p = buf;
                                while (*p == ' ') p++; // skip leading space                               
                                while (*p && *p != '\n') // copy text
                                {
                                    str_len++;
                                    fputc(*p++, out);
                                }
                            }
                            else
                            {
                                // this is a translate string tag _0.nnnn.
                                fseek(trans_in, lastpos, SEEK_SET); //new text string, roll back file position to previous line
                                break;
                            }
                        }
                        int filler_len = old_size - 1 - str_len;
                        for (int j = 0; j < filler_len; j++)
                        {
                            str_len++;
                            fputc(0, out);
                        }
                        str_len++;
                        fputc(0, out); // terminate string with 0
                    }
                    else
                    {
                        //str_len++;
                        //fputc(ch, out);
                        //if (ch != 0)
                        {
                            while ((ch = fgetc(in)) != 0)
                            {
                                str_len++;
                                fputc(ch, out);
                            }
                            // terminate string with value 0
                            str_len++;
                            fputc(ch, out);

                        }

                    }
                    // offset of next output strings
                    output_off += str_len;
                }
                fseek(out, offset_table_start, SEEK_SET);  // go back to where the string offset table is
                fwrite(offset_table, 4, num_str + 1, out); // write out new string offset table
                fflush(out);
                fseek(out, 0, SEEK_END); // move to end of output file

                long src = ftell(in);
                long dst = ftell(out);
                if (src != dst)
                {
                    printf("Warning: patched GOP is not the same size as original GOP. new compressed GOP may be larger than original.\n");
                }
                
                // make sure output section end at 16 bytes align
                while (ftell(out) % 0x10)
                    fputc(0, out);               
                
                ch = fgetc(in);
                
                while (!feof(in))
                {
                    if (ch == 'G')  // find input GOP GDAT
                        break;
                    ch = fgetc(in);
                }
                // copy rest of the gop file.
                while (!feof(in))
                {
                    //putchar(ch);
                    fputc(ch, out);
                    ch = fgetc(in);
                }
            }
            else
                printf("memory allocation error\n");
        }
        else (" No GERESTRT section, aborted\n");
        return;


    }
    printf("not GOP file");
}

//
// 
//
//

int main(int argc, char*argv[])
{
    //std::cout << "Hello World!\n";
    if (argc > 1)
    {
        FILE* fp;
        FILE* out;
        FILE* trans_f;
        std::string outfile = argv[1];
        
        fopen_s(&fp, argv[1], "rb"); // Japanese gop file name
        if (!fp) {
            printf("file open error %s!\n", argv[1]);
            return -1;
        }
        if (argc > 2) //  a translated text file name is provided, output file is a translated GOP file with .trn extension. 
        {
            fopen_s(&trans_f, argv[2], "rt");

            outfile += ".trn";  
            fopen_s(&out, outfile.c_str(), "w+b");   // out put file name

            if ( trans_f && out)
            {
                CreateTranslatedGOP(fp, trans_f, out);
            }
            fclose(fp);
            fclose(trans_f);
            fclose(out);

        }
        else  // extract japanese text for translation. output to console
        {

            if (fp)  // make sure all file open okay
            {
                unsigned char ch;
                ch = fgetc(fp);
                while (!feof(fp))
                {
                    if (ch == 'G')
                    {
                        fread(buf, 7, 1, fp);
                        if (!strncmp(buf, "ENESTRT", 7))
                        {
                            //printf("found!!!\n");
                            unsigned int total_size, str_tab_offset, num_str;
                            readBEint(total_size, fp);
                            //printf("%x\n", total_size);
                            read4;
                            long file_start = ftell(fp);
                            readBEint(num_str, fp);
                            //printf("%x\n", num_str);
                            read4;
                            readBEint(str_tab_offset, fp);
                            //printf("%x\n", str_tab_offset);
                            read4;
                            long lastpos;
                            long str_off;
                            file_start += str_tab_offset;
                            bool bCheckValue = false;
                            bool EndOfText = false;
                            for (unsigned int i = 0; i < num_str; i++)
                            {
                                readBEint(str_off, fp); // fetch a string offset in big-endian format
                                lastpos = ftell(fp);
                                //printf("%x\n", file_start + str_off);
                                
                                // jump to text string offset
                                fseek(fp, (file_start + str_off), SEEK_SET);

                                long linepos = ftell(fp);
                                bool utf8 = false;
                                int bi = 0;
                                if (!EndOfText)
                                {
                                    ch = fgetc(fp);
                                    while (ch != '\0')
                                    {
                                        buf[bi++] = ch;
                                        if ((ch & 0xc0) == 0xc0) // lazy way to detect utf-8
                                        {
                                            utf8 = true;
                                            break;
                                        }
                                        ch = fgetc(fp);

                                    }
                                    fseek(fp, linepos, SEEK_SET);

                                    if (!strncmp(buf, "GOP_TYPE", 8))
                                        EndOfText = true;
                                }
                                if (!EndOfText && utf8)                // found utf string, output index number and break string into multiple lines by line-breaks
                                {                                            
                                    printf("_0.%d\n", i);
                                    
                                    while ((ch = fgetc(fp)) != '\0')
                                    {
                                        if (ch == '\\')
                                        {
                                            unsigned char ch1 = fgetc(fp);
                                            if (ch1 == 'n')
                                                
                                                printf("\n");
                                            else
                                            {
                                                putchar(ch);
                                                putchar(ch1);
                                            }
                                        }
                                        else
                                            putchar(ch);
                                    }
                                    printf("\n");
                                }
                                fseek(fp, lastpos, SEEK_SET);

                            }
                            break;
                        }
                    }
                    ch = fgetc(fp);
                }
                fclose(fp);
            }
        }
    }
    else
    printf ("Macross 30 GOP text export/import tool. Version %s, Usage:\n"
    " %s <GOP file.gop>  > gop_text.txt     :::dump text strings to  a text file\n"
    " %s <GOP file.gop> <gop_text.txt>  :::import text to gop, generate a gop.trn file\n"
    " Copy gop.trn over original gop. Use QuickBMS to import gop to game archives\n", VER_STR, argv[0], argv[0]);
}

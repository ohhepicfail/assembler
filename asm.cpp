#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <windows.h>

#define MAX_SIZE    256
#define SIGN        1
#define MAX_JUMPS   16
#define FIRST_VIEW  1
#define R_NUMBER    17
#define MAX_INT     2147483647



#define DEF_CMD( name, num, dont_need1, dont_need2 )      CMD_##name = num,
enum Commands
{
    CMD_FIRST = 0,
#include "cmdList.h"
    CMD_LAST
};
#undef DEF_CMD



enum Param
{
    PARAM0 = 0,
    PARAMN = 1,
    PARAMR = 2,
    PARAMJ = 3,
    PARAML = 4
};



size_t  translate           (char * program, unsigned char * translated_program, unsigned char * jump);
int     convert_tochar      (char * command, unsigned char * program, int type, unsigned char * jump, size_t trp_size);
size_t  count_file_size     (const char * filename);
void    fread_file          (char * buffer, size_t file_size, const char * filename);
void    fwrite_file         (const unsigned char * out, size_t out_size, const char * filename);
void    program_dump        (const unsigned char * program, size_t prog_size);


int main()
{
    char filename_in[FILENAME_MAX];
    fprintf (stdout, "Enter input filename:\t");
    fscanf  (stdin, "%s", filename_in);

    char filename_out[FILENAME_MAX];
    fprintf (stdout, "Enter output filename:\t");
    fscanf  (stdin, "%s", filename_out);

    size_t in_size = count_file_size (filename_in);
    printf ("Program size:\t%u byte(s)\n", in_size);

    char * program = (char *) calloc (in_size, sizeof (char));
    unsigned char * translated_program = (unsigned char *) calloc (in_size, sizeof (unsigned char));
    assert (program);
    assert (translated_program);

    fread_file ((char *) program, in_size, filename_in);

    unsigned char * jumps = (unsigned char *) calloc (MAX_JUMPS, sizeof (unsigned char));
    assert (jumps);

    translate (program, translated_program, jumps);
    printf ("\nfirst translation completed\n");

    fread_file ((char *) program, in_size, filename_in);
    size_t program_counter = translate (program, translated_program, jumps);
    printf ("\nsecond translation completed\n");

    printf ("\nTranslated program:\t");
    for (int i = 0; i < program_counter; i++)
        printf ("%d ", translated_program[i]);

    fwrite_file (translated_program, program_counter, filename_out);

    fprintf (stdout, "\n\n*****All commands successfuly translated!*****\n\n");

    free    (jumps);
    free    (program);
    free    (translated_program);

    system ("pause");
    return 0;
}



size_t count_file_size (const char * filename)
{
    assert (filename);

    FILE * in = fopen (filename, "rb");
    if ( ! in)
    {
        fprintf (stdout, "ERROR: cannot open:\t%s", filename);
        abort ();
    }
    fprintf (stdout, "File is open:\t\t%s\n\n", filename);

    fseek (in, 0, SEEK_END);
    size_t in_size = ftell (in);

    fclose (in);
    return in_size;
}



void fread_file (char * buffer, size_t file_size, const char * filename)
{
    assert (filename);
    assert (buffer);
    assert (file_size > 0);

    FILE * in = fopen (filename, "rb");
    if ( ! in)
    {
        fprintf (stdout, "ERROR: cannot open:\t%s for fread ()", filename);
        abort ();
    }

    fread ((char *) buffer, sizeof (char), file_size, in);

    fclose (in);
}



void fwrite_file (const unsigned char * out, size_t out_size, const char * filename)
{
    assert (out);
    assert (filename);
    assert (out_size > 0);

    FILE * fout = fopen (filename, "wb");
    if ( ! fout)
    {
        fprintf (stdout, "ERROR: cannot open:\t%s for fwrite ()", filename);
        abort ();
    }

    fwrite (out, sizeof (unsigned char), out_size, fout);

    fclose (fout);
}



void program_dump (const unsigned char * program, size_t prog_size)
{
    assert (program);
    assert (prog_size > 0);

    printf ("\n\nTranslated program: ");
    int i = 0;
    for (i = 0; i < prog_size; i++)
        printf ("%d ", program[i]);
}


// function need some memory
size_t translate (char * program, unsigned char * translated_program, unsigned char * jump)
{
    assert (program);
    assert (translated_program);

    unsigned char * trp_p = translated_program;
    size_t trp_size = 0;

    char * command = (char *) calloc (MAX_SIZE, sizeof (char));
    assert (command);

    command = strtok (program, " \n\r\0");
    assert (command);

    jump[0] += 1; //flag for reentrancy

    while (command)
    {
        assert (command);
        fprintf (stdout, "\nLine in file:\t%s", command);

#define DEF_CMD( name, num, extra, dont_need2 )                                                                                                             \
                                                            if ( ! strcmp (command, #name))                                                                 \
                                                            {                                                                                               \
                                                                if (extra != PARAML)                                                                        \
                                                                {                                                                                           \
                                                                    *(translated_program++) = num;                                                          \
                                                                    trp_size++;                                                                             \
                                                                }                                                                                           \
                                                                if (extra > 0)                                                                              \
                                                                {                                                                                           \
                                                                    command = strtok (NULL, " \n\r\0");                                                     \
                                                                    if (command == NULL)                                                                    \
                                                                    {                                                                                       \
                                                                        printf ("\nFAIL: bad command");                                                     \
                                                                        abort ();                                                                           \
                                                                    }                                                                                       \
                                                                    printf ("\t%s", command);                                                               \
                                                                    size_t shift = convert_tochar (command, translated_program, extra, jump, trp_size);     \
                                                                    if (extra != PARAML)                                                                    \
                                                                    {                                                                                       \
                                                                        translated_program += shift;                                                        \
                                                                        trp_size += shift;                                                                  \
                                                                    }                                                                                       \
                                                                }                                                                                           \
                                                            }                                                                                               \
                                                            else


        #include "cmdList.h"
        {
            printf ("\nERROR: Unknown command: %s\n", command);
            abort ();
        }

        #undef DEF_CMD

        command = strtok (NULL, " \n\r\0");
    }

    free (command);
    translated_program = trp_p;
    return trp_size;
}



int convert_tochar (char * command, unsigned char * program, int type, unsigned char * jump, size_t trp_size)
{
    assert (command);
    assert (program);
    assert (jump);
    assert (type > 0);

    if(type == PARAMR)
    {
        if (*command != 'R') //because i have 'R' to designate registers
        {
            printf ("\nFAIL: bad register %c\t\n", *command);
            abort ();
        }
        int R_num = *(++command) - '0';
        if (R_num < 0 && R_num > R_NUMBER)
        {
            printf ("\nFAIL: bad register number. It must be more than 0 and less than 256\n %c\t\n", *command);
            abort ();
        }
        *program = R_num;
        return sizeof (unsigned char);
    }
    else if (type == PARAMN)
    {
        long long int int_number = strtol (command, NULL, 0);
        if (fabs (int_number) > MAX_INT)
        {
            printf ("\nFAIL: too much number\n");
            abort ();
        }

        if (int_number >= 0)
            *program = 0;
        else
            *program = 1;

        int_number = (int) fabs (int_number);
        for (int i = 1; int_number > 0; i++)
        {
            program[i] = int_number % 256;
            int_number /= 256;
        }

        assert (program);
        return (sizeof (int) + SIGN);
    }
    else if (type == PARAMJ)
    {
        int number = strtol (command, NULL, 0);
        if (number < 0 || number >= 256)
        {
            printf ("\nFAIL: bad label. Label number must be more than 0 and less than 256\n");
            abort ();
        }
        *program = jump[number];
        return sizeof (unsigned char);
    }
    else if (type == PARAML)
    {
        int number = strtol (command, NULL, 0);
        if (number <= 0 || number >= 256)
        {
            printf ("\nFAIL: bad label. Label number must be from 1 to 255\n");
            abort ();
        }
        if (jump[number] && jump[0] == FIRST_VIEW)
        {
            printf ("\nFAIL: this label appears more than once. Please fix it\n");
            abort ();
        }
        jump[number] = trp_size + 1; // because unsigned char and I need number for flag "is empty"

        return sizeof (unsigned char);
    }
}



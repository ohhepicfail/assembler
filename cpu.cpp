#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <windows.h>

#define INIT_SIZE               2
#define R_NUMBER                16
#define NAME( name )            #name
#define ASSERT_OBJ( type, obj )   if ( ! (type ## _is_ok(obj)))\
                                {\
                                    fprintf (stderr,    "\n\n********   ERROR   ********\n\n");\
                                    fprintf (stderr,    "ERROR:\t\t"#type"\n"\
                                                        "In file:\t%s\n"\
                                                        "In function:\t%s\n"\
                                                        "Line:\t\t%d\n"\
                                                        "Object:\t\t"#obj"\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);\
                                    fprintf (stderr,    "\n\n***************************\n\n");\
                                    abort ();\
                                }

typedef struct
{
    int     * data;
    int     sz;
    int     next;
} Stack_t;



#define DEF_CMD( name, num, dont_need1, dont_need2 )    CMD_##name = num,
enum stack_actions
{
    CMD_FIRST = 0,
    #include "cmdList.h"
    CMD_LAST
};
#undef DEF_CMD



Stack_t     * stack_init            (int stack_size);
void        stack_erase             (Stack_t ** stk);
int         Stack_t_is_ok           (Stack_t * stk);
void        stack_dump              (Stack_t * stk, const char * obj_name);
size_t      stack_resize_increase   (Stack_t * stk);
size_t      stack_resize_decrease   (Stack_t * stk);
int         stack_push              (Stack_t * stk, int elem);
int         stack_pop               (Stack_t * stk);
int         load_commands           (unsigned char * buffer, const char * filename, size_t sz);
size_t      count_commands_size     (const char * filename);
void        dump_commands           (const unsigned char * commands, size_t sz);
int         run_commands            (Stack_t * stk, int * R, const unsigned char * commands, size_t sz);
void        R_dump                  (int * R);


int main()
{
    char * filename = (char *) calloc (FILENAME_MAX, sizeof (char));
    assert (filename);
    printf ("Enter name of the file:\t");
    scanf ("%s", filename);

    size_t commands_size = count_commands_size (filename);
    unsigned char * commands = (unsigned char *) calloc (commands_size, sizeof (unsigned char));
    assert (commands);
    if ( ! load_commands (commands, filename, commands_size))
        abort ();

    dump_commands (commands, commands_size);


    //program block
    Stack_t * stk = stack_init (INIT_SIZE);
    ASSERT_OBJ(Stack_t, stk)

    int * registers = (int *) calloc (R_NUMBER + 1, sizeof (int));

    run_commands (stk, registers, commands, commands_size);
    stack_dump (stk, NAME (stk));
    R_dump (registers);

    free (commands);
    free (filename);
    free (registers);
    stack_erase (&stk);

    system ("pause");

    return 0;
}



Stack_t * stack_init (int stack_size)
{
    assert (stack_size >= 0);

    Stack_t * new_stack = (Stack_t *) calloc (1, sizeof (Stack_t));
    new_stack -> data   = (int *) calloc (stack_size, sizeof (int));
    new_stack -> sz   = stack_size;
    new_stack -> next   = 0;

    ASSERT_OBJ(Stack_t, new_stack)
    return new_stack;
}



void stack_erase (Stack_t ** stk)
{
    assert    (stk);
    ASSERT_OBJ(Stack_t, *stk)

    int i = 0;
    for (i = 0; i < (* stk) -> next; i++)
        ( * stk) -> data[i] = 0xBADF00D;

    (* stk) -> next = -1;
    (* stk) -> sz = -1;

    free ((* stk) -> data);
    free (* stk);
    * stk = NULL;
}



int Stack_t_is_ok (Stack_t * stk)
{
    return (stk != NULL) && (stk -> sz >= 0) && (stk -> next >= 0);
}



void stack_dump (Stack_t * stk, const char * obj_name)
{
    ASSERT_OBJ(Stack_t, stk)
    assert (obj_name);

    fprintf (stdout,    "\n\n********   DUMP   ********\n\n");
    fprintf (stdout,    "DUMP:\t\t%s\n\n", obj_name);
    fprintf (stdout,    "stk (ptr):\t%d\n"\
            "stk -> next:\t%d\n"\
            "stk -> sz:\t%d\n\n", stk, stk -> next, stk -> sz);
    fprintf (stdout,    "%s:\t\t", obj_name);
    int i = 0;
    for (i = 0; i < stk -> next; i++) fprintf (stdout, "%d  ", stk -> data[i]);
    fprintf (stderr,    "\n\n**************************\n\n");
}



size_t stack_resize_increase (Stack_t * stk)
{
    ASSERT_OBJ(Stack_t, stk)

    stk -> sz *= INIT_SIZE;
    stk -> data = (int *) realloc (stk -> data, stk -> sz * sizeof (stk -> data));

    if ( ! Stack_t_is_ok (stk))
    {
        stk -> sz /= INIT_SIZE;
        stk -> sz += INIT_SIZE;
        stk -> data = (int *) realloc (stk -> data, stk -> sz * sizeof (stk -> data));
    }

    ASSERT_OBJ(Stack_t, stk)

    return stk->sz;
}



size_t stack_resize_decrease (Stack_t * stk)
{
    ASSERT_OBJ(Stack_t, stk)

    stk -> sz /= INIT_SIZE;
    stk -> data =  (int *) realloc (stk -> data, stk -> sz * sizeof (stk -> data));

    ASSERT_OBJ(Stack_t, stk)

    return stk->sz;
}



int stack_push (Stack_t * stk, int elem)
{
    ASSERT_OBJ(Stack_t, stk)

    if (stk -> next == stk -> sz)
        stack_resize_increase (stk);

    stk -> data[stk -> next] = elem;
    stk -> next++;

    ASSERT_OBJ(Stack_t, stk)

    return elem;
}



int stack_pop (Stack_t * stk)
{
    ASSERT_OBJ(Stack_t, stk)

    stk -> next--;

    if (stk -> next > 0 && stk -> sz / 4 >= stk -> next)
        stack_resize_decrease (stk);

    ASSERT_OBJ(Stack_t, stk);

    return stk -> data[stk -> next];
}



int load_commands (unsigned char * buffer, const char * filename, size_t sz)
{
    assert (buffer);
    assert (filename);
    assert (sz > 0);

    FILE * file = fopen (filename, "rb");
    if ( ! file)
    {
        printf ("ERROR: CANNOT OPEN THE FILE %s\n", filename);
        return 0;
    }

    if (sz / sizeof (unsigned char) != fread (buffer, sizeof (unsigned char), sz / sizeof (unsigned char), file))
    {
        printf ("ERROR: can't read file by fread()\n");
        return 0;
    }

    fclose (file);
    assert (buffer);

    return 1;
}



size_t count_commands_size (const char * filename)
{
    assert (filename);

    FILE * file = fopen (filename, "rb");
    if ( ! file)
    {
        printf ("ERROR: CANNOT OPEN THE FILE %s\n", filename);
        abort ();
    }

    fseek (file, 0, SEEK_END);
    size_t sz = ftell (file);
    assert (sz > 0);

    fclose (file);

    return sz;
}



void dump_commands (const unsigned char * commands, size_t sz)
{
    assert (commands);
    assert (sz > 0);

    printf ("dump_commands: ");
    int i = 0;
    for (i = 0; i < sz / sizeof (unsigned char); i++)
        printf ("%d ", commands[i]);
    printf ("\n");
}



void R_dump (int * R)
{
    printf ("\nRegisters dump:\t");
    for (int i = 1; i <= R_NUMBER; i++)
        printf ("%d\t", R[i]);
    printf ("\nEND\n");
}



int run_commands (Stack_t * stk, int * R, const unsigned char * commands, size_t sz)
{
    ASSERT_OBJ  (Stack_t, stk);
    assert      (commands);
    assert      (sz > 0);

    #define DEF_CMD( name, num, extra_data, function )      \
                                case CMD_##name:            \
                                    function                \
                                    break;


    size_t cur = 0;
    int tmp = 0;
    int sign = 0;

    while (cur < sz)
    {
        tmp = 0;
        sign = 0;

        //stack_dump (stk, NAME( stk ));

        switch (commands[cur++])
        {
        default:
            printf ("ERROR: unknown command %d", commands[--cur]);
            return 1;
            break;
        #include "cmdList.h"
        }
    }

    #undef DEF_CMD
}

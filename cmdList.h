DEF_CMD(PUSH ,  1, 1, {tmp = 1 - 2 * commands[cur++]; stack_push (stk, tmp * (*(int *)(commands + cur))); cur += sizeof (int);})
DEF_CMD(PUSHR,  2, 2, {stack_push (stk, R[commands[cur++]]);})
DEF_CMD(POP  ,  3, 2, {R[commands[cur++]] = stack_pop (stk);})
DEF_CMD(ADD  ,  4, 0, {tmp = stack_pop (stk) + stack_pop (stk); (fabs (tmp) <= MAX_INT) ? stack_push (stk, tmp) : (printf ("\nERROR: overflow\n"), abort (), 1);})
DEF_CMD(SUB  ,  5, 0, {tmp = stack_pop (stk); tmp = stack_pop (stk) - tmp; (fabs (tmp) <= MAX_INT) ? stack_push (stk, tmp) : (printf ("\nERROR: overflow\n"), abort (), 1);})
DEF_CMD(MUL  ,  6, 0, {tmp = stack_pop (stk) * stack_pop (stk); (fabs (tmp) <= MAX_INT) ? stack_push (stk, tmp) : (printf ("\nERROR: overflow\n"), abort (), 1);})
DEF_CMD(DIV  ,  7, 0, {tmp = stack_pop (stk); (tmp != 0) ? stack_push (stk, stack_pop (stk) / tmp) : (printf ("\nERROR: divide by zero\n"), abort (), 1);})
DEF_CMD(OUT  ,  8, 0, {tmp = stack_pop (stk); stack_push (stk, tmp); printf ("OUT\t%d\n", tmp);})
DEF_CMD(JMP  ,  9, 3, {cur = commands[cur] - 1;})
DEF_CMD(JG   , 10, 3, {tmp = stack_pop (stk); int tmp2 = stack_pop (stk); stack_push (stk, tmp2); stack_push (stk, tmp); (tmp2 > tmp) ? cur++ : cur = commands[cur] - 1;})
DEF_CMD(LABEL, 11, 4, {printf ("\nHELLO, I'M LABEL AND WHAT I'M DOING HERE?\n");})
DEF_CMD(END  , 12, 0, {cur = sz; printf ("END");})

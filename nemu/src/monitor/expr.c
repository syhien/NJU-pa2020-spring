#include "nemu.h"
#include "cpu/reg.h"
#include "memory/memory.h"

#include <stdlib.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
uint32_t look_up_symtab(char *sym, bool *success);
enum
{
	NOTYPE = 256,
	NEG,
	POIT,
	EQ,
	NEQ,
	NUM,
	HEX,
	REG,
	SYMB,
	NL,// >=
	NG,// <=
	AND,
	OR

	/* TODO: Add more token types */

};

static struct rule
{
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	{" +", NOTYPE}, // white space
	{"0[xX][0-9a-fA-F]+", HEX},
	{"[0-9]+", NUM},
	
	{"\\$e[a-z]{2}", REG},
	{"[a-zA-Z_]{1}[a-zA-Z0-9_]*", SYMB},
	
	{"==", EQ},
	{"!=", NEQ},
	
	{"\\+", '+'},
	{"\\-", '-'},
	{"\\*", '*'},
	{"/", '/'},
	{"%", '%'},
	{"!={0}", '!'},
	
	{">=", NL}, // not less
	{"<=", NG}, // not greater
	{">={0}", '>'},
	{"<={0}", '<'},
	{"\\(", '('},
	{"\\)", ')'},
	
	{"&&", AND},
	{"\\|\\|", OR}
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]))

static regex_t re[NR_REGEX];

/* Rules are used for more times.
 * Therefore we compile them only once before any usage.
 */
void init_regex()
{
	int i;
	char error_msg[128];
	int ret;

	for (i = 0; i < NR_REGEX; i++)
	{
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if (ret != 0)
		{
			regerror(ret, &re[i], error_msg, 128);
			assert(ret != 0);
		}
	}
}

typedef struct token
{
	int type;
	char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e)
{
	int position = 0;
	int i;
	regmatch_t pmatch;

	nr_token = 0;

	while (e[position] != '\0')
	{
		/* Try all rules one by one. */
		for (i = 0; i < NR_REGEX; i++)
		{
			if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0)
			{
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				//printf("match regex[%d] at position %d with len %d: %.*s\n", i, position, substr_len, substr_len, substr_start);
				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i]. 
				 * Add codes to perform some actions with this token.
				 */

				switch (rules[i].token_type)
				{
				case NOTYPE:
				    break;
				case NUM:
				case HEX:
				case REG:
				case SYMB:
				    *tokens[nr_token].str = 0;
				    strncat(tokens[nr_token].str, substr_start, substr_len);
				default:
					tokens[nr_token].type = rules[i].token_type;
					nr_token++;
				}

				break;
			}
		}

		if (i == NR_REGEX)
		{
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}

    for (int i = 0; i < nr_token; i++)
    {
        if (tokens[i].type == '*')
        {
            if (!i || (tokens[i-1].type!=NUM && tokens[i-1].type!=REG && tokens[i-1].type!=')'))
                tokens[i].type = POIT;
        }
        else if(tokens[i].type == '-')
        {
            if (!i || (tokens[i-1].type!=NUM && tokens[i-1].type!=REG && tokens[i-1].type!=')'))
                tokens[i].type = NEG;
        }
    }
	return true;
}

bool parent_match(int p, int q)
{
    if (p > q)
        return false;
    int cnt = 0;
    for (int i = p; i<= q; i++)
    {
        if (tokens[i].type == '(')
            cnt++;
        else if (tokens[i].type == ')')
            cnt--;
        if (cnt < 0)
            return false;
    }
    return cnt ? false: true;
}

bool check_parentness(int p, int q, bool *success)
{
    //printf("checking parentness is %d\n", (tokens[p].type == '(' && tokens[q].type == ')') && parent_match(p + 1, q - 1));
    return (tokens[p].type == '(' && tokens[q].type == ')') && parent_match(p + 1, q - 1);
}

int get_priority(int op)
{
    switch (op)
    {
    case '(':
    case ')':
        return 1;
        
    case '!':
    case POIT:
    case NEG:
        return 2;
        
    case '*':
    case '/':
        return 3;
        
    case '+':
    case '-':
        return 4;
        
    case '<':
    case '>':
    case NL:
    case NG:
        return 6;
        
    case EQ:
    case NEQ:
        return 7;
    
    case AND:
        return 11;
        
    case OR:
        return 12;
    
    default:
        return 0;
    }
}

int dominant_operator(int p, int q, bool *success)
{
    int domiop = p;
    int cnt = 0;
    for (int i = p + 1; i <= q; i++)
        if (tokens[i].type == '(')
            cnt++;
        else if (tokens[i].type == ')')
            cnt--;
        else if (!cnt)
        {
            if (get_priority(tokens[i].type) == 2)
            {
                if (get_priority(tokens[i].type) > get_priority(tokens[domiop].type))
                    domiop = i;
            }
            else
            {
                if (get_priority(tokens[i].type) >= get_priority(tokens[domiop].type))
                    domiop = i;
            }
        }
    return domiop;
}

uint32_t cal_1(int optype, uint32_t val, bool *success)
{
    switch(optype)
    {
    case NEG: 
        return -val;
    case POIT: 
        return vaddr_read(val, SREG_DS, 4);
    case '!': 
        return !val;
    default: 
        *success = false; 
    }
    return 0;
}

uint32_t cal_2(uint32_t val1, int optype, uint32_t val2, bool *success)
{
    switch(optype)
    {
    case '+': 
        return val1 + val2;
    case '-': 
        return val1 - val2;
    case '*': 
        return val1 * val2;
    case '/': 
        if(val2 == 0)
        {
            *success = false;
            break;
        }
    case AND: 
        return val1 && val2;
    case OR: 
        return val1 || val2;
    case EQ: 
        return val1 == val2;
    case NEQ: 
        return val1 != val2;
    case '>': 
        return val1 > val2;
    case NL: 
        return val1 >= val2;
    case '<': 
        return val1 < val2;
    case NG: 
        return val1 <= val2;
    default: 
        *success = false;
    }
    return 0;
}

uint32_t eval(int p, int q, bool *success)
{
    if (p > q)
    {
        *success = false;
        return 0;
    }
    if (p == q)
    {
        uint32_t res = 0;
        switch (tokens[p].type)
        {
        case HEX:
            sscanf(tokens[p].str, "%x", &res);
            *success = true;
            return res;
            break;
        case NUM:
            sscanf(tokens[p].str, "%d", &res);
            *success = true;
            return res;
            break;
        case REG:
            return get_reg_val(tokens[p].str + 1, success);
        case SYMB:
            return look_up_symtab(tokens[p].str, success);
        default:
            *success = false;
            return 0;
        }
    }
    if (check_parentness(p, q, success))
    {
        return eval(p + 1, q - 1, success);
    }
    int op = dominant_operator(p, q, success);
    //printf("op is %s\n", tokens[op].str);
    if (tokens[op].type == NEG || tokens[op].type == POIT || tokens[op].type == '!')
        return cal_1(tokens[op].type, eval(p+1, q, success), success);
    if (tokens[op].type == NUM || tokens[op].type == REG || tokens[op].type == SYMB)
    {
        *success = false;
        return 0;
    }
    return cal_2(eval(p, op-1, success), tokens[op].type, eval(op+1, q, success), success);
    return 0;
}

uint32_t expr(char *e, bool *success)
{
	if (!make_token(e))
	{
		*success = false;
		return 0;
	}
	return eval(0, nr_token - 1, success);
    
	//printf("\nPlease implement expr at expr.c\n");
	//fflush(stdout);
	//assert(0);

	//return 0;
}

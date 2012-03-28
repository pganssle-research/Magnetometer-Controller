/* This is a library intended to parse basic math to doubles or 
// boolean values (always returns a double, since this is c and 
// there is no such type as bool). 
// //
// Supports the following operations: //
// +-/*^() -> Basic arithmetic operations //
// exp(), log() -> Exponents and natural logs //
// &&, ||, ! -> Boolean operators //
// =, !=, >, <, >=, <= -> Comparators //
// &, | -> Bitwise AND and OR //
// //
//
/*
Version History
----------------------------------------------------------------------
07/15/2011 - v0.1 -> The first version, no changes.
03/05/2012 - v1.0 -> Tokenized, non-recursive.  Use stack to parse
		     from infix expression to postfix expression
		     and then evaluate.
*/
/* On versions of C that have ansi_c.h, uncomment and probably don't
// use  add_ons.h
#include <ansi_c.h> 
*/
#define max_inputs 200
#define STACKSIZE max_inputs 
#include "add_ons.h"
#include "MathParserLib.h"
#define max_expr_len 500
#define num_type 1
#define op_type 2
#define lp_type 3
#define rp_type 4


/*//////////////////////////////////////////////////////////////////////
// //
// This is the call to use from the outside...  Currently constants
//  argument is not used.
// //
//////////////////////////////////////////////////////////////////////
*/

double parse_math(char *expr, constants *c, int *err_val, int s_pos) {


/* Error Values:
0 => No error;
1 => Parenthesis mismatch
2 => String replacement error
3 => Expression max length exceeded
4 => Function output is undefined.
5 => No return value.
6 => Parse error.
7 => Malformed input string.
*/

double vals [max_inputs];
double vals_c [max_inputs];   
unsigned short isvar [max_inputs];

char toks [max_expr_len/2][25];
double result = 0;

int i=0, j=0, s=-1, e=-1, n=-1, v=-1, curr_v=0;
char decimaldigits[]= ".0123456789"; 
char op_chars[] = "+-*/%<>^&|=!~#$,:;?@`";
char delims[] = "*%/^+~-()&|!=><#$,:;?@`"; /* all delimiters to get vals */
char temp [30];
int itype, prev_type;
double item;
double item1;
double item2;
char expr_out[max_expr_len];
char expr_out1[max_expr_len];
char expr_post[max_expr_len];   /* postfix expression no parens  */
int ret;

err_val[0] = 0; 

if(strlen(expr) <= 0) {
  err_val[0] = 7; 
  return (double) 0;
}

if(strlen(expr) >= max_expr_len) {
  err_val[0] = 3; 
  return (double) 0;
}

strcpy(expr_out, expr); /* Copy the input to our output. */

ret = cleanup_input_string (expr_out);
/*printf ("after cleanup, string, length = %s, %d\n", expr_out, strlen(expr_out));
*/

if(strlen(expr_out) == 0) {
  err_val[0] = 5; 
  return (double) 0;
}

memset (expr_post, 0, sizeof (expr_post));

strrep (expr_out, "exp", "1#", strlen(expr_out));
strrep (expr_out, "EXP", "1#", strlen(expr_out));
strrep (expr_out, "log", "1$", strlen(expr_out));
strrep (expr_out, "LOG", "1$", strlen(expr_out));
strrep (expr_out, "!", "1!", strlen(expr_out));
strrep (expr_out, "&&", ",", strlen(expr_out));
strrep (expr_out, "||", ":", strlen(expr_out));
strrep (expr_out, "!=", ";", strlen(expr_out));
strrep (expr_out, ">=", "?", strlen(expr_out));
strrep (expr_out, "<=", "@", strlen(expr_out));
strrep (expr_out, "==", "~", strlen(expr_out));


/*printf ("replacing multichars, string = %s\n", expr_out);
*/

strcpy(expr_out1, expr_out); /* Additional copy as strtok modifies its input */

/* Get array of numbers as sanity check */
ret = find_tokens (expr_out1, delims, toks);
while (strcmp (toks[i], "TheEnd") != 0) {
  v++;
  vals[v] = atof (toks[i]);
  i++;
}

/* Parse infix to postfix */
i = 0;
prev_type = -1;

while (i < strlen(expr_out)) {

/* if the item is a left paren 
//	Push the item on the stack 
*/
  if (expr_out[i] == '(') {
      s++;              /* count left parens */
      prev_type = lp_type;
      push (&stk_ptr, '(', lp_type);   
/*      printf ("pushed left paren\n"); */
      i++;
  }
/*	else if the item is a right paren 
//	Pop a thing off of the stack. 
//	while that thing is not a left paren 
//	  Add the thing to the string with a space 
//	  Pop a thing off of the stack 
*/
  else if (expr_out[i] == ')') {
      e++;		/* count right parens */
      prev_type = rp_type;
      itype = 0;
      while (itype != 99) {  /* 99 is stack empty */
        item = pop(&stk_ptr, &itype);
/*        printf ("expr_out is rp, popped  %c type %d\n", (char) item, itype);
*/
	if (itype != lp_type) {
	  if (itype == num_type) {
	    sprintf (temp, "%e ", item);
	    strcat (expr_post, temp);
          }
          else {
	    sprintf (temp, "%c ", (char) item);
	    strcat (expr_post, temp);
	  }
/*          printf ("conc %s to expr_post\n", temp); */
        }
	else {
	  break;
        }
      }
      i++;
  }
/*  if the item is an operator then 
//	While the stack is not empty and an operator is at the top and the
//	operator at the top is higher priority that the item then 
//	  Pop the operator on the top of the stack 
//	  Add the popped operator to the string with a space 
//	Push the item on the stack 
*/
  else if (strchr(op_chars, expr_out[i]) != NULL) {
      itype = 0;
/*    Handle unary - operator now */
      if ((expr_out[i] == '-') && ((prev_type != num_type)
				&& (prev_type != rp_type))) {
        expr_out[i] = '`';   /* ` is unary -  */
      }
      prev_type = op_type;
      while (itype != 99) {  /* 99 is stack empty */
        item = pop(&stk_ptr, &itype);
/*        printf ("expr_out[i] is %c, popped  %c type %d\n", expr_out[i],
//		 (char) item, itype);
*/
	if ((itype == op_type) && 
	    op_priority ((int) item) < op_priority ((int) expr_out[i])) {
	  sprintf (temp, "%c ", (char) item);
/*          printf ("op, conc %s to expr_post\n", temp); */
	  strcat (expr_post, temp);
        }
	else {
	  if (itype != 99) {
            push (&stk_ptr, item, itype);   /* put back on stack */
/*            printf ("op, pushed item back on stk %c type %d\n", 
//			(char) item, itype);
*/
	  }
	  break;
        }
      }
      push (&stk_ptr, expr_out[i], op_type);   
/*      printf ("pushed op from expr %c\n", (char) expr_out[i]); */
      i++;
  }
  else {
      prev_type = num_type;
      j = i;
      while ((i < strlen (expr_out)) &&
	     (strchr(decimaldigits, expr_out[i]) != NULL)) i++;
      strncpy (temp, &expr_out [j], i-j);
      temp [i-j] = ' ';
      temp [i-j+1] = 0;
/*      printf ("number, conc %s to expr_post i= %d\n", temp, i); */
      strcat (expr_post, temp);
      vals_c[curr_v] = atof (temp);
      curr_v++;
  }
/*  printf ("vals[%d] = %e i = %d\n", curr_v, vals[curr_v], i); */
}

itype = 0;
while (itype != 99) {  /* 99 is stack empty */
  item = pop(&stk_ptr, &itype);
  if (itype == num_type) {
    sprintf (temp, "%e ", item);
/*    printf ("last loop, popped  %e type %d\n", item, itype); */
    strcat (expr_post, temp);
  }
  else if (itype == op_type) {
    sprintf (temp, "%c", (char) item);
/*    printf ("last loop, popped  %c type %d\n", (char) item, itype);  */
    strcat (expr_post, temp);
  }
  /* don't add parens */
}

if (s != e) {
  err_val[0] = 1; /* paren mismatch */
  return (double) 0;
}

/* compare numbers found with 2 methods as sanity check */
for (i = 0; i <= v; i++) {
  if ((vals_c[i] > vals[i] +.0001) || (vals_c[i] < vals[i] - .0001)) {
    err_val[0] = 6; 
    return (double) 0;
  }
}

/* printf ("Expr_post is %s len %d\n", expr_post, strlen (expr_post)); */

/* Now we have a postfix expression
// When the loop is done the answer is the only item left on the stack.
// For each item in the postfix expression from the left:
*/
i = 0;
while (i < strlen(expr_post)) {
  if (expr_post[i] == ' ') {
  }
  /* if the item is an operator then */
  else if (strchr(op_chars, expr_post[i]) != NULL) {
      if (expr_post[i] == '`') {
        item1 = pop(&stk_ptr, &itype);
      }
      /* pop two numbers off the stack */
      else {
        item1 = pop(&stk_ptr, &itype);
        item2 = pop(&stk_ptr, &itype);
      }
      /* make a calculation: the second number popped-operator-first number */
      switch (expr_post[i]) {
      case '`':
        item = item1 * -1; break;
      case '+':
        item = item2 + item1; break;
      case '-':
        item = item2 - item1; break;
      case '*':
        item = item2 * item1; break;
      case '/':
        item = item2 / item1; break;
      case '%':
        item = fmod (item2, item1); break;
      case '!':
        item = (double) (! (int) item1); /* Negation */
        break;
      case '#':
	item = exp (item1); break;
      case '$':
	item = log (item1); break;
      case '^':
        item = pow(item2, item1); 
	break;
      case '&':
        item = (double) (((int) item2) & ((int) item)); /* Bitwise AND */
        break;
      case '|':
        item = (double) (((int) item2) | ((int) item1)); /* Bitwise OR */
        break;
      case ',':
        item = (double) (((int) item2) && ((int) item1)); /* Logical AND */
        break;
      case ':':
        item = (double) (((int) item2) || ((int) item1)); /* Logical OR */
        break;
      case ';':
        item = (double) (((int) item2) != ((int) item1)); /* Comparator */
        break;
      case '?':
        item = (double) (((int) item2) >= ((int) item1)); /* Comparator */
        break;
      case '@':
        item = (double) (((int) item2) <= ((int) item1)); /* Comparator */
        break;
      case '~':
        item = (double) (((int) item2) == ((int) item1)); /* Comparator */
        break;
      case '>':
        item = (double) (((int) item2) > ((int) item1)); /* Comparator */
        break;
      case '<':
        item = (double) (((int) item2) < ((int) item1)); /* Comparator */
        break;
      }
/*      printf ("op is %c, result, item2, item1 are %e %e %e\n", expr_post[i],
//		item, item2, item1);
*/
      /* push the result on the stack */
      push (&stk_ptr, item, num_type);   
  }
  /* if the item is a number push it on the stack */
  else {
      j = i;
      while (strchr(decimaldigits, expr_post[i]) != NULL) i++;
      strncpy (temp, &expr_post [j], i-j);
      temp [i-j] = 0;
      item = atof(temp);
      push (&stk_ptr, item, num_type);   
/*    printf ("pushed item is %e %s\n", item, temp); */
  }
  i++;
}

result = pop(&stk_ptr, &itype);
return result;

}  /* parse_math */

/* Order of operations: Powers, multiplication/division/modulo, 
// addition/subtraction, bitwise
*/

int op_priority (int op) {
char curr_op;

  curr_op = (char) op;
  /* log or exp */
  if ((curr_op == '#') || (curr_op == '$')) return 1;
  if (curr_op == '`') return 2;
  if (curr_op == '!') return 3;
  if (curr_op == '^') return 4;
  if ((curr_op == '*') || (curr_op == '/') || (curr_op == '%')) return 5;
  if ((curr_op == '+') || (curr_op == '-')) return 6;

  /* logical comparator */
  if ((curr_op == '<') || (curr_op == '>') || (curr_op =='@') ||
      (curr_op == '?') || (curr_op == ';') || (curr_op == '~')) return 7;
  if ((curr_op == '&') || (curr_op == '|')) return 8;
  /* logical and or */
  if ((curr_op == ',') || (curr_op == ':')) return 9;

  printf ("Error, operator unknown \n");
  return 10;
}


/* Delete a single character from *input at index. */

char *del_char(char *input, int index) {
int i;

if(input == NULL)
  return NULL;

for(i=index; i<strlen(input); i++) {
  input[i] = input[i+1]; /* Move the next character over. */
}
input[i] = '\0'; /* Null terminate. */

return input;
}

char *strrep(char *str, char *s1, char *s2, int max_len) {
/* Replace all instances of s1 with s2 in str */
char temp_s[max_expr_len];
char *tok;

if(strcmp (s1, s2) == 0) {
  /* Thought you could trick me into generating an infinite loop? */
  return str;
}
tok = strstr(str, s1);
if (tok == NULL) return NULL;

strcpy (temp_s, s2);
strcat (temp_s, tok + strlen(s1));
*tok = NULL;
strcat (str, temp_s);
return str;
}

char *chrrep(char *str, char c1, char c2) {
/* Replace all instances of c1 with c2 in str; */
char *tok;

if(c1 == c2) {
  /* Thought you could trick me into generating an infinite loop? */
  return str;
}
tok = strchr(str, c1);
while(tok != NULL) {
  tok[0] = c2; /* Replace it. */
  tok = strchr(tok, c1); /* Find the first instance. */
}

return str;
}

int cleanup_input_string (char * expr_out) {
char *pos;
char cb;
int i, j, n_pos = 0;
static char op_chars[7] = "*%/^+-"; /* Reserved characters. */

/* Start by removing whitespace. It's unnecessary.
// Find any whitespace or other undesirables
*/

pos = strpbrk(expr_out, " \n\"\\'?;:@_"); 
while(pos != NULL) {
  pos = del_char(pos, 0); /* Delete the whitespace thing. */
  /* No need to decrement pos, it will just start looking at the next char, 
  // since we deleted one.  */
  pos = strpbrk(pos, " \n\"\\'?;:@#$_"); 
}

/* The beginning can't be a special character (unless it's ! or -) */

pos = strpbrk(expr_out, "+*/%<>^&|#&");
/* If we started with any of these operators, that's extraneous, delete it. */
if(pos != NULL && pos == expr_out) {
  expr_out = del_char(expr_out, 0);
} 

/* Similarly, the end can't be a special character. */
pos = strpbrk(&expr_out[strlen(expr_out)-1], "+-*/%<>^&|=!~$#");
while(pos != NULL) {
  /* Just keep deleting the trailing special characters. */
  expr_out = del_char(expr_out, strlen(expr_out)-1); 
  pos = strpbrk(&expr_out[strlen(expr_out)-1], "+-*/%<>^&|=!~");
}

/* strip out extraneous + signs, detect errors. */
i = 0;
pos = expr_out;
while(i++ < strlen(expr_out)) {
  pos = strpbrk(pos, "+-"); /* Find the next instance of a + or - sign. */
  if(pos == NULL) {
    break;
  }
  pos++;  /* Increment pos. */
}
return 1;
}

int find_tokens (char *inp_string, char *delims, char toks[][25]) {
  char *token;
  int i=0;
     
  token = strtok (inp_string, delims);      
  strcpy (toks [i], token);
  i++;
  while (token != NULL) {
     token = strtok (NULL, delims);    
     if (token != NULL) {
       strcpy (toks [i], token);
       i++;
     }     
  } 
  strcpy (toks [i], "TheEnd");

  return 1;
}

/* The push() operation is used both to initialize the stack, and to 
//store values to it. It is responsible for inserting (copying) the value 
//into the ps->items[] array and for incrementing the element counter 
// (ps->size).  */

int push(STACK *ps, double x, int t) {
    if (ps->size == STACKSIZE) {
        printf("Error: stack overflow\n");
        exit (0);
    } else {
        ps->items[ps->size] = x;
        ps->item_type[ps->size] = t;
	ps->size++;
    }
    return 1;
}

/* The pop() operation is responsible for removing a value from the stack, and
//decrementing the value of ps->size.
*/

double pop (STACK *ps, int *t) {
    if (ps->size == 0){
        *t = 99;
	return 0;
    } else {
        ps->size--;
        *t = ps->item_type[ps->size];
        return ps->items[ps->size];
    }
}



 
int main ()
{
  int ierr, pos;
  double resultc, resultm;
  char inp_line[100];

  pos = 0;

  resultc = 1 + pow(exp(33), 2);

  strcpy (inp_line, "1+exp(33)^2");
  printf ("input is %s\n", inp_line);

  resultm = parse_math (inp_line, NULL, &ierr, pos);
  if (ierr != 0) 
    printf ("Error from parse_math: %d\n", ierr);
  else
    printf ("C calculated result: %e, parse_math: %e\n", resultc, resultm);

  resultc = (((23.553+2.99/1.33)/.923)-(10*33.3));

  strcpy (inp_line, "(((23.553+2.99/1.33)/.923)-(10*33.3))");
  printf ("input is %s\n", inp_line);

  resultm = parse_math (inp_line, NULL, &ierr, pos);
  if (ierr != 0) 
    printf ("Error from parse_math: %d\n", ierr);
  else
    printf ("C calculated result: %e, parse_math: %e\n", resultc, resultm);

  resultc = (log(9.55) * exp (3)) - 3.21 * 35;

  strcpy (inp_line, "(log(9.55) * exp(3)) - 3.21 * 35");
  printf ("input is %s\n", inp_line);

  resultm = parse_math (inp_line, NULL, &ierr, pos);
  if (ierr != 0) 
    printf ("Error from parse_math: %d\n", ierr);
  else
    printf ("C calculated result: %e, parse_math: %e\n", resultc, resultm);

  resultc = pow(99.2, 3) * 1.22 /0.9 + 3 -99;

  strcpy (inp_line, "99.2 ^3 * 1.22 /0.9 + 3 -99");
  printf ("input is %s\n", inp_line);

  resultm = parse_math (inp_line, NULL, &ierr, pos);
  if (ierr != 0) 
    printf ("Error from parse_math: %d\n", ierr);
  else
    printf ("C calculated result: %e, parse_math: %e\n", resultc, resultm);

  resultc = (2-3) == (4-5);

  strcpy (inp_line, "(2-3) == (4-5)");
  printf ("input is %s\n", inp_line);

  resultm = parse_math (inp_line, NULL, &ierr, pos);
  if (ierr != 0) 
    printf ("Error from parse_math: %d\n", ierr);
  else
    printf ("C calculated result: %e, parse_math: %e\n", resultc, resultm);

  resultc = !((2-3) == (4-5));

  strcpy (inp_line, "!((2-3) == (4-5))");
  printf ("input is %s\n", inp_line);

  resultm = parse_math (inp_line, NULL, &ierr, pos);
  if (ierr != 0) 
    printf ("Error from parse_math: %d\n", ierr);
  else
    printf ("C calculated result: %e, parse_math: %e\n", resultc, resultm);

  resultc = 9 <= 3 * 23 /3;

  strcpy (inp_line, "9 <= 3 * 23 /3");
  printf ("input is %s\n", inp_line);

  resultm = parse_math (inp_line, NULL, &ierr, pos);
  if (ierr != 0) 
    printf ("Error from parse_math: %d\n", ierr);
  else
    printf ("C calculated result: %e, parse_math: %e\n", resultc, resultm);

  resultc = ((2*4-6/3)*(3*5+8/4))-(2+3);

  strcpy (inp_line, "((2*4-6/3)*(3*5+8/4))-(2+3)");
  printf ("input is %s\n", inp_line);

  resultm = parse_math (inp_line, NULL, &ierr, pos);
  if (ierr != 0) 
    printf ("Error from parse_math: %d\n", ierr);
  else
    printf ("C calculated result: %e, parse_math: %e\n", resultc, resultm);

  resultc = exp(5.332) - log(10+3.1/2);

  strcpy (inp_line, "exp(5.332) - log(10+3.1/2)");
  printf ("input is %s\n", inp_line);

  resultm = parse_math (inp_line, NULL, &ierr, pos);
  if (ierr != 0) 
    printf ("Error from parse_math: %d\n", ierr);
  else
    printf ("C calculated result: %e, parse_math: %e\n", resultc, resultm);

  resultc = (3.2 / 3.21 < 1) && ! (10 < 3*8);

  strcpy (inp_line, "(3.2 / 3.21 < 1) && ! (10 < 3*8)");
  printf ("input is %s\n", inp_line);

  resultm = parse_math (inp_line, NULL, &ierr, pos);
  if (ierr != 0) 
    printf ("Error from parse_math: %d\n", ierr);
  else
    printf ("C calculated result: %e, parse_math: %e\n", resultc, resultm);

  resultc = (pow(2,3) - pow(3,.5)) + pow(4,9);

  strcpy (inp_line, "(2^3 - 3^.5) + 4^9");
  printf ("input is %s\n", inp_line);

  resultm = parse_math (inp_line, NULL, &ierr, pos);
  if (ierr != 0) 
    printf ("Error from parse_math: %d\n", ierr);
  else
    printf ("C calculated result: %e, parse_math: %e\n", resultc, resultm);

  resultc = pow((.12345 / .90123), 3) * exp(1.07);

  strcpy (inp_line, "(.12345 / .90123) ^ 3 * exp(1.07)");
  printf ("input is %s\n", inp_line);

  resultm = parse_math (inp_line, NULL, &ierr, pos);
  if (ierr != 0) 
    printf ("Error from parse_math: %d\n", ierr);
  else
    printf ("C calculated result: %e, parse_math: %e\n", resultc, resultm);

  resultc = log((.12345 / .90123) * 3) / exp(1.07);

  strcpy (inp_line, "log((.12345 / .90123) * 3) / exp(1.07)");
  printf ("input is %s\n", inp_line);

  resultm = parse_math (inp_line, NULL, &ierr, pos);
  if (ierr != 0) 
    printf ("Error from parse_math: %d\n", ierr);
  else
    printf ("C calculated result: %e, parse_math: %e\n", resultc, resultm);

  resultc = (8129321.2 /(900 > 3)) * exp(1.9);

  strcpy (inp_line, "(8129321.2 /(900 > 3)) * exp(1.9)");
  printf ("input is %s\n", inp_line);

  resultm = parse_math (inp_line, NULL, &ierr, pos);
  if (ierr != 0) 
    printf ("Error from parse_math: %d\n", ierr);
  else
    printf ("C calculated result: %e, parse_math: %e\n", resultc, resultm);

  resultc = pow(-3,2) + pow(-4, 3);

  strcpy (inp_line, "(-3) ^2 + (-4) ^ 3");
  printf ("input is %s\n", inp_line);

  resultm = parse_math (inp_line, NULL, &ierr, pos);
  if (ierr != 0) 
    printf ("Error from parse_math: %d\n", ierr);
  else {
    printf ("C calculated result: %e, parse_math: %e\n", resultc, resultm);
  }

  resultc = exp (-3.22) + log (32);

  strcpy (inp_line, "exp (-3.22) + log (32)");
  printf ("input is %s\n", inp_line);

  resultm = parse_math (inp_line, NULL, &ierr, pos);
  if (ierr != 0) 
    printf ("Error from parse_math: %d\n", ierr);
  else {
    printf ("C calculated result: %e, parse_math: %e\n", resultc, resultm);
  }

  resultc = -22.23 + 0.123;

  strcpy (inp_line, "-22.23 + 0.123");
  printf ("input is %s\n", inp_line);

  resultm = parse_math (inp_line, NULL, &ierr, pos);
  if (ierr != 0) 
    printf ("Error from parse_math: %d\n", ierr);
  else {
    printf ("C calculated result: %e, parse_math: %e\n", resultc, resultm);
  }

  resultc = (5 + -3) / 2;

  strcpy (inp_line, "(5 + -3) / 2");
  printf ("input is %s\n", inp_line);

  resultm = parse_math (inp_line, NULL, &ierr, pos);
  if (ierr != 0) 
    printf ("Error from parse_math: %d\n", ierr);
  else {
    printf ("C calculated result: %e, parse_math: %e\n", resultc, resultm);
  }

  resultc = (33 - (-3.22)) + 0 -23.1;

  strcpy (inp_line, "(33 - (-3.22)) + 0 -23.1");
  printf ("input is %s\n", inp_line);

  resultm = parse_math (inp_line, NULL, &ierr, pos);
  if (ierr != 0) 
    printf ("Error from parse_math: %d\n", ierr);
  else {
    printf ("C calculated result: %e, parse_math: %e\n", resultc, resultm);
  }

  resultc = exp (-3.22) * (-32);

  strcpy (inp_line, "exp (-3.22) * (-32)");
  printf ("input is %s\n", inp_line);

  resultm = parse_math (inp_line, NULL, &ierr, pos);
  if (ierr != 0) 
    printf ("Error from parse_math: %d\n", ierr);
  else {
    printf ("C calculated result: %e, parse_math: %e\n", resultc, resultm);
  }

  resultc = 2*3 - !(48/4-4*5);

  strcpy (inp_line, "2*3 - !(48/4-4*5)");
  printf ("input is %s\n", inp_line);

  resultm = parse_math (inp_line, NULL, &ierr, pos);
  if (ierr != 0) 
    printf ("Error from parse_math: %d\n", ierr);
  else {
    printf ("C calculated result: %e, parse_math: %e\n", resultc, resultm);
  }

  resultc = - 5 < 0;

  strcpy (inp_line, "-5 < 0");
  printf ("input is %s\n", inp_line);

  resultm = parse_math (inp_line, NULL, &ierr, pos);
  if (ierr != 0) 
    printf ("Error from parse_math: %d\n", ierr);
  else {
    printf ("C calculated result: %e, parse_math: %e\n", resultc, resultm);
  }

  resultc = -55 == 55;

  strcpy (inp_line, "-55 == 55");
  printf ("input is %s\n", inp_line);

  resultm = parse_math (inp_line, NULL, &ierr, pos);
  if (ierr != 0) 
    printf ("Error from parse_math: %d\n", ierr);
  else {
    printf ("C calculated result: %e, parse_math: %e\n", resultc, resultm);
  }

  resultc = (16 | 32);

  strcpy (inp_line, "(16 | 32)");
  printf ("input is %s\n", inp_line);

  resultm = parse_math (inp_line, NULL, &ierr, pos);
  if (ierr != 0) 
    printf ("Error from parse_math: %d\n", ierr);
  else {
    printf ("C calculated result: %e, parse_math: %e\n", resultc, resultm);
  }

  resultc = (16 & 32);

  strcpy (inp_line, "(16 & 32)");
  printf ("input is %s\n", inp_line);

  resultm = parse_math (inp_line, NULL, &ierr, pos);
  if (ierr != 0) 
    printf ("Error from parse_math: %d\n", ierr);
  else {
    printf ("C calculated result: %e, parse_math: %e\n", resultc, resultm);
  }

  resultc = 2*3-48/4-4*5;

  strcpy (inp_line, "2*3-48/4-4*5");
  printf ("input is %s\n", inp_line);

  resultm = parse_math (inp_line, NULL, &ierr, pos);
  if (ierr != 0) 
    printf ("Error from parse_math: %d\n", ierr);
  else {
    printf ("C calculated result: %e, parse_math: %e\n", resultc, resultm);
  }
  		
  printf ("The difference is the algorithm goes right to left in the absence \n");
  printf (" of parens,  See the following example... \n");
 
  resultc = 2*3-(48/4-4*5);

  strcpy (inp_line, "2*3-(48/4-4*5)");
  printf ("input is %s\n", inp_line);

  resultm = parse_math (inp_line, NULL, &ierr, pos);
  if (ierr != 0) 
    printf ("Error from parse_math: %d\n", ierr);
  else {
    printf ("C calculated result: %e, parse_math: %e\n", resultc, resultm);
  }


} 

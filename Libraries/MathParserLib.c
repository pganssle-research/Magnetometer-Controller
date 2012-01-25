//////////////////////////////////////////////////////////////////////
// 																	//
//					Math Parsing Libary, v.0.1						//
//					Paul Ganssle, 07/15/2011						//
//																	//
//	This is a library intended to parse basic math to doubles or	//
//  boolean values (always returns a double, since this is c and	//
//	there is no such type as bool). The main reason I implemented	//
//	this is because muparser was not working. I highly recommend	//
//	trying to get that working for a little bit before actually		//
//	using this library.												//
//																	//
//	Supports the following operations:								//
//  +-*/^() -> Basic arithmetic operations							//
//	exp(), log() -> Exponents and natural logs						//
//  &&, ||, ! -> Boolean operators									//
// 	=, !=, >, <, >=, <=	-> Comparators								//
//	&, | -> Bitwise AND and OR										//
//	Adding constant values											//
// 																	// 
//////////////////////////////////////////////////////////////////////


/*
Version History
----------------------------------------------------------------------
07/15/2011 - v0.1 -> The first version, no changes.


To Do:
----------------------------------------------------------------------
Title: Variable evaluation
Priority: Low

The current implementation always just returns numbers, but it should
likely be changed to return an array of operators and pointers to 
variables. This way, if you are going to evaluate an expression for 
100 different values of 3 variables, you just load up a single function
and iterate through, rather than parsing the same expression 1 million
times.*/


#include <ansi_c.h>
#include <MathParserLib.h>

//////////////////////////////////////////////////////////////////////
// 																	//
//						Parsing Functions							//
// 																	//
//////////////////////////////////////////////////////////////////////

double parse_math(char *expr, constants *c, int *err_val, int s_pos) {
	// This function breaks a function down into its basic components by calling itself recursively until it's just evaluating basic operations.
	// The output is dynamically allocated, so to avoid memory leaks, make sure to free it when you are done.
	
	err_val[0] = 0; // No errors yet.
	
	/* Error Values:
	0 => No error;
	1 => Parenthesis mismatch
	2 => String replacement error
	3 => Recursion error.
	4 => Function output is undefined.
	5 => No return value.
	7 => Malformed input string.
	*/
	
	
	if(strlen(expr) <= 0) {
		err_val[0] = 7; // Invalid input string.
		return NULL;
	}
	
	// Useful variables.
	char *expr_out = malloc(sizeof(char)*strlen(expr)+1), *expr_buff, *expr_buff2, *pos;
	char op_chars[7] = "*%/^+~"; // Reserved characters.
	int scl = strlen(op_chars); // How many are there?
	strcpy(expr_out, expr); // Copy the input to our output.
	int i, j, l = strlen(expr_out), n_pos = 0;
	
	///////////////////////////////////////////////////////////
	///														///
	///            		   Input Cleanup    				///
	///														///
	///////////////////////////////////////////////////////////
	
	// Start by removing whitespace. It's unnecessary.
	pos = strpbrk(expr_out, " \n\"\\'?;:@#$_"); // Find any whitespace or other undesirables
	while(pos != NULL) {
		pos = del_char(pos, 0); // Delete the whitespace thing.
		pos = strpbrk(pos, " \n\"\\'?;:@#$_"); //No need to decrement pos, it will just start looking at the next char, since we deleted one.
	}
	
	l = strlen(expr_out); // String length has probably changed.
	
	// The beginning can't be a special character (unless it's ! or ~)
	pos = strpbrk(expr_out, "+*/%<>^&|");
	if(pos != NULL && pos == expr_out) // If we started with any of these operators, that's extraneous, delete it.
	{
		expr_out = del_char(expr_out, 0);
		l = strlen(expr_out); // Length may have changed.
	} else if (expr_out[0] == '-') { // We're going to convert all -s to ~s, because there's confusion about whether it's an operator or not.
		expr_out[0] = '~';
	}
	
	// Similarly, the end can't be a special character.
	pos = strpbrk(&expr_out[l-1], "+-*/%<>^&|=!~");
	while(pos != NULL) {
		expr_out = del_char(expr_out, l-1); // Just keep deleting the trailing special characters.
		l = strlen(expr_out);
		pos = strpbrk(&expr_out[l-1], "+-*/%<>^&|=!~");
	}
	
	// Convert negative signs to ~, strip out extraneous + signs, detect errors.
	i = 0;
	pos = expr_out;
	while(i++ < l) {
		pos = strpbrk(pos, "+-"); // Find the next instance of a + or - sign.
		if(pos == NULL) {
			break;
		}
		
		char cb = (pos-1)[0]; // It's not possible for pos to be the start of the string, since that would have been converted.
		for(j = 0; j<scl; j++) {
			if(cb == op_chars[j]) { // If this immediately follows a special character, it's a sign.
				if(pos[0] == '+') {
					del_char(pos, 0);
					l = strlen(expr_out);
				} else if(pos[0] == '-') {
					pos[0] = '~';
					if(op_chars[j] == '~') {  // If you have a situation like this: ~-, just delete both.
						pos = del_char(pos-1, 0);
						pos = del_char(pos, 0);
						l = strlen(expr_out);
					}
				}
				break;
			}
		}
		
		if(j == scl) { // If we didn't match the look-behind condition, try and match the look-ahead conditions.
			cb = (pos+1)[0];
			if(cb == '\0')  // Can't have either one as the last thing.
			{
				pos[0] = '\0';
				l--;
				break;
			}
			
			if(cb == '-')
			{
				cb = '~'; //If we're at +- or --, the second one is the negative sign.
			}
		}
	
		pos++; // Increment pos.
	}
	
	if(strlen(expr_out) == 0) {
		err_val[0] = 5; // Empty string.
		return NULL;
	}
	
	
	///////////////////////////////////////////////////////////
	///														///
	///            		   Start Evaluation    				///
	///														///
	///////////////////////////////////////////////////////////
	
	if(s_pos <= ++n_pos) {
		expr_buff = eval_funcs_and_constants(expr_out, c, err_val, n_pos); // Strip out the constants and functions.
		
		free(expr_out); // Free the old expr_out;
		expr_out = expr_buff; // Point the main pointer at the new one.
		
		if(err_val[0] != 0) {
			return NULL; // Error.
		}
	}
	
	j = 0; 
	if(s_pos <= ++n_pos) {
		while(j++ < l) {
			pos = strchr(expr_out, '('); // Find the first parenthesis.
		
			if(pos == NULL)
				break;
		
			char *substring = malloc(sizeof(char)*strlen(pos)+1); // Allocate a substring.
			int offset = find_paren_contents(pos, substring); // Get the contents
			int e_val = 0;; // Error return value for the replacement. 	
		
			if (offset > 0) {
				double val = parse_math(substring, c, err_val, n_pos); // Parse the contents recursively.
			
				// If there's an error, free expr_buff.
				if(err_val[0] == 0) {
					expr_buff = malloc(500); // Allocate some room for the value.
					sprintf(expr_buff, "%lf", val);
					
					int ind = pos-expr_out;
					expr_buff2 = replace_chars(expr_out, expr_buff, ind, offset+ind+1, &e_val); // Replace the function in parens with whatever it evaluates to, inclusive of the parens.
					
					free(expr_buff);
				}
			
				if(e_val != 0) {
					free(expr_buff2);
					err_val[0] = 2; // String replacement error.
				}

			} else if(offset == 0) {
				expr_buff2 = replace_chars(expr_out, "", (pos-expr_out), (pos-expr_out)+offset+1, &e_val);
				if(e_val != 0) {
					free(expr_buff2);
					err_val[0] = 2; // String replacement error.
				}
			} else {
				err_val[0] = 1; // Parenthesis mismatch.
			}
		
			free(substring); // Free the substring.
			free(expr_out);
		
			if(err_val[0] != 0) {
				return NULL;
			}
	
			expr_out = expr_buff2; // Set the output to be what we just changed.
		
		}
	}

	// At this point in the function, we know that all functions and parens have been broken out and we are on to straight-up order-of-operations.
	// Now we need to break this up into chunks for boolean evaluation, if appropriate.
	
	///////////////////////////////////////////////////////////
	///														///
	///           		Boolean Operations    				///
	///														///
	///////////////////////////////////////////////////////////
	
	// The simplest one, which needs to be implemented on its own anyway, is the ! operator.
	if(s_pos <= ++n_pos) {
		pos = strchr(expr_out, '!'); // Find out if it's there at all.
		while(pos != NULL) {
			char cb = (++pos)[0];
			// Check if it's part of !=, if so, ignore it.
			if(cb == '=') {
				pos = strchr(pos, '!');
				continue;
			}
			
			char *pos2 = strpbrk(pos, "+-*/%^&|~><=!"); // Find the next operator -> We're going to NOT everything from here until that.
			int sublen;
			if(pos2 != NULL) {
				sublen = pos2-pos;
				if(sublen == 1) { // Delete the ! operator if it's right next to some other operator that's not an equal sign.
					del_char(&(--pos)[0], 0); // Delete the ! operator.
					pos = strchr(pos-1, '!'); // Search for the next one.
					continue;
				}
			} else {
				sublen = strlen(pos);
			}
			
			expr_buff = malloc(sublen+1); // We're going to grab the substring.
			strncpy(expr_buff, pos, sublen); // Copied over the substring now.
			expr_buff[sublen] = '\0'; // Null terminate.
			
			double val = parse_math(expr_buff, NULL, err_val, n_pos); // Parse what we just found.
			
			free(expr_buff); // Not needed anymore.
			if(err_val[0] != 0) { // Clean up
				free(expr_out);
				return NULL;
			}
			
			// Now we do the NOT operation.
			expr_buff = malloc(2);
			if(val) {
				strcpy(expr_buff, "0");
			} else {
				strcpy(expr_buff, "1");
			}
			
			// Now replace the expression with the inverted boolean value.
			int e_val = 0; // Error value.
			expr_buff2 = replace_chars(expr_out, expr_buff, (pos-expr_out), (pos-expr_out)+sublen-1, &e_val);
			
			free(expr_out);
			free(expr_buff);
			
			if(e_val > 0) {
				err_val[0] = 2; // String replacement error.
				free(expr_buff2);
				return NULL;
			}
			
			expr_out = expr_buff2;
			pos = strchr(expr_out, '!'); // Look for the next one.
		}
	}
	
	
	// Define a little string array for the operators
	int ls[2] = {2, 6};
	char opers1[2][3] = {"&&", "||"}; // Booleans
	char opers2[6][3] = {"!=", ">=", "<=", "=", ">", "<"}; // Comparators - Longest first, because some of these are subsets of the others.

	// Iterate through the list of of bools, then comparators.
	for(i=0; i<2; i++) {
		if(s_pos <= ++n_pos) {

			// Because C is a little bitch, I think the most efficient way to do this is to use strstr to find the first index of
			// each operator and put those indexes into a sorted list.
			int *op_ar = malloc(sizeof(int)); // The array indicating which operator it is.
			char **op_inds = malloc(sizeof(char*)); // The array of indexes.

			// The first index should be expr_out
			op_ar[0] = -1;
			op_inds[0] = expr_out;

			
			int count = 0; // Counter.
			char *op; // Used to store the operator.
			
			for(j = 0; j<ls[i]; j++) {
				switch(i) { // Get the operator.
					case 0:
						op = opers1[j];
						break;
					case 1:
						op = opers2[j];
						break;
				}
				
				int ol = strlen(op);
				
				pos = strstr(expr_out, op); // Initialize a pointer to the first position.
				while(pos != NULL) { // Loop through the string and find all instances.
					if(strcmp(op, "=") == 0) { // Only if this is its own thing.
						char cb  = (pos-1)[0]; // The character before.
						if(cb == '>' || cb == '<' || cb == '!') {
							pos = strstr(++pos, op); // See if there's another one.
							continue; // And move on.
						} 
					} else if ((strcmp(op, ">") == 0) || (strcmp(op, "<") == 0)) {
						char cb = (pos+1)[0]; // Only if it's by itself.
						if(cb == '=') {
							pos += 2; // Skip this operator.
							pos = strstr(pos, op);
							continue;
						}

					}
					
					count++; // We've found one, so increment the counter.
		
					pos+=ol;
				
					// Re-allocate the memory for the arrays.
					op_ar = realloc(op_ar, sizeof(int)*(count+2));
					op_inds = realloc(op_inds, sizeof(char*)*(count+2));
				
					// Insertion sort - the last piece is an empty spot. Move everything over until k is bigger than the next one.
					for(int k = count-1; k>=0; k--) {
						if(pos < op_inds[k]) { // If it's smaller, move it up one.
							op_inds[k+1] = op_inds[k];
							op_ar[k+1] = op_ar[k];
						}  else { // Everything that was already there is sorted, so if it's smaller, insert here.
							op_inds[k+1] = pos;
							op_ar[k+1] = j;
						}
					}
				
					pos = strstr(pos, op);
				}
			}
			
			if(count > 0) { // If we found anything, parse it.
				// Now add in the end character.
				op_ar[count+1] = -2; // End char.
				op_inds[count+1] = &expr_out[strlen(expr_out)+1]; // This is the position of the null char.
		
				// Now you've got a sorted array of indexes. Go through and parse.
				double *vals = malloc(sizeof(double)*(count+1)); // An output array.
		
				for(j = 0; j<=count; j++) {
					int sublen = op_inds[j+1]-op_inds[j]; // This is how many characters to read. We can read out the special
														  // characters, since trailing specials are eliminated by the parser.
					expr_buff = malloc(sublen+1); 
					strncpy(expr_buff, op_inds[j], sublen); // Copy the substring
					expr_buff[sublen] = '\0'; // Null terminate.
			
					vals[j] = parse_math(expr_buff, NULL, err_val, n_pos); // Parse the substring.
			
					free(expr_buff); // Free the old buffer.
			
					if(err_val[0] != 0) { // Error checking.
						free(vals);
						free(expr_out);
						free(op_ar);
						free(op_inds);
						return NULL;
					}
				}
			
				// Now we just need op_ar and the values.
				free(expr_out);
			
				// Initialize the boolean list
				int bool = 1;
			
				for(j=0; j<count; j++) {
					switch(i) { // Get the operator.
						case 0:
							op = opers1[op_ar[j+1]];
							break;
						case 1:
							op = opers2[op_ar[j+1]];
							break;
					}
					
					// Figure out what operation it is and then do it.
					int cb;
					if(strcmp(op, "=") == 0)       // strcmp has a weird return value. 0 is a match.
						cb = vals[j] == vals[j+1];
					else if(strcmp(op, ">") == 0)  
						cb = vals[j] > vals[j+1];
					else if(strcmp(op, "<") == 0)
						cb = vals[j] < vals[j+1];
					else if(strcmp(op, ">=") == 0)
						cb = vals[j] >= vals[j+1];
					else if(strcmp(op, "<=") == 0)
						cb = vals[j] <= vals[j+1];
					else if(strcmp(op, "!=") == 0)
						cb = vals[j] != vals[j+1];
					else if(strcmp(op, "&&") == 0) // Boolean AND
						cb = vals[j] && vals[j+1];
					else if(strcmp(op, "||") == 0) // Boolean OR
						cb = vals[j] || vals[j+1];
				
					bool = bool && cb;
				}
			
				free(vals);
				free(op_ar);
				free(op_inds);
				
				return (double)bool;
				
			} else {
			  free(op_ar);
			  free(op_inds);
			}
		} 
		
	}
	
	// Since this is a recursive function we want to do this in reverse order of operations, so that we keep calling this function with smaller and smaller chunks
	
	///////////////////////////////////////////////////////////
	///														///
	/// 	        Mathematical Operations					///
	///														///
	///////////////////////////////////////////////////////////
	
	
	// Order of operations: Powers, multiplication/division/modulo, addition/subtraction, bitwise
	for(j = 0; j<4; j++) {
		if(s_pos <= ++n_pos) {
			char *dels = malloc(4);
			switch(j) {
				case(0):
					dels = "&|"; // Bitwise AND and OR. OK to do just this since we are past the boolean evaluation stage.
					break;
				case(1):
					dels = "+-";
					break;
				case(2):
					dels = "*/%";
					break;
				case(3):
					dels = "^";
					break;
			}
			
			char *tok = strpbrk(expr_out, dels), delimiter;
			if(tok != NULL) {
				delimiter = *tok; // Keep track of signs.
		
				int sublen = tok-expr_out;
				expr_buff = malloc(sublen+1); // Allocate some space for a substring.
				strncpy(expr_buff, expr_out, sublen); // Copy everything before the token to a buffer variable.
				expr_buff[sublen] = '\0'; // Null terminate.
		
				double val = parse_math(expr_buff, c, err_val, n_pos);
	
				free(expr_buff); // Don't need this anymore.
	
				if(err_val[0] != 0) {
					free(expr_out);
					return NULL;
				}
	
				double v2 = parse_math(tok+1, c, err_val, n_pos); //Now parse everything after the token.
		
				free(expr_out); // Don't need this either, since tok isn't used anymore.
		
				if(err_val[0] != 0) {
					return NULL;
				}
			
				switch(delimiter) {
					case '+':
						val += v2;  //Addition.
						break;
					case '-':
						val -= v2; // Subtraction
						break;
					case '*':
						val *= v2; // Multiplication
						break;
					case '/':
						val /= v2; // Division
						break;
					case '%':
						val = (double)(((int)val)%((int)v2)); // Modulo is ints only.
						break;
					case '^':
						val = pow(val, v2); //val^v2
						break;
					case '&':
						val = (double)(((int)val)&((int)v2)); // Bitwise AND;
						break;
					case '|':
						val = (double)(((int)val)|((int)v2)); // Bitwise OR
					break;
				}
				
				return val;

			}
		}
	}
	
	// At this point it must just be a number that was passed to this
	// There's not a good way of error checking this.
	return atof(expr_out);
}

int find_paren_contents(char *string, char *outstring) {
	// Returns the string between the first parens that this function encounters.
	// Allocate enough space for outstring - suggestion is to allocate the length of string
	// On a successful match, returns the offset.
	// Returns -1 if there was no match 
	// Returns -2 if there was no first open parens.

	
	// Find the first one.
	char *pos1 = strchr(string, '(');
	if(pos1 == NULL || strlen(pos1) <= 1) {
		// Either there was no first one, or it's the last character in the string
		return -1;
	}
	
	int count = 1, offset = -1;
	
	for(int i = 1; i<strlen(pos1); i++) {
		if(pos1[i] == '(')
			count++; // Increment any time we find an open parens
		if(pos1[i] == ')' && --count == 0) {  // Decrement if we find a close parens -> this will break if if() evaluation is not lazy
			offset = i-1; // If count == 0, we've had the same number of opens as closeds, so we've found the spot.
			break;
		}
	}
	
	if(offset >= 1) {
		strncpy(outstring, pos1+1, offset); // Copy the substring
		outstring[offset] = '\0'; // Set the null flag
	}
	
	return offset;
}

char *eval_funcs_and_constants(char *expr, constants *c, int *err_val, int n_pos) {
	// Transform a function by turning all the constants and functions into numbers first.
	err_val[0] = 0; // No errors yet.
	
	if(strlen(expr) <= 0) {
		err_val[0] = 7; // Invalid input string.
		return NULL;
	}
	
	// Useful variables.
	char *expr_out = malloc(sizeof(char)*strlen(expr)+1), *expr_buff, *expr_buff2, *pos;
	char op_chars[7] = "*/%^+~"; // Reserved characters.
	int scl = strlen(op_chars); // How many are there?
	strcpy(expr_out, expr); // Copy the input to our output.
	int i, j, l = strlen(expr_out);

	// Replace the constants as appropriate.
	if(c != NULL && c->num >= 1) {
		char *name;
		int type;
		char *vals = malloc(300);
		for(i = 0; i<c->num; i++) {
			name = c->c_names[i];
			type = c->c_types[i];
			
			double val; 
			if(c->c_types[i] == C_INT) {
				val = (double)c->c_ints[c->c_locs[i]];
			} else if(c->c_types[i] == C_DOUBLE) {
				val = c->c_doubles[c->c_locs[i]];
			}
			
			int n_len = strlen(c->c_names[i]);
			int e_val;
			sprintf(vals, "%lf", val); // Make a string out of the value
			
			pos = strstr(expr_out, name); // Find instances of this constant.
			while(pos != NULL) {
				expr_buff = replace_chars(expr_out, vals, (pos-expr_out), (pos-expr_out)+n_len-1, &e_val); // Replace the appropriate section.
				free(expr_out); // Free the original memory.

				if(e_val > 0) {
					free(expr_buff);
					free(vals);
					err_val[0] = 2; // String replacement error.
					
					return NULL;
				}

				
				expr_out = expr_buff; // This needs to be a three-stage process so I can properly free the memory allocated.
				pos = strstr(expr_out, name); // Find instances of this constant.
			}
		}
		free(vals);
	}
	
	// First let's find all the functions (exp(), log()) and if they happen to be exp() or log(), replace them with 1.
	char functions[2][5] = {"exp(", "log("};

	for(i = 0; i<2; i++) {
		j = 0;
		while(j++ < l) {
			pos = strstr(expr_out, functions[i]);
			if(pos == NULL)
				break;
		
			char *substring = malloc(sizeof(char)*(strlen(pos)+1));
			int e_val = 0, offset = find_paren_contents(pos, substring); // How many chars between the parens.
			if(offset > 0) {
				// If there's stuff between the parens, we pass it recursively.
				double val = parse_math(substring, c, err_val, n_pos); // This should return a number.

				if(expr_buff == NULL) {
					err_val[0] = 3; // Recursion error
					free(expr_out);
					free(substring);
					return NULL;
				} else {
					//Evaluate the function
					switch(i) {
					case 0: // exp()
						val = exp(val);
					break;
					case 1: // log()
						if(val <= 0.0)
							err_val[0] = 4; // Undefined error.
						else
							val = log(val);
					break;
					}
					
					if(err_val[0] == 0) {
						free(expr_buff); // Free this guy.
						
						expr_buff = malloc(400); // Going to print our double to this.
						sprintf(expr_buff, "%lf", val);
						
						int paren_index = pos-expr_out;
						expr_buff2 = replace_chars(expr_out, expr_buff, paren_index, offset+paren_index+4, &e_val); // Replace the function in parens with whatever it evaluates to.

						if(e_val != 0) {
							err_val[0] = 2; // String replacement error
							free(expr_buff2); // Avoid memory leaks.
						}
					}
				}

				// We've moved the pointers, so to avoid memory leaks, free expr_buff.
				free(expr_buff);

			} 
			free(substring);
			if(offset < 0) {
				err_val[0] = 1; // Parenthesis mismatch.
			} else if(offset == 0) {
				// If there's nothing between the parens, just interpret the function as 1.
				expr_buff2 = replace_chars(expr_out, "1", (pos-expr_out), (pos-expr_out)+4, &e_val); // exp()/log() = 5 characters.
				if(e_val != 0)
				{
					err_val[0] = 2; // String replacement error.
					free(expr_buff2);
				}
			}


			free(expr_out); // Either there's an error or this pointer is going to be moved. Either way, free what it's pointing at now.

			if(err_val[0] != 0)
				return NULL;

			// If there wasn't an error, expr_buff2 is pointing at what expr_out should be pointing at..
			expr_out = expr_buff2;
		}
	}
	
	// If there are any alphanumeric characters left, there's an error, set the flag.
	pos = strpbrk(expr_out, "abcdefghijlkmnopqrstuvwxyz@#$_`?[]\\{}"); // None of these characters should be present.
	if(pos != NULL)
		err_val[0] = 4; // Undefined constant present.
	
	return expr_out;

}

//////////////////////////////////////////////////////////////////////
// 																	//
//						Constants Functions							//
// 																	//
//////////////////////////////////////////////////////////////////////

void add_constant(constants *c, char *name, int type, void *val) {
	// Add a constant to your constants structure.
	// Type 0 = Int
	// Type 1 = Double
	// Honor system about how to define constants for now - don't be a dick
	
	// Allocate more memory as appropriate.
	int n_el = ++c->num; // Increment c->num, grab the new number of elements
	int n_l = strlen(name);
	char *n = malloc(n_l+1);
	strcpy(n, name);
	c->c_names = realloc(c->c_names, n_el*sizeof(char*)); // Allocate one more for a pointer to a name.
	c->c_names[n_el-1] = malloc(n_l+1); // Allocate space for the name
	c->c_types = realloc(c->c_types, n_el*sizeof(int));  // Allocate space for one more type
	c->c_locs = realloc(c->c_locs, sizeof(int)*n_el--); // Allocate space for the locations, decrement n_el for later use
	
	// This stuff can happen before the insertion sort because it's indexed by c->c_locs;
	int loc;
	switch(type) {
		case C_INT:
			int i_val = *((int *)val); // Cast to int
			loc = c->num_ints++; // Where are we putting it in the array
			c->c_ints = realloc(c->c_ints, c->num_ints*sizeof(int)); // Allocate space for one more int
	
			c->c_ints[loc] = i_val; // Put the value where it belongs
		break;
		case C_DOUBLE:
			double d_val = *((double *)val); // Cast to int
			loc = c->num_doubles++; // Where are we putting it in the array.
			c->c_doubles = realloc(c->c_doubles, c->num_doubles*sizeof(double)); // Allocate space for one more double.
	
			c->c_doubles[loc] = d_val; // Put the value where it belongs
		break;
	}

	// Do an insertion sort by the string length of the names so that if one constant is the subset of another one,
	// it won't fuck anything up.
	
	int i;
	for(i=n_el; i>0; i--) {
		if(n_l > strlen(c->c_names[i-1])) {
			c->c_names[i] = c->c_names[i-1]; // If our new guys is longer, it should go further back.
			c->c_types[i] = c->c_types[i-1];
			c->c_locs[i] = c->c_locs[i-1];
		} else 
			break;
	}
	
	// Add these in at the end, just in case the for loop never executes. 
	c->c_names[i] = n;
	c->c_types[i] = type;
	c->c_locs[i] = loc;
	
	return;
}

void change_constant(constants *c, char *name, int type, void *val) {
	// Allows you to change the value of a constant of a given name.
	// If you feed it a name that's not in the list, it will be added to the list.
	
	int n_el = c->num, i;
	for(i = 0; i<n_el; i++) {
		if(strcmp(name, c->c_names[i]) == 0) 
			break; // We're just looking for the index here.
	}
	
	if(i == n_el) { 
		add_constant(c, name, type, val); // If we didn't find it, add it in.
		return;
	}
	
	int j, old_type = c->c_types[i]; // Keep the old type
	if(type != old_type) { // If you are changing the type, you need to change the sizes of the arrays.
		int old_loc = c->c_locs[i];
		for(int j = 0; j<n_el; j++) { // Have to look at all of them because the order could be anything
			if(c->c_types[j] == old_type && c->c_locs[j] > old_loc) {
				c->c_locs[j]--; // Everything's going to move up one.
			}
		}
		
		switch(old_type) {
			case C_INT:
				for(j=old_loc; j<c->num_ints-1; j++) 
					 c->c_ints[j] = c->c_ints[j+1]; // Move everything back one.
				
				c->c_ints = realloc(c->c_ints, --c->num_ints*sizeof(int)); // Decrement num_ints, reallocate memory.
				break;
			case C_DOUBLE:
				for(j = old_loc; j<c->num_doubles-1; j++) 
					c->c_doubles[j] = c->c_doubles[j+1]; // Move everything back one.
				
				c->c_doubles = realloc(c->c_doubles, --c->num_doubles*sizeof(double)); // Decrement num_doubles, reallocate memory
				break;
		}
		
		c->c_types[i] = type;
	}
	
	switch(type) {
		case C_INT:
			if(type != old_type) {
				c->c_locs[i] = c->num_ints; // It's the last element.
				c->c_ints = realloc(c->c_ints, ++c->num_ints*sizeof(int)); // Add room for one more int.
			}
				
			int i_val = *((int *)val); // Cast to an int.
			c->c_ints[c->c_locs[i]] = i_val; 
			break;
		case C_DOUBLE:
			if(type != old_type) {
				c->c_locs[i] = c->num_doubles; // It's the last element.
				c->c_ints = realloc(c->c_ints, ++c->num_doubles*sizeof(int)); // Add room for one more int.
			}
			
			double d_val = *((double *)val); // Cast to a double
			c->c_doubles[c->c_locs[i]] = d_val;
			break;
	}
	
	return;		
}

constants *malloc_constants() {
	// Allocate the constant itself
	constants *c = malloc(sizeof(constants));
	
	// Initialize the pointers with one slot each.
	c->c_names = malloc(sizeof(char*));
	c->c_types = malloc(sizeof(int));
	c->c_locs = malloc(sizeof(int));
	
	c->c_ints = malloc(sizeof(int));
	c->c_doubles = malloc(sizeof(double));
	
	// Also initialize the constants.
	c->num = 0;
	c->num_doubles = 0;
	c->num_ints = 0;
	
	return c;
}

void free_constants(constants *c) {
	// Free up a constant.
	if(c == NULL)
		return;
	
	free(c->c_locs);
	free(c->c_types);
	free(c->c_ints);
	free(c->c_doubles);

	for(int i = 0; i<c->num; i++) {
		free(c->c_names[i]);
	}
	
	free(c);
}


//////////////////////////////////////////////////////////////////////
// 																	//
//						String Functions							//
// 																	//
//////////////////////////////////////////////////////////////////////
char *replace_chars(char *main_str, char *reps, int s, int e, int *err_val) {
	// Replace the characters in main_str from main_str[s] to main_str[e] (inclusive)
	// with the characters from reps. This generates a new string, so you should free
	// main_str and reps if they are dynamically allocated.
	
	int l1 = strlen(main_str), l2 = strlen(reps), nc = e-s; // Length info
	err_val[0] = 0; // No errors yet.

	char *out_str = malloc(sizeof(char)*(l1-nc+l2+1)); // Allocates enough space for both strings and a null char.
	
	if(nc < 0 || e > l1 || s < 0) {
		err_val[0] = 1; // Malformed query.
		return out_str;
	}
	
	// Allocate memory for the return string.
	strncpy(out_str, main_str, s); // Copy all the characters up to (but not including) s.
	strncpy(&out_str[s], reps, l2); // Copy in reps.
	strcpy(&out_str[s+l2], &main_str[e+1]); // Copy everything from e to the end, including the null character.
	
	if(out_str[l1-nc+l2-1] != '\0') {
		err_val[0] = 2;
		out_str[l1-nc+l2-1] = '\0';
	}
	
	return out_str;
	
}

char *del_char(char *input, int index) {
	// Delete a single character from *input at index.
	
	if(input == NULL)
		return NULL;
	
	int i;
	for(i=index; i<strlen(input); i++) {
		input[i] = input[i+1]; // Move the next character over.
	}
	input[i] = '\0'; // Null terminate.
	
	return input;
}

char *strrep(char *str, char c1, char c2) {
	// Replace all instances of c1 with c2 in str;
	
	if(c1 == c2) {
		// Thought you could trick me into generating an infinite loop, did you? Well I put this check in here you wily fucker.
		return str;
	}
	char *tok = strchr(str, c1);
	while(tok != NULL) {
		tok[0] = c2; // Replace it.
		tok = strchr(tok, c1); // Find the first instance.
	}
	
	return str;
}


//////////////////////////////////////////////////////////////////////
// 																	//
//						General Functions							//
// 																	//
//////////////////////////////////////////////////////////////////////

int get_parse_error(int err_code, char *err_message) {
	// Give this an error code and it returns the error message.
	// Return value is the size (including null) of the message. 
	// Passing NULL to err_message still returns the size.
	// Pass -1 to err_code to get the highest error code.
	
		/* Error Values:
	0 => No error;
	1 => Parenthesis mismatch
	2 => String replacement error
	3 => Recursion error.
	4 => Function output is undefined.
	5 => No return value.
	7 => Malformed input string.
	*/
	
	if(err_code < 0)
		return 7;	// Maximum error code.
	
	char *err_buff;
	switch(err_code) {
		case 1:
			err_buff = "Parenthesis mismatch.";
			break;
		case 2:
			err_buff = "String replacement error";
			break;
		case 3:
			err_buff = "Recursion error.";
			break;
		case 4:
			err_buff = "Function output is undefined.";
			break;
		case 5:
			err_buff = "No return value";
			break;
		case 7:
			err_buff = "Malformed input string.";
			break;
		default:
			err_buff = "No error";
	}
	
	if(err_message != NULL)
		strcpy(err_message, err_buff);
	
	return strlen(err_buff)+1;
}

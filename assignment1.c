/* @author Rufina Talalaeva 
 * I've used the idea of Dijkstra from 
 * https://www.thecrazyprogrammer.com/2014/03/dijkstra-algorithm-for-finding-shortest-path-of-a-graph.html
 */
#include <stdio.h>
#include <stdlib.h>

#define LOW 5             /* lower limit of the number of cities */  
#define UP 50             /* upper limit of the number of cities */
#define MIN 1             /* minimum distance between cities     */
#define MAX 20            /* maximum distance between cities     */
#define UNACHIEVABLE '*'  /* no road from one city to another    */
#define INFINITY 10000    /* alternative for UNACHIEVABLE in Int */

/*   errors code    */

#define errCity 1
#define errInitCity 2
#define errDestCity 3
#define errMatrixSize 4
#define errDist 5
#define errConnection 6
#define errInput 7

/* global variables */

FILE *fin, *fout;           /* input and output streams */
short N;                    /* number of cities         */
short init;                 /* initial city             */
short dest;                 /* destination city         */
short path;                 /* path number in output    */
short numberOfPaths;        /* paths from init to dest  */

/*    functions     */

void error(short err);
short isDigit(char c);
short nextInt(short left, short right, short err, char until);
short nextDist(char until);
void readFirstLine();
void readMatrix(short**);
void findPath(short city, short stack[], short size, short distFromInit[], short **matrix);
short dijkstra(short distFromInit[], short **dists);

int main(){
	fout = fopen("RufinaTalalaevaOutput.txt", "w");       /* Creates the output file and opens it.                     */

	if((fin = fopen("input.txt", "r")) == NULL ){         /* Opens the file of input if it exists.                     */
		error(errInput);                                  /* If does not, print about invalid input to the output.     */
	}

	readFirstLine();

	if(fgetc(fin) != '\n'){                               /* According to the task second line is empty. Reads it.     */
		error(errInput);                                  /* If it is not, calls @function error with @const errInput. */
	}

	short **dists = (short**) calloc(N, sizeof(short*));  /* Creates the matrix of distances between cities.           */
	for (short i = 0; i < N; ++i){
		*(dists + i) = (short*) calloc(N, sizeof(short));
	}

	readMatrix(dists);                                    /* Fills the matrix with values of distances.                */

	short distFromInit[N];                                /* Array of the shortest distances from init city to others. */
		

	short shortestDist = dijkstra(distFromInit, dists);

	if(shortestDist == INFINITY){
			error(errConnection);
	}else{
		fprintf(fout, "The shortest path is %d.\n", shortestDist);
		fprintf(fout, "The number of shortest paths is %d:\n", numberOfPaths);
	}

	// creating the stack for backtracking the shortest paths.
	short *stack = (short*) calloc(N, sizeof(short));
	// first element of stack is destination city.
	*stack = dest;
	// search for the shortest paths from dest to init.
	findPath(dest, stack, 1, distFromInit, dists);
	
	// freeing the memory we have allocated during the program.
	for (int i = 0; i < N; ++i){
		free(*(dists + i));
	}
	free(dists);

	//Closing input file and output file.
	fclose(fin);
	fclose(fout);
	return 0;
}

/**
 * Reads the matrix size @const N by @const N from fin 
 *
 * @param **matrix 2D array to store distances.
 * @exception error(errInput) on Input error
 * @exception error(errDist) on Distance out of bounds
 */
void readMatrix(short **matrix){
	for(short i = 0; i < N; ++i){
		for(short j = 0; j < N; ++j){
			if(i == j){                                                /* When i equals j, in the table of distances it should be 0 */
				if(fgetc(fin) == '0'){                                 /* Every city is assumed to be 0 distance from itself.       */
					int c = fgetc(fin);
					if((c == ' ') || ((c == EOF) && (i == (N - 1)))){  
						*(*(matrix + i) + j) = 0;                             
					}else{                                             /* If next symbol after 0 isn't space or end of file         */
						error(errInput);                               /* It is considered as input error.                          */
					}
				}else{
					error(errDist);
				}
			}else{                                                     /* Try to read next distance from j to i city                */
				if(j == N - 1){                                        /* When j last element in a row, we read dist until '\n'     */
					*(*(matrix + i) + j) = nextDist('\n');
				}else{                                                 /* Otherwise we read until space                             */
					*(*(matrix + i) + j) = nextDist(' ');
				}	
			}
		}
	}
}

/**
 * Function responsible for throwing exception.

 * @param err The code of error that appeared.
 * @return Prints the name of exception to output stream and finishes the programme.
 */
void error(short err){
	switch(err){
		case 1: fprintf(fout, "Number of cities is out of range"); exit(0);
		case 2: fprintf(fout, "Chosen initial city does not exist"); exit(0);
		case 3: fprintf(fout, "Chosen destination city does not exist"); exit(0);
		case 4: fprintf(fout, "Matrix size does not suit to the number of cities"); exit(0);
		case 5: fprintf(fout, "The distance between some cities is out of range"); exit(0);
		case 6: fprintf(fout, "Initial and destination cities are not connected"); exit(0);
		case 7: fprintf(fout, "Structure of the input is invalid"); exit(0);
		default: fprintf(fout, "Structure of the input is invalid"); exit(0);
	}
}

/**
 * Function that determines whether the character digit or not.
 * According to ASCII table.
 *
 * @param c Character to check.
 * @return int Returns 1 if c is digit, and 0 if is not.
 */
short isDigit(char c){
	if((0 <= (c - '0')) && (c - '0') <= 9) return 1;
	else return 0;
}

/**
 * Function that reads a predictable(bounds: [left, right]) unsigned integer 
 * character by character until mets the character equal to @param until.
 * If read integer is out of bounds, then 
 * the @function error is called with @param err. 
 * If something except digits is met, then 
 * the @function error is called with @const errInput.
 * 
 * @param left This is left boundary for Integer.
 * @param right This is right boundary for Integer.
 * @param err This is the code of error which will be used if Integer is out of bounds.
 * @param until Until this character Integer will be tried to read.
 * @return short int Integer that was read.
 * @exception error(errInput) on input error.
 * @exception error(err) on the Integer out of bounds.
 */
short nextInt(short left, short right, short err, char until){
	char first, second, last;                               /* at most can be used three symbols for reading the integer with separator */
	                                                        /* because maximum correct int that can be in the input is 50 + separator   */
	short var;                                              /* variable for storing our result - read Integer                           */
	first = fgetc(fin);                                     /* reading first symbol                                                     */
	second = fgetc(fin);                                    /* reading second symbol                                                    */

	/* We have two possibilities:
	 * 1. First and second symbols are representing number and third must be separator.
	 * 2. Only first symbol represents the number and second must be separator in this case.
	 */
	if(isDigit(second)){
		if(first == '0'){                                   /* First symbol is 0 and second is digit is invalid input                   */
			error(errInput);
		}else if(first == '-'){                             /* First symbol is - and second is digit is error of bounderies,            */
			error(err);                                     /* Because according to the task we cannot have negative numbers            */
		}else if(!isDigit(first)){                          /* First symbol is not digit and second is digit - invalid input            */
			error(errInput);
		}else{                                              /* Only one option is left - first & second symbols are digits              */
			last = fgetc(fin);                              /* Reading one more symbol, it must be our separator @param until           */
			if(last == until){                              /* If last symbol is separator                                              */
				var = (first - '0') * 10 + (second - '0');  /* Finding out what integer is composed by first 2 symbols                  */
				if((var < left) || (right < var)){          /* Checking bounderies for our integer, if it is out of them - err          */
					error(err);
				}
			}else if(isDigit(last)){                        /* If last symbol is not separator, but digit, it implies err(on bounderies)*/
				error(err);                                 /* Because we can maximum have integer 50, which is out of 2 digits, not 3  */
			}else{                                          /* If last digit is not separator and is not digit, it implies error input  */
				error(errInput);
			}
		}
	}else if(second == until){                              /* If second symbol is separator, we do not need last symbol to be read     */
		if(!isDigit(first)){                                /* In this case our first symbol must be digit, if it is not, invalid input */
			error(errInput);
		}else{                                              /* If it is digit, we find out what integer it is and check bounds          */
			var = (first - '0');
			if((var < left) || (right < var)){              /* If it is out of bounds, error(err) is called                             */
				error(err);
			}
		}
	}else{                                                  /* If our second symbol is not separator and is not digit,                  */
		error(errInput);                                    /* Then it implies invalid input.                                           */   
	}
	return var;                                             /*If our function gets this moment, it implies that integer is correct,return*/
}

/**
 * The same function as @function nextInt, but:
 * 1. Accepts in input symbol '*';
 * 2. Has boundaries already: [MIN, MAX].
 * 3. Has error code: errDist.
 * 4. Has additional exception for incorrect matrix size.
 * 
 * @param until Until this character distanceValue will be tried to read.
 * @return short int distanceValue that was read.
 * @exception error(errInput) on input error.
 * @exception error(errMatrixSize) on incorrect matrix size.
 * @exception error(errDist) on the distance out of bounds.
 */
short nextDist(char until){
	char first, second, last;                               /* at most can be used three symbols for reading the integer with separator */
	                                                        /* because maximum correct int that can be in the input is 50 + separator   */
	short var;                                              /* variable for storing our result - read Integer                           */
	first = fgetc(fin);                                     /* reading first symbol                                                     */
	second = fgetc(fin);                                    /* reading second symbol                                                    */

	/* We have two possibilities:
	 * 1. First and second symbols are representing number and third must be separator.
	 * 2. Only first symbol represents the number(* or digit) and second must be separator in this case.
	 */
	if(isDigit(second)){
		if(first == '0'){                                   /* First symbol is 0 and second is digit is invalid input                   */
			error(errInput);
		}else if(first == '-'){                             /* First symbol is - and second is digit is error of bounderies of Dist     */
			error(errDist);                                 /* Because according to the task we cannot have negative numbers            */
		}else if(!isDigit(first)){                          /* First symbol is not digit and second is digit - invalid input            */
			error(errInput);
		}else{                                              /* Only one option is left - first & second symbols are digits              */
			last = fgetc(fin);                              /* Reading one more symbol, it must be our separator @param until           */
			if(last == until){                              /* If last symbol is separator                                              */
				var = (first - '0') * 10 + (second - '0');  /* Finding out what integer is composed by first 2 symbols                  */
				if((var < MIN) || (MAX < var)){             /* Checking bounderies for our integer, if it is out of them - errDist      */
					error(errDist);
				}
			}else if(isDigit(last)){                        /* If last symbol is not separator, but digit, it implies errDist           */
				error(errDist);                             /* Because we can maximum have integer 21, which is out of 2 digits, not 3  */
			}else{                                          /* If last digit is not separator and is not digit, it implies error input  */
				error(errInput);
			}
		}
	}else if(second == until){                              /* If second symbol is separator, we do not need last symbol to be read     */
		if(first == UNACHIEVABLE){                          /* If first symbol equals to unachievable, then var is set to infinity      */
			var = INFINITY;
		}
		else if(!isDigit(first)){                           /* In this case our first symbol must be digit, if it is not, invalid input */
			error(errInput);
		}else{                                              /* If it is digit, we find out what integer it is and check bounds          */
			var = (first - '0');
			if((var < MIN) || (MAX < var)){                 /* If it is out of bounds, error(errDist) is called                         */
				error(errDist);
			}
		}
	}else if((until == ' ' && second == '\n') || (until == '\n' && second == ' ')){ 
		error(errMatrixSize);                               /*If we met another separator, it implies that the matrix size is not correct*/
	}else{                                                  /* If our second symbol is not separator and is not digit,                  */
		error(errInput);                                    /* Then it implies invalid input.                                           */   
	}
	return var;                                             /*If our function gets this moment, it implies that integer is correct,return*/
}

/**
 * Function that reads first line of input stream.
 * Predicts that there are 3 Integers: Number Of Cities, Initial city, Destination city.  
 */
void readFirstLine(){
	//Reading the 1st entry(until space), which must be the number of cities.
	//The number of cities must be in [LOW, UP].(According to the task)
	N = nextInt(LOW, UP, errCity, ' ');
	
	//Reading the 2nd entry(until space), which must be the initial city.
	//The initial city must be in [0, N-1].(According to the task)
	init = nextInt(0, N - 1, 2, ' ');
	
	//Reading the 3nd entry(until \\n'), which must be the destination city.
	//The destination city must be in [0, N-1].(According to the task)
	dest = nextInt(0, N - 1, 3, '\n');
}

/**
 * Finds the shortest distance between @global init and @global dest.
 *
 * @param distFromInit[] array of the shortest distances from init city to others.
 * @param **matrix array of distances between cities. 
 * @return shortest distance between @global init and @global dest.
 */
short dijkstra(short distFromInit[], short **dists){
	short used[N];                                        /* Array for marking whether we visited the city or not.     */
	short paths[N];                                       /* Array of paths from init city to others.                  */

	for(short i = 0; i < N; ++i){                         /* Initializing all these arrays.                            */
		used[i] = 0;                                      /* We have not visited any city yet.                         */
		distFromInit[i] = dists[i][init];                 /* The shortest path from init to other city is from matrix. */
		paths[i] = 0;                                     /* No any path yet.                                          */
	}

	paths[init] = 1;                                      /* There is only one path to init.                           */

	for(short k = 0; k < N; ++k){
		short minimum = INFINITY;
		short node;
		// searching for node that we have not visited yet, and which is nearest to init.
		for (short i = 0; i < N; ++i){
			if((distFromInit[i] < minimum) && (!used[i])){
				minimum = distFromInit[i];
				node = i;
			}
		}
		
		// marking this node as visited.
		used[node] = 1;

		for (short i = 0; i < N; ++i){
			if(!used[i]){
				// if distance to init for remaining cities through the node is shorter, then refresh the shortest distance of i.
				// and refresh the number of paths for i, because now all shortest paths to i are through node, so 
				// number of paths to i equals to number of paths to node.
				if(distFromInit[node] + dists[i][node] < distFromInit[i]){
					distFromInit[i] = distFromInit[node] + dists[i][node];
					paths[i] = paths[node];
				}
				// if we found distance through node that is same as shortest dist to i
				// we just add number of paths of node to the number of paths of i.
				else if(distFromInit[node] + dists[i][node] == distFromInit[i]){
						paths[i] += paths[node];
				}
			}
		}	
	}

	// number of different shortest paths from init to dest assign to @global numberOfPaths.
	numberOfPaths = paths[dest];

	// return the shortest distance from init to dest.
	return distFromInit[dest];
}

/**
 * This function prints all shortest paths from init to city.
 *
 * @param city From this city we should find the shortest path to init.
 * @param stack[] array that plays role of stack of cities we are visiting when backtracking our shortest path.
 * @param size current size of the stack.
 * @param distFromInit[] array of the shortest distances from init city to others.
 * @param **matrix array of distances between cities. 
 */
void findPath(short city, short stack[], short size, short distFromInit[], short **matrix){
	// Base of recursion, if we met the init city, print the path(all cities from stack)
	if(city == init){
		// new path was found, so increase our number of paths to be printed.
		path++;
		// print the number of path to be printed.
		fprintf(fout, "%d. ", path);
		// print all elements of stack except first one
		for (short i = size - 1; i > 0; --i)
		{
			fprintf(fout, "%d -> ",stack[i]);	
		}
		// if it is last path to be printed, there is no need to print \n after
		if(path == numberOfPaths){
			fprintf(fout,"%d", stack[0]);
		}else{
			fprintf(fout,"%d\n", stack[0]);
		}
		return;
	}

	// if we are not in the init city yet, we are looking for city from which we could come.
	for (short i = 0; i < N; ++i){
		if(i != city){
			// If we came from city i, the shortest dist to i + distance between city and i-th city
			// should be equal to the shortest dist to city
			if(distFromInit[city] == distFromInit[i] + matrix[city][i]){
				// if so, we adding our i-th city to the stack and incrementing the size of stack.
				stack[size] = i;
				size++;
				// calling function of searching the shortest path from i-th city to init
				findPath(i, stack, size, distFromInit, matrix);
				// after finding the path, decrementing our size of stack with aim to not print previous cities again
				size--;
			}
		}
	}
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*       constants       */

#define max_input_name_size 100
#define max_output_name_size 100
#define maxlen 256
#define name_maxlen 100
#define surname_maxlen 100
#define course_maxlen 100
#define TA_maxnum 100
#define Professor_maxnum 100
#define Student_maxnum 100
#define course_maxnum 30
#define studentID_max 5
#define labs_maxnum 100
#define files 51

/*       structures      */

struct Course;
typedef struct Course* course;


typedef struct {
    char* name;
    char* surname;
    int courses_num;
    course courses;
    int labs_assigned;
}Teacher_Assistant;

typedef struct {
    char* name;
    char* surname;
    int courses_num;
    course courses;
    int courses_assigned;
}Professor;

typedef struct {
    char* name;
    char* surname;
    char* ID;
    int courses_num;
    course courses;
    course not_assigned_courses;
    int assigned;
}Student;

typedef struct Course{
    char* name;
    int students_number;
    int labs_number;
    short profs_num;
    Professor* professor;
    Professor** professors;
    int tas_num;
    int tas_assigned;
    Teacher_Assistant** assigned_tas;
    Teacher_Assistant** teacher_assistants;
    int stud_num;
    Student** students;
    int prof_assigned;
}Course;

typedef struct{
    unsigned short err : 1;
}flags;

/*       functions       */

int valid_ID(char []);
int valid_name(char []);
int valid_number(char []);
int str_to_int(char []);
void error();
void print_email();

void print_invalid_files(char **p_to_print, FILE * fout){
    for (int i = 1; i < files; ++i)
    {
        if((*p_to_print)[i] == 1){

            (*p_to_print)[i] = 0;

            char output_name[max_output_name_size];
            sprintf(output_name, "RufinaTalalaevaOutput%d.txt", i);

            // Creating the output file.
            fout = fopen(output_name, "w");
            fprintf(fout, "Invalid input.");
        }
    }
}

char* next_word_until(char **line, char delim){
	char word[name_maxlen];
	strcpy(word, "");
	while((**line != '\0') && (**line != delim)){
		int len = strlen(word);
		word[len] = *(*line);
		(*line)++;
		word[len+1] = '\0';
	}
	char* result = (char*) calloc(strlen(word)+1, sizeof(char));
	strcpy(result, word);
	return result;
}

short check(int (*valid)(char[]), char* s, char* line, flags* flag, FILE* fout){
	if((valid(s) == 0) || (line == '\0')){
		error(fout);
		(*flag).err = 1;
		return 1;
	}
	return 0;
}

Course* find_in_courses(char* name, Course** courses, size_t size){
	for (int i = 0; i < size; ++i)
	{
		if(strcmp((*courses)[i].name, name) == 0){
			return &(*courses)[i];
		}
	}
	return NULL;
}

Course* read_courses(char **line,flags* flag, size_t* size, FILE *fin, FILE *fout){
	Course* courses = (Course*) calloc(course_maxnum, sizeof(Course));
	int courses_cnt = 0;
	while(fgets(*line, maxlen, fin) != NULL){
		/* If we reached the line "P\n", it means that next lines will include 
		 * info about Professors, but not about Courses, so we exit from the loop.
		 */
		if (strcmp(*line, "P\n") == 0) break;

		courses[courses_cnt].professors = (Professor**) calloc(Professor_maxnum, sizeof(Professor*));
		courses[courses_cnt].profs_num = 0;

		courses[courses_cnt].teacher_assistants = (Teacher_Assistant**) calloc(TA_maxnum, sizeof(Teacher_Assistant*));
		courses[courses_cnt].tas_num = 0;

		courses[courses_cnt].students = (Student**) calloc(Student_maxnum, sizeof(Student*));
		courses[courses_cnt].stud_num = 0;

		courses[courses_cnt].name = next_word_until(line, ' ');
		

		if(check(valid_name, courses[courses_cnt].name, *line, flag, fout)) break;

		(*line)++;

		char* number = next_word_until(line, ' ');
		
		if(check(valid_number, number, *line, flag, fout)) break;

		courses[courses_cnt].labs_number = str_to_int(number);

		free(number);

		(*line)++;

		number = next_word_until(line, '\n');

		if(check(valid_number, number, *line, flag, fout)) break;

		courses[courses_cnt].students_number = str_to_int(number);

		free(number);
		courses_cnt++;
	}
	if (feof(fin)){
			error(fout);
			flag->err = 1;
	}

	*size = courses_cnt;
	return courses;
}

Professor* read_professors(char **line,flags* flag, size_t* size, Course** courses, size_t size_courses, FILE *fin, FILE *fout){
	Professor* professors = (Professor*) calloc(Professor_maxnum, sizeof(Professor));

	int professors_cnt = 0;
	int prof_courses_cnt;

	while(fgets(*line, maxlen, fin) != NULL){
		size_t line_len = strlen(*line);
		/* If we reached the line "P\n", it means that next lines will include 
		 * info about Professors, but not about Courses, so we exit from the loop.
		 */
		if (strcmp(*line, "T\n") == 0) break;

		professors[professors_cnt].name = next_word_until(line, ' ');
		
		if(check(valid_name, professors[professors_cnt].name, *line, flag, fout)) break;

		(*line)++;

		professors[professors_cnt].surname = next_word_until(line, ' ');
		
		if(check(valid_name, professors[professors_cnt].surname, *line, flag, fout)) break;

		(*line)++;

		professors[professors_cnt].courses = (course) calloc(course_maxnum, sizeof(Course));
		int prof_courses_cnt = 0;


		char* course_name = (char*) calloc(maxlen, sizeof(char));
		Course* cur_course;
		strcpy(course_name, "");

		while((**line) != '\n'){
			if(**line == ' '){
				if(check(valid_name, course_name, *line, flag, fout)) break;

				if((cur_course = find_in_courses(course_name, courses, size_courses)) != NULL){
					professors[professors_cnt].courses[prof_courses_cnt] = *cur_course;
					cur_course->professors[cur_course->profs_num] = &professors[professors_cnt]; 
					(cur_course->profs_num)++;
					prof_courses_cnt++;
				}else{
					error(fout);
					flag->err = 1;
					break;
				}

				
				strcpy(course_name, "");
			}else{
				int len = strlen(course_name);
				course_name[len] = *(*line);
				course_name[len+1] = '\0';
			}
			(*line)++;
		}

		if(flag->err == 1){
			break;
		}

		if(check(valid_name, course_name, *line, flag, fout)) break;

		if((cur_course = find_in_courses(course_name, courses, size_courses)) != NULL){
		 	professors[professors_cnt].courses[prof_courses_cnt] = *cur_course;	
		 	cur_course->professors[cur_course->profs_num] = (&professors)[professors_cnt];
			(cur_course->profs_num)++;
			prof_courses_cnt++;				
		}else{
			flag->err = 1;
		 	error(fout);
		 	flag->err = 1;
		}
		professors[professors_cnt].courses_num = prof_courses_cnt;
		professors_cnt++;
	}

	*size = professors_cnt;
	return professors;
}

Teacher_Assistant* read_tas(char **line,flags* flag, size_t* size, Course** courses, size_t size_courses, FILE *fin, FILE *fout){
	Teacher_Assistant* tas = (Teacher_Assistant*) calloc(TA_maxnum, sizeof(Teacher_Assistant));
	int tas_cnt = 0;

	while(fgets(*line, maxlen, fin) != NULL){
		size_t line_len = strlen(*line);
		/* If we reached the line "P\n", it means that next lines will include 
		 * info about Professors, but not about Courses, so we exit from the loop.
		 */
		if (strcmp(*line, "S\n") == 0 || strcmp(*line, "S\0") == 0) break;
		tas[tas_cnt].name = next_word_until(line, ' ');
		
		if(check(valid_name, tas[tas_cnt].name, *line, flag, fout)) break;

		(*line)++;

		tas[tas_cnt].surname = next_word_until(line, ' ');
		
		if(check(valid_name, tas[tas_cnt].surname, *line, flag, fout)) break;

		(*line)++;

		tas[tas_cnt].courses = (course) calloc(course_maxnum, sizeof(Course));
		int tas_courses_cnt = 0;

		char* course_name = (char*) calloc(name_maxlen, sizeof(char));
		Course* cur_course;
		strcpy(course_name, "");

		while((**line) != '\n'){
			if(**line == ' '){
				if(check(valid_name, course_name, *line, flag, fout)) break;

				if((cur_course = find_in_courses(course_name, courses, size_courses)) != NULL){
					tas[tas_cnt].courses[tas_courses_cnt] = *cur_course;
					cur_course->teacher_assistants[cur_course->tas_num] = &tas[tas_cnt]; 
					cur_course->tas_num ++;	

				}else{
					error(fout);
					flag->err = 1;
					break;
				}

				tas_courses_cnt++;
				strcpy(course_name, "");
				(*line)++;
			}else{
				int len = strlen(course_name);
				course_name[len] = *(*line);
				(*line)++;
				course_name[len+1] = '\0';
			}
		}

		if(flag->err == 1){
			break;
		}

		if(check(valid_name, course_name, *line, flag, fout)) break;

		if((cur_course = find_in_courses(course_name, courses, size_courses)) != NULL){
		 	tas[tas_cnt].courses[tas_courses_cnt] = *cur_course;
		 	cur_course->teacher_assistants[cur_course->tas_num] = &tas[tas_cnt]; 
			cur_course->tas_num ++;	
			tas_courses_cnt++;				
		}else{
			flag->err = 1;
		 	error(fout);
		 	break;
		}
		tas[tas_cnt].courses_num = tas_courses_cnt;
		tas_cnt++;
	}


	*size = tas_cnt;
	return tas;
}

Student* read_students(char **line,flags* flag, size_t* size, Course** courses, size_t size_courses, FILE *fin, FILE *fout){
	Student* students = (Student*) calloc(Student_maxnum, sizeof(Student));
	int students_cnt = 0;

	
	fgets(*line, maxlen, fin);
	do{
		students[students_cnt].name = next_word_until(line, ' ');
		if(check(valid_name, students[students_cnt].name, *line, flag, fout)) break;
		(*line)++;
		students[students_cnt].surname = next_word_until(line, ' ');
		if(check(valid_name, students[students_cnt].surname, *line, flag, fout)) break;
		(*line)++;
		students[students_cnt].ID = next_word_until(line, ' ');
		if(check(valid_ID, students[students_cnt].ID, *line, flag, fout)) break;
		(*line)++;

		students[students_cnt].courses = (course) calloc(course_maxnum, sizeof(Course));
		int students_courses_cnt = 0;

		char* course_name = (char*) calloc(maxlen, sizeof(char));
		Course* cur_course;
		strcpy(course_name, "");


		while((**line) != '\n' && (**line != '\0')){
			if(**line == ' '){
				if(check(valid_name, course_name, *line, flag, fout)) break;
				if(strcmp(course_name,"") == 0 || size_courses == 0);
				else if((cur_course = find_in_courses(course_name, courses, size_courses)) != NULL){
					students[students_cnt].courses[students_courses_cnt] = *cur_course;					
				}else{
					error(fout);
					flag->err = 1;
					break;
				}

				students_courses_cnt++;
				strcpy(course_name, "");
				(*line)++;
			}else{
				int len = strlen(course_name);
				course_name[len] = *(*line);
				(*line)++;
				course_name[len+1] = '\0';
			}
		}


		if(flag->err == 1){
			break;
		}

		if(check(valid_name, course_name, *line, flag, fout)) break;
		if(strcmp(course_name,"") == 0 || size_courses == 0);
		else if((cur_course = find_in_courses(course_name, courses, size_courses)) != NULL){
		 	students[students_cnt].courses[students_courses_cnt] = *cur_course;	
			students_courses_cnt++;				
		}else{
			flag->err = 1;
		 	error(fout);
		 	break;
		}
		students[students_cnt].courses_num = students_courses_cnt;
		students_cnt++;

	}while(fgets(*line, maxlen, fin) != NULL);

	*size = students_cnt;
	return students;
}

int professor_knows(char* s, Professor* prof){
	for (int i = 0; i < prof->courses_num; ++i)
	{
		if(strcmp(prof->courses[i].name, s) == 0){
			return 1;
		}
	}
	return 0;
}

int students_wants(char* s, Student *stud){
	for (int i = 0; i < stud->courses_num; ++i)
	{
		if(strcmp(stud->courses[i].name, s) == 0){
			return 1;
		}
	}
	return 0;
}

void input(){
    // Variables for input and output streams.
    FILE* fin, *fout;

    char *to_print = (char*) calloc(files, sizeof(char));
    for (int i = 1; i < files; ++i)
    {
        to_print[i] = 0;
    }

    // Counter for files.
    int n = 1;

    flags flag;

    /* There could be at most 50 files, so we have to check each file.
     * If there is no such file, we stop searching for files.
     */
    while(n < files){
        // Generating the name of the input file according to its order.
        char input_name[max_input_name_size];
        sprintf(input_name, "input%d.txt", n);

        /* Trying to open the file with generated input name.
         * If there is no such file, we go out from the loop.
         * Otherwise, we start to read from that file.
         */
        if((fin = fopen(input_name, "r")) == NULL){
            to_print[n] = 1;
        }else{
            print_invalid_files(&to_print, fout);

            // Generating the name of the output file according to its order.
            char output_name[max_output_name_size];
            sprintf(output_name, "RufinaTalalaevaOutput%d.txt", n);

            // Creating the output file.
            fout = fopen(output_name, "w");

            /* Creating an array of Courses, Professors, TAs & Students,
             * where we will store the info from the input.
             */
            Course *courses;
            Professor *professors;
            Teacher_Assistant *teacher_assistants;
            Student *students;

            /* This line is for storing current line from the input file.
             * Because we will read the input line by line.
             */
            char* line = (char*) calloc(maxlen, sizeof(char));

            // Creating the variable for storing the number of courses.
            size_t size_courses;

            // Read Courses.
            courses = read_courses(&line, &flag, &size_courses, fin, fout);

            if(flag.err){
            	n++;
            	flag.err = 0;
            	continue;
            }


            // Counter for Courses and size for Courses.
            int i = 0;
            size_t size_professors;

            professors = read_professors(&line, &flag, &size_professors, &courses, size_courses, fin, fout);
            if(flag.err){
            	n++;
            	flag.err = 0;
            	continue;
            }

            size_t size_tas;
            teacher_assistants = read_tas(&line, &flag, &size_tas, &courses, size_courses, fin, fout);
            if(flag.err){
            	n++;
            	flag.err = 0;
            	continue;
            }


            size_t size_students;
            students = read_students(&line, &flag, &size_students, &courses, size_courses, fin, fout);
            if(flag.err){
            	n++;
            	flag.err = 0;
            	continue;
            }


            for (int i = 0; i < size_courses; ++i)
            {

            	for (int j = 0; j < size_professors; ++j)
            	{
            		if(professor_knows(courses[i].name, &professors[j])){
            			if(courses[i].prof_assigned == 0){
	            			if((professors[j].courses_num) > 1 && professors[j].courses_assigned == 1){
			        			courses[i].professor = &professors[j];
			        			professors[j].courses_assigned = 2;
			        			courses[i].prof_assigned = 1;
			        			break;
			        		}else if(professors[j].courses_num > 1 && professors[j].courses_assigned == 0){
			        			courses[i].professor =  &professors[j];
			        			professors[j].courses_assigned = 1;
			        			courses[i].prof_assigned = 1;
			        			break;
			        		}
	            		}
            		}
       
            	}

            	for (int j = 0; j < size_professors; ++j)
            	{
            		if(professor_knows(courses[i].name, &professors[j])){
	            		if(courses[i].prof_assigned == 0){
		            		if(professors[j].courses_num == 1 && professors[j].courses_assigned == 0){
		            			courses[i].professor = &(professors[j]);
		            			professors[j].courses_assigned = 1;
		            			courses[i].prof_assigned = 1;
		            			break;
		            		}
		            	}
		            }
            	}

            	for (int j = 0; j < size_professors; ++j)
            	{
            		if(courses[i].prof_assigned == 0){
	            		if(professors[j].courses_assigned == 0){
	            			courses[i].professor = &(professors[j]);
	            			professors[j].courses_assigned = 1;
	            			courses[i].prof_assigned = 1;
	            			break;
	            		}
	            	}
            	}

            }


            for (int i = 0; i < size_courses; ++i)
            {
            	if(courses[i].prof_assigned){
            		courses[i].assigned_tas = (Teacher_Assistant**) calloc(courses[i].labs_number, sizeof(Teacher_Assistant*));
            		for (int j = 0; j < courses[i].labs_number; ++j)
            		{
           				if(courses[i].tas_assigned < courses[i].labs_number){
           					for (int k = 0; k < courses[i].tas_num; ++k)
	            			{
	            				
	            				if(courses[i].teacher_assistants[k]->labs_assigned < 4){
	            					courses[i].assigned_tas[j] = courses[i].teacher_assistants[k];
	            					courses[i].tas_assigned++;
	            					courses[i].teacher_assistants[k]->labs_assigned++;
	            					break;
	            				}
	            			}
           				}
            			
            		}
            	}
            }

            for (int i = 0; i < size_courses; ++i)
            {
            	if((courses[i].prof_assigned) && (courses[i].tas_assigned == courses[i].labs_number)){
            		for (int j = 0; j < size_students; ++j)
	            	{
	            		if(students_wants(courses[i].name, &students[j])){
	            			if(courses[i].stud_num < courses[i].students_number){
		            			courses[i].students[courses[i].stud_num] = &students[j];
		            			students[j].assigned++;
		            			courses[i].stud_num++; 
	            			}
	            		}
	            		
	            	}
            	}
            	else{
            		courses[i].prof_assigned = 0;
            		courses[i].professor->courses_assigned--;

            		for (int j = 0; j < courses[i].tas_assigned; ++j){
            			courses[i].teacher_assistants[j]->labs_assigned--;

            		}
            		courses[i].stud_num = 0;
            		courses[i].students_number = 0;
            		courses[i].tas_assigned = 0;
            	}
            	
            }


            for (int i = 0; i < size_courses; ++i)
            {
            	if((courses[i].prof_assigned) && (courses[i].tas_assigned == courses[i].labs_number)){
            		fprintf(fout, "%s\n", courses[i].name);
					
					fprintf(fout, "%s %s\n", courses[i].professor->name, courses[i].professor->surname);

					for (int j = 0; j < courses[i].labs_number; ++j)
					{
						fprintf(fout, "%s %s\n", courses[i].assigned_tas[j]->name, courses[i].assigned_tas[j]->surname);
					}
					for (int j = 0; j < courses[i].stud_num; ++j)
					{
						fprintf(fout, "%s %s %s\n", courses[i].students[j]->name, courses[i].students[j]->surname, courses[i].students[j]->ID);
					}
					fprintf(fout, "\n");
            	}	
            	
            	
            	
            	
            }
            int total_score = 0;

            for (int i = 0; i < size_courses; ++i)
            {
            	if(!(courses[i].prof_assigned) || !(courses[i].tas_assigned == courses[i].labs_number)){
            		fprintf(fout, "%s cannot be run.\n", courses[i].name);
            		total_score += 20;

            	}	
            	
            }

            for (int j = 0; j < size_professors; ++j)
            {
            	if(professors[j].courses_assigned == 0){
            		fprintf(fout, "%s %s is unassigned.\n", professors[j].name, professors[j].surname);
            		total_score += 10;
            	}
            }

            for (int i = 0; i < size_courses; ++i)
            {
            	if((courses[i].professor->courses_assigned == 1) && !(professor_knows(courses[i].name, courses[i].professor))){
            		fprintf(fout, "%s %s is not trained for %s.\n", courses[i].professor->name, courses[i].professor->surname, courses[i].name);
            		total_score += 5;
            	}
            }

            for (int i = 0; i < size_courses; ++i)
            {
            	if((courses[i].professor->courses_assigned == 1) && (professor_knows(courses[i].name, courses[i].professor))){
            		fprintf(fout, "%s %s is lacking class.\n", courses[i].professor->name, courses[i].professor->surname);
            		total_score += 5;
            	}
            }

            for (int i = 0; i < size_tas; ++i)
            {
            	if(teacher_assistants[i].labs_assigned < 4){
            		fprintf(fout, "%s %s is lacking %d lab(s).\n", teacher_assistants[i].name, teacher_assistants[i].surname, 4-teacher_assistants[i].labs_assigned);
            		total_score += 2*(4-teacher_assistants[i].labs_assigned);
            	}
            }


           for (int i = 0; i < size_students; ++i)
           {
           		if(students[i].assigned < students[i].courses_num){
       				for (int k = 0; k < size_courses; ++k)
       				{

       					if(students_wants(courses[k].name, &students[i])){
       						int f = 0;
       						for (int t = 0; t < courses[k].students_number; ++t)
           					{
           						if(strcmp(courses[k].students[t]->ID, students[i].ID) == 0){

           							f = 1;
           							break;
           						}
           					}
           					if(!f){
           						fprintf(fout, "%s %s is lacking %s.\n", students[i].name, students[i].surname, courses[k].name);
           						total_score += 1;
           					}
       					}
       				}
       
           		}
           }
          

           fprintf(fout, "Total score is %d.", total_score);


            for (int i = 0; i < size_students; ++i)
            {
            	free(students[i].surname);
            	free(students[i].name);
            	free(students[i].ID);
            }
            free(students);

            for (i = 0; i < size_tas; ++i)
            {
            	free(teacher_assistants[i].surname);
            	free(teacher_assistants[i].name);
            }
            free(teacher_assistants);

            for (i = 0; i < size_professors; ++i)
            {
            	free(professors[i].surname);
            	free(professors[i].name);
            }
            free(professors);

            for (i = 0; i < size_courses; ++i)
            {
            	free(courses[i].name);
            }
            free(courses);
		}
        n++;
    }

    free(to_print);
}


int main(){

    // Generating a file RufinaTalalaevaEmail.txt with my Innopolis email.
    print_email();

    // Strating to input.
    input();

    return 0;
}



/**
 * Function is responsible for checking that
 * Student ID contains only English letters and digits.
 *
 * @param s String to check.
 * @return 1 If consists of only English letters and digits.
 * @return 0 If contains smth else except English letters and digits.
 */
int valid_ID(char s[]){
    if(strlen(s) != 5 && strlen(s) > 0){
        return 0;
    }else{
        for (int i = 0; i < strlen(s); ++i)
        {
            if((('a' <= s[i]) && (s[i] <= 'z')) || (('A' <= s[i]) && (s[i] <= 'Z')) || (('0' <= s[i]) && (s[i] <='9')));
            else return 0;
        }
    }
    return 1;
}

/**
 * Function is responsible for checking that
 * the string contains only English letters.
 *
 * @param s String to check.
 * @return 1 If consists of only English letters.
 * @return 0 If contains smth else except English letters.
 */
int valid_name(char s[]){
    for (int i = 0; i < strlen(s); ++i)
    {
        if((('a' <= s[i]) && (s[i] <= 'z')) || (('A' <= s[i]) && (s[i] <= 'Z')));
        else{
            return 0;
        }
    }
    return 1;
}

/**
 * Function is responsible for checking that
 * the string contains non-negative integer without zeros in the beginning.
 *
 * @param s String representation of number to check.
 * @return 1 If contains valid non-negative integer.
 * @return 0 Otherwise.
 */
int valid_number(char s[]){
    if(s[0] == '0'){
        if(strcmp(s, "0") == 0){
            return 1;
        }
        else{
            return 0;
        }
    }else if(s[0] == '-'){
        return 0;
    }else{
        for (int i = 0; i < strlen(s); ++i)
        {
        	if(('0' <= s[i]) && (s[i] <= '9'));
        	else{
        		return 0;
        	}
        }
    }
    return 1;
}

/**
 * Function is responsible for converting a valid string
 * representation of the number to integer representation.
 *
 * @param s String representation of the number.
 * @return Integer representation of the number.
 */
int str_to_int(char s[]){
    int res = s[0] - '0';
    for (int i = 1; i < strlen(s); ++i)
    {
        res = res*10 + (s[i] - '0');
    }
    return res;
}

/**
 * Function is responsible for printing "Invalid input" to the given output file.
 *
 * @param fout Existing output file.
 */
void error(FILE* fout){
    fprintf(fout, "Invalid input.");
}

/**
 * Function is responsible for generating a file
 * RufinaTalalaevaEmail.txt with my Innopolis email.
 */
void print_email(){
    FILE* email;
    email = fopen("RufinaTalalaevaEmail.txt", "w");
    fprintf(email, "r.talalaeva@innopolis.university");
}
/*
 * main.cpp
 *
 *      Author: apiskin
 */


#include <iostream>
#include <stdlib.h>
#include <stdio.h>
using namespace std;
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <string.h>
#include <iomanip>

// Function prototypes for the functions defined before operate() and timeval_convert()
void operate(int argc, char** argv);
double timeval_convert(struct timeval t);

// Prompt struct to make "set prompt =" process easier for the second section
struct Prompt {
	char* prompt;

	// Returns the prompt
	char* get_prompt() {
		return prompt;
	}

	// Changes the prompt
	void set_prompt(char* newprompt) {
		prompt = newprompt;
	}
};

// For the default commands in the shell like cd, exit and set prompt
bool builtin(int argc, char** argv, Prompt *prompt) {
    if (argc < 1) return false;

    // Applies chdir() for cd. To return to the user directory, only "cd" command can be used
    if (strcmp(argv[0], "cd") == 0) {
        if (argc == 1) {
            // For sole "cd" command to return to default user directory in shell
            if (chdir(getenv("HOME")) != 0) {
                cerr << "ERROR! (Change direction failed)" << endl;
            }
        }
        else {
            int exec = chdir(argv[1]);
            if (exec != 0) {
                cerr << "ERROR! (Change direction failed)" << endl;
                exit(1);
            }
        }
    }
    else if (strcmp(argv[0], "exit") == 0) {
        // Exits :)
        exit(0);
    }

    else if (strcmp(argv[0], "set") == 0) {
        // Checks for "set prompt =" command and sets the prompt from the struct
        if (strcmp(argv[1], "prompt") == 0 && strcmp(argv[2], "=") == 0) {
            prompt->set_prompt(argv[3]);
        }
        else { exit(1); }
    }
    else {
        return false;
    }
    return true;
}

// Changes the gathered data from timeval struct into milliseconds unit
double timeval_convert(struct timeval t) {
	return (double)t.tv_sec * 1000.0 + (double)t.tv_usec / 1000.0;
}

// Uses fork and executes the given command from the shell
// Expects the command after ./doit (i.e wc)
void operate(int argc, char** argv) {
	if (argc == 0) return;

	// Strips the ./doit from the 0th index of the argument array
	char *argn[argc];
	for (int i = 1; i < argc; ++i) {
		argn[i - 1] = argv[i];
	}
	argn[argc - 1] = NULL;

	struct timeval before;
	struct timeval after;
	struct rusage res_before;

	// Fills up the struct for data before the execution
	getrusage(RUSAGE_CHILDREN, &res_before);

	// Records "Wall Clock" time before the execution starts
	gettimeofday(&before, NULL);
    pid_t pid = fork();

    // Error case
    if (pid < 0) {
    	wait(0);
    	cerr << "Fork error\n";
        exit(1);
    }
    else if (pid == 0) {
        execvp(argn[0], argn);
        cerr << "Invalid command\n";
        exit(1);
    }
    else {
    	wait(0);

    	// Fills up a new struct for data gathered after the execution
    	struct rusage rexec;
    	getrusage(RUSAGE_CHILDREN, &rexec);

    	// Records CPU Time in total and separately
    	struct timeval time_user = rexec.ru_utime;
    	struct timeval time_sys = rexec.ru_stime;
    	double cpu_time = timeval_convert(time_user) + timeval_convert(time_sys);

    	// Records "Wall Clock" after the execution and calculates the elapsed time
    	gettimeofday(&after, NULL);
    	double wall_clock = timeval_convert(after) - timeval_convert(before);

    	// Records gathered data for page faults and process state transitions
    	long preempted = rexec.ru_nivcsw - res_before.ru_nivcsw;
    	long voluntarily = rexec.ru_nvcsw - res_before.ru_nvcsw;
    	long major = rexec.ru_majflt - res_before.ru_majflt;
    	long minor = rexec.ru_minflt - res_before.ru_minflt;

    	cout << endl << "***************** STATISTICS *****************" << endl;
    	cout << "Total CPU Time Used (in milliseconds): " << cpu_time << endl;
    	cout << "User Time (CPU) Used (in milliseconds): " << timeval_convert(time_user) << endl;
    	cout << "System Time (CPU) Used (in milliseconds): " << timeval_convert(time_sys) << endl;
    	cout << "Elapsed Wall Clock Duration (in milliseconds): " << wall_clock << endl;
    	cout << "The process was involuntarily preempted " << preempted << " times" << endl;
    	cout << "The process was voluntarily stopped " << voluntarily << " times" << endl;
    	cout << "The process had " << major << " major page faults" << endl;
    	cout << "The process had " << minor << " minor page faults" << endl;
    	cout << "***************** END OF STATISTICS *****************" << endl << endl;

    }
}

// Same as the previous operation without stripping the argument array
// Separated for the second section of the assignment but functions the same
void operate2(int argc, char** argn) {
	if (argc == 0) return;

	struct timeval before;
	struct timeval after;
	struct rusage res_before;

	// Fills up the struct for data before the execution
	getrusage(RUSAGE_CHILDREN, &res_before);

	// Records "Wall Clock" time before the execution starts
	gettimeofday(&before, NULL);
    pid_t pid = fork();
    if (pid < 0) {
    	wait(0);
    	cerr << "Fork error\n";
        exit(1);
    }
    else if (pid == 0) {
        execvp(argn[0], argn);
        cerr << "Invalid command\n";
        exit(1);
    }
    else {
    	wait(0);
    	// Fills up a new struct for data gathered after the execution
    	struct rusage rexec;
    	getrusage(RUSAGE_CHILDREN, &rexec);

    	// Records CPU Time in total and separately
    	struct timeval time_user = rexec.ru_utime;
    	struct timeval time_sys = rexec.ru_stime;
    	double cpu_time = timeval_convert(time_user) + timeval_convert(time_sys);

    	// Records "Wall Clock" after the execution and calculates the elapsed time
    	gettimeofday(&after, NULL);
    	double wall_clock = timeval_convert(after) - timeval_convert(before);

    	// Records gathered data for page faults and process state transitions
    	long preempted = rexec.ru_nivcsw - res_before.ru_nivcsw;
    	long voluntarily = rexec.ru_nvcsw - res_before.ru_nvcsw;
    	long major = rexec.ru_majflt - res_before.ru_majflt;
    	long minor = rexec.ru_minflt - res_before.ru_minflt;

    	cout << endl << "***************** STATISTICS *****************" << endl;
    	cout << "Total CPU Time Used (in milliseconds): " << cpu_time << endl;
    	cout << "User Time (CPU) Used (in milliseconds): " << timeval_convert(time_user) << endl;
    	cout << "System Time (CPU) Used (in milliseconds): " << timeval_convert(time_sys) << endl;
    	cout << "Elapsed Wall Clock Duration (in milliseconds): " << wall_clock << endl;
    	cout << "The process was involuntarily preempted " << preempted << " times" << endl;
    	cout << "The process was voluntarily stopped " << voluntarily << " times" << endl;
    	cout << "The process had " << major << " major page faults" << endl;
    	cout << "The process had " << minor << " minor page faults" << endl;
    	cout << "***************** END OF STATISTICS *****************" << endl << endl;

    }
}

int readprompt(char** argv) {
	// Created a buffer for maximum 128 characters
    char* buffer = new char[128];
    buffer = fgets(buffer, 128, stdin);
    size_t len = strlen(buffer);

    // Changed any new line into null terminator
    if (buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    }

    // Defined a space delimiter to slice the input into tokens
    const char* space = " ";
    char* tok = strtok(buffer, space);

    // Fills up the argument array from the user input by adding tokens into the array
    int argc = 0;
    while (tok != NULL) {
        argv[argc] = strdup(tok);
        argc++;
        tok = strtok(NULL, space);
    }
    argv[argc] = NULL;

    // Returns the argc after argv is filled up
    return argc;
}

int main(int argc, char** argv) {
	// Default prompt for the shell
	Prompt prompt;
	char def[] = "==>";
	prompt.set_prompt(def);

	// Section 1 of the assignment
	if (argc > 1) {
		operate(argc, argv);
	}
	// Section 2 of the assignment
	else {
		while(true) {
			// Prints out default prompt
			cout << prompt.get_prompt();
			argc = readprompt(argv);
			// If builtin functions cannot handle it, sends the argument array to execute with execvp and display stats
			if (!builtin(argc, argv, &prompt)) {
				operate2(argc, argv);
			}
		}
	}

    return 0;
}

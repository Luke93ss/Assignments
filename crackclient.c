#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <netdb.h>
#include <unistd.h>
#include <csse2310a3.h>

#define BUFFER_SIZE 50

// Enumerated type holding exit status information.
typedef enum {	
    NORMAL_EXIT = 0,
    ARGS_ERROR = 1,
    JOB_OPEN_ERROR = 2,
    PORT_CONNECT_ERROR = 3,
    SERVER_TERMINATED_ERROR = 4,
    ADDR_INFO_ERROR = 5, 
} ExitStatus;

// Structure to hold program parameters obtained from the command line.
typedef struct {
    char* port;
    char* jobFile;
} ProgramParams;

// Function prototypes - see functions for their descriptions
void args_error(void);
void job_file_error(char* jobFile);
ProgramParams process_command_line(int argc, char* argv[]);
int connect_to_port(ProgramParams params);
void process_job_commands(FILE* jobs, int socketFd, int stream);
void server_terminated_error(void);
void respond_to_server(FILE* to, FILE* from);

/*****************************************************************************/
int main(int argc, char* argv[]) {

    int socketFd;
    int stream = 0;
    FILE* jobs;
    // Process command line
    ProgramParams params = process_command_line(argc, argv);    
    if (params.jobFile != 0) {
	if ((jobs = fopen(params.jobFile, "r")) == NULL) {
	    job_file_error(params.jobFile);
	}
	stream = 1;
    } else {
	stream = 2;
    }
    // try and connect to port supplied
    socketFd = connect_to_port(params);
    //process_job_commands(jobs, socketFd, stream);
    process_job_commands(jobs, socketFd, stream);

    return 0;
}

// Function that prints the args error message and exits with a non zero exit
// status
void args_error() {
    fprintf(stderr, "Usage: crackclient portnum [jobfile]\n");
    exit(ARGS_ERROR);
}

// Function that prints the job file error message, takes in the name of the
// failed jobfile as an argument and that name is included in the message. 
// Exits a non zero exit status.
void job_file_error(char* jobFile) {
    fprintf(stderr, "crackclient: unable to open job file \"%s\"\n", jobFile);
    exit(JOB_OPEN_ERROR);
}

// Function that prints the port connection error, takes in the name of the
// port that was requested to connect to as an argument and include that 
// port name in the error message. Exits with a non zero exit status
void port_connection_error(const char* port) {
    fprintf(stderr, "crackclient: unable to connect to port %s\n", port);
    exit(PORT_CONNECT_ERROR);
}

// Function that prints the server terminated error, exits with a non-zero
// exit status
void server_terminated_error() {
    fprintf(stderr, "crackclient: server connection terminated\n");
    exit(SERVER_TERMINATED_ERROR);
}

// Function to process the job command, takes in a FILE* jobs which is the
// stream to be read from, the connected socket filedescriptor (socketFd) and
// the type of stream we are read commands from.
void process_job_commands(FILE* jobs, int socketFd, int stream) {
    // stream == 1 means reading from a jobfile
    // stream == 2 means reading from stdin
    int fd;
    char buffer[80];
    fd = dup(socketFd);
    FILE* to = fdopen(fd, "w");
    FILE* from = fdopen(socketFd, "r");
    while (1) {
	switch (stream) {
	    case 1:
		if ((fgets(buffer, BUFFER_SIZE, jobs)) != NULL) {
		    if (buffer[0] == '#' || strlen(buffer) < 1
			    || buffer[0] == '\n') {
			continue;
		    } else {
			fprintf(to, buffer);
			fflush(to);
			break;
		    }
		} else {
		    //close(socketFd);
		    fclose(to);
		    fclose(from);
		    exit(NORMAL_EXIT);
		}
	    case 2: 
		if ((fgets(buffer, BUFFER_SIZE, stdin)) != NULL) {
		    if (buffer[0] == '#' || strlen(buffer) < 1
			    || buffer[0] == '\n') {
			continue;
		    } else {
			fprintf(to, buffer);
			fflush(to);
			break;
		    }
		} else {
		    //close(socketFd);
		    fclose(to);
		    fclose(from);
		    exit(NORMAL_EXIT);
		}
	    
	}
	// wait for a server response;
	respond_to_server(to, from);
    }
}

// Function to process the command line arguments, takes in argc the number
// of arguments, argv[] the list of commands as strings. Prints out error
// messages if commands are invalid or returns a ProgramParams struct 
// containing valid commands.
ProgramParams process_command_line(int argc, char* argv[]) {
    
    ProgramParams params = { .port = 0, .jobFile = 0 };
    
    // Skip over the program name argument (./crackclient)
    argc--;
    argv++;
    if (argc < 1 || argc > 2) {
	args_error();
    } else if (argc == 2) {
	params.port = argv[0];
	params.jobFile = argv[1];
    } else {
	params.port = argv[0];
    }
    return params;
}

// Handles responding to the server, takes in 2 FILE* objects which allows the
// client to receive and send data to the server. If EOF is received the
// function closes the connection and exits with a non-zero exit status.
void respond_to_server(FILE* to, FILE* from) {
    
    char buffer[BUFFER_SIZE];
    
    if ((fgets(buffer, BUFFER_SIZE, from)) != NULL) {
	if (strcmp(buffer, ":invalid\n") == 0) {
	    printf("Error in command\n");
	    fflush(stdout);
	} else if (strcmp(buffer, ":failed\n") == 0) {
	    printf("Unable to decrypt\n");
	    fflush(stdout);
	} else {
	    printf("%s", buffer);
	    fflush(stdout);
	}
    } else {
	fclose(to);
	fclose(from);
	server_terminated_error(); 
    }
}

// Function that attempts to connect to a supplied port number. Takes in 
// a ProgramParams struct as an argument which contains the port number to
// connect to. Returns the connected port as a filedescriptor or exits
// with a non zero exit status if connection failed.
int connect_to_port(ProgramParams params) {
    
    int fd;
    struct addrinfo* ai = 0;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    int err;
    if ((err = getaddrinfo("localhost", params.port, &hints, &ai))) {
	freeaddrinfo(ai);
	fprintf(stderr, "%s\n", gai_strerror(err));
	exit(ADDR_INFO_ERROR);
    }
    fd = socket(AF_INET, SOCK_STREAM, 0); // 0 = default protocol (IPV4)
    if (connect(fd, ai->ai_addr, sizeof(struct sockaddr))) {
	port_connection_error(params.port);
    }
    return fd;
}

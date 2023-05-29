#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <csse2310a3.h>
#include <csse2310a4.h>
#include <crypt.h>
#include <ctype.h>
#include <math.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <errno.h>

#define BUFFER_SIZE 50
#define SALT_SIZE 2
#define MAX_PHRASE_SIZE 8
#define MAX_CIPHER_SIZE 13
#define MAX_NUM_LENGTH 6
#define MIN_PORT 1024
#define MAX_PORT 65535
#define CHAR_SET "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ./"\
	"0123456789"

// Structure to hold the program parameters - obtained from the command lin
typedef struct {
    int connections; 
    const char* port;
    char* fileName;
} ProgramParams;

//Structure that acts as a dictionary
typedef struct {
    int numWords;
    char** words;
} Dictionary;

// Structure that stores statistics related to client requests.
typedef struct {
    volatile int connectedClients;
    volatile int completedClients;
    volatile int crackRequests;
    volatile int failedRequests;
    volatile int successfulRequests;
    volatile int cryptRequests;
    volatile int cryptCalls;
} Statistics;

// Structure to hold information relevant to each thread performing
// a cracking request.
struct CrackCommInfo {
    volatile int* startIndex;
    volatile bool* found;
    volatile int* threadsInitiated;
    int numThreads;
    char* cipherText;
    Statistics* stats;
    sem_t* dataSem;
    Dictionary* dict;
};

// Structure to hold information required by the stats_on_sighup thread
// function
struct SigInfo {
    Statistics* stats;
    sigset_t* set;
};

// Structure to pass required info to a client thread
typedef struct {
    int* connectedFd;
    Dictionary* dict;
    sem_t* dataSem;
    sem_t* clientSem;
    ProgramParams params;
    Statistics* stats;
} ClientInfo;

// Enumerated type with various exit status'
typedef enum {
    USAGE_ERROR = 1,
    DICT_OPEN_ERROR = 2,
    DICT_TEXT_ERROR = 3,
    SOCKET_OPEN_ERROR = 4,
    NUMBER_ERROR = 5,
} ExitStatus;

/* Function prototypes - see decriptions with the functions themselves */
void usage_error(void);
void dictionary_open_error(char* fileName);
void dictionary_text_error(void);
void socket_open_error(void);
ProgramParams process_command_line(int argc, char* argv[]);
Dictionary parse_dictionary(char* fileName);
int is_valid_number(char* number);
void init_lock(sem_t* l, int value);
void take_lock(sem_t* l);
void release_lock(sem_t* l);
void* crack_cipher(void* ptr);
char* retrieve_salt(char* cipherText);
int get_serv_socket(const char* port);
void process_connections(ProgramParams params, Dictionary dict,
	Statistics* stats);
void* handle_client(void* ptr);
int list_length(char** list);
char* handle_crack_request(char** args, int length, ClientInfo* clientInfo);
char* handle_crypt_request(char** args, int length); 
int valid_chars(char* word);
bool valid_args(char** args, int length, int jobType);
void* stats_on_sighup(void* ptr);
void update_client_count(sem_t* dataSem, int operation, Statistics* stats);
void update_completed_clients(sem_t* dataSem, Statistics* stats);
void update_crack_requests(sem_t* dataSem, int stream, Statistics* stats);
void update_crypt_requests(sem_t* dataSem, Statistics* stats);
void update_crypt_calls(sem_t* dataSem, Statistics* stats);

/*****************************************************************************/
int main(int argc, char* argv[]) {
    
    Statistics* stats = malloc(sizeof(Statistics));
    memset(stats, 0, sizeof(Statistics));
    Dictionary dictionary;
    // Get program parameters
    ProgramParams params = process_command_line(argc, argv);
    // Establish dictionary
    if (params.fileName != 0) {
	dictionary = parse_dictionary(params.fileName);
    } else {
	dictionary = parse_dictionary("/usr/share/dict/words");
    }
    // Tell all threads to ignore SIGHUP signal,
    // Create a thread specifically to handle the signal.
    pthread_t sigthread;
    sigset_t set;
    int s;
    sigemptyset(&set);
    sigaddset(&set, SIGHUP);
    s = pthread_sigmask(SIG_BLOCK, &set, NULL);
    if (s != 0) {
	fprintf(stderr, "Error initialising masking\n");
    }
    struct SigInfo sigInfo;
    memset(&sigInfo, 0, sizeof(struct SigInfo));
    sigInfo.stats = stats, sigInfo.set = &set;
    s = pthread_create(&sigthread, NULL, &stats_on_sighup, (void*)&sigInfo);
    // Process requests from clients
    process_connections(params, dictionary, stats);
    return 0;
}

// Thread function dedicated to handling printing of statistics. 
// Takes in a void* variable which should be cast to a SigInfo struct.
// Uses the information within the structure to allow waiting for a particular
// signal and print statistics
void* stats_on_sighup(void* ptr) {
    
    struct SigInfo* s = (struct SigInfo*)ptr;
    sigset_t* set = s->set;
    Statistics* stats = s->stats;
    int sig;
    while (1) {
	sigwait(set, &sig);
	fprintf(stderr, "Connected clients: %i\n", stats->connectedClients);
	fprintf(stderr, "Completed clients: %i\n", stats->completedClients);
	fprintf(stderr, "Crack requests: %i\n", stats->crackRequests);
	fprintf(stderr, "Failed crack requests: %i\n",stats->failedRequests);
	fprintf(stderr, "Successful crack requests: %i\n", 
		stats->successfulRequests);
	fprintf(stderr, "Crypt requests: %i\n", stats->cryptRequests);
	fprintf(stderr, "crypt()/crypt_r() calls: %i\n", stats->cryptCalls);
	fflush(stderr);
    }
    return (void*)0;
}

// Function that prints the usage error message and exits with a non zero exit
// status, does not take any input. 
void usage_error() {
    
    fprintf(stderr, "Usage: crackserver [--maxconn connections]"
	    " [--port portnum] [--dictionary filename]\n");
    exit(USAGE_ERROR);
}

// Function that prints the dictionary open error messsage and refers to the
// relevant filename. Exits with a non zero exit status.
void dictionary_open_error(char* fileName) {
    
    fprintf(stderr, "crackserver: unable to open dictionary file \"%s\"\n", 
	    fileName);
    exit(DICT_OPEN_ERROR);
}

// Function that prints the dictionary text error message if no words are
// present in a dictionary, no input required, exits with a non-zero 
// exit status.
void dictionary_text_error() {
    
    fprintf(stderr, "crackserver: no plain text words to test\n");
    exit(DICT_TEXT_ERROR);
}

// Function that prints the socket error message, when a socket cannot be 
// establish, exits with a non-zero exit status.
void socket_open_error() {
    
    fprintf(stderr, "crackserver: unable to open socket for listening\n");
    exit(SOCKET_OPEN_ERROR);
}

// Function that checks the validity of the given number argument. Returns 
// NUMBER_ERROR when not valid and 0 if number is valid.
int is_valid_number(char* number) {
	
    int length = 0;
    for (int i = 0; i < strlen(number); i++) {
	length++;
	if (isdigit(number[i]) == 0 && number[i] != '\n') {
	    return NUMBER_ERROR;
	}
	if (length > MAX_NUM_LENGTH) {
	    //too many digits
	    return NUMBER_ERROR;
	}
    }
    return 0;
}

// Function that checks if the given word argument contains any invalid
// characters. Returns 1 if valid, 0 if invalid.
int valid_chars(char* word) {
    
    int result = 1;
    int index = 0;
    while (word[index]) {
	if (strchr(CHAR_SET, word[index]) == NULL && word[index] != '\n') {
	    return 0;
	}
	index++;
    }
    return result;
}

// Function that calculates the length of the list argument. Returns the
// length of the list
int list_length(char** list) {
   
    int i = 0;
    while (list[i]) {
	i++;
    }
    return i;
}

// Function to initialise the given semaphore.
void init_lock(sem_t* l, int value) {
    sem_init(l, 0, value);
}

// Function to wait until the supplied semaphore is released.
void take_lock(sem_t* l) {
    sem_wait(l);
}

// Function that releases the supplied semaphore
void release_lock(sem_t* l) {
    sem_post(l);
}

// Function that processes the connection from incoming clients
// Takes in a ProgramParams structure argument to set certain conditions for
// the client and a dictionary which is used for crypting and cracking.
void process_connections(ProgramParams params, Dictionary dict,
	Statistics* stats) {
    
    int connectedFd;
    int socketFd = get_serv_socket(params.port);
    struct sockaddr_in fromAddr;
    socklen_t fromAddrSize;
    sem_t clientSem;
    sem_t dataSem;
    init_lock(&dataSem, 1);
    // limit client connetions;
    if (params.connections != 0) {
	init_lock(&clientSem, params.connections);	
    }
    while (1) {

	fromAddrSize = sizeof(struct sockaddr_in);
	if (params.connections != 0) {
	    take_lock(&clientSem);
	}
	connectedFd = accept(socketFd, (struct sockaddr*)&fromAddr, 
		&fromAddrSize);
	if (connectedFd < 0) {
	    socket_open_error();
	}	    
	update_client_count(&dataSem, 0, stats);
	int* connectedPtr = malloc(sizeof(int));
	*connectedPtr = connectedFd;
	pthread_t threadId;
	ClientInfo* clientInfo = malloc(sizeof(ClientInfo));
	memset(clientInfo, 0, sizeof(ClientInfo));
	clientInfo->connectedFd = connectedPtr, clientInfo->dict = &dict; 
	clientInfo->params = params, clientInfo->clientSem = &clientSem;
	clientInfo->dataSem = &dataSem, clientInfo->stats = stats;
	pthread_create(&threadId, 0, handle_client, clientInfo);
	pthread_detach(threadId);
    }
    sem_destroy(&dataSem);
    sem_destroy(&clientSem);
}

// A threading function to establish connection with a client and handle 
// the clients requests. Takes in void* ptr as an argument which will be a
// structure containing required information. Returns NULL.
void* handle_client(void* ptr) { 
    // Client info containsconnectedPtr, stats pointer, dictionary pointer
    ClientInfo* clientInfo = (ClientInfo*)ptr;
    // Establish communication with client
    int fd2 = *(clientInfo->connectedFd);
    sem_t* dataSem = clientInfo->dataSem;
    Statistics* stats = clientInfo->stats;
    int fd = dup(fd2);
    FILE* to = fdopen(fd, "w");
    FILE* from = fdopen(fd2, "r");
    char** args;
    char* result = "";
    size_t length;
    char buffer[BUFFER_SIZE];
    while (fgets(buffer, BUFFER_SIZE, from) != NULL) {
	// Lets say its a standard command of crack "q904idDRadd" 5
	args = split_by_char(buffer, ' ', 0);
	length = list_length(args);
	if (length <= 1 || length > 3) {
	    fprintf(to, ":invalid\n");
	    fflush(to);
	    continue;
	}
	if (strcmp(args[0], "crack") == 0) {
	    result = handle_crack_request(args, length, clientInfo);
	} else if (strcmp(args[0], "crypt") == 0) {
	    update_crypt_requests(dataSem, stats);
	    update_crypt_calls(dataSem, stats);
	    result = handle_crypt_request(args, length);
	    
	} else {
	    fprintf(to, ":invalid\n");
	    fflush(to);
	    continue;
	}
	fprintf(to, result);
	fprintf(to, "\n");
	fflush(to);
    }
    update_client_count(dataSem, 1, clientInfo->stats);
    update_completed_clients(dataSem, clientInfo->stats);
    if (clientInfo->params.connections != 0) {
	release_lock(clientInfo->clientSem);
    }
    fclose(to);
    fclose(from);
    return NULL;
}

// Function called by handle_client() to handle crypt requests.
// Takes in a list of string arguments which are used in the crypting 
// process and the length of that list. Returns the encrypted word 
// or ":invalid" if certain argument requirements have not been met.
char* handle_crypt_request(char** args, int length) {

    struct crypt_data data;
    args++;
    length--;
    if (!valid_args(args, length, 0)) {		
	return ":invalid";
    }
    char* string = args[0];
    char* salt = args[1];
    return crypt_r(string, salt, &data);
}

//Function to update the client count, takes the given sempahore and updates
//the stats.
void update_client_count(sem_t* dataSem, int operation, Statistics* stats) {
    
    // Operation = 0 means increment, Operation = 1 means decrement;
    take_lock(dataSem);
    if (operation == 0) {
	stats->connectedClients += 1;
    } else {
	stats->connectedClients -= 1;
    }
    release_lock(dataSem);
}

// Function to update completed clients, takes the given semaphore and
// updates the stats.
void update_completed_clients(sem_t* dataSem, Statistics* stats) {
    take_lock(dataSem); 
    stats->completedClients += 1;
    release_lock(dataSem);
}

// Function to update the crack requests, takes the given semaphore
// and depending on the stream will update the stats,
void update_crack_requests(sem_t* dataSem, int stream, Statistics* stats) {

    // Stream == 0 means update total crack requests
    // Stream == 1 means update failed requests
    // Stream == 2 means update successful requests
    take_lock(dataSem);
    if (stream == 0) {
	stats->crackRequests += 1;
    } else if (stream == 1) {
	stats->failedRequests += 1;
    } else {
	stats->successfulRequests += 1;
    }
    release_lock(dataSem);
}

// Function to update the crypt requests, takes the given semaphore and
// updates the stats.
void update_crypt_requests(sem_t* dataSem, Statistics* stats) {
    
    take_lock(dataSem);
    stats->cryptRequests += 1;
    release_lock(dataSem);
}

// Function to update the crypt calls, takes the given sempahore and updates
// the stats.
void update_crypt_calls(sem_t* dataSem, Statistics* stats) {
    take_lock(dataSem);
    stats->cryptCalls += 1;
    release_lock(dataSem);
}

// Function called by handle_client() to handle cracking requests.
// The function takes in a list of string arguments which are used in the 
// cracking process, the length` of that list and a ClientInfo struct pointer. 
// Returns ":failed" if the cracking process did not find a matching cipher
// or the word of the matching cipher.
char* handle_crack_request(char** args, int length, ClientInfo* clientInfo) {
    volatile int startIndex = 0;
    volatile bool found = false;
    volatile int threadsInitiated = 0;
    sem_t* dataSem = clientInfo->dataSem;
    char* string;
    int numThreads = 1;
    // Skip over crack command
    args++;
    length--;
    update_crack_requests(dataSem, 0, clientInfo->stats);
    if (!valid_args(args, length, 1)) {
	return ":invalid";
    }
    string = args[0];
    if (length > 1) {
	numThreads = atoi(args[1]);
    }
    pthread_t tids[numThreads];
    struct CrackCommInfo info[numThreads];
    for (int i = 0; i < numThreads; i++) {
	info[i].startIndex = &startIndex;
	info[i].dataSem = dataSem;
	info[i].stats = clientInfo->stats;
	info[i].dict = clientInfo->dict;
	info[i].numThreads = numThreads; 
	info[i].cipherText = string;
	info[i].found = &found;
	info[i].threadsInitiated = &threadsInitiated;
	pthread_create(&(tids[i]), NULL, crack_cipher, info + i);
    }
    for (int i = 0; i < numThreads; i++) {
	void* result;
	pthread_join(tids[i], &result);
	if (strcmp((char*)result, "") != 0) {
	    update_crack_requests(dataSem, 2, clientInfo->stats);
	    return (char*)result;
	}
    }
    update_crack_requests(dataSem, 1, clientInfo->stats);
    return ":failed";
}

// Function to crack the cipher, takes in a void* ptr which is typically a
// CrackCommInfo struct pointer which points to information required by
// each cipher cracking thread. The function returns an empty string "" if
// no matching cipher was found or it returns the word of the matching cipher.
void* crack_cipher(void* ptr) {

    struct CrackCommInfo* p = (struct CrackCommInfo*)ptr;
    Dictionary* dict = p->dict;
    Statistics* stats = p->stats;
    sem_t* dataSem = p->dataSem;
    struct crypt_data data;
    memset(&data, 0, sizeof(struct crypt_data));
    int numWords = p->dict->numWords;
    int numThreads = p->numThreads;
    int cryptRequests = 0;
    int range = floor(numWords / numThreads);
    char* cipherText = p->cipherText; 
    char* cipherFromDict;
    char* string;
    int index = 0;
    int endRange = 0;
    char* result = "";
    char* salt = retrieve_salt(cipherText);

    take_lock(dataSem);
    index = *(p->startIndex);
    *(p->startIndex) += range;
    *(p->threadsInitiated) += 1;
    if (*(p->threadsInitiated) == numThreads) {
	endRange = numWords;
    } else {
	endRange = index + range;
    }
    release_lock(dataSem);

    while (index < endRange) {
	if (*(p->found)) {
	    break;
	}
	string = dict->words[index];
	cipherFromDict = crypt_r(string, salt, &data);
	update_crypt_calls(dataSem, stats);
	cryptRequests++;
	if (strcmp(cipherFromDict, cipherText) == 0) {
	    take_lock(dataSem);
	    *(p->found) = true;
	    release_lock(dataSem);
	    result = string;
	    break;
	}
	index++;
    }
    return (void*)result;
}

// Function to check that the supplied arguments are valid. 
// Takes in a list of string arguments, the list's length and the jobType 
// using this function. If all the supplied arguments are valid then true is
// returned otherwise false.
bool valid_args(char** args, int length, int jobType) {

    // Job type 0 = crypt, Job Type 1 = crack
    bool result = true;
    int threadCount;
    int valid = 0;
    switch (jobType) {
	case 0: 
	    if (length != 2) {
		result = false;
		break;
	    }
	    if (strlen(args[0]) > 8 || strlen(args[0]) == 0) {
		result = false;
	    }
	    if (!valid_chars(args[1])) {
		result = false;
	    }
	    if (strlen(args[1]) != 3) {
		result = false;
	    }
	    break;
	case 1:
	    if (strlen(args[0]) != 13) {
		// check ciphertext validity
		result = false;
		break;
	    }
	    if (strchr(CHAR_SET, args[0][0]) == NULL 
		    || strchr(CHAR_SET, args[0][1]) == NULL) {
		result = false;
	    }
	    if (length == 2) {
		valid = is_valid_number(args[1]);
		// check thread number validity
		if (valid != 0) {
		    result = false;
		}
		threadCount = atoi(args[1]);
		if (threadCount < 1 || threadCount > 50) {
		    result = false;
		}
	    }
	    break;
    }
    return result;
}

// Function to obtain the server socket on the given port. Takes in a port
// number and attempts to gather address info. If successful it attempts to 
// create a socket and bind it to the port then listen for incoming
// connections. Returns the server socket.
int get_serv_socket(const char* port) {

    struct addrinfo* ai = 0;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int err;
    if ((err = getaddrinfo(NULL, port, &hints, &ai))) {
	freeaddrinfo(ai);
	socket_open_error();
    }
    // create a socket and bind it to a port
    int serv = socket(AF_INET, SOCK_STREAM, 0);
    int optVal = 1;
    if (setsockopt(serv, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(int)) < 0) {
	socket_open_error();
    }
    if (bind(serv, ai->ai_addr, sizeof(struct sockaddr))) {
	socket_open_error();
    }
    if (listen(serv, 10) < 0) {	
	socket_open_error();
    }
    // Get port number if supplied port was zero.
    if (strcmp(port, "0") == 0) {
	struct sockaddr_in ad;
	memset(&ad, 0, sizeof(struct sockaddr_in));
	socklen_t len = sizeof(struct sockaddr_in);
	if (getsockname(serv, (struct sockaddr*)&ad, &len)) {
	    socket_open_error();	
	}
	// print port number obtained ad.sin_port
	fprintf(stderr, "%u\n", ntohs(ad.sin_port));
	fflush(stderr);
    } else {
	//print supplied port
	fprintf(stderr, "%s\n", port);
	fflush(stderr);
    }
    return serv;
}

// Function that retrieves the salt from the supplied cipherText argument.
// Returns the salt.
char* retrieve_salt(char* cipherText) {
    
    char* salt = malloc(sizeof(char) * (SALT_SIZE + 1));
    salt[0] = cipherText[0];
    salt[1] = cipherText[1];
    salt[2] = '\0';
    
    return salt;
}

// Function that attempts to open a file from the given argument and read in
// and store its contents. Returns a Dictionary struct containing all the
// words read and the number of words found.
Dictionary parse_dictionary(char* fileName) {

    Dictionary dictionary = { .numWords = 0, .words = 0};
    FILE* dict;
    int numWords = 0;
    char** words;
    char* line;
    if ((dict = fopen(fileName, "r")) == NULL) {
	dictionary_open_error(fileName);
    }
    words = malloc(sizeof(char*) * (numWords + 1));
    while ((line = read_line(dict)) != NULL) {
	if (line[strlen(line) - 1] == '\n') {
	    line--;
	}
	if (strlen(line) > MAX_PHRASE_SIZE) {
	    continue;
	}
	words[numWords++] = line;
	words = realloc(words, sizeof(char*) * numWords + 1);
	words[numWords] = '\0';
    }
    if (list_length(words) == 0) {
	dictionary_text_error();
    }
    dictionary.words = words;
    dictionary.numWords = numWords;
    fclose(dict);

    return dictionary;
}
    
// Function that handles the processing of command line arguments.
// Takes in argc (the number of commands), and argv[] the list of commands
// as strings, if a particular command line argument is invalid it prints
// the appropriate error message and exits with a non zero exit status. If
// all received arguments are valid then a ProgramParams struct is returned
// with the valid command line arguments.
ProgramParams process_command_line(int argc, char* argv[]) {

    int portNum;
    ProgramParams params = { .connections = 0, .port = "0", .fileName = 0};

    // Skip over the program name
    argc--;
    argv++;

    while (argc >= 2 && argv[0][0] == '-') {
	if (!strcmp(argv[0], "--maxconn") && params.connections == 0
		&& argc >= 2) {
	    if (is_valid_number(argv[1]) == 0) {
		params.connections = atoi(argv[1]);
	    } else {
		usage_error();
	    }
	} else if (!strcmp(argv[0], "--port") && strcmp(params.port, "0") == 0 
		&& argc >= 2) {
	    if (is_valid_number(argv[1]) == 0) {
		portNum = atoi(argv[1]);
		if ((portNum >= MIN_PORT && portNum <= MAX_PORT) 
			|| portNum == 0) {
		    params.port = argv[1];
		} else {
		    usage_error();
		}
	    } else {
		usage_error();
	    }

	} else if (!strcmp(argv[0], "--dictionary") && params.fileName == 0
		&& argc >= 2) {
	    params.fileName = argv[1];
	} else {
	    usage_error();
	}
	argc -= 2;
	argv += 2;
    }	
    if (argc != 0) {
	usage_error();
    }

    return params;
}

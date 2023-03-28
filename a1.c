#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <csse2310a1.h>

#define DEFAULT_DICT "/usr/share/dict/words"
#define WELCOME_MESSAGE "Welcome to UQWordiply!\n"
#define INVALID_STARTER "uqwordiply: invalid starter word\n"
#define DICT_ERROR "uqwordiply: dictionary file \"%s\" cannot be opened\n" 
#define BUFFER_SIZE 52
#define STARTER_BUFFER 5
#define USER_PROMPTS 5

/* is_valid_word()
 * --------
 * Takes in a pointer to a string and a usecase integer, the function
 * determines if it is a valid word for the cases: (0) being a valid
 * starter word; (1) being a valid work in the dictionary i.e no symbols
 * present in the word.
 *
 * word: pointer to a string (char*) of the word to be checked.
 * usecase: integer value of specified usecase.
 *
 * Returns: int value, 1 if the input is a valid word, 0 othereise.
 */
int is_valid_word(char* word, int usecase) {

    int len = strlen(word);
    int value = 0;
    char letter;
    
    switch (usecase) {
	case 0:
	    if (len == 3 || len == 4) {
		for (int i = 0; i < len; i++) {
		    letter = word[i];
		    if (isalpha(letter)) {
			value = 1;
	    	    } else {
			return 0;
	    	    }		
		}		
	    } else {
         	return 0;
    	    }
    	    return value;
	case 1:
	    for (int i = 0; i < len; i++) {
		letter = word[i];
		if (isalpha(letter)) {
		    value = 1;
		} else if (!isalpha(letter) && i != len - 1) {
		    return 0;
		}
	    }
    }
    return value;
}

/* is_valid_number()
 * --------
 * Takes in a pointer to a string and determines if it is a valid number.
 *
 * number: pointer to a string (char*) of the number to be checked.
 *
 * Returns: int value, 1 if the input is a valid number, 0 otherwise.
 */
int is_valid_number(char* number) { 

    int num;
    int value;

    if ((int)strlen(number) == 1) {
	num = atoi(number);
	if (num == 3 || num == 4) {
	    value = 1;
        } else {
	    value = 0;
   	}
    } else {
	return 0;
    }
    return value;
}

/* starter_word_validity()
 * --------
 * Checks the validity of the word that follows the --start input.
 *
 * num: the number of entries in the command line (argc) of type (int).
 * string: a pointer to an array (char**) of the command line inputs (argv).

 *
 * Returns: int value, 1 if the input is a valid word, 0 otherwise.
 */
int starter_word_validity(int num, char** string) { 

    int value = 0;
    int present;

    for (int i = 1; i < num; i++) {
	present = !strcmp(string[i], "--start");
	if (present) { 
	    if (is_valid_word(string[i + 1], 0)) {
		return 1;;
	    } else { 
		return 2;
	    }
	} else {
	    value = 0;
	}
    }
    return value;
}

/* dict_present()
 * --------
 * Checks if the --dictionary argument is present in the command line.
 *
 * num: the number of entries in the command line (argc) of type (int).
 * string: a pointer to an array (char**) of the command line inputs.
 *
 * Returns: int value, 1 if the --dictionary command is present, 0 otherwise.
 */
int dict_present(int num, char** string) {

    int value = 0;
    int present;

    for (int i = 1; i < num; i++) {
	present = !strcmp(string[i], "--dictionary");
	if (present) { 
	    return 1;
	} else {
	    value = 0;
	}
    }
    return value;
}

/* len_present()
 * --------
 * Checks if the --len argument is present in the command line.
 *
 * num: the number of entries in the command line (argc) of type (int).
 * string: a pointer to an array (char**) of the command line inputs.
 *
 * Returns: int value, 1 if the --len argument is present, 0 othereise.
 */
int len_present(int num, char** string) {

    int present;

    for (int i = 1; i < num; i++) {
	present = !strcmp(string[i], "--len");
	if (present) {
	    return 1;
	} else {
	    return 0;
	}
    }
    return 0;
}

/* get_dict()
 * --------
 * Returns the name of the dictionary to be used.
 *
 * num: the number of entries in the command line (argc) of type (int).
 * string: a pointer to an array (char**) of the command line inputs.
 *
 * Returns: pointer to a string (char*) of the dictionary name.
 */
char* get_dict(int num, char** string) {
    int present;
    for (int i = 1; i < num; i++) {
	present = !strcmp(string[i], "--dictionary");
	if (present) { 
	    char* dict = string[i + 1];
	    return dict;
	}
    }
    return 0;
}

/* get_starter_word()
 * --------
 * Returns the starter word entered after the --start input
 *
 * num: the number of entries in the command line (argc) of type (int).
 * string: a pointer to an array (char**) of the command line inputs.
 *
 * Returns: A pointer to a string (char*) of the starter word to be used.
 */
char* get_starter_word(int num, char** string, char* starter) {

    int present;

    for (int i = 1; i < num; i++) {
	present = !strcmp(string[i], "--start");
	if (present) { 
	    //char* word = malloc(sizeof(char) * len + 1);
	    strcpy(starter, string[i + 1]);
	    return starter;
	}
    }
    return 0;
}

/* get_length()
 * --------
 * Gets the len of the word the user has specified after the
 * --len command line input.
 *
 * num: the number of entries in the command line (argc) of type (int).
 * string: a pointer to an array (char**) of the command line inputs.
 *
 * Returns: An integer of the length of the word the user has specified.
 */
int get_length(int num, char** string) {

    int len;
    int present;

    for (int i = 1; i < num; i++) {
	present = !strcmp(string[i], "--len");
	if (present) { 
	    len = atoi(string[i + 1]);
	    return len;
	}
    }
    return 0;
}

/* switch_uppercase()
 * --------
 * Changes a word to all uppercase letters.
 *
 * word: Pointer to a string (char*).
 *
 * Returns: Pointer to a string (char*) of the word
 */
char* switch_uppercase(char* word) {
    int len = strlen(word);
    for (int i = 0; i < len; i++) {
	word[i] = toupper(word[i]);
    }
    word[len] = '\0';
    return word;
}
   
/* command_line_validity()
 * --------
 * Checks the command line inputs for validity according to the assignment 
 * specsheet.
 *
 * num: the number of entries in the command line (argc) of type (int).
 * string: a pointer to an array (char**) of the command line inputs.
 *
 * Returns: int value, 1 if the command line is valid, 0 otherwise.
 */
int command_line_validity(int num, char** strings) {

    int index = 1;
    int startOrLen = 0;
    int start, len, dict;
    int dictCalled = 0;
    int value = 0;
	
    if (num == 1) {
    	return 1;
    } else if (num == 3 || num == 5) {
	while (strings[index]) {
	    char* input = strings[index];
	    start = !strcmp(input, "--start");
	    len = !strcmp(input, "--len");
	    dict = !strcmp(input, "--dictionary");
	    if (len && startOrLen == 0) {
		startOrLen++;
		if (is_valid_number(strings[index + 1])) {
		    value = 1;
		} else {
		    return 0;
		}
	    } else if (len && startOrLen == 1) {
		return 0;
	    } else if (start && startOrLen == 0) {
		startOrLen++;
		value = 1;
	    } else if (start && startOrLen == 1) {
		return 0;
	    } 
	    if (dict && dictCalled == 0) {
		dict++;
		value = 1;
	    } else if (dict && dictCalled == 1) {
		return 0;
	    }
	    index += 2;
	}
    } else if (num == 2 || num == 4 || num > 5) {
    	return 0;
    } else {
	return 0;
    }
    return value;
}

/* read_line()
 * --------
 * Reads a single line in a given FILE* and returns the line.
 *
 * dictionary: a dictionary of type FILE*.
 * buffer: a pointer to a string.
 *
 * Returns: the line that was read in the dictionary.
 */
char* read_line(FILE* dictionary, char* buffer) {

    int bufferSize = BUFFER_SIZE;
    char* copy = malloc(sizeof(char) * bufferSize);
    int numRead = 0;
    int next;
    if (feof(dictionary)) {
	return NULL;
    }

    while (1) {
	next = fgetc(dictionary);
	if (next == EOF && numRead == 0) {
	    free(copy);
	    return NULL;
	}
	if (next == '\n' || next == EOF) {
	    copy[numRead] = '\0';
	    break;
	}
	copy[numRead++] = next;
    }
    strcpy(buffer, copy);
    free(copy);
    return buffer;
}

/* get_max()
 * --------
 * Determines the max value between two given integers.
 *
 * len1: length of a specified word of type int.
 * len2: length of a specified word of type int.
 *
 * Returns: the larger of the two given numbers.
 */
int get_max(int len1, int len2) {

    if (len1 >= len2) {
	return len1;
    } else if (len2 >= len1) {
	return len2;
    }
    return 0;
}

/* in_dictionary()
 * --------
 * Searches each line of a dictionary to check if the given guess word
 * matches any entries of the dictionary.
 *
 * dictionary: the dictionary to be searched of type FILE*.
 * guess: the word that is compared to entries in the dictionary
 * of type char*.
 *
 * Returns: int value, 1 if there is a match in the dictionary, 0 otherwise.
 */
int in_dictionary(char** dictionary, char* guess) {

    int i = 0;
    int lenDictWord = 0;
    int lenGuess = 0;
    int max = 0;
    char* copy = malloc(sizeof(char) * BUFFER_SIZE);
    int check = 0;

    while (dictionary[i]) {
	strcpy(copy, dictionary[i++]);
	lenDictWord = strlen(copy) - 1;
    	lenGuess = strlen(guess) - 1;
	max = get_max(lenDictWord, lenGuess);
	check = strncasecmp(copy, guess, max);

	if (check == 0) {
	    return 1;
	}
    }
    free(copy);
    return 0;
}

/* words_with_starter()
 * --------
 * Searches a dictionary for words which contain the starter word
 * and appends them to a pointer to an array then returns that array 
 * once all words in the dictionary have been checked.
 *
 * dictionary: dictionary to be checked of type FILE*.
 * starter: the starter word of the game of type char*.
 *
 * Returns: a pointer to an array containing words similar to the starterword.
 */
char** words_with_starter(FILE* dictionary, char* starter,
	char** similarWords) {

    int bufferSize = BUFFER_SIZE;
    char* word = malloc(sizeof(char) * bufferSize);
    word = read_line(dictionary, word);
    int numWords = 0;

    while(word != NULL) {
	if (strcasestr(word, starter) != NULL) {
	    similarWords = realloc(similarWords, sizeof(char*) * 
		    (numWords + 2));
	    similarWords[numWords] = malloc(sizeof(char) * (strlen(word) + 1));
	    similarWords[numWords++] = strdup(word);
	    similarWords[numWords] = '\0';
	}
	word = read_line(dictionary, word);
    }
    free(word);
    return similarWords;
}

/* guess_validity()
 * --------
 * Takes in a guessed word and checks that the word hasn't previously been
 * guessed and that it is a valid guess i.e, does not match the starter word,
 * is made up of letters only, is not blank, the guess contains the 
 * starter word, and it exists in the similarWords dictionary. If the guess is
 * not valid the functions prints statements related to the validity
 * check via stdout.
 *
 * guess: the guessed word to be checked for validity of type char*.
 * starter: the starter word of type char*.
 * similarWords: dictionary of words containing the starter word of type
 * pointer to an array.
 * guessedWords: dictionary of words containing valid guesses of type pointer 
 * to an array.
 *
 * Returns: int value, 1 if the guess is valid, 0 otherwise. 
 */
int guess_validity(char* guess, char* starter, char** similarwords, 
	char** guessedwords) {

    int validWord = is_valid_word(guess, 1);
    int inDict = in_dictionary(similarwords, guess);
    int inGuesses = in_dictionary(guessedwords, guess);
    int len = strlen(guess);
    char* check1 = NULL;
    int check2 = 0;
    check1 = strcasestr(guess, starter);
    check2 = strncasecmp(guess, starter, len - 1);

    if (feof(stdin)) {
	return 0;
    }

    if (!validWord && len > 1) {
	printf("Guesses must contain only letters - try again.\n");
	return 0;
    }

    if (check1 == NULL || len <= 1) {
	printf("Guesses must contain the starter word - try again.\n");
	return 0;
    }
	
    if (check2 == 0) {
	printf("Guesses can't be the starter word - try again.\n");
	return 0;
    }
	
    if (!inDict) {
	printf("Guess not found in dictionary - try again.\n");
	return 0;
    }
	
    if (inGuesses) {
	printf("You've already guessed that word - try again.\n");
	return 0;
    }
    return 1;
}

/* count_guesses()
 * --------
 * Counts the number of entries in the guessedWords dictionary.
 *
 * guessedWords: dictionary containing the words guessed of type char**.
 *
 * Returns: int value, returns the count.
 */
int count_guesses(char** guessedWords) {

    int count = 0;

    while (guessedWords[count]) {
	count++;
    }
    return count;
}

/* total_length_guessed()
 * --------
 * Calculates the total length of the words contained in the guessedWords
 * dictinary.
 *
 * guessedWords: dictionary containing the words guessed of type char**
 *
 * Returns: an integer value of the total length of words.
 */
int total_length_guesses(char** guessedWords) {

    int count = 0;
    int len = 0;
    int totalLength = 0;

    while (guessedWords[count]) {
	len = strlen(guessedWords[count]);
	count++;
	totalLength += len;
    }
    return totalLength - count;
}

/* get_longest_words()
 * --------
 * Finds and stores the longest words in a given dictionary
 *
 * guessedWords: dictionary of words of type char**.
 *
 * Returns: a dictionary containing the longest word or words from the 
 * supplied dictionary
 */
char** get_longest_words(char** guessedWords) {

    char** longestWords = calloc(1, sizeof(char*));
    int longestLength = 0;
    int wordLength;
    int wordsAdded = 0;
    int count = 0;
    int validWord = 0;

    while (guessedWords[count]) {
	wordLength = strlen(guessedWords[count]);
	validWord = is_valid_word(guessedWords[count], 1);
	if ((wordLength > longestLength) && validWord) {
	    wordsAdded = 1;
	    longestWords = realloc(longestWords, sizeof(char*) * 2);
	    longestWords[wordsAdded - 1] = malloc(sizeof(char) * 
		    (wordLength + 1)); 
	    strcpy(longestWords[wordsAdded - 1], guessedWords[count]);
	    longestWords[wordsAdded] = '\0';
	    longestLength = wordLength;
	    count++;
	    wordLength = 0;
	} else if ((wordLength == longestLength) && validWord) {
	    wordsAdded++;
	    longestWords = realloc(longestWords, sizeof(char*) * 
		    (wordsAdded + 1));
	    longestWords[wordsAdded - 1] = malloc(sizeof(char) * 
		    wordLength + 1);
	    strcpy(longestWords[wordsAdded - 1], guessedWords[count]);
	    longestWords[wordsAdded] = '\0';
	    count++;
	} else {
	    count++;
	}
    }
    return longestWords;
}

/* free_dictionary()
 * --------
 * Takes in a dictionary and frees all the memory allocated to each individual
 * pointer then frees the dictionary as a whole.
 *
 * dictionary: dictionary containing words of type char**.
 */
void free_dictionary(char** dictionary) {
    
    int i = 0;
    int getCount = 0;
    while (dictionary[i]) {
	i++;
	getCount++;
    }
    for (int i = 0; i < getCount - 1; i++) {
	free(dictionary[i]);
    }
}

/* start_of_game_messages()
 * --------
 * Prints the starting messages of the game.
 *
 * starter: the starter word of the game
 */
void start_of_game_messages(char* starter) {

    printf(WELCOME_MESSAGE);
    printf("The starter word is: %s\n", starter);
    printf("Enter words containing this word.\n");
}

/* final_stats()
 * --------
 * Prints the final stats of the game.
 *
 * longestWords: dictionary of the longestWords the user has guessed.
 * guessedWords: dictionary of the words the user has guessed.
 * bestWords: dictionary of the longest words the user could have guessed.
 */
void final_stats(char** longestWords, char** guessedWords,
	char** bestWords) {
    
    int totalLength = total_length_guesses(guessedWords);
    int count = 0;
    int wordLength = 0;
    char* words = malloc(sizeof(char) * BUFFER_SIZE);

    printf("\nTotal length of words found: %i\n", totalLength);
    printf("Longest word(s) found:\n");
    while (longestWords[count]) {
	words = switch_uppercase(longestWords[count]);
	wordLength = strlen(longestWords[count]) - 1;	
	for (int i = 0; i < wordLength; i++) {
	    printf("%c", words[i]);
	}
	printf(" (%i)\n", wordLength);
	count++;
    }	
    count = 0;
    printf("Longest word(s) possible:\n");
    while (bestWords[count]) {
	words = switch_uppercase(bestWords[count]);
	wordLength = strlen(bestWords[count]);
	for (int i = 0; i < wordLength; i++) {
	    printf("%c", words[i]);
	}
	printf(" (%i)\n", wordLength);
	count++;
    }
    free(words);
}

/* retrieve_guessed()
 * --------
 * A function that manages the section of the game where the user
 * is prompted to enter in guesses.
 *
 * guessedWords: a dictionary allocated for guessedWords, of type char**.
 * similarWords: a dictionary containing similar words to the starter word, 
 * of type char**.
 * starter: the starter word of the game, of type char*.
 *
 * Returns: a dictionary of the valid guesses the user has made.
 */
char** retrieve_guessed(char** guessedWords, char** similarWords,
	char* starter) {
    
    char* buffer = malloc(sizeof(char) * BUFFER_SIZE);
    int guess = 1;
    int guessValid = 0;
    
    while (guess <= 5 && !feof(stdin)) {
	printf("Enter guess %i:\n", guess);
	fgets(buffer, BUFFER_SIZE, stdin);
	if (!feof(stdin)) {
	    guessValid = guess_validity(buffer, starter, similarWords, 
		    guessedWords);	
	}

	while (!guessValid && !feof(stdin)) {
	    printf("Enter guess %i:\n", guess);
	    fgets(buffer, BUFFER_SIZE, stdin);
	    if (!feof(stdin)) {
		guessValid = guess_validity(buffer, starter, similarWords,
			guessedWords);
	    }
	}
	if (!feof(stdin)) {
	    guessedWords[guess - 1] = malloc(sizeof(char) * 
		    (strlen(buffer) + 1));
	    strcpy(guessedWords[guess - 1], buffer);
	    guessedWords = realloc(guessedWords, sizeof(char*) * (guess + 1));
	    guessedWords[guess] = '\0';
	    guess++;
	}
    } 
    free(buffer);
    return guessedWords;
}

/* starter_word_checks()
 * --------
 * A function that handles the part of the game where the starter word is 
 * selected, exits the game if the starter word is not valid.
 *
 * argc: number of entries in the command line, of type int.
 * argv: the entires in the command line, of type char**.
 * starterWord: an int value relating to the starter word scenario, i.e
 * 0 if a starter word was not specified, 1 if the word was specified, or 2
 * if the starter word given is invalid.
 * starter: a pointer to allocated memory for a starter word.
 *
 * Returns: the starter word.
 */
char* starter_word_checks(int argc, char** argv, int starterWord,
	char* starter) {

    int lenPresent = len_present(argc, argv);

    if (starterWord == 1) {
	starter = switch_uppercase(get_starter_word(argc, argv, starter));

    } else if (starterWord == 2) {
	fprintf(stderr, INVALID_STARTER);
	exit(2);
	
    } else if (starterWord == 0) {

	if (lenPresent) {
	    unsigned int starterLen = get_length(argc, argv);
	    starter = (char*)get_wordiply_starter_word(starterLen);

	} else if (!lenPresent) {
	    starter = (char*)get_wordiply_starter_word(0);
	} 
    }
    return starter;
}

/* dictionary_checks()
 * --------
 * Carries out initial checks of the --dictionary input on the command line
 * and if dictionary argument is present, it attempts to open the dictionary
 * with the provided pathway, if not present it creates a dictionary with the
 * standard pathway.
 *
 * argc: integer, number of inputs to the command line.
 * argv: array of inputs to the command line, of type char**.
 *
 * Returns: the dictionary created.
 */
FILE* dictionary_checks(int argc, char** argv) {
    
    FILE* dict;
    int dictPresent = dict_present(argc, argv);

    if (dictPresent) {
		
	char* dictName = get_dict(argc, argv);

	dict = fopen(dictName, "r");
	if (dict == NULL) {
	    fprintf(stderr, DICT_ERROR, dictName);
	    exit(3);
	}
    } else if (!dictPresent) {
	dict = fopen(DEFAULT_DICT, "r");
	if (dict == NULL) {
	    fprintf(stderr, DICT_ERROR, DEFAULT_DICT);
	    exit(3);
	}
    }
    return dict;
}

int main(int argc, char** argv) {

    char* userErrorMessage = ("Usage: uqwordiply [--start starter-word |"
	    " --len length] [--dictionary filename]\n");
    int commandValid = command_line_validity(argc, argv);
    char** guessedWords = calloc(1, sizeof(char*));
    char** similarWords = calloc(1, sizeof(char*)); 
    char* starter = malloc(sizeof(char) * STARTER_BUFFER);
    int countGuesses = 0;
	
    if (!commandValid) {
	fprintf(stderr, userErrorMessage);
	free_dictionary(guessedWords);
	return 1;
    }
		
    int starterWord = starter_word_validity(argc, argv);
    starter = starter_word_checks(argc, argv, starterWord, starter);
    FILE* dict = dictionary_checks(argc, argv);
    similarWords = words_with_starter(dict, starter, similarWords);
    start_of_game_messages(starter);
    guessedWords = retrieve_guessed(guessedWords, similarWords, starter);
    countGuesses = count_guesses(guessedWords);	

    if (countGuesses < 1) {
	return 4;

    } else if (countGuesses != 0) {
	char** longestWords = get_longest_words(guessedWords); 
	char** bestWords = get_longest_words(similarWords);
	final_stats(longestWords, guessedWords, bestWords);
	free_dictionary(guessedWords);
	free_dictionary(bestWords);
	free_dictionary(similarWords);
	free_dictionary(longestWords);
	free(similarWords);
	free(longestWords);
	free(bestWords);
	free(guessedWords);
	return 0;
    }
    return 0;
}


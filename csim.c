/* Cache Simulator Project
 *
 * csim.c: Simulates the behavior of a cache memory (this file)
 * 
 * Implements the Least Recently Used (LRU) replacement policy
 *
 * By: Chanik Bryan Lee
 */

#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

/* typedef struct stringList to be called stringList_t */
typedef struct stringList stringList_t;
/* struct StringList: Linked list of strings (char*) struct */
struct stringList {
    char* string; // will dynamically allocate when creating the struct
    stringList_t *next;
};
/* cacheSimDate_t: typedef for struct cacheSimData, which is a structure
 * (struct) for holding information on the number of hits, misses, and
 * from a cache simulation */
typedef struct cacheSimData {
    int hits;
    int misses;
    int evictions;
} cacheSimData_t;
/* lineInfo_t: typedef for struct lineInfo, which is a structure
 * (struct) for holding information on the operation type and address
 * of a cache trace line that is being evaluated */
typedef struct lineInfo {
    char operation;
    long long address;
} lineInfo_t;
/* cache_t: cache struct to represent a cache with variable size, depending
 * on s, E, and b. A constructor function will properly allocate space and
 * make this cache struct based on the specified parameters. */
typedef struct myCache {
    /* 2D array of cache_new integers for addresses,
     * representing cache, variable size */
    int** cache;
    // Same size as cache_t->cache, but set iff a corresponding element is valid
    int** valid;
    // s: number of set bits
    int s;
} cache_t;
/* cache_new: allocates space on the heap and returns a pointer to a newly
 * constructed cache, given s, E, and b. */
cache_t* cache_new(int s, int E, int b) {
    int rows, cols;
    rows = (int) round(pow(2, s)); // 2^s == S (when ^ is exponent operator)
    cols = E;
    cache_t* res = (cache_t*) malloc(sizeof(cache_t));
    assert(res != NULL);
    /* cache_t cache field */
    int** cacheField = (int**) malloc(rows * sizeof(int*));
    assert(cacheField != NULL);
    unsigned int i, j;
    for (i=0; i<rows; i++) {
        cacheField[i] = (int*) malloc(cols * sizeof(int));
        assert(cacheField[i] != NULL);
        for (j=0; j<cols; j++) {
            cacheField[i][j] = -1; // Initialize all element values to -1
        }
    }
    res->cache = cacheField;
    /* cache_t valid field */
    int** validField = (int**) malloc(rows * sizeof(int*));
    assert(validField != NULL);
    for (i=0; i<rows; i++) {
        validField[i] = (int*) malloc(cols * sizeof(int));
        assert(validField[i] != NULL);
        for (j=0; j<cols; j++) {
            validField[i][j] = 0; // Initialize all valid values to 0
        }
    }
    res->valid = validField;
    res->s = s;
    return res; // On the heap
}
/* cache_free: Given a cache_t pointer of type cache_t*, free the cache_t and
 * its contents from the heap */
int cache_free(cache_t *toFree) {
    if (toFree == NULL) {
        fprintf(stderr, "cache_free: Cannot free a NULL pointer\n");
        exit(1);
    }
    int rows;
    // 2^s == S (when ^ is exponent operator)
    rows = (int) round(pow(2, toFree->s));
    unsigned int i;
    for (i=0; i<rows; i++) {
        free(toFree->cache[i]);
        free(toFree->valid[i]);
    }
    free(toFree->cache);
    free(toFree->valid);
    free(toFree);
    return 0;
}
/* str_to_addr: given a string (char*), returns the address associated with it
 * (removes the last two characters, which are a comma and size and turns them
 * into an int to return) */
long long str_to_addr(char* string) {
    int i = 0;
    while (string[i + 2] != '\0') {
        i++;
    }
    string[i] = '\0';
    return strtoll(string, NULL, 16);
}
/* lineInfo_new: allocate space on the heap and make a new lineInfo_t
 * pointer to return, given an operation and an address (CONSTRUCTOR) */
lineInfo_t* lineInfo_new(char op, char* addr) {
    lineInfo_t* p = (lineInfo_t*) malloc(sizeof(lineInfo_t));
    assert(p != NULL);
    p->operation = op;
    p->address = str_to_addr(addr);
    return p;
} // If a lineInfo_t is called p, free it with simply "free(p);"
/* stringList_new: allocate space on the heap and make a new stringList_t
 * pointer to return, given an input string (CONSTRUCTOR) */
stringList_t* stringList_new(char* str) {
    assert(str != NULL);
    // unsigned int numChars = strlen(str) + 1;
    stringList_t* p = (stringList_t*) malloc(sizeof(stringList_t));
    assert(p != NULL);
    p->string = (char*) malloc(strlen(str) + 1);
    strcpy(p->string, str);
    p->next = NULL; // By convention
    return p;
}
/* stringList_free: free an allocated stringList_t (struct) from the heap */
int stringList_free(stringList_t *strList) {
    stringList_t *m, *n;
    m = strList;
    while (m != NULL) {
        n = m;
        m = m->next;
        free(n->string);
        free(n);
    } // m == NULL
    return 0;
}
/* cacheSimData_new: allocate space on the heap and make a new cacheSimData_t
 * pointer to return, given number of hits, misses, and evictions
 * (CONSTRUCTOR) */
cacheSimData_t* cacheSimData_new(int hitNum, int missNum, int evictionNum) {
    cacheSimData_t *res;
    // Dynamic memory allcoated on the heap
    res = (cacheSimData_t*) malloc(sizeof(cacheSimData_t));
    assert(res != NULL);
    res->hits = hitNum;
    res->misses = missNum;
    res->evictions = evictionNum;
    return res; // Return the pointer
} // If cacheSimData_t pointer is called toFree, free it using "free(toFree);"

/* stringList_append: given last element and a string, append a string to the
 * stringList_t linked list */
void stringList_append(stringList_t* strList, char* str) {
    assert(strList != NULL);
    strList->next = stringList_new(str);
}
/* fileList: takes in a filename string, parses it line by line, and returns a
 * list of relevant strings representing a line from a valgrind memory trace */
stringList_t* fileList(char* filename) {
    // Returns linked list of only relevant file lines
    stringList_t* res = NULL; // Initialize res to NULL
    FILE* traceFile = fopen(filename, "r");
    assert(traceFile != NULL);

    stringList_t *last; // Pointer to the tail of the string linked list

    // Allocated and stored on the stack
    char line[20];
    int numChars;
    while (fgets(line, sizeof(line), traceFile)) {
        // line is an individual line
        numChars = (int) strlen(line); // Includes '\n'
        line[numChars - 1] = '\0';
        if (line[0] == ' ') { // SPACE character
            if (res == NULL) {
                res = stringList_new(line);
                last = res;
            } else {
                stringList_append(last, line);
                last = last->next;
            }
        } else {
            continue;
        }
    }
    fclose(traceFile);
    // Return the constructed list
    return res;
}
/* in_argList: Is a given string in argList? Return an integer that represents 
 * a boolean */
int in_argList(int argc, char* argv[], char* str) {
    int truth = 0;
    for (int i=0; i<argc; i++) {
        if (!strcmp(argv[i], str)) {
            // If both strings are equal
            truth++;
            break; // Leave the loop
        }
    }
    return truth;
}
/* argList_index: Given an arglist (argv), its length, and a string,
 * return the index of the string in the argList if it is in the argList.
 * Assume that the input string is in the argList (assert) */
int argList_index(int argc, char* argv[], char* str) {
    int index = 0;
    assert(in_argList(argc, argv, str)); // assertion for the assumption
    while (index < argc) {
        if (!strcmp(argv[index], str)) {
            break;
        }
        index++;
    }
    return index; // index number in the argList (argv)
}
/* stringParse: Given a string that is to be evaluated from a valgrind trace
 * file, return a lineInfo_t (struct that consists of a character and a string)
 */
lineInfo_t* stringParse(char* str) {
    char* tok1 = strtok(str, " ");
    char* tok2 = strtok(NULL, " ");
    lineInfo_t* res = lineInfo_new(tok1[0], tok2);
    return res;
}
/* Given an address, isolate its set bits using bit shifts and masking
 * to get its set location in a cache */
int addr_to_setNum(long long addr, int s, int b) {
    int mask, count;
    count = b;
    mask = 1;
    while (count > 0) {
        mask = (mask << 1);
        count--;
    }
    count = s;
    while (count > 1) {
        mask = (mask | (mask << 1));
        count--;
    }
    unsigned int res = (addr & mask);
    count = b;
    while (count > 0) {
        res = (res >> 1);
        count--;
    }
    mask = (int) res; // Type-cast: reinterpret as int
    // Check
    int rows = (int) round(pow(2, s)); //2^s == S (when ^ is exponent operator)
    if (res >= rows) {
        fprintf(stderr, "addr_to_setNum: setNum out of bounds\n");
        exit(1);
    }
    return mask; // Set number
}
/* is_set_full: Is the specified cache set full? */
int is_set_full(cache_t *simCache, int setNum, int E) {
    int cols = E;
    int j = 0;
    while (j<cols) {
        if (simCache->valid[setNum][j] == 0) {
            return 0;
        }
        j++;
    }
    return 1;
}
/* Given an address, convert it to return its tag */
int addr_to_tag(long long addr, int s, int b) {
    unsigned count = s + b;
    long long tmp = addr;
    while (count > 0) {
        tmp = (tmp >> 1);
        count--;
    }
    int res = (int) tmp;
    return res;
}
/* simulate: simulate the behavior of a cache memory, given a list of strings
 * representing a line of relevant valgrind memory trace. Returns a pointer to
 * a cacheSimData_t (struct) holding the information on number of hits, misses,
 * and evictions, respectively, for the given simulation.
 *  - Doesn't take care of freeing from the heap after usage (unless allocated
 *    in this function).
 *  - (This is done in main).
 */
cacheSimData_t* simulate(int s, int E, int b, stringList_t *strList)
{
    int hitCount, missCount, evCount, setNum, tag, tmp;
    hitCount = missCount = evCount = 0; // Initialize all to 0
    // Local followers (on the stack)
    lineInfo_t* lineFoll; // holds current value of a line for data reasons
    stringList_t* listPointFoll = strList; // Moves down strList
    // Allocate new cache struct
    cache_t *simCache = cache_new(s, E, b);
    int inSet, i, j;
    int* workingSet;
    int rows = (int) round(pow(2, s));
    int lruMatrix[rows][E]; // On the Stack
    for (i=0; i<rows; i++) {
        for (j=0; j<E; j++) {
            lruMatrix[i][j] = j + 1;
        }
    }
    while (listPointFoll != NULL) { // NULL-terminated list
        // Traverse down the linked list
        lineFoll = stringParse(listPointFoll->string);
        // printf("OPERATION %s\n", listPointFoll->string);
        setNum = addr_to_setNum(lineFoll->address, s, b);
        tag = addr_to_tag(lineFoll->address, s, b);
        /* Determine cache set availability */
        workingSet = simCache->cache[setNum];
        twice:
            inSet = 0;
            for (i=0; i<E; i++) {
                if (workingSet[i] == tag && simCache->valid[setNum][i] == 1) {
                    inSet++;
                    break;
                }
            }
            if (inSet) { // Hit
                tmp = lruMatrix[setNum][i];
                    for (j=0; j<E; j++) {
                        // Increment
                        if (lruMatrix[setNum][j] < tmp) lruMatrix[setNum][j]++;
                    }
                    lruMatrix[setNum][i] = 1;
                    
                hitCount++;
            } else { // Not in set (Miss)
                if (is_set_full(simCache, setNum, E)) {
                    // Then we need to evict but don't worry about valid
                    for (i=0; i<E; i++) {
                        if (lruMatrix[setNum][i] == E) {
                            lruMatrix[setNum][i] = 1;
                            tmp = i; // Save index
                        } else {
                            lruMatrix[setNum][i]++;
                        }
                    } // tmp is evict index
                    workingSet[tmp] = tag;
                    evCount++;
                    missCount++;
                } else {
                    for (i=0; i<E; i++) {
                            if (lruMatrix[setNum][i] == E) {
                                lruMatrix[setNum][i] = 1;
                                tmp = i; // save index
                            } else {
                                lruMatrix[setNum][i]++; // increment
                            }
                        }
                        workingSet[tmp] = tag;
                        simCache->valid[setNum][tmp] = 1;
                    missCount++;
                }
            }
            if (lineFoll->operation == 'M') {
                lineFoll->operation = 'S';
                goto twice;
            }
        listPointFoll = listPointFoll->next;
        free(lineFoll);
    }
    cacheSimData_t* res = cacheSimData_new(hitCount, missCount, evCount);
    assert(0 == cache_free(simCache));
    return res;
}
/* Call simulate and run the cache simulation given command-line arguments */
int main(int argc, char* argv[])
{
    int s, E, b, i, tmp;
    char* filename;
    /* Pull command-line arguments: get filename string */
    if (in_argList(argc, argv, "-t")) {
        i = argList_index(argc, argv, "-t");
    } else {
        fprintf(stderr, "main: -t command-line argument missing\n");
        exit(1);
    }
    tmp = (int) strlen(argv[i + 1]);
    filename = (char*) malloc(tmp + 1);
    assert(filename != NULL);
    unsigned int j;
    for (j=0; j<tmp; j++) {
        filename[j] = argv[i + 1][j];
    }
    filename[j] = '\0';
    /* Pull command-line args: Assign cache data specifications */
    if (in_argList(argc, argv, "-s")) {
        i = argList_index(argc, argv, "-s");
    } else {
        fprintf(stderr, "main: -s command-line argument missing\n");
        exit(1);
    }
    s = atoi(argv[i + 1]);
    if (in_argList(argc, argv, "-E")) {
        i = argList_index(argc, argv, "-E");
    } else {
        fprintf(stderr, "main: -E command-line argument missing\n");
        exit(1);
    }
    E = atoi(argv[i + 1]);
    if (in_argList(argc, argv, "-b")) {
        i = argList_index(argc, argv, "-b");
    } else {
        fprintf(stderr, "main: -b command-line argument missing\n");
        exit(1);
    }
    b = atoi(argv[i + 1]);
    stringList_t *strList = fileList(filename); // This is correct
    cacheSimData_t *outPut = simulate(s, E, b, strList);
    // Arguments: hitCount, missCount, evictionCount
    printSummary(outPut->hits, outPut->misses, outPut->evictions);
    // Free from the heap
    free(filename);
    assert(stringList_free(strList) == 0);
    free(outPut);
    return 0;
}

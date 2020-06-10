/*
**  @(#)util.c
**
**  delimit - utility functions for the text file delimiter
**  -------------------------------------------------------
**
**  copyright (c) 1993-2020 Code Construct Systems (CCS)
*/
#include "delimit.h"

/*
** Debug mode flag
*/
static bool_c_t debug_mode = FALSE;

/*
** Local function prototypes
*/
static void PrintDebugMemoryAllocate(void *, const long);
static void PrintDebugMemoryFree(void *);

/*
** Memory allocate
*/
void *MemoryAllocate(const long size) {
    void *address;

    /*
    ** Allocate heap memory
    */
    address = (void *)malloc(size);
    if (address == NULL) {
        printf("error-> memory allocation failed (%d)\n", errno);
        EXIT_APPLICATION(EXIT_FAILURE);
    }

    /*
    ** Print debug heap memory allocation parameters
    */
    PrintDebugMemoryAllocate(address, size);

    /*
    ** Return allocated heap memory address
    */
    return (address);
}

/*
** Free memory
*/
void MemoryFree(void *address) {
    /*
    ** Print debug heap memory free parameters
    */
    PrintDebugMemoryFree(address);

    /*
    ** Free heap memory if address is not null
    */
    if (address) {
        free(address);
    }
}

/*
** Set memory debug mode
*/
void MemoryDebugMode(const bool_c_t flag) {
    if (flag == TRUE || flag == FALSE) {
        debug_mode = flag;
    }
}

/*
** File copy
*/
void FileCopy(FILE *ifp, FILE *ofp) {
    unsigned char *buffer;
    char message[BUFSIZ];
    size_t count = 0;

    /*
    ** Allocate memory for I/O buffer
    */
    buffer = (unsigned char *)MemoryAllocate(_IO_BUFFER_SIZE);
    if (buffer == NULL) {
        return;
    }

    /*
    ** Process input file until end of file
    */
    while (!feof(ifp)) {
        /*
        ** Read a block of data from input file into I/O buffer
        */
        count = fread(buffer, sizeof(char), _IO_BUFFER_SIZE, ifp);
        if (ferror(ifp)) {
            strerror_p(message, sizeof(message), errno);
            printf("error-> unable to read from input file: %s\n", message);
            break;
        }

        /*
        ** Write a block of data to output file from I/O buffer
        */
        fwrite(buffer, sizeof(char), count, ofp);
        fflush(ofp);
        if (ferror(ofp)) {
            strerror_p(message, sizeof(message), errno);
            printf("error-> unable to write to output file: %s\n", message);
            break;
        }
    }

    /*
    ** Free I/O buffer allocated memory
    */
    MemoryFree((string_c_t)buffer);
}

/*
** Print debug memory allocation parameters
*/
static void PrintDebugMemoryAllocate(void *address, const long size) {
    char message[_MAX_MESSAGE_SIZE];

    if (debug_mode) {
#ifdef _FORMAT_P_SPECIFIER
        strfmt_p(message, sizeof(message), (string_c_t)"%p: MEMORY DEBUG: MemoryAllocate(%ld)\n", address, size);
#else
        strfmt_p(message, sizeof(message), (string_c_t)"%x: MEMORY DEBUG: MemoryAllocate(%ld)\n", address, size);
#endif
        printf("%s", message);
    }
}

/*
** Print debug memory free parameters
*/
static void PrintDebugMemoryFree(void *address) {
    char message[_MAX_MESSAGE_SIZE];

    if (debug_mode) {
#ifdef _FORMAT_P_SPECIFIER
        strfmt_p(message, sizeof(message), (string_c_t)"%p: MEMORY DEBUG: MemoryFree(address)\n", address);
#else
        strfmt_p(message, sizeof(message), (string_c_t)"%x: MEMORY DEBUG: MemoryFree(address)\n", address);
#endif
        printf("%s", message);
    }
}

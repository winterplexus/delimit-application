/*
**  @(#)delimit.c
**
**  delimit - text file delimiter utility
**  -------------------------------------
**
**  copyright (c) 1993-2020 Code Construct Systems (CCS)
*/
#include "delimit.h"

/*
** Unwind buffer stack pointer
*/
jmp_buf unwind_buffer_sp;

/*
** Local function prototypes
*/
static void ProcessFiles(options_t *, delimit_specs_t *);
static void OpenFiles(options_t *, delimit_specs_t *);
static void ParseFormatFile(delimit_specs_t *);
static void SetDelimiters(options_t *, delimit_specs_t *);
static void CloseFiles(delimit_specs_t *);
static void FreeLexicalBuffers(void);
static void OverWriteInputFile(options_t *, delimit_specs_t *);
static void SetOptions(void);
static void SetSystemSignals(void);
static void InterruptHandler(int);
static void DisplayTotals(options_t *, delimit_specs_t *);

/*
** Text file delimiter utility driver
*/
int main(int argc, string_c_t argv[]) {
    options_t opts;
    delimit_specs_t ds;

    /*
    ** Set system signals and if stack was unwound, close files and exit
    */
    SetSystemSignals();
    if (setjmp(unwind_buffer_sp) != 0) {
        CloseFiles(&ds);
        FreeLexicalBuffers();
        return (EXIT_FAILURE);
    }

    /*
    ** Set options (special)
    */
    SetOptions();

    /*
    ** Get options
    */
    GetOptions(argc, argv, &opts);

    /*
    ** Process input and output files
    */
    ProcessFiles(&opts, &ds);
}

/*
** Process files
*/
static void ProcessFiles(options_t *opts, delimit_specs_t *ds) {
    /*
    ** Open files
    */
    OpenFiles(opts, ds);

    /*
    ** Parse format file
    */
    ParseFormatFile(ds);

    /*
    ** Set delimiters
    */
    SetDelimiters(opts, ds);

    /*
    ** Delimit file
    */
    DelimitFile(ds);

    /*
    ** Over-write input file
    */
    OverWriteInputFile(opts, ds);

    /*
    ** Display totals
    */
    DisplayTotals(opts, ds);

    /*
    ** Free dictionary
    */
    FreeDictionary();

    /*
    ** Close files
    */
    CloseFiles(ds);
}

/*
** Open files
*/
static void OpenFiles(options_t *opts, delimit_specs_t *ds) {
    /*
    ** Open input and output files and if over-write option is true, open input file with both read and write capability
    */
    if (DelimitOpen(ds, opts->input, opts->output, opts->overwrite) != EXIT_SUCCESS) {
        DelimitClose(ds);
        EXIT_APPLICATION(EXIT_FAILURE);
    }

    /*
    ** Open format file for parser
    */
    fopen_p(&yyin, opts->format, (string_c_t)_F_RO);
    if (yyin == NULL) {
        perror(opts->format);
        EXIT_APPLICATION(EXIT_FAILURE);
    }
}

/*
** Parse format file
*/
static void ParseFormatFile(delimit_specs_t *ds) {
    /*
    ** Parser initialization
    */
    yyparseinit(ds);

    /*
    ** Parse format file
    */
    if (yyparse()) {
        CloseFiles(ds);
        EXIT_APPLICATION(EXIT_FAILURE);
    }

    /*
    ** Print dictionary if required
    */
#ifdef _DEBUG_DICTIONARY
    PrintDictionary();
#endif
}

/*
** Set delimiters
*/
static void SetDelimiters(options_t *opts, delimit_specs_t *ds) {
    /*
    ** Set unique character delimiter
    */
    if (DelimitSetUnique(ds, opts->unique, opts->unique_delimiter) != EXIT_SUCCESS) {
        DelimitClose(ds);
        EXIT_APPLICATION(EXIT_FAILURE);
    }

    /*
    ** Set other delimiters
    */
    DelimitSetTab(ds, opts->tab);
    DelimitSetComma(ds, opts->comma);
    DelimitSetSpace(ds, opts->space);
    DelimitSetDouble(ds, opts->double_quote);
    DelimitSetSingle(ds, opts->single_quote);
}

/*
** Close files
*/
static void CloseFiles(delimit_specs_t *ds) {
    /*
    ** Flush and close delimiter file
    */
    fflush(yyin);
    fclose_p(yyin);

    /*
    ** Close input and output files
    */
    DelimitFileClose(ds);
}

/*
** Free lexical buffers
*/
static void FreeLexicalBuffers(void) {
    /*
    ** Free lexical scanner buffers
    */
#ifdef _FREE_LEXICAL_BUFFERS
    yylex_destroy();
#endif
}

/*
** Over-write input file
*/
static void OverWriteInputFile(options_t *opts, delimit_specs_t *ds) {
    if (opts->overwrite) {
        /*
        ** Close input and output temporary files
        */
        DelimitFileCloseInput(ds);
        if (DelimitFileOpenInputAsOutput(ds) != EXIT_SUCCESS) {
            return;
        }

        /*
        ** Rewind output file and write output file over input file
        */
        rewind(ds->output.fp);
        FileCopy(ds->output.fp, ds->input.fp);
    }
}

/*
** Set options (special)
*/
static void SetOptions(void) {
#ifdef _DEBUG_MEMORY
    MemoryDebugMode(TRUE);
#endif
}

/*
** Set system signals
*/
static void SetSystemSignals(void) {
    signal(SIGINT, InterruptHandler);
    signal(SIGTERM, InterruptHandler);
}

/*
** Interrupt handler
*/
static void InterruptHandler(int signal_number) {
    printf("signal detected (%d)!\n", signal_number);
    EXIT_APPLICATION(EXIT_SUCCESS);
}

/*
** Display totals
*/
static void DisplayTotals(options_t *opts, delimit_specs_t *ds) {
    if (opts->statistics) {
        printf("statistics\n");
        printf("----------\n");
        printf("input file  ->  ");
        printf("total lines: %010ld\t", ds->input.line_count);
        printf("total count: %010ld\n", ds->input.counter);
        printf("output file ->  ");
        printf("total lines: %010ld\t", ds->output.line_count);
        printf("total count: %010ld\n\n", ds->output.counter);
    }
}

#include "chunk_file.h"

#include <stdio.h>
#include <stdlib.h>

#ifndef CELLOX_TESTS_RUNNING
#ifndef BENCHMARKS_RUNNING
#include "cellox_config.h"
#endif
#endif

#include "object.h"


/// @brief Chunk segment prefixes
typedef enum
{
    /// Prefix of a bytecode segment
    CHUNK_SEGMENT_TYPE_BYTECODE = 2,
    /// Prefix of a constant segment
    CHUNK_SEGMENT_TYPE_CONSTANTS = 0,
    /// @brief Prefix of a inner seqment
    /// @details These are functions that are nested inside another function or script
    CHUNK_SEGMENT_TYPE_INNER = 1   
}chunk_segment_prefix_t;

/// @brief Constant prefixes for cellox
typedef enum
{
    /// Prefix of a class constant
    CONSTANT_TYPE_CLASS,
    /// Prefix of a function constant
    CONSTANT_TYPE_FUNCTION,
    /// Prefix of a numerical constant
    CONSTANT_TYPE_NUMBER,
    /// Prefix of a string constant
    CONSTANT_TYPE_STRING
}constant_type_prefix_t;

/*
 * TODO: 
 * Add line info add think about what robert wrote
 * Error handling imrpovements
 * Add a name reference strucure for constants
 */

static void chunk_file_append_chunk(chunk_t, chunk_file_compile_flags, FILE *);
static void chunk_file_append_code_segment(uint8_t *, uint32_t, FILE *);
static void chunk_file_append_constant(value_t,  dynamic_value_array_t *, chunk_file_compile_flags, FILE *);
static void chunk_file_append_constant_segment(dynamic_value_array_t, dynamic_value_array_t *, chunk_file_compile_flags, FILE *);
static void chunk_file_append_function_meta_data(object_function_t, FILE *);
static void chunk_file_append_inner_segment(dynamic_value_array_t, chunk_file_compile_flags, FILE *);
static void chunk_file_append_meta_data(chunk_file_compile_flags, FILE *);

int chunk_file_store(chunk_t chunk, char const * programmPath, chunk_file_compile_flags flags)
{        
    if(flags & COMPILE_FLAG_LINE_INFO_INCLUDED || flags & COMPILE_FLAG_ANONYMIZE_FUNCTIONS || flags & COMPILE_FLAG_OPTIMIZE )
    {
        fprintf(stderr, "Compile options not implemented yet");
        return -1;
    }
    FILE * filePointer;
    size_t fileNameLength = strlen(programmPath) + 2;
    char * filename = malloc(fileNameLength);
    for (size_t i = 0; i < fileNameLength - 2; i++)
        filename[i] = programmPath[i];
    filename[fileNameLength - 4] = 'x';
    filename[fileNameLength - 3] = 'c';
    filename[fileNameLength - 2] = 'f';
    filename[fileNameLength - 1] = '\0';
    filePointer = fopen(filename, "w");

    if(!filePointer)
    {
        printf("Unable to create file.\n");
        return - 1;
    }
    chunk_file_append_meta_data(flags, filePointer);
    chunk_file_append_chunk(chunk, flags, filePointer);

    fclose(filePointer);
    free(filename);
    return 0;
}

chunk_t * chunk_file_load(char const * filePath)
{
    chunk_t * mainChunk = malloc(sizeof(chunk_t));
    if(!mainChunk)
        return mainChunk;
    // TODO: Read a cellox bytecode file and prepare the chunk accordingly
    return mainChunk;
}

/**
 * @brief Writes a single chunk and all its child chunks to a file
 * @details A chunk that is stored in a file has the following structure
 * ----constants-----
 * ----inner---------
 * ----bytecode------
 * @param chunk The chunk that is written to a file
 * @param flags Compile flags used to compile the sourcecode to a cellox chunk file
 * @param filePointer Pointer to the file
 */
static void chunk_file_append_chunk(chunk_t chunk,  chunk_file_compile_flags flags, FILE * filePointer)
{
    dynamic_value_array_t functions;
    dynamic_value_array_init(&functions);
    chunk_file_append_constant_segment(chunk.constants, &functions, flags, filePointer);

    if(functions.count)
        chunk_file_append_inner_segment(functions, flags, filePointer);
        
    chunk_file_append_code_segment(chunk.code, chunk.count, filePointer);
}

/// @brief Appends the bytecode stored in a chunk to the file
/// @param code Pointer to the bytecode of the chunk
/// @param codeSize The amount of bytecode instructions stored in the chunk
/// @param filePointer Pointer to the file
static void chunk_file_append_code_segment(uint8_t * code, uint32_t codeSize, FILE * filePointer)
{
    fputc(CHUNK_SEGMENT_TYPE_BYTECODE, filePointer);
    for (uint32_t i = 0; i < codeSize; i++)
        fputc((char) code[i], filePointer);
    fputc(255, filePointer);
}

/// @brief Appends a single constant to a cellox bytecode file
/// @param value The value of the constant that is appended
/// @param functions Pointer to an dynamic array of functions that are used to build the inner segmment of the chunk later
/// @param flags Compile flags used to compile the sourcecode to a cellox chunk file
/// @param filePointer Pointer to the file
static void chunk_file_append_constant(value_t value, dynamic_value_array_t * functions, chunk_file_compile_flags flags, FILE * filePointer)
{
    if(IS_OBJECT(value))
    {
        switch (OBJECT_TYPE(value))
        {
            case OBJECT_FUNCTION:
                dynamic_value_array_write(functions, value);
                break;
            case OBJECT_STRING:
                fputc(CONSTANT_TYPE_STRING, filePointer);
                fputs(AS_CSTRING(value), filePointer);
                fputc(0, filePointer);
                break;
            default:
                fprintf(stderr, "Object type not spupported");
                exit(EXIT_CODE_COMPILATION_ERROR);
        }
    }
    else // Numbers
    {
        fputc(CONSTANT_TYPE_NUMBER, filePointer);
        uint64_t number = AS_NUMBER(value);
        fputc((number & 0xff00000000000000) >> 56, filePointer);
        fputc((number & 0x00ff000000000000) >> 48, filePointer);
        fputc((number & 0x0000ff0000000000) >> 40, filePointer);
        fputc((number & 0x000000ff00000000) >> 32, filePointer);
        fputc((number & 0x00000000ff000000) >> 24, filePointer);
        fputc((number & 0x0000000000ff0000) >> 16, filePointer);
        fputc((number & 0x000000000000ff00) >> 8, filePointer);
        fputc(number  & 0x00000000000000ff, filePointer);
    }
}

/// @brief Appends the constants segment of a chunk to the file
/// @param constants The constants that are stored in the chunk
/// @param functions Pointer to an dynamic array of functions that are used to build the inner segmment of the chunk later
/// @param flags Compile flags used to compile the sourcecode to a cellox chunk file
/// @param filePointer Pointer to the file
static void chunk_file_append_constant_segment(dynamic_value_array_t constants, dynamic_value_array_t * functions, chunk_file_compile_flags flags, FILE * filePointer)
{
    fputc(CHUNK_SEGMENT_TYPE_CONSTANTS, filePointer);
    for (size_t i = 0; i < constants.count; i++)   
        chunk_file_append_constant(constants.values[i], functions, flags, filePointer);
    
    fputc(255, filePointer);
}

/// @brief Appends the meta data of a function to the file (arity and upvalue count)
/// @param function The function that has it's metadata appended to the file 
/// @param filePointer Pointer to the file
static void chunk_file_append_function_meta_data(object_function_t function, FILE * filePointer)
{
    fputs(function.name->chars, filePointer);
    fputc(0, filePointer);
    fputc((function.arity & 0xff000000) >> 24, filePointer);
    fputc((function.arity & 0x00ff0000) >> 16, filePointer);
    fputc((function.arity & 0x0000ff00) >> 8, filePointer);
    fputc(function.arity &  0x000000ff, filePointer);
    fputc((function.upvalueCount & 0xff000000) >> 24, filePointer);
    fputc((function.upvalueCount & 0x00ff0000) >> 16, filePointer);
    fputc((function.upvalueCount & 0x0000ff00) >> 8, filePointer);
    fputc(function.upvalueCount &  0x000000ff, filePointer);
}

/// @brief Appends the inner segment of the chunk to tthe file
/// @param functions The inner functions of the chunk
/// @param flags Compile flags used to compile the sourcecode to a cellox chunk file
/// @param filePointer Pointer to the file
static void chunk_file_append_inner_segment(dynamic_value_array_t functions, chunk_file_compile_flags flags, FILE * filePointer)
{
    fputc(CHUNK_SEGMENT_TYPE_INNER, filePointer);
    for (size_t i = 0; i < functions.count; i++)
    {
        chunk_file_append_function_meta_data(*AS_FUNCTION(functions.values[i]), filePointer);
        chunk_file_append_chunk(AS_FUNCTION(functions.values[i])->chunk, flags, filePointer);
    }
    fputc(255, filePointer);
}

/// @brief Appends the meta data of a cellox bytecode file (chunk file writer options and the cellox version)
/// @param flags Compile flags used to compile the sourcecode to a cellox chunk file
/// @param filePointer Pointer to the file
static void chunk_file_append_meta_data(chunk_file_compile_flags flags, FILE * filePointer)
{
    fputc(flags, filePointer);
    #ifndef CELLOX_TESTS_RUNNING
    #ifndef BENCHMARKS_RUNNING
    fputc(PROJECT_VERSION_MAJOR, filePointer);    
    fputc(PROJECT_VERSION_MINOR, filePointer);
    #endif
    #endif
}

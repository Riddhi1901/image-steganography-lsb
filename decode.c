#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "decode.h"
#include "types.h"
#include "common.h"
#define GREEN "\033[0;32m"
#define RESET "\033[0m"
/* Function Definitions */

/* * Read and validate Decode args from argv
 * Input: argv and DecodeInfo structure
 * Output: Status (e_success/e_failure)
 */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *dncInfo)
{
    // Validate source image file extension (.bmp)
    if(argv[2] != NULL && strstr(argv[2], ".bmp") != NULL)
    {
        dncInfo->stego_image_fname = argv[2];
    }
    else
    {
        return e_failure;
    }

    // Validate secret file name 
    if(argv[3] != NULL)
    {
        strcpy(dncInfo->output_secret_fname, argv[3]);
    }
    else
    {
        // Default name if not provided 
        strcpy(dncInfo->output_secret_fname, "decoded.txt");
    }

    return e_success;
}

/* * Get File pointers for i/p and o/p files
 * Note: Only opens the stego image image initially. 
 * The secret file is opened later after decoding the extension.
 */
Status open_files_decode(DecodeInfo *dncInfo)
{
    // Src Image file
    dncInfo->fptr_stego_image = fopen(dncInfo->stego_image_fname, "r");
    
    // Do Error handling
    if(dncInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", dncInfo->stego_image_fname);
        return e_failure;
    }

    return e_success;
}

/* * Decode a byte from LSB of image data array 
 * Input: Pointer to 8 bytes of image data
 * Returns: The decoded character (casted to Status/int for function signature compatibility)
 */
Status decode_byte_to_lsb(char *image_buffer)
{
    char data = 0;
    for(int i = 0; i < 8; i++)
    {
        // Get LSB from image byte and shift it to the correct position
        data = data | (image_buffer[i] & 1) << i;
    }
    return (Status)data; // Return the decoded character
}

/* * Decode data to image 
 * Input: Buffer to store decoded data, size (bytes to decode), dncInfo
 */
Status decode_data_to_image(char *data, int size, DecodeInfo *dncInfo)
{
    char image_buffer[8];
    for(int i = 0; i < size; i++)
    {
        // Read 8 bytes from source image
        fread(image_buffer, 8, 1, dncInfo->fptr_stego_image);
        // Decode 1 byte of data from those 8 bytes
        data[i] = (char)decode_byte_to_lsb(image_buffer);
    }
    data[size] = '\0'; // Null terminate string
    return e_success;
}

/*  Decode Magic String 
 * checks if the decoded string matches MAGIC_STRING
 */
Status decode_magic_string(char *magic_string, DecodeInfo *dncInfo)
{
    // Skip the 54-byte BMP header
    fseek(dncInfo->fptr_stego_image, 54, SEEK_SET);

    int len = strlen(magic_string);
    char *decoded_string = malloc(len + 1);

    decode_data_to_image(decoded_string, len, dncInfo);

    if(strcmp(decoded_string, magic_string) == 0)
    {
        free(decoded_string);
        return e_success;
    }
    else
    {
        free(decoded_string);
        return e_failure;
    }
}

/* Decode size from LSB 
 * Reads 32 bytes from image and reconstructs an integer
 */
Status decode_size_to_lsb(long *size, DecodeInfo *dncInfo)
{
    char image_buffer[32];
    fread(image_buffer, 32, 1, dncInfo->fptr_stego_image);
    
    int value = 0;
    for(int i = 0; i < 32; i++)
    {
        value = value | (image_buffer[i] & 1) << i;
    }
    
    *size = value;
    return e_success;
}

/* Decode secret file extension size */
Status decode_secret_file_extn_size(long size, DecodeInfo *dncInfo)
{
    // We ignore the 'size' parameter passed in and read from file instead
    long ext_size;
    decode_size_to_lsb(&ext_size, dncInfo);
    dncInfo->extension_size = (int)ext_size;
    return e_success;
}

/* Decode secret file extension */
Status decode_secret_file_extn(char *file_extn, DecodeInfo *dncInfo)
{
    decode_data_to_image(dncInfo->secret_file_extn, dncInfo->extension_size, dncInfo);
    return e_success;
}

/* Decode secret file size */
Status decode_secret_file_size(long file_size, DecodeInfo *dncInfo)
{
    long secret_size;
    decode_size_to_lsb(&secret_size, dncInfo);
    dncInfo->size_secret_file = secret_size;
    return e_success;
}

/* Decode secret file data */
Status decode_secret_file_data(DecodeInfo *dncInfo)
{
    // Allocate memory for the secret data
    char *secret_data = malloc(dncInfo->size_secret_file + 1);
    if(secret_data == NULL)
    {
        return e_failure;
    }
    // Decode the data
    decode_data_to_image(secret_data, dncInfo->size_secret_file, dncInfo);
    // Write to the output file
    fwrite(secret_data, dncInfo->size_secret_file, 1, dncInfo->fptr_output_secret);
    free(secret_data);
    return e_success;
}

/* Perform the decoding */
Status do_decoding(DecodeInfo *dncInfo)
{
    if(open_files_decode(dncInfo) == e_success)
    {
        printf("Open files ia a success\n");
    }
    else
    {
        printf("Failed to open files\n");
        return e_failure;
    }
    if(decode_magic_string(MAGIC_STRING, dncInfo) == e_success)
    {
        printf("Decoded magic string successfully\n");
    }
    else
    {
        printf("Failed to decode magic string\n");
        return e_failure;
    }
    if(decode_secret_file_extn_size(0, dncInfo) == e_success)
    {
        printf("Decoded secret file extension size successfully\n");
    }
    else
    {
        printf("Failed to decode secret file extension size\n");
        return e_failure;
    }
    if(decode_secret_file_extn(NULL, dncInfo) == e_success)
    {
        printf("Decoded secret file extension successfully\n");
    }
    else
    {
        printf("Failed to decode secret file extension\n");
        return e_failure;
    }
    //open output file
    dncInfo->fptr_output_secret = fopen(dncInfo->output_secret_fname, "w");
    if(dncInfo->fptr_output_secret == NULL)
    {
        perror("fopen secret file");
        return e_failure;
    }
    printf(GREEN "Created output file: %s\n" RESET, dncInfo->output_secret_fname);
    if(decode_secret_file_size(0, dncInfo) == e_success)
    {
        printf("Decoded secret file size successfully\n");
    }
    else
    {
        printf("Failed to decode secret file size\n");
        return e_failure;
    }
    if(decode_secret_file_data(dncInfo) == e_success)
    {
        printf("Decoded secret file data successfully\n");
    }
    else
    {
        printf("Failed to decode secret file data\n");
        return e_failure;
    }
    // Cleanup
    fclose(dncInfo->fptr_stego_image);
    fclose(dncInfo->fptr_output_secret);
    return e_success;
}
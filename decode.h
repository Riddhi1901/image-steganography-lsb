#ifndef DECODE_H
#define DECODE_H

#include "types.h" // Contains user defined types

/* * Structure to store information required for
 * decoding secret data from a Stego Image.
 */

#define MAX_SECRET_BUF_SIZE 1
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)
#define MAX_FILE_SUFFIX 4

typedef struct _DecodeInfo
{
    /* Stego Image Info (INPUT) */
    char *stego_image_fname;      // Name of the image containing hidden data
    FILE *fptr_stego_image;       // File pointer to read the stego image

    /* Decoded Output Info (OUTPUT) */
    char output_secret_fname[20]; // Name of the file to save decoded secret
    FILE *fptr_output_secret;     // File pointer to write the secret data
    
    /* Internal Decoding Data */
    char secret_file_extn[MAX_FILE_SUFFIX + 1];
    char secret_data[MAX_SECRET_BUF_SIZE];
    long size_secret_file;
    int extension_size;
    
    

} DecodeInfo;


/* Decoding function prototype */

/* Check operation type */
OperationType check_operation_type(char *argv[]); 

/* Read and validate Decode args from argv */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *dnccInfo); 

/* Perform the decoding */
Status do_decoding(DecodeInfo *dncInfo);

/* Get File pointers for i/p and o/p files */
Status open_files_decode(DecodeInfo *dncInfo);

/* Store Magic String */
Status decode_magic_string(char *magic_string, DecodeInfo *dncInfo);

/* Decode secret file extenstion */
Status decode_secret_file_extn(char *file_extn, DecodeInfo *dncInfo);

/* Dncode secret file extension size*/
Status decode_secret_file_extn_size(long size, DecodeInfo *dncInfo);

/* Decode secret file size */
Status decode_secret_file_size(long file_size,  DecodeInfo *dncInfo);

/* Decode secret file data*/
Status decode_secret_file_data(DecodeInfo *dncInfo);

/* Decode function, which does the real encoding */
Status decode_data_to_image(char *data, int size, DecodeInfo *dncInfo);

/* Decode a byte from LSB of image data array */
Status decode_byte_to_lsb(char *image_buffer);

/* Decode size to LSB*/
Status decode_size_to_lsb(long *size,  DecodeInfo *dncInfo);

#endif

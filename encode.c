#include <stdio.h>
#include<string.h>
#include "encode.h"
#include "types.h"
#include "common.h"
#define GREEN "\033[0;32m"
#define RESET "\033[0m"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    //argv[2] -> .bmp -> store argv[2] in structure encInfo->src_image_fname = argv[2];
    if(argv[2] != NULL && strstr(argv[2],".bmp") != NULL)
    {
        encInfo->src_image_fname = argv[2];
    }
    else
    {
        return e_failure;
    }
    //argv[3] -> .txt -> encInfo->secret_fname = argv[3] -> strcpy(encInfo->extn_secret_file, strstr(argv[3], "."))
    if(argv[3] != NULL && strstr(argv[3],".txt")!=NULL)
    {
        encInfo->secret_fname = argv[3];
    }
    else
    {
        return e_failure;
    }
    //   -> no NULL -> .bmp -> store argv[4] in structure
    if(argv[4] != NULL)
    {
        encInfo->stego_image_fname = argv[4];
    }
    else 
    {
        encInfo->stego_image_fname = "stego.bmp";  //argv[4] -> NULL-> store "stego.bmp" in structure
    }

    return e_success;
}

uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

    	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

    	return e_failure;
    }

    // No failure return e_success
    return e_success;
}

uint get_file_size(FILE *fptr)
{
    fseek(fptr, 0,SEEK_END);

    return ftell(fptr);
}

Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);

    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);

    if((encInfo->image_capacity-54) > (strlen(MAGIC_STRING) + sizeof(int) + strlen(encInfo->exten_secret_file) + sizeof(int) + encInfo->size_secret_file )*8)
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    fseek(fptr_src_image,0,SEEK_SET);
    char str[54];
    fread(str,54,1,fptr_src_image);
    fwrite(str,54,1,fptr_dest_image);
    return e_success;
}

Status encode_data_to_image(char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
    char arr[8];
    for(int i = 0; i < size; i++)
    {
        fread(arr, 8, 1, fptr_src_image);
        encode_byte_to_lsb(data[i], arr);
        fwrite(arr, 8, 1, fptr_stego_image);
    }
}

Status encode_magic_string(char *magic_string, EncodeInfo *encInfo)
{
    encode_data_to_image(MAGIC_STRING, strlen(MAGIC_STRING), encInfo->fptr_src_image, encInfo->fptr_stego_image);
    return e_success;
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for(int i=0;i<8;i++)
    {
        image_buffer[i] = image_buffer[i] & 0xFE | ((data>>i) & 1);
    }
}

Status encode_size_to_lsb(long size, EncodeInfo *encInfo)
{
    char str[32];
    fread(str,32,1,encInfo->fptr_src_image);
    for(int i=0;i<32;i++)
    {
        str[i] = (str[i] & 0xFE) | (size>>i) & 1;
    }
    fwrite(str,32,1,encInfo->fptr_stego_image);
}

Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{
    encode_size_to_lsb(size,encInfo);
    return e_success;
}

Status encode_secret_file_extn(char *file_extn, EncodeInfo *encInfo)
{
    
    encode_data_to_image(file_extn, strlen(file_extn), encInfo->fptr_src_image, encInfo->fptr_stego_image);
    return e_success;   
}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    encode_size_to_lsb(file_size,encInfo);
    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char str[encInfo->size_secret_file];
    fseek(encInfo->fptr_secret,0,SEEK_SET);
    fread(str,encInfo->size_secret_file,1,encInfo->fptr_secret);
    encode_data_to_image(str, encInfo->size_secret_file, encInfo->fptr_src_image, encInfo->fptr_stego_image);
    return e_success;
}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    while(fread(&ch,1,1,fptr_src)>0)
    {
        fwrite(&ch,1,1,fptr_dest);
    }
    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
    // open_files() == e_failure -> error -> return e_failure
    if(open_files(encInfo) == e_success)
    {
        printf("Open files is a success\n");
    }
    else
    {
        printf("Failed to open files\n");
        return e_failure;
    }
    printf(GREEN "Created output file: %s\n" RESET, encInfo->stego_image_fname);
    //check_capacity() == e_failure -> error -> return e_failure
    if(check_capacity(encInfo) == e_success)
    {
        printf("Check capacity successful\n");
    }
    else
    {
        printf("Check capacity is a failure\n");
        return e_failure;
    }
    //copy_bmp_header() == e_failure -> error -> return e_failure
    if(copy_bmp_header(encInfo->fptr_src_image,encInfo->fptr_stego_image) == e_success)
    {
        printf("Copied bmp header successfully\n");
    }
    else
    {
        printf("Failed to copy bmp header\n");
        return e_failure;
    }
    //encode_magic_string() == e_failure -> error -> return e_failure
    if(encode_magic_string(MAGIC_STRING, encInfo) == e_success)
    {
        printf("Encoded magic string successfully\n");
    }
    else
    {
        printf("Failed to encode magic string\n");
        return e_failure;
    }
    strcpy(encInfo->exten_secret_file,strchr(encInfo->secret_fname,'.'));
    if(encode_secret_file_extn_size(strlen(encInfo->exten_secret_file),encInfo) == e_success)
    {
        printf("Encoded secret file extension size successfully\n");
    }
    else
    {
        printf("Failed to encode secret file extension size\n");
        return e_failure;
    }
    if(encode_secret_file_extn(encInfo->exten_secret_file, encInfo) == e_success)
    {
        printf("Encoded secret file extension successfully\n");
    }
    else
    {
        printf("Failed to encode secret file extension\n");
        return e_failure;
    }
    if(encode_secret_file_size(encInfo->size_secret_file,encInfo) == e_success)
    {
        printf("Encoded secret file size sucessfully\n");
    }
    else
    {
        printf("Failed to encode secret file size\n");
        return e_failure;
    }
    if(encode_secret_file_data(encInfo) == e_success)
    {
        printf("Encoded secret file data successfully\n");
    }
    else
    {
        printf("Failed to encode secret file data\n");
        return e_failure;
    }
    if(copy_remaining_img_data(encInfo->fptr_src_image,encInfo->fptr_stego_image) == e_success)
    {
        printf("Copied remaining data successfully\n");
    }
    else
    {
        printf("Failed to copy remaining data\n");
        return e_failure;
    }
    return e_success;
}
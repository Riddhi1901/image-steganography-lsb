/*
    Name: Riddhi Balaji Shitole
    Project Name Steganography
    Batch: 25034f
    Admission Number: 25034f_036
*/
#include <stdio.h>
#include<string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

OperationType check_operation_type(char *argv[])
{
    if(strcmp(argv[1],"-e")==0) // Check if the 2nd argument is "-e"
    {
        return e_encode; // Return encode opration type
    }
    else if(strcmp(argv[1],"-d")==0)  // Check if the 2nd argument is "-d"
    {
        return e_decode; // Return deode operation type
    }
    else
    {
        return e_unsupported; // Return unsuported if argument doesn't match
    }
}

int main(int argc, char *argv[])
{
     //res = check_operationType(argv);
    if(argc<2)
    {
        printf("Invalid arguments input.\n");
        printf("For encoding : ./a.out -e image_file_name.bmp secret_text_file_name.txt [stego.bmp]\n");
        printf("For decoding : ./a.out -d stego.bmp [decode.txt]\n");
        return 0;
    }
    //res -> e_encode -> read_and_validate_encodde_arg(); -> do_encoding();
    if(check_operation_type(argv) == e_encode)
    {
        if(argc<4)
        {
            printf("Not enough arguments\n");
            return 0;
        }
        printf("Operation Selected: Encoding\n");
        EncodeInfo encode;
        if(read_and_validate_encode_args(argv,&encode) == e_success)
        {
            printf("Successfully read and validated encode arguments.\n");
            printf("--------------Encoding Process Started------------------\n");
            if(do_encoding(&encode) == e_success)
            {
                printf("Encoding completed successfully!\n");
            }
            else
            {
                printf("Failed to perform encoding\n");
            }
        }
    }

    //res -> e_decode -> read_and_validate_decode_arg();  -> do_decoding();
    if(check_operation_type(argv) == e_decode)
    {
        if(argc<3)
        {
            printf("Not enough arguments\n");
            return 0;
        }
        printf("Operation Selected: Decoding\n");
        DecodeInfo decode;
        if(read_and_validate_decode_args(argv,&decode) == e_success)
        {
            printf("Successfully read and validated decode arguments.\n");
            printf("--------------Decoding Process Started------------------\n");
            if(do_decoding(&decode) == e_success)
            {
                printf("Decoding completed successfully!\n");
            }
            else
            {
                printf("Failed to perform Decoding\n");
            }
        }
    }
    //res -> e_usupported -> error -> terminate
    return 0;
}

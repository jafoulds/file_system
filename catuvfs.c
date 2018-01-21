/*
July 31, 2017
Jared Foulds
CSC 360 Assignment #4, Task #3
Michael Zastre

This program  is responsible for outputting the contents of a file in the disk image to the 
host console's stdout stream.
*/

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include "disk.h"
#include <stdbool.h>

int byte_position;
int times_run;
int DIR_start;
int block_size;
int num_blocks;
int number_of_DIR;
int current_status;
int starting_block;
int FAT_start;
int total_blocks;
int DIR_byte_position;

bool is_end_of_file;
bool is_file_name_match;

/*
This function is responsible for extracting a certain section of the superblock 
and is dependent on the byte size passed. It then converts that value to decimal
and depending on what part of the superblock and then is stored in a global variable.
*/
void extract_superblock_vals(FILE * f, int start, long int size)
{
	times_run ++;
	byte_position = byte_position + size;

	int i;
	int a;
	int sum =0;
	int position = 1;

	unsigned char list[size];

    fseek(f, start, 0);
    if (fread(list, 1, size, f) <= 0) {
        fprintf(stderr, "problems reading directory from image\n");
        exit(1);
    }
	if (size > 2){
		position = size -1;
	}

	for (i = 0; i < size; i++) {
		if(position > 0){
			a =  256 * position * list[i];
		}
		else{
			a = list[i];
		}
		sum = sum + a;
		position --;
	}	

	if(times_run == 1){
		block_size = sum;
	}
	if(times_run == 2){
		total_blocks = sum;
	}
	if(times_run == 3){
		FAT_start = sum;
	}
	if(times_run == 4){
		num_blocks = sum;
	}
	if(times_run == 5){
		DIR_start = sum;
	}
	if(times_run == 6){
		number_of_DIR = sum;
	}
}
/*
This function is responsible for outputting the data held in a certain data block.
*/
void print_file_data(FILE * f, int start, long int size)
{
	int i;
	unsigned char list[size+1];

    fseek(f, start, 0);
    if (fread(list, 1, size, f) <= 0) {
        fprintf(stderr, "problems reading directory from image\n");
        exit(1);
    }

	for (i = 0; i < size; i++) {
		fprintf(stdout, "%c", list[i]);
	}
}

/*
This function is responsible for determining the if the file name passed
as a command line argument matches any of the file names in any of the root directory entries.
*/
void check_file_name(FILE * f, int start, long int size, char * test)
{
	byte_position = byte_position + size;

	int length = strlen(test);
	char list[length+1];

    fseek(f, start, 0);
    if (fread(list, 1, length+1, f) <= 0) {
        fprintf(stderr, "problems reading directory from image\n");
        exit(1);
    }

	else {
		list[length+1] = '\0';
        if (strcmp(list, test) == 0) {
		is_file_name_match = true;
        } 
    }	
}

/*
This function is responsible for extracting caclulating the starting block
so the program knows where to print the file name's data from.
*/
void get_starting_block (FILE * f, int start, long int size)
{
	byte_position = byte_position + size;
	int i;
	unsigned char list[size];
	fseek(f, start, 0);
    if (fread(list, 1, size, f) <= 0) {
        fprintf(stderr, "problems reading directory from image\n");
        exit(1);
    }
        int a;
        int sum = 0;

        int position = size - 1;
        
        for (i = 0; i < size; i++) {
                //printf("HEX: %X\n", list[i]);//print hex
                if(position > 0){
                        a =  256 * position * list[i];
                }
                else{
                        a = list[i];
                }
                sum = sum + a;
                position --;
        }
	starting_block = sum;
}

/*
This function is responsible for checking a FAT entry and determing if the value
in that FAT entry is allocated (i.e. continue checking) or represents the last block of a file 
(i.e. stop checking).
*/
void check_FAT(FILE * f, int start, long int size)
{
	int i;
	unsigned char list[size];

	unsigned int a;
	unsigned int sum = 0;
	unsigned int two_fifty_six = 256;
	unsigned int max = 4294967040;
	int position = size -1;

    fseek(f, start, 0);
    if (fread(list, 1, size, f) <= 0) {
        fprintf(stderr, "problems reading directory from image\n");
        exit(1);
    }
	
 	for (i = 0; i < size; i++){
		if(position == 3){
			a =  two_fifty_six * two_fifty_six * two_fifty_six * list[i];
		}
		else if(position == 2){
			a =  two_fifty_six * two_fifty_six * list[i];
		}
		else if(position == 1){
			a = two_fifty_six * list[i];
		}
		else{
			a = list[i];
		}
		sum = sum + a;
		position --;
	}
	if(sum > 1 && sum <= max){
	//do nothing and continue
	}
	else{
		is_end_of_file = true;
	}
	starting_block = sum;
}
/*
This function checks the status byte of a directory entry to ensure that this 
entry is a file and not a directory entry so the program knows to print it or not.
*/
void check_status(FILE * f, int start, long int size)
{
	byte_position = byte_position + size;

	int i;
	int a;
	int sum = 0;

	unsigned char list[size];

	fseek(f, start, 0);
    if (fread(list, 1, size, f) <= 0) {
        fprintf(stderr, "problems reading directory from image\n");
        exit(1);
    }

	for (i = 0; i < size; i++) {
		a = list[i];
		sum = sum + a;
	}
	current_status = sum;
}

int main(int argc, char *argv[]) {

    char *imagename = NULL;
    char *filename  = NULL;
    FILE *f;

	int  i;
	int dir_entry_byte_size = 64;
	int one_byte = 1;
	int two_bytes = 2;	
	int four_bytes = 4;
	int eight_bytes = 8;
	int twenty_four_bytes = 24;
	int byte_position1 = 0;
	int byte_position_start;
	int byte_calc;

	is_file_name_match = false;
	is_end_of_file = false;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--image") == 0 && i+1 < argc) {
            imagename = argv[i+1];
            i++;
        } else if (strcmp(argv[i], "--file") == 0 && i+1 < argc) {
            filename = argv[i+1];
            i++;
        }
    }

    if (imagename == NULL || filename == NULL) {
        fprintf(stderr, "usage: catuvfs --image <imagename> " \
            "--file <filename in image>");
        exit(1);
    }

	f = fopen(imagename, "r");

	byte_position = eight_bytes; 
	extract_superblock_vals(f, byte_position, two_bytes);
	for(i = 0; i < 5; i++){
		extract_superblock_vals(f, byte_position, four_bytes);
	}

	byte_position_start = num_blocks * block_size;
	byte_position = byte_position_start + block_size + 1;
	get_starting_block(f, byte_position, two_bytes);	
	byte_position = byte_position + twenty_four_bytes;

	byte_position_start = DIR_start*block_size;//start of root DIR
	byte_position = byte_position_start;

	int mult = block_size / dir_entry_byte_size;

	for (i = 0; i < number_of_DIR*mult; i++){
		check_status(f, byte_position, one_byte);
		if (current_status == 1){
			
			get_starting_block(f, byte_position, four_bytes);
			byte_position = byte_position + 22;
			check_file_name(f, byte_position, 31, filename);
			byte_position = byte_position + 6;
			if(is_file_name_match){
				while(!is_end_of_file){
					DIR_byte_position = block_size * num_blocks;
					DIR_byte_position = DIR_byte_position + block_size;//end of FAT
					byte_calc = block_size*number_of_DIR;
					DIR_byte_position = DIR_byte_position + byte_calc;// end of DIR entries
					byte_calc = starting_block* block_size;
					DIR_byte_position = starting_block* block_size;
					print_file_data(f, DIR_byte_position, block_size);
					byte_position1 = four_bytes * starting_block;
					byte_position1 = byte_position1 + block_size;
					check_FAT(f, byte_position1, four_bytes);
				}
				break;
			}
		}
		else{
			byte_position = byte_position + dir_entry_byte_size -1;
		}
	}

	if(!is_file_name_match){
		printf("File not found.\n");
	}
    return 0; 
}

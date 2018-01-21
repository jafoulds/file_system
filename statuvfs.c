/*
July 31, 2017
Jared Foulds
CSC 360 Assignment #4, Task #1
Michael Zastre

This program is used to print information about a disk image such as blocksize, what block the FAT starts, 
and the block the root directory starts.
*/
#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include "disk.h"
#include <string.h>

int byte_position;
int times_run;
int total_blocks;
int FAT_start;
int starting_block;
int DIR_start;
int DIR_count;
int alloc_number;
int num_blocks_FAT;
int byte_size;

/*
This function titles for the free blocks, reserved blocks, and the allocated blocks.
*/
void print_last_line_titles()
{
	printf(" Free   Resv  Alloc\n");
}

/*
This function calculates and printsthe amount of free blocks, 
reserved blocks, and the allocated blocks.
*/
void print_last_line_values()
{
printf("%5d  %5d  %5d\n", total_blocks-DIR_start-alloc_number, DIR_start,  alloc_number);
}

/*
This function simply prints the titles for the values of the various
sections of the superblock.
*/
void print_img_section_titles()
{
	printf("  Bsz   Bcnt  FATst FATcnt  DIRst DIRcnt\n");
}

/*
This function simply prints the required dashes for output.
*/
void print_dashes(int num_dashes)
{
	printf("\n\n");
	int i = 0;
	for (i = 0; i < num_dashes; i ++){
		printf("-");
	}
	printf("\n");
}

/*
This function simply prints the file name to the screen when called
at the beginning of the program run.
*/
void print_file_sys_identifier(FILE * f, int start, long int size)
{
	byte_position = byte_position + size;
	int i;
	char list[size];
	
    fseek(f, start, SEEK_SET);
    if (fread(list, sizeof(char), size, f) <= 0) {
        fprintf(stderr, "problems reading directory from image\n");
        exit(1);
    }
	for (i = start; i < start + size; i++) {
		printf("%c", list[i]);
	}
	printf(" ");
}

/*
This function is responsible for extracting a certain section of the superblock 
and is dependent on the byte size passed. It then converts that value to decimal
and depending on what part of the superblock, it is stored in a global variable
which is eventually printed.
*/
void print_img_section(FILE * f, int start, long int size)
{
	times_run ++;
	byte_position = byte_position + size;
	int i;
	unsigned char list[size];

    fseek(f, start, 0);
    if (fread(list, 1, size, f) <= 0) {
        fprintf(stderr, "problems reading directory from image\n");
        exit(1);
    }
	int a;
	int sum =0;

	int position = size -1;
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
	printf("%5d", sum);
	printf("  ");		
	
	if(times_run == 1){
		byte_size = sum;
	}
	if(times_run == 2){
		total_blocks = sum;
	}
	if(times_run == 3){
		FAT_start = sum;
	}	
	if(times_run == 4){
		num_blocks_FAT = sum;
	}
	if(times_run == 5){
		DIR_start = sum;
	}
	if(times_run == 6){
		DIR_count =sum;
	}
}


/*
This function is responsible for calculating the decimal value of a FAT block
and using that value to determine if a block has been allocated or it.
*/
void count_alloc(FILE * f, int start, long int size)
{
	byte_position = byte_position + size;
	unsigned char list[size];
	int i;
	unsigned int a;
	unsigned int sum =0;

    fseek(f, start, 0);
    if (fread(list, 1, size, f) <= 0) {
        fprintf(stderr, "problems reading directory from image\n");
        exit(1);
    }

	int position = size -1;
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
	if(sum > 1){
		alloc_number ++;
	}
}

int main(int argc, char *argv[]) {
	alloc_number = 0;
  
    int  i;
    char *imagename = NULL;
    FILE  *f;
    
	unsigned int num_dashes = 49;
	unsigned int filename_path_offset = 7;
	int directory_byte_size = 64;

	byte_position = 0;
	int two_bytes = 2;
	int four_bytes = 4;
	int eight_bytes = 8;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--image") == 0 && i+1 < argc) {
            imagename = argv[i+1];
            i++;
        }
    }

    if (imagename == NULL)
    {
        fprintf(stderr, "usage: statuvfs --image <imagename>\n");
        exit(1);
    }
	char * filename_only = &imagename[filename_path_offset];
	f = fopen(imagename, "r");
    
	print_file_sys_identifier(f, byte_position, eight_bytes); //print file name
	printf("(%s)", filename_only);
	print_dashes(num_dashes);
	print_img_section_titles();
	print_img_section(f, byte_position, two_bytes); //extract blocksize
	print_img_section(f, byte_position, four_bytes); // extract file system size
	print_img_section(f, byte_position, four_bytes);// extract FAT block start
	print_img_section(f, byte_position, four_bytes); //extract # of FAT blocks
	print_img_section(f, byte_position, four_bytes); //extract block where root DIR starts
	print_img_section(f, byte_position, four_bytes); //extract # of blocks in root DIR
	print_dashes(num_dashes);
	byte_position = directory_byte_size;
	
	/* iterate through each FAT block and call count_alloc to see if
	theres any data there */
	for (i = 0; i < byte_size*four_bytes; i++){
			count_alloc(f, byte_position, 4);
	}

	print_last_line_titles();
	print_last_line_values();

	

    return 0; 
}

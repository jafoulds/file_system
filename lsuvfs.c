/*
July 31, 2017
Jared Foulds
CSC 360 Assignment #4, Task #2
Michael Zastre

This program is responsible for printing a directory listing of files in a disk image.
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

/*
This function is passed the month in decimal form and returns
the corresponding month in string form.
*/
char *month_to_string(short m) {
    switch(m) {
    case 1: return "Jan";
    case 2: return "Feb";
    case 3: return "Mar";
    case 4: return "Apr";
    case 5: return "May";
    case 6: return "Jun";
    case 7: return "Jul";
    case 8: return "Aug";
    case 9: return "Sep";
    case 10: return "Oct";
    case 11: return "Nov";
    case 12: return "Dec";
    default: return "?!?";
    }

}

/*
This function checks the status byte of a directory entry to ensure that this 
entry is a file and not a directory entry so the program knows to print it or not.
*/
void check_status(FILE * f, int start, long int size)
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
		if(position > 0){
			a =  256 * position * list[i];
		}
		else{
			a = list[i];
		}
		sum = sum + a;
		position --;
	}
	current_status = sum;
}

/*
This function is simply used to extract the creation time for a directory entry.
It extracts the necessary hex values and converts them to decimal with the exception of
the month which is passed month_to_string to get the month in string form.
*/
void get_create_time(FILE * f, int start, long int size, bool is_month)
{
	byte_position = byte_position + size;
	
	unsigned char list[size];
	char *  month = "";

	int i;
	int a;
	int sum = 0;
	int position = size - 1;

	fseek(f, start, 0);
    if (fread(list, 1, size, f) <= 0) {
        fprintf(stderr, "problems reading directory from image\n");
        exit(1);
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
	if(is_month){
		month = month_to_string(sum);
		printf("%s", month);
	}
	else{
		printf("%02d", sum);
	}
}


/*
This function is responsible for extracting the file size from the directory entry.
It converts the hex values to decimal and stores it in a global variable.
*/
void get_file_size (FILE * f, int start, long int size)
{
	byte_position = byte_position + size;

	int i;
	unsigned int a;
    unsigned int sum = 0;
	unsigned int two_fifty_six = 256;
    int position = size - 1;

	unsigned char list[size];

	fseek(f, start, 0);
    if (fread(list, 1, size, f) <= 0) {
        fprintf(stderr, "problems reading directory from image\n");
        exit(1);
    }

	for (i = 0; i < size; i++) {
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
	printf("%8d ", sum);
}

/*
This function simply prints the file name to the screen when the program 
is iterating through the directory entries to locate file names.
*/
void print_file_name(FILE * f, int start, long int size)
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
	printf(" ");
	int position = size -1;
	for (i = 0; i < size; i++) {
		printf("%c", list[i]);//print hex
		if(position > 0){
			a =  256 * position * list[i];
		}
		else{
			a = list[i];
		}
		sum = sum + a;
		position --;
	}
}
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
	int sum = 0;
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
	if(times_run == 4){
		num_blocks = sum;
	}
	if(times_run == 5){
		DIR_start = sum;
		//printf("DIR_start: %d\n", DIR_start);
	}
	if(times_run == 6){
		number_of_DIR = sum;
	}
}
/*
Prints colons for required output.
*/
void print_colon()
{
	printf(":");
}
/*
Prints dashes for required output.
*/
void print_dash()
{
	printf("-");
}

/*
Prints spaces for required output.
*/
void print_space()
{
	printf(" ");
}

int main(int argc, char *argv[]) {
    int  i;
    char *imagename = NULL;
    FILE *f;
	
	bool is_month = false;

	int dir_entry_byte_size = 64;
	int modify_time_byte_size = 7;	
	int one_byte = 1;
	int two_bytes = 2;	
	int four_bytes = 4;
	int eight_bytes = 8;
	int filename_bytes = 31;
	int byte_position_start;
	
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--image") == 0 && i+1 < argc) {
            imagename = argv[i+1];
            i++;
        }
    }

    if (imagename == NULL)
    {
        fprintf(stderr, "usage: lsuvfs --image <imagename>\n");
        exit(1);
    }
	
	f = fopen(imagename, "r");

	byte_position = eight_bytes; 
	extract_superblock_vals(f, byte_position, two_bytes);
	for(i = 0; i < 5; i++){
		extract_superblock_vals(f, byte_position, four_bytes);
	}

	byte_position_start = DIR_start*block_size; 
	byte_position = byte_position_start;
	int mult = block_size / dir_entry_byte_size;

	for (i = 0; i < number_of_DIR*mult; i++){
		check_status(f, byte_position, one_byte);
		if (current_status == 1){
			byte_position = byte_position + eight_bytes; //skip 4 bytes for starting 
														//block + another 4 bytes for # blocks in file
			get_file_size(f, byte_position, four_bytes);//extract file size (in bytes)
			get_create_time(f, byte_position, two_bytes, is_month);// year
			is_month = true;
			print_dash();
			get_create_time(f, byte_position, one_byte, is_month);// month
			print_dash();
			is_month = false;	
			get_create_time(f, byte_position, one_byte, is_month);//day
			print_space();
			get_create_time(f, byte_position, one_byte, is_month);//hour
			print_colon();
			get_create_time(f, byte_position, one_byte, is_month);//minute
			print_colon();
			get_create_time(f, byte_position, one_byte, is_month);//second
			byte_position = byte_position + modify_time_byte_size;//skip modify time
			print_file_name(f, byte_position, filename_bytes);
			byte_position = byte_position + 6;
			printf("\n");
		}
		else{
			byte_position = byte_position + dir_entry_byte_size -1;
		}
	}
    return 0; 
}

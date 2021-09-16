#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>


#include "mu-mips.h"

/***************************************************************/
/* Print out a list of commands available                                                                  */
/***************************************************************/
void help() {        
	printf("------------------------------------------------------------------\n\n");
	printf("\t**********MU-MIPS Help MENU**********\n\n");
	printf("sim\t-- simulate program to completion \n");
	printf("run <n>\t-- simulate program for <n> instructions\n");
	printf("rdump\t-- dump register values\n");
	printf("reset\t-- clears all registers/memory and re-loads the program\n");
	printf("input <reg> <val>\t-- set GPR <reg> to <val>\n");
	printf("mdump <start> <stop>\t-- dump memory from <start> to <stop> address\n");
	printf("high <val>\t-- set the HI register to <val>\n");
	printf("low <val>\t-- set the LO register to <val>\n");
	printf("print\t-- print the program loaded into memory\n");
	printf("?\t-- display help menu\n");
	printf("quit\t-- exit the simulator\n\n");
	printf("------------------------------------------------------------------\n\n");
}

/***************************************************************/
/* Read a 32-bit word from memory                                                                            */
/***************************************************************/
uint32_t mem_read_32(uint32_t address)
{
	int i;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		if ( (address >= MEM_REGIONS[i].begin) &&  ( address <= MEM_REGIONS[i].end) ) {
			uint32_t offset = address - MEM_REGIONS[i].begin;
			return (MEM_REGIONS[i].mem[offset+3] << 24) |
					(MEM_REGIONS[i].mem[offset+2] << 16) |
					(MEM_REGIONS[i].mem[offset+1] <<  8) |
					(MEM_REGIONS[i].mem[offset+0] <<  0);
		}
	}
	return 0;
}

/***************************************************************/
/* Write a 32-bit word to memory                                                                                */
/***************************************************************/
void mem_write_32(uint32_t address, uint32_t value)
{
	int i;
	uint32_t offset;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		if ( (address >= MEM_REGIONS[i].begin) && (address <= MEM_REGIONS[i].end) ) {
			offset = address - MEM_REGIONS[i].begin;

			MEM_REGIONS[i].mem[offset+3] = (value >> 24) & 0xFF;
			MEM_REGIONS[i].mem[offset+2] = (value >> 16) & 0xFF;
			MEM_REGIONS[i].mem[offset+1] = (value >>  8) & 0xFF;
			MEM_REGIONS[i].mem[offset+0] = (value >>  0) & 0xFF;
		}
	}
}

/***************************************************************/
/* Execute one cycle                                                                                                              */
/***************************************************************/
void cycle() {                                                
	handle_instruction();
	CURRENT_STATE = NEXT_STATE;
	INSTRUCTION_COUNT++;
}

/***************************************************************/
/* Simulate MIPS for n cycles                                                                                       */
/***************************************************************/
void run(int num_cycles) {                                      
	
	if (RUN_FLAG == FALSE) {
		printf("Simulation Stopped\n\n");
		return;
	}

	printf("Running simulator for %d cycles...\n\n", num_cycles);
	int i;
	for (i = 0; i < num_cycles; i++) {
		if (RUN_FLAG == FALSE) {
			printf("Simulation Stopped.\n\n");
			break;
		}
		cycle();
	}
}

/***************************************************************/
/* simulate to completion                                                                                               */
/***************************************************************/
void runAll() {                                                     
	if (RUN_FLAG == FALSE) {
		printf("Simulation Stopped.\n\n");
		return;
	}

	printf("Simulation Started...\n\n");
	while (RUN_FLAG){
		cycle();
	}
	printf("Simulation Finished.\n\n");
}

/***************************************************************/ 
/* Dump a word-aligned region of memory to the terminal                              */
/***************************************************************/
void mdump(uint32_t start, uint32_t stop) {          
	uint32_t address;

	printf("-------------------------------------------------------------\n");
	printf("Memory content [0x%08x..0x%08x] :\n", start, stop);
	printf("-------------------------------------------------------------\n");
	printf("\t[Address in Hex (Dec) ]\t[Value]\n");
	for (address = start; address <= stop; address += 4){
		printf("\t0x%08x (%d) :\t0x%08x\n", address, address, mem_read_32(address));
	}
	printf("\n");
}

/***************************************************************/
/* Dump current values of registers to the teminal                                              */   
/***************************************************************/
void rdump() {                               
	int i; 
	printf("-------------------------------------\n");
	printf("Dumping Register Content\n");
	printf("-------------------------------------\n");
	printf("# Instructions Executed\t: %u\n", INSTRUCTION_COUNT);
	printf("PC\t: 0x%08x\n", CURRENT_STATE.PC);
	printf("-------------------------------------\n");
	printf("[Register]\t[Value]\n");
	printf("-------------------------------------\n");
	for (i = 0; i < MIPS_REGS; i++){
		printf("[R%d]\t: 0x%08x\n", i, CURRENT_STATE.REGS[i]);
	}
	printf("-------------------------------------\n");
	printf("[HI]\t: 0x%08x\n", CURRENT_STATE.HI);
	printf("[LO]\t: 0x%08x\n", CURRENT_STATE.LO);
	printf("-------------------------------------\n");
}

/***************************************************************/
/* Read a command from standard input.                                                               */  
/***************************************************************/
void handle_command() {                         
	char buffer[20];
	uint32_t start, stop, cycles;
	uint32_t register_no;
	int register_value;
	int hi_reg_value, lo_reg_value;

	printf("MU-MIPS SIM:> ");

	if (scanf("%s", buffer) == EOF){
		exit(0);
	}

	switch(buffer[0]) {
		case 'S':
		case 's':
			runAll(); 
			break;
		case 'M':
		case 'm':
			if (scanf("%x %x", &start, &stop) != 2){
				break;
			}
			mdump(start, stop);
			break;
		case '?':
			help();
			break;
		case 'Q':
		case 'q':
			printf("**************************\n");
			printf("Exiting MU-MIPS! Good Bye...\n");
			printf("**************************\n");
			exit(0);
		case 'R':
		case 'r':
			if (buffer[1] == 'd' || buffer[1] == 'D'){
				rdump();
			}else if(buffer[1] == 'e' || buffer[1] == 'E'){
				reset();
			}
			else {
				if (scanf("%d", &cycles) != 1) {
					break;
				}
				run(cycles);
			}
			break;
		case 'I':
		case 'i':
			if (scanf("%u %i", &register_no, &register_value) != 2){
				break;
			}
			CURRENT_STATE.REGS[register_no] = register_value;
			NEXT_STATE.REGS[register_no] = register_value;
			break;
		case 'H':
		case 'h':
			if (scanf("%i", &hi_reg_value) != 1){
				break;
			}
			CURRENT_STATE.HI = hi_reg_value; 
			NEXT_STATE.HI = hi_reg_value; 
			break;
		case 'L':
		case 'l':
			if (scanf("%i", &lo_reg_value) != 1){
				break;
			}
			CURRENT_STATE.LO = lo_reg_value;
			NEXT_STATE.LO = lo_reg_value;
			break;
		case 'P':
		case 'p':
			print_program(); 
			break;
		default:
			printf("Invalid Command.\n");
			break;
	}
}

/***************************************************************/
/* reset registers/memory and reload program                                                    */
/***************************************************************/
void reset() {   
	int i;
	/*reset registers*/
	for (i = 0; i < MIPS_REGS; i++){
		CURRENT_STATE.REGS[i] = 0;
	}
	CURRENT_STATE.HI = 0;
	CURRENT_STATE.LO = 0;
	
	for (i = 0; i < NUM_MEM_REGION; i++) {
		uint32_t region_size = MEM_REGIONS[i].end - MEM_REGIONS[i].begin + 1;
		memset(MEM_REGIONS[i].mem, 0, region_size);
	}
	
	/*load program*/
	load_program();
	
	/*reset PC*/
	INSTRUCTION_COUNT = 0;
	CURRENT_STATE.PC =  MEM_TEXT_BEGIN;
	NEXT_STATE = CURRENT_STATE;
	RUN_FLAG = TRUE;
}

/***************************************************************/
/* Allocate and set memory to zero                                                                            */
/***************************************************************/
void init_memory() {                                           
	int i;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		uint32_t region_size = MEM_REGIONS[i].end - MEM_REGIONS[i].begin + 1;
		MEM_REGIONS[i].mem = malloc(region_size);
		memset(MEM_REGIONS[i].mem, 0, region_size);
	}
}

/**************************************************************/
/* load program into memory                                                                                      */
/**************************************************************/
void load_program() {                   
	FILE * fp;
	int i, word;
	uint32_t address;

	/* Open program file. */
	fp = fopen(prog_file, "r");
	if (fp == NULL) {
		printf("Error: Can't open program file %s\n", prog_file);
		exit(-1);
	}

	/* Read in the program. */

	i = 0;
	while( fscanf(fp, "%x\n", &word) != EOF ) {
		address = MEM_TEXT_BEGIN + i;
		mem_write_32(address, word);
		printf("writing 0x%08x into address 0x%08x (%d)\n", word, address, address);
		i += 4;
	}
	PROGRAM_SIZE = i/4;
	printf("Program loaded into memory.\n%d words written into memory.\n\n", PROGRAM_SIZE);
	fclose(fp);
}

/************************************************************/
/* decode and execute instruction                                                                     */ 
/************************************************************/
void handle_instruction()
{
			
	/*IMPLEMENT THIS*/
	/* execute one instruction at a time. Use/update CURRENT_STATE and and NEXT_STATE, as necessary.*/
}


/************************************************************/
/* Initialize Memory                                                                                                    */ 
/************************************************************/
void initialize() { 
	init_memory();
	CURRENT_STATE.PC = MEM_TEXT_BEGIN;
	NEXT_STATE = CURRENT_STATE;
	RUN_FLAG = TRUE;
}

/************************************************************/
/* Print the program loaded into memory (in MIPS assembly format)    */ 
/************************************************************/
void print_program(){
	int i;
	uint32_t addr;
	
	for(i=0; i<PROGRAM_SIZE; i++){
		addr = MEM_TEXT_BEGIN + (i*4);
		printf("[0x%x]\t", addr);
		print_instruction(addr);
	}
}

/************************************************************/
/* Print the instruction at given memory address (in MIPS assembly format)    */
/************************************************************/
void print_instruction(uint32_t addr){
	/*IMPLEMENT THIS*/
}

/***************************************************************/
/* main                                                                                                                                   */
/***************************************************************/
int main(int argc, char *argv[]) {                              
	printf("\n**************************\n");
	printf("Welcome to MU-MIPS SIM...\n");
	printf("**************************\n\n");
	uint32_t hex = 0xFFFFFFFF;
	printf("%s", binaryMips(hex));
	if (argc < 2) {
		printf("Error: You should provide input file.\nUsage: %s <input program> \n\n",  argv[0]);
		exit(1);
	}

	strcpy(prog_file, argv[1]);
	initialize();
	load_program();
	help();
	while (1){
		handle_command();
	}
	return 0;
}
const char* binaryMips(uint32_t input)
{
	char hexString[9] = {}; //create a string to store the hexadecimal form
	static char binString[33] = {}; //a string to contain the binary representation
	int i = 0;
	printf("%X", input);
	sprintf(hexString, "%X", input); //store the hex into the string
	printf("%s", hexString); //test for the correct string 
	for(i = 0; i < 8; i++) //convert the hex into binary values
	{
		switch (hexString[i])
		{
			case '0':
				strcat(binString, "0000");
				break;
			case '1':
				strcat(binString, "0001");
				break;
			case '2':
				strcat(binString, "0010");
				break;
			case '3':
				strcat(binString, "0011");
				break;
			case '4':
				strcat(binString, "0100");
				break;
			case '5': 
				strcat(binString, "0101");
				break;
			case '6':
				strcat(binString, "0110");
				break;
			case '7':
				strcat(binString, "0111");
				break;
			case '8':
				strcat(binString, "1000");
				break;
			case '9': 
				strcat(binString, "1001");
				break;
			case 'A':
				strcat(binString, "1010");
				break;
			case 'B':
				strcat(binString, "1011");
				break;
			case 'C':
				strcat(binString, "1100");
				break;
			case 'D':
				strcat(binString, "1101");
				break;
			case 'E':
				strcat(binString, "1110");
				break;
			case 'F':
				strcat(binString, "1111");
				break;
		}
	}

	return binString;
}
/*const char* instMips(char *input)
{

}*/
int instFormat(const char* opcode)
{
	int type = 0;
	if(strcmp(opcode, "000000") == 0)
	{
		type = 0; //R type instructions
		return type;
	} 
	type = 1;
	return type;
}
void handleItype(const char* bits)
{
	char rs[6] = {};
	char rt[6] = {};
	char imm[16] = {};
	for(int i = 0; i < 6; i++)
	{
		rs[i] = bits[i+6];
		rt[i] = bits[i+11];
	}

}
void handleRtype(const char* bits)
{
	char rs[6] = {};
	char rt[6] = {};
	char rd[6] = {};
	char shamt[6] = {};
	char funct[7] = {};
	int temp = 0, inst = 0, opcode = 0;
	for(int i = 0; i < 6; i++)
	{
		rs[i] = bits[i+6];
		rt[i] = bits[i+11];
		rd[i] = bits[i+16];
		shamt[i] = bits[i+21];
	}
	for(int i = 0; i < 7; i++)
	{
		funct[i] = bits[i+26];
	}
	opcode = atoi(funct);
	switch(opcode)
	{
		case 100000: //ADD 
		{
			temp = CURRENT_STATE.REGS[binToDec(rs)] + CURRENT_STATE.REGS[binToDec(rt)];	
			if(temp >= 0x7FFFFFFF)
			{
				break;
			}
			CURRENT_STATE.REGS[binToDec(rd)] = CURRENT_STATE.REGS[binToDec(rs)] + CURRENT_STATE.REGS[binToDec(rt)];
			break;
		}
		case 100001: //ADDU
		{
			CURRENT_STATE.REGS[binToDec(rd)] = CURRENT_STATE.REGS[binToDec(rs)] + CURRENT_STATE.REGS[binToDec(rt)];
			break;
		}
		case 100100: //AND
		{
			
			CURRENT_STATE.REGS[binToDec(rd)] = CURRENT_STATE.REGS[binToDec(rt)] & CURRENT_STATE.REGS[binToDec(rs)];
			break;

		}
		case 001000: //JR
		{
			//Need to implement J-type instructions before we implement this. 
		}
		case 100111: //NOR
		{
			CURRENT_STATE.REGS[binToDec(rd)] = ~CURRENT_STATE.REGS[binToDec(rt)] ^ ~CURRENT_STATE.REGS[binToDec(rs)];
			break;
		}
		case 100101: //OR
		{
			CURRENT_STATE.REGS[binToDec(rd)] = CURRENT_STATE.REGS[binToDec(rt)] | CURRENT_STATE.REGS[binToDec(rs)];
			break;
		}
		case 101010: //SLT
		{
			if(CURRENT_STATE.REGS[binToDec(rs)] < CURRENT_STATE.REGS[binToDec(rt)])
			{
				CURRENT_STATE.REGS[binToDec(rd)] = 1;
				break;
			}
			CURRENT_STATE.REGS[binToDec(rd)] = 0;
		}
		case 000000: //SLL
		{
			
		}
		case 000010: //SRL
		{
			
		}
		case 100010: //SUB
		{
			
		}
		case 100011: //SUBU	
		{
			
		}
		case 011000: //MULT
		{
			
		}
		case 101001: //MULTU
		{
			
		}
		case 011010: //DIV
		{
			
		}
		case 011011: //DIVU 
		{
			
		}
		case 100110: //XOR
		{
			
		}
		case 000011: //SRA
		{
			
		}
		case 010000: //MFHI
		{
			
		}
		case 010010: //MFLO
		{
			
		}
		case 010001: //MTHI
		{
			
		}
		case 010011: //MTLO
		{
			
		}
		case 001001: //JALR
		{
			
		}
		default:
			printf("Error: An I-type instruction with opcode %d does not exist in the MIPS instruction set.\n", opcode);
	}

}
int binToDec(const char* bits)
{
	int i = 0;
	int result = 0;
	for(i = 0; i < 5; i++)
	{
		if(bits[i] == '1')
		{
			result += pow(2, (4-i));
		}
	}
	return result; 
}
/*int instFind(int format, int opcode)
{
	if(format != 0)
	{
		switch(opcode)
		{
			case 001000: //ADDI
				return 8;
			case 001001: //ADDIU
				return 9;
			case 001100: //ANDI
				return 12;
			case 001101: //ORI
				return 13;
			case 001110: //XORI
				return 14;
			case 001010: //SLTI
				return 10;
			case 100011: //LW
				return 35;
			case 100000: //LB
				return 32;
			case 100001: //LH
				return 33;
			case 001111: //LUI
				return 15;
			case 101011: //SW
				return 43;
			case 101000: //SB
				return 40;
			case 101001: //SH
				return 41;
			case 000100: //BEQ
				return 4;
			case 000101: //BNE
				return 5;
			case 000110: //BLEZ
				return 6;
			case 000111: //BGTZ
				return 7;
			case 000010: //J
				return 2;
			case 000011: //JAL
				return 3;
			default: 
				printf("Error: An R-type instruction with opcode %d does not exist in the MIPS instruction set.\n", opcode);
				return 1000;
		}
	}
	else 
		switch(opcode)
		{
			case 100000: //ADD 
				return 96;
			case 100001: //ADDU
				return 97;
			case 100100: //AND
				return 100;
			case 001000: //JR
				return 72;
			case 100111: //NOR
				return 103;
			case 100101: //OR
				return 101;
			case 101010: //SLT
				return 106;
			case 000000: //SLL
				return 64;
			case 000010: //SRL
				return 66;
			case 100010: //SUB
				return 98;
			case 100011: //SUBU	
				return 99;
			case 011000: //MULT
				return 88;
			case 101001: //MULTU
				return 89;
			case 011010: //DIV
				return 90;
			case 011011: //DIVU 
				return 91;
			case 100110: //XOR
				return 102;
			case 000011: //SRA
				return 67;
			case 010000: //MFHI
				return 80;
			case 010010: //MFLO
				return 82;
			case 010001: //MTHI
				return 81;
			case 010011: //MTLO
				return 83;
			case 001001: //JALR
				return 73;
			default:
				printf("Error: An I-type instruction with opcode %d does not exist in the MIPS instruction set.\n", opcode);
				return 1000
		}
}*/


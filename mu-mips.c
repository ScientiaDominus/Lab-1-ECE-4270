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
	char* inst;
	inst = (char *)malloc(33*sizeof(char));
	inst = (char *)binaryMips(mem_read_32(CURRENT_STATE.PC));
	char temp[7] = {};
	if(atoi(inst)==0)
	{
		RUN_FLAG = FALSE;
	}
	for(int i = 0; i < 7; i++)
	{
		temp[i] = inst[i];
	}
	if(instFormat(temp) == 0)
	{
		handleRtype(inst);
	}
	else
	{
		handleItype(inst);
	}
	free(inst);
	if(NEXT_STATE.PC == CURRENT_STATE.PC)
	{
		NEXT_STATE.PC = CURRENT_STATE.PC+4;		
	}
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
void print_instruction(uint32_t addr)
{
	char* inst;
	inst = (char *)malloc(33*sizeof(char));
	inst = (char *)binaryMips(mem_read_32(addr));
	char temp[7] = {};
	for(int i = 0; i < 6; i++)
	{
		if(i < 6)
		{
			temp[i] = inst[i];
		}
	}
	if(instFormat(temp) == 0)
	{
		//printf("R-type instruction\n");
		printf("%s\n", rTypeString(inst));
	}
	else
	{
		//printf("I-type instruction\n");
		printf("%s\n", iTypeString(inst));
	}



}

/***************************************************************/
/* main                                                                                                                                   */
/***************************************************************/
int main(int argc, char *argv[]) {                              
	printf("\n**************************\n");
	printf("Welcome to MU-MIPS SIM...\n");
	printf("**************************\n\n");
	/*uint32_t hex = 0xFFFFFFFF;
	const char *hexinst = binaryMips(hex);
	printf("\n%s\n", hexinst);
	hex = 0x3C031000;
	hexinst = binaryMips(hex);
	printf("\n%s\n", hexinst);*/
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
	//free((char *)hexinst);
	return 0;
}
//this function is now working. Previously values resulted in a buffer overflow but that issue has been resolved. 
const char* binaryMips(uint32_t input)
{
	char hexString[9] = {}; //create a string to store the hexadecimal form
	char *binString;
	binString = (char *)malloc(33*sizeof(char)); //a string to contain the binary representation
	int i = 0;
	//printf("\n%X\n", input);
	//printf("Hey jude\n");
	sprintf(hexString, "%08X", (uint32_t)input); //store the hex into the string
	//printf("\nhexString is %s\n", hexString); //test for the correct string 
	for(i = 0; i < 8; i++) //convert the hex into binary values
	{
		switch (hexString[i]) //check the character and convert to a 4-bit binary representation.
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
	//printf("\nbinString is %s\n", binString);
	return binString;
}
/*const char* instMips(char *input)
{

}*/
int instFormat(const char* opcode)
{
	int type = 0;
	//printf("OPCODE IS %s\n", opcode);
	if(strcmp(opcode, "000000\0") == 0)
	{
		type = 0; //R type instructions
		return type;
	} 
	type = 1;
	return type;
}
void handleItype(const char* bits)
{
	int opcode = 0;
	char op[7] = {};
	char rs[6] = {};
	char rt[6] = {};
	char imm[17] = {};
	/*for(int i = 0; i < 6; i++)
	{
		rs[i] = bits[i+6];
		rt[i] = bits[i+11];
	}
	for(int i = 0; i < 7; i++)
	{
		op[i] = bits[i];
	
	}*/
	for(int i = 0; i < 16; i++)
	{
		if(i < 6)
		{
			op[i] = bits[i]; //convert the opcode into a string.
			if(i < 5)
			{ 
				rs[i] = bits[i+6]; //convert rs and rt into strings for editing later
				rt[i] = bits[i+11];
			}
		}
		imm[i] = bits[i+16]; //convert the immediate address into a string.
	}
	opcode = longBinToDec(op);
	switch(opcode)
	{
		case 8: //ADDI
		{
			NEXT_STATE.REGS[binToDec(rt)] = (int32_t)CURRENT_STATE.REGS[binToDec(rs)] + (int32_t)longBinToDec(imm);
			break;
		}
		case 9: //ADDIU
		{
			NEXT_STATE.REGS[binToDec(rt)] = CURRENT_STATE.REGS[binToDec(rs)] + (uint32_t)longBinToDec(imm);
			break;
		}
		case 12: //ANDI
		{
			NEXT_STATE.REGS[binToDec(rt)] = CURRENT_STATE.REGS[binToDec(rs)] & (uint32_t)longBinToDec(imm);
			break;
		}
		case 13: //ORI
		{
			NEXT_STATE.REGS[binToDec(rt)] = CURRENT_STATE.REGS[binToDec(rs)] | (uint32_t)longBinToDec(imm);
			break;
		}
		case 14: //XORI
		{
			NEXT_STATE.REGS[binToDec(rt)] = CURRENT_STATE.REGS[binToDec(rs)] ^ (uint32_t)longBinToDec(imm);
			break;
		}
		case 10: //SLTI
		{
			if(CURRENT_STATE.REGS[binToDec(rs)] < (uint32_t)longBinToDec(imm)) //compare the registers. if rs is less than rt than store 1 in rd. otherwise store a 0 in rd.
			{
				NEXT_STATE.REGS[binToDec(rt)] = 1;
				break;
			}
			NEXT_STATE.REGS[binToDec(rt)] = 0;
			break;
		}
		case 35: //LW
		{
			int32_t temp = 0;
			temp = CURRENT_STATE.REGS[binToDec(rs)] + (int32_t)((int16_t)longBinToDec(imm));
			NEXT_STATE.REGS[binToDec(rt)] = mem_read_32(temp);
			break;
		}
		case 32: //LB
		{
			int32_t temp = 0;
			temp = CURRENT_STATE.REGS[binToDec(rs)] + (int32_t)((int16_t)longBinToDec(imm));
			NEXT_STATE.REGS[binToDec(rt)] = mem_read_32(temp) >> 24;
			break;
		}
		case 33: //LH
		{
			int32_t temp = 0;
			temp = CURRENT_STATE.REGS[binToDec(rs)] + (int32_t)((int16_t)longBinToDec(imm));
			NEXT_STATE.REGS[binToDec(rt)] = mem_read_32(temp) >> 16;
			break;
		}
		case 15: //LUI
		{
			int32_t temp = 0;
			temp = ((int32_t)longBinToDec(imm)) << 16;
			NEXT_STATE.REGS[binToDec(rt)] = temp;
			break;
		}
		case 43: //SW
		{
			uint32_t temp = 0;
			temp = CURRENT_STATE.REGS[binToDec(rs)] + (uint32_t)((int16_t)longBinToDec(imm));
			mem_write_32(temp, CURRENT_STATE.REGS[binToDec(rt)]);
			break;
		}
		case 40: //SB
		{				
			NEXT_STATE.REGS[binToDec(rs)] =  CURRENT_STATE.REGS[binToDec(rs)] + (uint32_t)((int16_t)longBinToDec(imm));
			mem_write_32(NEXT_STATE.REGS[binToDec(rs)], CURRENT_STATE.REGS[binToDec(rt)] & 0x000000FF);
			break;
		}
		case 41: //SH
			{
			NEXT_STATE.REGS[binToDec(rs)] =  CURRENT_STATE.REGS[binToDec(rs)] + (uint32_t)((int16_t)longBinToDec(imm));
			mem_write_32(NEXT_STATE.REGS[binToDec(rs)], CURRENT_STATE.REGS[binToDec(rt)] & 0x0000FFFF);
			break;

			}
		case 4: //BEQ
		{
			if (CURRENT_STATE.REGS[binToDec(rs)] == CURRENT_STATE.REGS[binToDec(rt)]){
				NEXT_STATE.PC = (uint32_t)((int16_t)longBinToDec(imm)) << 2;

			}
			
			break;
		}
			
		case 5: //BNE
		{
			if (CURRENT_STATE.REGS[binToDec(rs)] != CURRENT_STATE.REGS[binToDec(rt)]){
				NEXT_STATE.PC = (uint32_t)((int16_t)longBinToDec(imm)) << 2;

			}
			
			break;
		}	
		case 6: //BLEZ
			{
			if (CURRENT_STATE.REGS[binToDec(rs)] == 0){
				NEXT_STATE.PC = (uint32_t)((int16_t)longBinToDec(imm)) << 2;

			}
			
			break;
		}
		case 7: //BGTZ
		{
			if (CURRENT_STATE.REGS[binToDec(rs)] >= 0){
				NEXT_STATE.PC = (uint32_t)((int16_t)longBinToDec(imm)) << 2;

			}
			
			break;
		}
		case 2: //J
		case 3: //JAL
		default: 
			printf("Error: An R-type instruction with opcode %d does not exist in the MIPS instruction set.\n", opcode);
	}

}
//This function handles all R-type instructions using an input binary string and decoding that into integers with the use of the
//binToDec() function. This function deconstructs the binary string into its different components and allows those to be interpreted accordingly
void handleRtype(const char* bits)
{
	char rs[6] = {};
	char rt[6] = {};
	char rd[6] = {};
	char shamt[6] = {};
	char funct[7] = {};
	int64_t temp = 0;
	int opcode = 0;
	for(int i = 0; i < 5; i++)
	{
		rs[i] = bits[i+6];
		rt[i] = bits[i+11];
		rd[i] = bits[i+16];
		shamt[i] = bits[i+21];
	}
	for(int i = 0; i < 6; i++)
	{
		funct[i] = bits[i+26];
	}
	opcode = longBinToDec(funct);
	switch(opcode)
	{
		case 32: //ADD 
		{
			//rewrite this function to properly use signed values instead of unsigned values. Should be pretty easy.
			temp = (int32_t)CURRENT_STATE.REGS[binToDec(rs)] + (int32_t)CURRENT_STATE.REGS[binToDec(rt)];	//add the values within the registers together
			if(temp >= 0x7FFFFFFF)
			{
				NEXT_STATE.PC = CURRENT_STATE.PC+4;
				break; //if the value is greater than the maximum possible value then do not place the value into the desitination register
			}
			NEXT_STATE.REGS[binToDec(rd)] = (int32_t)CURRENT_STATE.REGS[binToDec(rs)] + (int32_t)CURRENT_STATE.REGS[binToDec(rt)]; //otherwise complete the summation and store in the destination register
			
			break;
		}
		case 33: //ADDU
		{
			NEXT_STATE.REGS[binToDec(rd)] = CURRENT_STATE.REGS[binToDec(rs)] + CURRENT_STATE.REGS[binToDec(rt)]; //add the values without an overflow exepction to be processed.
			
			break;
		}
		case 36: //AND
		{
			
			NEXT_STATE.REGS[binToDec(rd)] = CURRENT_STATE.REGS[binToDec(rt)] & CURRENT_STATE.REGS[binToDec(rs)]; //using the bitwise and operator compare the two values and store their result in the desitination register
			
			break;

		}
		case 8: //JR
		{
			//Need to implement J-type format instructions before we implement this. 
			NEXT_STATE.REGS[31] = CURRENT_STATE.PC+4;
			NEXT_STATE.PC = CURRENT_STATE.REGS[binToDec(rs)];
			break;
		}
		case 39: //NOR
		{
			NEXT_STATE.REGS[binToDec(rd)] = ~CURRENT_STATE.REGS[binToDec(rt)] | ~CURRENT_STATE.REGS[binToDec(rs)]; //using the bitwise nor operator to compare the two registers and store their result in the destination register
			
			break;
		}
		case 37: //OR
		{
			NEXT_STATE.REGS[binToDec(rd)] = CURRENT_STATE.REGS[binToDec(rt)] | CURRENT_STATE.REGS[binToDec(rs)]; //using the bitwise or operation compare the two registers and store the result into the destination register. 
			
			break;
		}
		case 42: //SLT
		{
			if(CURRENT_STATE.REGS[binToDec(rs)] < CURRENT_STATE.REGS[binToDec(rt)]) //compare the registers. if rs is less than rt than store 1 in rd. otherwise store a 0 in rd.
			{
				NEXT_STATE.REGS[binToDec(rd)] = 1;
				
				break;
			}
			NEXT_STATE.REGS[binToDec(rd)] = 0;
			
			break;
		}
		case 0: //SLL
		{
			NEXT_STATE.REGS[binToDec(rd)] = (CURRENT_STATE.REGS[binToDec(rt)] << binToDec(shamt)); 	//shift register by a specified number of bits and store the result in the desition register
			
			break;
		}
		case 2: //SRL
		{
			//shift the register to the right by the specified shift amount and store that in the destination register.
			NEXT_STATE.REGS[binToDec(rd)] = (CURRENT_STATE.REGS[binToDec(rt)] >> binToDec(shamt)); 
			
			break;
		}
		case 34: //SUB
		{
			//functions similary to add, find the difference between the two values and store the result in the destination register. Check for overflow.
			//rewrite this function to properly use signed values instead of unsigned values. Should be pretty easy. 
			int temp = 0;
			temp = CURRENT_STATE.REGS[binToDec(rs)] - CURRENT_STATE.REGS[binToDec(rt)];
			if(temp < 0x80000000)
			{ 
				
				break;
			}
			NEXT_STATE.REGS[binToDec(rd)] = temp;
			
			break;
		}
		case 35: //SUBU	
		{
			//subtract the two values. do not check for overflow.
			NEXT_STATE.REGS[binToDec(rd)] = CURRENT_STATE.REGS[binToDec(rs)] - CURRENT_STATE.REGS[binToDec(rt)];
			
			break;
		}
		case 24: //MULT
		{ //update this function to properly multiply unsigned values.
			//update this function to properly store values in the HI and LO registers.
			int64_t temp = (int64_t)(int32_t)CURRENT_STATE.REGS[binToDec(rs)] * (int64_t)(int32_t)CURRENT_STATE.REGS[binToDec(rt)];
			NEXT_STATE.HI = (uint32_t)((temp>>32) & 0xFFFFFFFF);
			NEXT_STATE.LO = (uint32_t)(temp & 0xFFFFFFFF);
			
			break;
		}
		case 25: //MULTU
		{
			//update this function to properly store values in the HI and LO registers.
			uint64_t tempu = (uint64_t)CURRENT_STATE.REGS[binToDec(rs)] * (uint64_t)CURRENT_STATE.REGS[binToDec(rt)];
			NEXT_STATE.HI = (uint32_t)((tempu>>32) & 0xFFFFFFFF);
			NEXT_STATE.LO = (uint32_t)(tempu & 0xFFFFFFFF);
			
			break;
		}
		case 26: //DIV
		{ //update this function to properly divide signed values.
			//update this function to properly store values in the HI and LO registers.
			int64_t temp = (int64_t)(int32_t)CURRENT_STATE.REGS[binToDec(rs)] / (int64_t)(int32_t)CURRENT_STATE.REGS[binToDec(rt)];
			NEXT_STATE.HI = (uint32_t)((temp>>32) & 0xFFFFFFFF);
			NEXT_STATE.LO = (uint32_t)(temp & 0xFFFFFFFF);
			
			break;
		}
		case 27: //DIVU 
		{
			//update this function to properly store values in the HI and LO registers.
			uint64_t tempu = (uint64_t)CURRENT_STATE.REGS[binToDec(rs)] * (uint64_t)CURRENT_STATE.REGS[binToDec(rt)];
			NEXT_STATE.HI = (uint32_t)((tempu>>32) & 0xFFFFFFFF);
			NEXT_STATE.LO = (uint32_t)(tempu & 0xFFFFFFFF);
			
			break;
		}
		case 38: //XOR
		{
			NEXT_STATE.REGS[binToDec(rd)] = CURRENT_STATE.REGS[binToDec(rt)] ^ CURRENT_STATE.REGS[binToDec(rs)];
			
			break;
		}
		case 3: //SRA
		{
			NEXT_STATE.REGS[binToDec(rd)] = (CURRENT_STATE.REGS[binToDec(rt)] >> binToDec(shamt)); 
			
			break;
		}
		case 16: //MFHI
		{
			//update this function to properly store values in the HI and LO registers.
			NEXT_STATE.REGS[binToDec(rd)] = CURRENT_STATE.HI;
			
			break;
		}
		case 18: //MFLO
		{
			//update this function to properly store values in the HI and LO registers.
			NEXT_STATE.REGS[binToDec(rd)] = CURRENT_STATE.LO;
			
			break;
		}
		case 17: //MTHI
		{
			//update this function to properly store values in the HI and LO registers.
			NEXT_STATE.HI = CURRENT_STATE.REGS[binToDec(rs)];
			
			break;
		}
		case 19: //MTLO
		{
			//update this function to properly store values in the HI and LO registers.
			NEXT_STATE.LO = CURRENT_STATE.REGS[binToDec(rs)];
			
			break;
		}
		case 9: //JALR
		{
			if(binToDec(rd) <= 0 || binToDec(rd) >= 32) //check that the destination register is not specified or is in a non-editable register ($zero)
			{
				NEXT_STATE.REGS[31] = CURRENT_STATE.PC+4; //store the next instructions address into the return address register ($ra)
				NEXT_STATE.PC = CURRENT_STATE.REGS[binToDec(rs)]; //store the address of the jump location into the next state of the program counter.
				break;
			}
			NEXT_STATE.REGS[binToDec(rd)] = CURRENT_STATE.PC+4; //store the next instruction's address into the specified register
			NEXT_STATE.PC = CURRENT_STATE.REGS[binToDec(rs)]; //set the program counter to the address of the jumped to instruction.
			break;
		}
		case 12: //SYSCALL
		{
			if(CURRENT_STATE.REGS[2] == 0xA)
			{
				RUN_FLAG = FALSE;
				break;
			}
		}

		default:
			printf("Error: An I-type instruction with opcode %d does not exist in the MIPS instruction set.\n", opcode);
	}

}
/* converts a 5 bit string into decimal for use within functions 
*/
int binToDec(const char* bits)
{
	int i = 0; 
	int result = 0;
	for(i = 0; i < 5; i++) //iterate through each character of the string
	{
		if(bits[i] == '1')
		{
			result += pow(2, (4-i)); //add the value of each place raised to the proper value. i.e. the first digit is added as 2 raised to the 0th power and the third digit is added as 2 raised to the 2nd power
		}
	}
	return result; 
}
int longBinToDec(const char* bits) //Used to convert the immediate values into a number that can be added to the registers
{
	int i = 0; 
	int result = 0;
	int size = 0;
	size = strlen(bits);
	for(i = 0; i < size; i++) //iterate through each character of the string
	{
		if(bits[i] == '1')
		{
			result += pow(2, ((size - 1)-i)); //add the value of each place raised to the proper value. i.e. the first digit is added as 2 raised to the 0th power and the third digit is added as 2 raised to the 2nd power
		}
	}
	return result;
}
const char* rTypeString(const char* bits)
{
	char rs[6] = {};
	char rt[6] = {};
	char rd[6] = {};
	char shamt[6] = {};
	char funct[7] = {};
	char saInt[4] = {};
	char* result;
	result = (char *)malloc(33*sizeof(char));
	//int64_t temp = 0;
	int opcode = 0;
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
	sprintf(saInt,"%d", binToDec(shamt));
	opcode = longBinToDec(funct);
	switch(opcode)
		{
			case 32: //ADD 
			{	
				strcat(result, "ADD ");
				strcat(result, decToReg(binToDec(rd)));
				strcat(result, ", ");
				strcat(result, decToReg(binToDec(rs)));
				strcat(result, ", ");
				strcat(result, decToReg(binToDec(rt)));
				return result;
			}
			case 33: //ADDU
			{
				strcat(result, "ADDU ");
				strcat(result, decToReg(binToDec(rd)));
				strcat(result, ", ");
				strcat(result, decToReg(binToDec(rs)));
				strcat(result, ", ");
				strcat(result, decToReg(binToDec(rt)));
				return result;
			}
			case 36: //AND
			{
				strcat(result, "AND ");
				strcat(result, decToReg(binToDec(rd)));
				strcat(result, ", ");
				strcat(result, decToReg(binToDec(rs)));
				strcat(result, ", ");
				strcat(result, decToReg(binToDec(rt)));
				return result;
			}
			case 8: //JR
			{			
				strcat(result, "JR ");
				strcat(result, decToReg(binToDec(rs)));
				return result;
			}			
			case 39: //NOR
			{
				strcat(result, "NOR ");
				strcat(result, decToReg(binToDec(rd)));
				strcat(result, ", ");
				strcat(result, decToReg(binToDec(rs)));
				strcat(result, ", ");
				strcat(result, decToReg(binToDec(rt)));
				return result;
			}
			case 37: //OR
			{
				strcat(result, "OR ");
				strcat(result, decToReg(binToDec(rd)));
				strcat(result, ", ");
				strcat(result, decToReg(binToDec(rs)));
				strcat(result, ", ");
				strcat(result, decToReg(binToDec(rt)));
				return result;
			}
			case 42: //SLT
			{
				strcat(result, "SLT ");
				strcat(result, decToReg(binToDec(rd)));
				strcat(result, ", ");
				strcat(result, decToReg(binToDec(rs)));
				strcat(result, ", ");
				strcat(result, decToReg(binToDec(rt)));
				return result;
			}
			case 0: //SLL
			{
				strcat(result, "SLL ");
				strcat(result, decToReg(binToDec(rd)));
				strcat(result, ", ");
				strcat(result, decToReg(binToDec(rt)));
				strcat(result, ", ");
				strcat(result, saInt);
				return result;
			}
			case 2: //SRL
			{
				strcat(result, "SRL ");
				strcat(result, decToReg(binToDec(rd)));
				strcat(result, ", ");
				strcat(result, decToReg(binToDec(rt)));
				strcat(result, ", ");
				strcat(result, saInt);
				return result;
			}
			case 34: //SUB
			{
				strcat(result, "SUB ");
				strcat(result, decToReg(binToDec(rd)));
				strcat(result, ", ");
				strcat(result, decToReg(binToDec(rs)));
				strcat(result, ", ");
				strcat(result, decToReg(binToDec(rt)));
				return result;
			}
			case 35: //SUBU	
			{
				strcat(result, "SUBU ");
				strcat(result, decToReg(binToDec(rd)));
				strcat(result, ", ");
				strcat(result, decToReg(binToDec(rs)));
				strcat(result, ", ");
				strcat(result, decToReg(binToDec(rt)));
				return result;
			}
			case 24: //MULT
			{
				strcat(result, "MULT ");
				strcat(result, decToReg(binToDec(rs)));
				strcat(result, ", ");
				strcat(result, decToReg(binToDec(rt)));
				return result;
			}
			case 41: //MULTU
			{
				strcat(result, "MULTU ");
				strcat(result, decToReg(binToDec(rs)));
				strcat(result, ", ");
				strcat(result, decToReg(binToDec(rt)));
				return result;
			}
			case 26: //DIV
			{
				strcat(result, "DIV ");
				strcat(result, decToReg(binToDec(rs)));
				strcat(result, ", ");
				strcat(result, decToReg(binToDec(rt)));
				return result;
			}
			case 27: //DIVU 
			{
				strcat(result, "DIVU ");
				strcat(result, decToReg(binToDec(rs)));
				strcat(result, ", ");
				strcat(result, decToReg(binToDec(rt)));
				return result;
			}
			case 38: //XOR
			{
				strcat(result, "XOR ");
				strcat(result, decToReg(binToDec(rd)));
				strcat(result, ", ");
				strcat(result, decToReg(binToDec(rs)));
				strcat(result, ", ");
				strcat(result, decToReg(binToDec(rt)));
				return result;
			}
			case 3: //SRA
			{
				strcat(result, "SRA ");
				strcat(result, decToReg(binToDec(rd)));
				strcat(result, ", ");
				strcat(result, decToReg(binToDec(rs)));
				strcat(result, ", ");
				strcat(result, decToReg(binToDec(rt)));
				return result;
			}
			case 16: //MFHI
			{
				strcat(result, "MFHI ");
				strcat(result, decToReg(binToDec(rd)));
				return result;
			}
			case 18: //MFLO
			{
				strcat(result, "MFLO ");
				strcat(result, decToReg(binToDec(rd)));
				return result;
			}
			case 17: //MTHI
			{
				strcat(result, "MTHI ");
				strcat(result, decToReg(binToDec(rs)));
				return result;
			}
			case 19: //MTLO
			{
				strcat(result, "MTLO ");
				strcat(result, decToReg(binToDec(rs)));
				return result;
			}
			case 9: //JALR
			{
				strcat(result, "JALR ");
				if(binToDec(rd) != 0)
				{
					strcat(result, decToReg(binToDec(rd)));
					strcat(result, ", ");
				}
				strcat(result, decToReg(binToDec(rs)));
				return result;
			}
			case 12:
			{
				return "SYSCALL";
			}
			default:
				printf("Error: An I-type instruction with opcode %d does not exist in the MIPS instruction set.\n", opcode);
				return "**ERROR INSTRUCTION NOT FOUND**";
		}
}
const char* iTypeString(const char* bits)
{
	int opcode = 0;
	char op[7] = {};
	char rs[6] = {};
	char rt[6] = {};
	char imm[17] = {};
	char hex[11] = {};
	char target[27] = {};
	char* result;
	result = (char *)malloc(33*sizeof(char));
	for(int i = 0; i < 26; i++)
	{
		target[i] = bits[i+6]; //convert the immediate address into a string.
		if(i < 16)
		{
			imm[i] = bits[i+16]; //convert the immediate address into a string.
			if(i < 6)
			{
				op[i] = bits[i]; //convert the opcode into a string.
				if(i < 5)
				{ 
					rs[i] = bits[i+6]; //convert rs and rt into strings for editing later
					rt[i] = bits[i+11];
				}
			}
		}
	}
	opcode = longBinToDec(op);
	//printf("\n%d\n",opcode);
	switch(opcode)
		{
			case 8: //ADDI
			{
				strcat(result, "ADDI ");
				strcat(result, decToReg(binToDec(rt)));
				strcat(result, ", ");
				strcat(result, decToReg(binToDec(rs)));
				strcat(result, ", ");
				sprintf(hex, "%X", longBinToDec(imm));
				strcat(result, hex);
				return result;
			}
			case 9: //ADDIU
			{
				strcat(result, "ADDIU ");
				strcat(result, decToReg(binToDec(rt)));
				strcat(result, ", ");
				strcat(result, decToReg(binToDec(rs)));
				strcat(result, ", ");
				sprintf(hex, "%X", longBinToDec(imm));
				strcat(result, hex);
				return result;
			}
			case 12: //ANDI
			{
				strcat(result, "ANDI ");
				strcat(result, decToReg(binToDec(rt)));
				strcat(result, ", ");
				strcat(result, decToReg(binToDec(rs)));
				strcat(result, ", ");
				sprintf(hex, "%X", longBinToDec(imm));
				strcat(result, hex);
				return result;
			}
			case 13: //ORI
			{
				strcat(result, "ORI ");
				strcat(result, decToReg(binToDec(rt)));
				strcat(result, ", ");
				strcat(result, decToReg(binToDec(rs)));
				strcat(result, ", ");
				sprintf(hex, "%X", longBinToDec(imm));
				strcat(result, hex);
				return result;
			}
			case 14: //XORI
			{
				strcat(result, "XORI ");
				strcat(result, decToReg(binToDec(rt)));
				strcat(result, ", ");
				strcat(result, decToReg(binToDec(rs)));
				strcat(result, ", ");
				sprintf(hex, "%X", longBinToDec(imm));
				strcat(result, hex);
				return result;
			}
			case 10: //SLTI
			{
				strcat(result, "SLTI ");
				strcat(result, decToReg(binToDec(rt)));
				strcat(result, ", ");
				strcat(result, decToReg(binToDec(rs)));
				strcat(result, ", ");
				sprintf(hex, "%X", longBinToDec(imm));
				strcat(result, hex);
				return result;
			}
			case 35: //LW
			{
				strcat(result, "LW ");
				strcat(result, decToReg(binToDec(rt)));
				strcat(result, ", ");
				sprintf(hex, "%d(%s)", longBinToDec(imm), decToReg(binToDec(rs)));
				//sprintf(hex, "%s)", decToReg(binToDec(rs)));
				strcat(result, hex);
				return result;
			}
			case 16: //LB
			{
				strcat(result, "LB ");
				strcat(result, decToReg(binToDec(rt)));
				strcat(result, ", ");
				sprintf(hex, "%d(%s)", longBinToDec(imm), decToReg(binToDec(rs)));
				//sprintf(hex, "%s)", decToReg(binToDec(rs)));
				strcat(result, hex);
				return result;
			}
			case 17: //LH
			{
				strcat(result, "LH ");
				strcat(result, decToReg(binToDec(rt)));
				strcat(result, ", ");
				sprintf(hex, "%d(%s)", longBinToDec(imm), decToReg(binToDec(rs)));
				//sprintf(hex, "%s)", decToReg(binToDec(rs)));
				strcat(result, hex);
				return result;
			}
			case 15: //LUI
			{
				strcat(result, "LUI ");
				strcat(result, decToReg(binToDec(rt)));
				strcat(result, ", ");
				sprintf(hex, "%d", longBinToDec(imm));
				strcat(result, hex);
				return result;
			}
			case 43: //SW
			{
				strcat(result, "SW ");
				strcat(result, decToReg(binToDec(rt)));
				strcat(result, ", ");
				sprintf(hex, "%d(%s)", longBinToDec(imm), decToReg(binToDec(rs)));
				//sprintf(hex, "%s)", decToReg(binToDec(rs)));
				strcat(result, hex);
				return result;
			}
			case 40: //SB
			{
				strcat(result, "SB ");
				strcat(result, decToReg(binToDec(rt)));
				strcat(result, ", ");
				sprintf(hex, "%d(%s)", longBinToDec(imm), decToReg(binToDec(rs)));
				//sprintf(hex, "%s)", decToReg(binToDec(rs)));
				strcat(result, hex);
				return result;
			}
			case 41: //SH
			{
				strcat(result, "SH ");
				strcat(result, decToReg(binToDec(rt)));
				strcat(result, ", ");
				sprintf(hex, "%d(%s)", longBinToDec(imm), decToReg(binToDec(rs)));
				//sprintf(hex, "%s)", decToReg(binToDec(rs)));
				strcat(result, hex);
				return result;
			}
			case 4: //BEQ
			{
				strcat(result, "BEQ ");
				strcat(result, decToReg(binToDec(rs)));
				strcat(result, ", ");
				strcat(result, decToReg(binToDec(rt)));
				strcat(result, ", ");
				sprintf(hex, "%d", longBinToDec(imm));
				strcat(result, hex);
				return result;
			}
			case 5: //BNE
			{
				strcat(result, "BNE ");
				strcat(result, decToReg(binToDec(rs)));
				strcat(result, ", ");
				strcat(result, decToReg(binToDec(rt)));
				strcat(result, ", ");
				sprintf(hex, "%d", longBinToDec(imm));
				strcat(result, hex);
				return result;
			}
			case 6: //BLEZ
			{
				strcat(result, "BLEZ ");
				strcat(result, decToReg(binToDec(rs)));
				strcat(result, ", ");
				sprintf(hex, "%d", longBinToDec(imm));
				strcat(result, hex);
				return result;
			}
			case 7: //BGTZ
			{
				strcat(result, "BGTZ ");
				strcat(result, decToReg(binToDec(rs)));
				strcat(result, ", ");
				sprintf(hex, "%d", longBinToDec(imm));
				strcat(result, hex);
				return result;
			}
			case 2: //J
			{
				strcat(result, "J ");
				sprintf(hex, " 0x%X", longBinToDec(target));
				strcat(result, hex);
				return result;
			}
			case 3: //JAL
			{
				strcat(result, "JAL ");
				sprintf(hex, " 0x%X", longBinToDec(target));
				strcat(result, hex);
				return result;
			}
			default: 
				return "ERROR INSTRUCTION NOT FOUND";
		}	
}
const char* decToReg(int value)
{
	switch(value)
	{
		case 0:
			return "$zero";
		case 1:
			return "$at";
		case 2:
			return "$v0";
		case 3:
			return "$v1";
		case 4:
			return "$a0";
		case 5:
			return "$a1";
		case 6:
			return "$a2";
		case 7:
			return "$a3";
		case 8:
			return "$t0";
		case 9:
			return "$t1";
		case 10:
			return "$t2";
		case 11:
			return "$t3";
		case 12:
			return "$t4";
		case 13:
			return "$t5";
		case 14:
			return "$t6";
		case 15:
			return "$t7";
		case 16:
			return "$s0";
		case 17:
			return "$s1";
		case 18:
			return "$s2";
		case 19:	
			return "$s3";
		case 20:
			return "$s4";
		case 21:
			return "$s5";
		case 22:
			return "$s6";
		case 23:
			return "$s7";
		case 24:
			return "$t8";
		case 25:
			return "$t9";
		case 26:
			return "$k0";
		case 27:
			return "$k1";
		case 28:
			return "$gp";
		case 29:
			return "$sp";
		case 30:
			return "$sp";
		case 31:
			return "$fp";
		default:
			return "NULL";
	}
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


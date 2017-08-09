#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "tables.h"
#include "translate_utils.h"
#include "translate.h"

/* SOLUTION CODE BELOW */
const int TWO_POW_SEVENTEEN = 131072;    // 2^17

/* Writes instructions during the assembler's first pass to OUTPUT. The case
   for general instructions has already been completed, but you need to write
   code to translate the li and other pseudoinstructions. Your pseudoinstruction 
   expansions should not have any side effects.

   NAME is the name of the instruction, ARGS is an array of the arguments, and
   NUM_ARGS specifies the number of items in ARGS.

   Error checking for regular instructions are done in pass two. However, for
   pseudoinstructions, you must make sure that ARGS contains the correct number
   of arguments. You do NOT need to check whether the registers / label are 
   valid, since that will be checked in part two.

   Also for li:
    - make sure that the number is representable by 32 bits. (Hint: the number 
        can be both signed or unsigned).
    - if the immediate can fit in the imm field of an addiu instruction, then
        expand li into a single addiu instruction. Otherwise, expand it into 
        a lui-ori pair.

   If you are going to use the $zero or $0, use $0, not $zero.

   MARS has slightly different translation rules for li, and it allows numbers
   larger than the largest 32 bit number to be loaded with li. You should follow
   the above rules if MARS behaves differently.

   Use fprintf() to write. If writing multiple instructions, make sure that 
   each instruction is on a different line.

   Returns the number of instructions written (so 0 if there were any errors).
 */
unsigned write_pass_one(FILE* output, const char* name, char** args, int num_args) {
    //correct num of args
    if (strcmp(name, "li") == 0) {
        if (num_args != 2){
            return 0;
        }

        //representable
        long int imm = 0;
        int result = translate_num(&imm, args[1], INT32_MIN, UINT32_MAX);
        if(result == -1){
            return 0;
        }

        //use addiu
        if(imm >= INT16_MIN && imm <=  UINT16_MAX){
            fprintf(output, "%s", "addiu");
            fprintf(output, " %s %s", args[0], "$0");
            fprintf(output, " %ld\n", imm);
            
            return 1;
        }

        //use lui+ori
        else{
            //print lui
            int immx = imm / (TWO_POW_SEVENTEEN / 2);
            fprintf(output, "%s", "lui");
            fprintf(output, " %s", "$at");
            fprintf(output, " %d\n", immx);

            //print ori
            immx = imm - immx * (TWO_POW_SEVENTEEN / 2);
            fprintf(output, "%s", "ori");
            fprintf(output, " %s", args[0]);
            fprintf(output, " %s", "$at");
            fprintf(output, " %d\n", immx);

            return 2;
        }
    } else if (strcmp(name, "push") == 0) {
        if(num_args != 1){
            return 0;
        }

        //print addiu
        fprintf(output, "%s", "addiu");
        fprintf(output, " %s", "$sp");
        fprintf(output, " %s", "$sp");
        fprintf(output, " %d\n", -4);

        //print sw
        fprintf(output, "%s", "sw");
        fprintf(output, " %s", args[0]);
        fprintf(output, " %s\n", "0($sp)");

        return 2;
        
    } else if (strcmp(name, "pop") == 0) {
        if(num_args != 1){
            return 0;
        }

        //print lw
        fprintf(output, "%s", "lw");
        fprintf(output, " %s", args[0]);
        fprintf(output, " %s\n", "0($sp)");

        //print addiu
        fprintf(output, "%s", "addiu");
        fprintf(output, " %s", "$sp");
        fprintf(output, " %s", "$sp");
        fprintf(output, " %d\n", 4);

        return 2;
        
    } else if (strcmp(name, "mod") == 0) {
        if(num_args != 3){
            return 0;
        }

        //print div
        fprintf(output, "%s", "div");
        fprintf(output, " %s", args[1]);
        fprintf(output, " %s\n", args[2]);

        //print mfhi
        fprintf(output, "%s", "mfhi");
        fprintf(output, " %s\n", args[0]);

        return 2;
        
    } else if (strcmp(name, "subu") == 0) {
        if(num_args != 3){
            return 0;
        }

        //print addiu
        fprintf(output, "%s", "addiu");
        fprintf(output, " %s", "$at");
        fprintf(output, " %s", "$0");
        fprintf(output, " %d\n", -1);

        //print xor
        fprintf(output, "%s", "xor");
        fprintf(output, " %s", "$at");
        fprintf(output, " %s", "$at");
        fprintf(output, " %s\n", args[2]);

        //print addiu
        fprintf(output, "%s", "addiu");
        fprintf(output, " %s", "$at");
        fprintf(output, " %s", "$at");
        fprintf(output, " %d\n", 1);

        //print addu
        fprintf(output, "%s", "addu");
        fprintf(output, " %s", args[0]);
        fprintf(output, " %s", args[1]);
        fprintf(output, " %s\n", "$at");

        return 4;
    }

    write_inst_string(output, name, args, num_args);
    return 1;
}

/* Writes the instruction in hexadecimal format to OUTPUT during pass #2.
   
   NAME is the name of the instruction, ARGS is an array of the arguments, and
   NUM_ARGS specifies the number of items in ARGS. 

   The symbol table (SYMTBL) is given for any symbols that need to be resolved
   at this step. If a symbol should be relocated, it should be added to the
   relocation table (RELTBL), and the fields for that symbol should be set to
   all zeros. 

   You must perform error checking on all instructions and make sure that their
   arguments are valid. If an instruction is invalid, you should not write 
   anything to OUTPUT but simply return -1. MARS may be a useful resource for
   this step.

   Some function declarations for the write_*() functions are provided in translate.h, and you MUST implement these
   and use these as function headers. You may use helper functions, but you still must implement
   the provided write_* functions declared in translate.h.

   Returns 0 on success and -1 on error. 
 */
int translate_inst(FILE* output, const char* name, char** args, size_t num_args, uint32_t addr,
    SymbolTable* symtbl, SymbolTable* reltbl) {
	//Check for any null argument
	if(!output || !name || !args || !symtbl || !reltbl){
		return -1;
	}
    if (strcmp(name, "addu") == 0)       return write_rtype (0x21, output, args, num_args);
    else if (strcmp(name, "or") == 0)    return write_rtype (0x25, output, args, num_args);
    else if (strcmp(name, "slt") == 0)   return write_rtype (0x2a, output, args, num_args);
    else if (strcmp(name, "sltu") == 0)  return write_rtype (0x2b, output, args, num_args);
    else if (strcmp(name, "sll") == 0)   return write_shift (0x00, output, args, num_args);
    else if (strcmp(name, "xor") == 0)    return write_rtype (0x26, output, args, num_args);
    else if (strcmp(name, "jr") == 0)    return write_jr(0x08, output, args, num_args);
    else if (strcmp(name, "addiu") == 0)    return write_addiu(0x09, output, args, num_args);
    else if (strcmp(name, "ori") == 0)    return write_ori (0x0d, output, args, num_args);			
    else if (strcmp(name, "lui") == 0)    return write_lui (0x0f, output, args, num_args);
    else if (strcmp(name, "lb") == 0)    return write_mem (0x20, output, args, num_args);
    else if (strcmp(name, "lbu") == 0)    return write_mem (0x24, output, args, num_args);
    else if (strcmp(name, "lw") == 0)    return write_mem (0x23, output, args, num_args);
    else if (strcmp(name, "sb") == 0)    return write_mem (0x28, output, args, num_args);
    else if (strcmp(name, "sw") == 0)    return write_mem (0x2b, output, args, num_args);
    else if (strcmp(name, "beq") == 0)    return write_branch (0x04, output, args, num_args, addr, symtbl);
    else if (strcmp(name, "bne") == 0)    return write_branch (0x05, output, args, num_args, addr, symtbl);
    else if (strcmp(name, "j") == 0)    return write_jump (0x02, output, args, num_args, addr, reltbl);
    else if (strcmp(name, "jal") == 0)    return write_jump (0x03, output, args, num_args, addr, reltbl);
    else if (strcmp(name, "mult") == 0)    return write_mult_div(0x18, output, args, num_args);
    else if (strcmp(name, "div") == 0)    return write_mult_div(0x1a, output, args, num_args);
    else if (strcmp(name, "mfhi") == 0)    return write_mfhi_mflo(0x10, output, args, num_args);
    else if (strcmp(name, "mflo") == 0)    return write_mfhi_mflo(0x12, output, args, num_args);
    else{
	return -1;
    }
}

/* A helper function for writing most R-type instructions. You should use
   translate_reg() to parse registers and write_inst_hex() to write to 
   OUTPUT. Both are defined in translate_utils.h.

   This function is INCOMPLETE. Complete the implementation below. You will
   find bitwise operations to be the cleanest way to complete this function.
 */
int write_rtype(uint8_t funct, FILE* output, char** args, size_t num_args) {
	//Check if number of arguments is correct
	if(num_args != 3){return -1;}	
	int rd = translate_reg(args[0]);
	int rs = translate_reg(args[1]);
	int rt = translate_reg(args[2]);
	//validate the register
	if(rd==-1 || rs==-1 || rt==-1){
		return -1;
	}
	
	uint32_t instruction =0;
	rs = rs << 21;
	rt = rt << 16;
	rd = rd << 11;
	instruction = instruction + rs + rt + rd + funct;	
	write_inst_hex(output, instruction);
	return 0;
}

/* A helper function for writing shift instructions. You should use 
   translate_num() to parse numerical arguments. translate_num() is defined
   in translate_utils.h.

   This function is INCOMPLETE. Complete the implementation below. You will
   find bitwise operations to be the cleanest way to complete this function.
*/
int write_shift(uint8_t funct, FILE* output, char** args, size_t num_args) {
	//check number of arguments
	if(num_args!=3){
		return -1;
	}

    	long int shamt;
    	int rd = translate_reg(args[0]);
    	int rt = translate_reg(args[1]);
    	int err = translate_num(&shamt, args[2], 0, 31);
	//Validate the registers and shift amount
	if(rd==-1 || rt==-1 || err==-1 || shamt>31 || shamt<0){
		return -1;
	}

    	uint32_t instruction = 0;
	rt = rt << 16;
	rd = rd << 11;
	shamt = shamt << 6;
	instruction = instruction + rt + rd + shamt + funct;
    	write_inst_hex(output, instruction);
    	return 0;
}

/* The rest of your write_*() functions below */

int write_jr(uint8_t funct, FILE* output, char** args, size_t num_args) {
    // Check the number of arguments
	if(num_args!=1){
		return -1;
	}
	int rs = translate_reg(args[0]);
	if(rs == -1){
		return -1;
	}

        uint32_t instruction = 0;
	rs = rs << 21;
	instruction = rs + funct;
    	write_inst_hex(output, instruction);
    return 0;
}

int write_addiu(uint8_t opcode, FILE* output, char** args, size_t num_args) {
    //check the number of arguments
    	if(num_args != 3){
		return -1;
	}

    	long int imm;
    	int rt = translate_reg(args[0]);
    	int rs = translate_reg(args[1]);
    	int err = translate_num(&imm, args[2], INT16_MIN, INT16_MAX);
	//Validate the registers
	if(rt==-1 || rs==-1 || err==-1 ){
		return -1;
	}

    	uint32_t instruction = 0;
	rt = rt << 16;
	rs = rs << 21;
	imm = imm&(TWO_POW_SEVENTEEN/2-1);
	uint32_t new_opcode = (uint32_t)opcode;
	new_opcode = new_opcode << 26;
	instruction =( new_opcode + instruction + rt + rs) | imm;
    	write_inst_hex(output, instruction);
    	return 0;
}

int write_ori(uint8_t opcode, FILE* output, char** args, size_t num_args) {
  
    	if(num_args!=3){
		return -1;
	}
    	long int imm;
    	int rt = translate_reg(args[0]);
    	int rs = translate_reg(args[1]);
    	int err = translate_num(&imm, args[2], 0, UINT16_MAX);
	//Validate the registers
	if(rt==-1 || rs==-1 || err==-1){
		return -1;
	}
	
    	uint32_t instruction = 0;
	rt = rt << 16;
	rs = rs << 21;
	uint32_t new_opcode = (uint32_t)opcode;
	new_opcode = new_opcode<<26;
	instruction = new_opcode + instruction + rt + rs + imm;
    	write_inst_hex(output, instruction);
    	return 0;
}

int write_mult_div(uint8_t funct, FILE* output, char** args, size_t num_args) {
    	
	if(num_args != 2){
		return -1;
	}
	int rs = translate_reg(args[0]);
	int rt = translate_reg(args[1]);
		        
	if (rs < 0 || rt < 0) {
		return -1;
	}
			    
	uint32_t instruction = 0;
	rs = rs << 21;
	rt = rt << 16;
	instruction = rs + rt + funct;
	write_inst_hex(output, instruction);
	return 0;
}

int write_mfhi_mflo(uint8_t funct, FILE* output, char** args, size_t num_args) {
    	//Check number of arguments
	if(num_args != 1){
		return -1;
	}
	int rd = translate_reg(args[0]);
	if (rd < 0) {
		return -1;
	}
	uint32_t instruction = 0;
	rd = rd << 11;
	instruction = rd + funct;
	write_inst_hex(output, instruction);
	return 0;
}

int write_lui(uint8_t opcode, FILE* output, char** args, size_t num_args) {
    //check number of arguments
   	if(num_args != 2){
		return -1;
	}
    	long int imm;
    	int rt = translate_reg(args[0]);
    	int err = translate_num(&imm, args[1], 0, UINT16_MAX);
	if(rt == -1 || err == -1){
		return -1;
	}
    	uint32_t instruction = 0;
	rt = rt << 16;
	uint32_t new_opcode = (uint32_t)opcode;
	new_opcode = new_opcode << 26;
	instruction = instruction + rt + imm + new_opcode;
    	write_inst_hex(output, instruction);
    	return 0;
}

int write_mem(uint8_t opcode, FILE* output, char** args, size_t num_args) {
    // Check the number of arguments
    if(num_args != 3){
	return -1;
    }
    long int imm;
    int rt = translate_reg(args[0]);
    int rs = translate_reg(args[2]);
    int err = translate_num(&imm, args[1], INT16_MIN, INT16_MAX);
    if(rt==-1 || rs==-1 || err==-1){
	return -1;
    }	    	
    uint32_t instruction =0;
    rt = rt << 16;
    rs = rs << 21;	
    uint32_t new_opcode = (uint32_t)opcode;
    new_opcode = new_opcode << 26;
    imm = imm & (TWO_POW_SEVENTEEN/2 -1);
    instruction = (instruction + new_opcode + rt + rs) | imm;
    write_inst_hex(output, instruction);
    return 0;
    
}
/*  A helper function to determine if a destination address
    can be branched to
*/
static int can_branch_to(uint32_t src_addr, uint32_t dest_addr) {
    int32_t diff = dest_addr - src_addr;
    return (diff >= 0 && diff <= TWO_POW_SEVENTEEN) || (diff < 0 && diff >= -(TWO_POW_SEVENTEEN - 4));
}


int write_branch(uint8_t opcode, FILE* output, char** args, size_t num_args, uint32_t addr, SymbolTable* symtbl) {
    //check the number of arguments
    	if(num_args!=3){
		return -1;
	}
        int rs = translate_reg(args[0]);
        int rt = translate_reg(args[1]);
        int label_addr = get_addr_for_symbol(symtbl, args[2]);
	if(rs==-1 || rt==-1 || label_addr==-1 || !(can_branch_to(addr, label_addr))){
		return -1;
	}
	//calculate the offset
    	int32_t offset = 0;
    	offset = label_addr - (addr+4);
	offset = offset >> 2;
	
	offset = offset&(TWO_POW_SEVENTEEN/2 -1 );

	uint32_t instruction = 0;
	rs = rs << 21;
	rt = rt << 16;
	uint32_t new_opcode = (uint32_t)opcode;
	new_opcode = new_opcode << 26;
	instruction = (rs + rt + new_opcode)|offset;
        write_inst_hex(output, instruction);        
	return 0;
}

int write_jump(uint8_t opcode, FILE* output, char** args, size_t num_args, uint32_t addr, SymbolTable* reltbl) {
    	if(num_args!=1){
		return -1;
	}
    	
    	uint32_t instruction = 0;
	//write the labels to be reallocated into the relocation table
	
		//whenever an unresolved label is encountered, save the label name and corresponding address to relocation
		//tabel. So that afterwards linker can replace the address of that label into their address in relocation table
	add_to_table(reltbl,args[0],addr);

	uint32_t new_opcode = (uint32_t)opcode;
	new_opcode = new_opcode << 26;	
	//leave the immediate field 0 waiting for further resolving job by linker
	instruction = new_opcode;
    	write_inst_hex(output, instruction);	
        return 0;
}

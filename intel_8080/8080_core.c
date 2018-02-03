#include <stdio.h>
#include <stdint.h>
#include "8080_core_SDL.h"
#include <stdbool.h>

void init_cpu(e8080_cpu *CPU) {
  //running
  CPU->running = 0x01;
  CPU->reg_A = 0x00;
  CPU->reg_B = 0x00;
  CPU->reg_C = 0x00;
  CPU->reg_D = 0x00;
  CPU->reg_E = 0x00;
  CPU->reg_H = 0x00;
  CPU->reg_L = 0x00;
  //flagbyte register
  CPU->reg_flagbyte = 0x00;
  //register pairs
  CPU->reg_pair_HL = 0x0000;
  CPU->reg_pair_BC = 0x0000;
  CPU->reg_pair_DE = 0x0000;
  CPU->reg_pair_PSW = 0x0000;

  //FLAGS
  CPU->sign_flag = 0x00;
  CPU->zero_flag = 0x00;
  CPU->aux_carry_flag = 0x00;
  CPU->parity_flag = 0x00;
  CPU->carry_flag = 0x00;
  CPU->interupts_enabled = false;

  //program counter and stack pointer
  CPU->SP = 0x0000;
  CPU->PC = 0x0000;
  //IO
  //memset(CPU->IN,0x00,0xFF);
  //memset(CPU->OUT,0x00,0xFF);
  for (int t = 0; t < 0xFF; t++) {
  CPU->IN[t] = 0x00;
  CPU->OUT[t] = 0x00;

  }


}


void e8080_interrupt_cpu(e8080_cpu *CPU, uint8_t *Memory, int rst_nr) {
  Memory[CPU->SP - 0x01] = ((CPU->PC - 0x03) & 0xFF00) >> 8;
  Memory[CPU->SP - 0x02] = ((CPU->PC - 0x03) & 0x00FF);
  CPU->SP = CPU->SP - 0x02;
  CPU->PC = (0x0008 * rst_nr);
  CPU->interupts_enabled = false;
}


void setflags(uint8_t flag_picker, uint8_t operand_1, char operator, uint8_t operand_2, e8080_cpu *CPU) {
  uint8_t result;
  uint8_t parity_count = 0x00;

  if (operator == '+') { //calc result
    result = (operand_1 + operand_2);
  } else if (operator == '-') {
    result = (operand_1 - operand_2);
  } else if (operator == '&') {
    result = (operand_1 & operand_2);
  } else if (operator == '^') {
    result = (operand_1 ^ operand_2);
  } else if (operator == '|') {
    result = (operand_1 | operand_2);
  }

  if ((flag_picker & 0x80) == 0x80) { //Sign flag
    if (result >= 0x80) {
      CPU->sign_flag = 0x01;
    } else {
      CPU->sign_flag = 0x00;
    }
  }

  if ((flag_picker & 0x40) == 0x40) { //Zero flag
    if (result == 0x00) {
      CPU->zero_flag = 0x01;
    } else {
      CPU->zero_flag = 0x00;
    }
  }

  if ((flag_picker & 0x10) == 0x10) { // auxilary C flag, checks for carry from 4 Least significant bits
    if (((operand_1 & 0x0F) + (operand_1 & 0x0F)) > 0x0F) {
      CPU->aux_carry_flag = 0x01;
    } else if  (((operand_1 & 0x0F) + (operand_1 & 0x0F)) > 0x0F) {
      CPU->aux_carry_flag = 0x00;
    }
  }


  // calc nr of 1s in binary nr
  if ((result & 0x01) == 0x01) {
    parity_count = parity_count + 0x01;
  }
  if ((result & 0x02) == 0x02) {
    parity_count = parity_count + 0x01;
  }
  if ((result & 0x04) == 0x04) {
    parity_count = parity_count + 0x01;
  }
  if ((result & 0x08) == 0x08) {
    parity_count = parity_count + 0x01;
  }
  if ((result & 0x10) == 0x10) {
    parity_count = parity_count + 0x01;
  }
  if ((result & 0x20) == 0x20) {
    parity_count = parity_count + 0x01;
  }
  if ((result & 0x40) == 0x40) {
    parity_count = parity_count + 0x01;
  }
  if ((result & 0x80) == 0x80) {
    parity_count = parity_count + 0x01;
  }
  //printf("parity_count: %02X\n", parity_count);
  // calc done

  if ((flag_picker & 0x04) == 0x04) { // check if parity_count is even or odd and set p flag accordingly
    if (parity_count % 0x02 == 0) {
      CPU->parity_flag = 0x01;
    } else {
      CPU->parity_flag = 0x00;
    }
  }


  if ((flag_picker & 0x01) == 0x01) { //carry flag
    if (operator == '+' && (operand_1 + operand_2) > 0xFF ) {   // Carryflag on addition
      CPU->carry_flag = 0x01;
    } else if (operator == '+' && (operand_1 + operand_2) < 0xFF) {
      CPU->carry_flag = 0x00;
    }

    if (operator == '-' && (operand_1 < operand_2)) { // Carryflag on subraction
      CPU->carry_flag = 0x01;
    } else if (operator == '-' && (operand_1 >= operand_2)) {
      CPU->carry_flag = 0x00;
    }
  }

}


int e8080_perform_instruction(e8080_cpu *CPU, uint8_t *Memory,int nr_of_cycles) {

    uint8_t OP_CODE_0 = Memory[CPU->PC];
    uint8_t OP_CODE_1 = Memory[CPU->PC + 1];
    uint8_t OP_CODE_2 = Memory[CPU->PC + 2];
    CPU->reg_pair_BC = (CPU->reg_B << 8) + CPU->reg_C;
    CPU->reg_pair_DE = (CPU->reg_D << 8) + CPU->reg_E;
    CPU->reg_pair_HL = (CPU->reg_H << 8) + CPU->reg_L;
    uint8_t inc_PC_with = 0x0001; // default, used for all one byte instructions
    uint8_t old_carry = CPU->carry_flag;
    uint8_t swap;
    int cycles_performed = 4; //start at 4 because its the most common ammount

    // switch case for all OP-Codes
    switch (OP_CODE_0) {
      case 0x00: /*printf("%04X   NOP\n",CPU->PC);*/ break;

      case 0x01:
        {
          //printf("%04X   LXI  B   #$%02X%02X\n", CPU->PC, OP_CODE_2, OP_CODE_1);
          CPU->reg_B = OP_CODE_2;
          CPU->reg_C = OP_CODE_1;
          //printf("B: %02X C: %02X\n",CPU->reg_B,CPU->reg_C);
          inc_PC_with = 0x03;
          cycles_performed = 10;
          break; // store data in CPU->reg_pair_BC
        }

      case 0x02:
        {
          //printf("%04X   STAX B\n",CPU->PC);
          Memory[CPU->reg_pair_BC] = CPU->reg_A;
          //printf("Memory[CPU->reg_pair_BC]: %02X A: %02X\n",Memory[CPU->reg_pair_BC],CPU->reg_A);
          cycles_performed = 7;
          break; // store the value of A in memorycell$CPU->reg_pair_BC
        }

      case 0x03:
        {
          //printf("%04X   INX  B\n",CPU->PC);
          if (CPU->reg_C == 0xFF) {
            CPU->reg_B = CPU->reg_B + 0x01;
          }
          CPU->reg_C= CPU->reg_C + 0x01;
          cycles_performed = 5;
          //printf("B:%02x C:%02X\n",CPU->reg_B,CPU->reg_C);
          break; // incr. CPU->reg_pair_BC by one
        }

      case 0x04:
        {
          //printf("%04X   INR  B\n",CPU->PC);
          setflags(0xD4,CPU->reg_B, '+', 0x01, CPU);
          CPU->reg_B = CPU->reg_B + 0x01;
          cycles_performed = 5;

          break; // incr. B by one
        }

      case 0x05:
        {
          //printf("%04X   DCR  B\n",CPU->PC);
          setflags(0xD4,CPU->reg_B, '-', 0x01, CPU);
          CPU->reg_B = CPU->reg_B - 0x01;
          cycles_performed = 5;
          break; // decr. B by one
        }

      case 0x06:
        {
          //printf("%04X   MVI  B   #$%02X\n",CPU->PC, OP_CODE_1);
          CPU->reg_B = OP_CODE_1;
          inc_PC_with = 0x02;
           cycles_performed = 7;
          break; // puts the value of the next bit into B
        }

      case 0x07:
        {
          //printf("%04X   RLC\n",CPU->PC);
          //printf("A before: %02X\n", CPU->reg_A);
          if (CPU->reg_A >= 0x80) {
            CPU->reg_A = (CPU->reg_A << 1) + 0x01;
            CPU->carry_flag = 0x01;
          } else {
            CPU->reg_A = (CPU->reg_A << 1);
            CPU->carry_flag = 0x00;
          }

          //printf("A after: %02X\n", CPU->reg_A);
          //printf("C flag: %02X\n", CPU->carry_flag);

          break;  //Left bitwise swap
        }

      case 0x08: /*printf("%04X   NOP\n",CPU->PC);*/ break;

      case 0x09:
        {

          //printf("CPU->reg_pair_BC: %04X  CPU->reg_pair_HL: %04X  H: %02X  L: %02X\n",CPU->reg_pair_BC,CPU->reg_pair_HL,CPU->reg_H,CPU->reg_L);
          //printf("%04X   DAD  B\n",CPU->PC);
          if ((CPU->reg_pair_HL + CPU->reg_pair_BC) > 0xFFFF) {
            CPU->carry_flag = 0x01;
          }else {
            CPU->carry_flag = 0x00;
          }
          CPU->reg_pair_HL = CPU->reg_pair_HL + CPU->reg_pair_BC;
          CPU->reg_H = (CPU->reg_pair_HL & 0xFF00) >> 8;
          CPU->reg_L = (CPU->reg_pair_HL & 0x00FF);
          //printf("CPU->reg_pair_HL: %04X  H: %02X  CARRY: %02X\n",CPU->reg_pair_HL,CPU->reg_H,CPU->carry_flag);

          cycles_performed = 10;
          break;  //adds two 16 bit numbers and loads them in CPU->reg_pair_HL, CPU->reg_pair_HL = CPU->reg_pair_HL + CPU->reg_pair_BC, carry flag is set if overflow
        }

      case 0x0A:
        {
          //printf("%04X   LDAX B\n",CPU->PC);
          CPU->reg_A = Memory[CPU->reg_pair_BC];
          //printf("A: %02X  Memory[CPU->reg_pair_BC]: %02X\n",CPU->reg_A,Memory[CPU->reg_pair_BC]);
          cycles_performed = 7;
          break;
        }   //loads A with data at Memory[CPU->reg_pair_BC]

      case 0x0B:
        {
          //printf("%04X   DCX  B\n",CPU->PC);
          if (CPU->reg_C == 0x00) {
            CPU->reg_B = CPU->reg_B - 0x01;
          }
          CPU->reg_C= CPU->reg_C - 0x01;
          //printf("B:%02x C:%02X\n",CPU->reg_B,CPU->reg_C);
          cycles_performed = 5;
           break;
        } //decr. CPU->reg_pair_BC by one

      case 0x0C:
        {
          //printf("%04X   INR  C\n",CPU->PC);
          setflags(0xD4,CPU->reg_C, '+', 0x01, CPU);
          CPU->reg_C = CPU->reg_C + 0x01;
          cycles_performed = 5;
          break;
        }  // incr. C by one

      case 0x0D:
        {
          //printf("%04X   DCR  C\n",CPU->PC);
          setflags(0xD4,CPU->reg_C, '-', 0x01, CPU);
          CPU->reg_C = CPU->reg_C - 0x01;
          cycles_performed = 5;
          break; // decr. C by one
        }

      case 0x0E:
        {
          //printf("%04X   MVI  C   #$%02X\n",CPU->PC, OP_CODE_1);
          CPU->reg_C = OP_CODE_1;
          inc_PC_with = 0x02;
           cycles_performed = 7;
          break; // puts the value of the next bit into C
        }

      case 0x0F:
        {
          //printf("%04X   RRC\n",CPU->PC);
          //printf("A before: %02X\n", CPU->reg_A);
          CPU->carry_flag = (CPU->reg_A & 0x01);
          CPU->reg_A = ((CPU->reg_A >> 1) | (CPU->carry_flag << 7));


          /*if (CPU->reg_A % 0x02 != 0) {
            CPU->reg_A = (CPU->reg_A >> 1) + 0x80;
            CPU->carry_flag = 0x01;
          } else {
            CPU->reg_A = (CPU->reg_A >> 1);
            CPU->carry_flag = 0x00;
          }*/

          //printf("A after: %02X\n", CPU->reg_A);
          //printf("C flag: %02X\n", CPU->carry_flag);

           break;
        }




        /*********************************************************************/



      case 0x10: /*printf("%04X   NOP\n",CPU->PC);*/ break;

      case 0x11:
        {
          //printf("%04X   LXI  D   #$%02X%02X\n", CPU->PC, OP_CODE_2, OP_CODE_1);
          CPU->reg_D = OP_CODE_2;
          CPU->reg_E = OP_CODE_1;
          //printf("D: %02X E: %02X\n",CPU->reg_D,CPU->reg_E);
          inc_PC_with = 3;
          cycles_performed = 10;
          break; // store data inCPU->reg_pair_DE
        }

      case 0x12:
        {
          //printf("%04X   STAX D\n",CPU->PC);
          Memory[CPU->reg_pair_DE] = CPU->reg_A;
          //printf("Memory[CPU->reg_pair_DE]: %02X A: %02X\n",Memory[CPU->reg_pair_DE],CPU->reg_A);
          cycles_performed = 7;
          break; // store the value of A in memorycell$DE
        }

      case 0x13:
        {
          //printf("%04X   INX  D\n",CPU->PC);
          if (CPU->reg_E == 0xFF) {
            CPU->reg_D = CPU->reg_D + 0x01;
          }
          CPU->reg_E = CPU->reg_E + 0x01;
          cycles_performed = 5;
          //printf("D:%02x E:%02X\n",CPU->reg_D,CPU->reg_E);
          break; // incr.CPU->reg_pair_DE by one
        }

      case 0x14:
        {
          //printf("%04X   INR  D\n",CPU->PC);
          setflags(0xD4,CPU->reg_D, '+', 0x01, CPU);
          CPU->reg_D = CPU->reg_D + 0x01;
          cycles_performed = 5;
          break; // incr. D by one
        }

      case 0x15:
        {
          //printf("%04X   DCR  D\n",CPU->PC);
          setflags(0xD4,CPU->reg_D, '-', 0x01, CPU);
          CPU->reg_D = CPU->reg_D - 0x01;
          cycles_performed = 5;
          break; // decr. D by one
        }

      case 0x16:
        {
          //printf("%04X   MVI  D   #$%02X\n",CPU->PC, OP_CODE_1);
          CPU->reg_D = OP_CODE_1;
          inc_PC_with = 0x02;
           cycles_performed = 7;
          break; // puts the value of the next bit into D
        }

      case 0x17:
        {
          //printf("%04X   RAL\n",CPU->PC);
          //printf("A before: %02X\n", CPU->reg_A);
          if (CPU->reg_A >= 0x80) {
            CPU->reg_A = (CPU->reg_A << 1) + old_carry;
            CPU->carry_flag = 0x01;
          } else {
            CPU->reg_A = (CPU->reg_A << 1) + old_carry;
            CPU->carry_flag = 0x00;
          }

          //printf("A after: %02X\n", CPU->reg_A);
          //printf("C flag: %02X\n", CPU->carry_flag);

          break;  //Left bitwise swap through carry
        }

      case 0x18: /*printf("%04X   NOP\n",CPU->PC);*/ break;

      case 0x19:
        {

          //printf("DE: %04X  CPU->reg_pair_HL: %04X  H: %02X  L: %02X\n",DE,CPU->reg_pair_HL,CPU->reg_H,CPU->reg_L);
          //printf("%04X   DAD  D\n",CPU->PC);
          if ((CPU->reg_pair_HL +CPU->reg_pair_DE) > 0xFFFF) {
            CPU->carry_flag = 0x01;
          }else {
            CPU->carry_flag = 0x00;
          }
          CPU->reg_pair_HL = CPU->reg_pair_HL +CPU->reg_pair_DE;
          CPU->reg_H = (CPU->reg_pair_HL & 0xFF00) >> 8;
          CPU->reg_L = (CPU->reg_pair_HL & 0x00FF);
          //printf("CPU->reg_pair_HL: %04X  H: %02X  CARRY: %02X\n",CPU->reg_pair_HL,CPU->reg_H,CPU->carry_flag);

          cycles_performed = 10;
          break;  //adds two 16 bit numbers and loads them in CPU->reg_pair_HL, CPU->reg_pair_HL = CPU->reg_pair_HL +CPU->reg_pair_DE, carry flag is set if overflow
        }

      case 0x1A:
        {
          //printf("%04X   LDAX D\n",CPU->PC);
          CPU->reg_A = Memory[CPU->reg_pair_DE];
          //printf("A: %02X  Memory[CPU->reg_pair_DE]: %02X\n",CPU->reg_A,Memory[CPU->reg_pair_DE]);
          cycles_performed = 7;
          break;
        }   //loads A with data at Memory[CPU->reg_pair_DE]

      case 0x1B:
        {
          //printf("%04X   DCX  D\n",CPU->PC);
          if (CPU->reg_E == 0x00) {
            CPU->reg_D = CPU->reg_D - 0x01;
          }
          CPU->reg_E= CPU->reg_E - 0x01;
          cycles_performed = 5;
          //printf("D:%02x E:%02X\n",CPU->reg_D,CPU->reg_E);
           break;
        } //decr.CPU->reg_pair_DE by one

      case 0x1C:
        {
          //printf("%04X   INR  E\n",CPU->PC);
          setflags(0xD4,CPU->reg_E, '+', 0x01, CPU);
          CPU->reg_E = CPU->reg_E + 0x01;
          cycles_performed = 5;
          break;
        }  // incr. E by one

      case 0x1D:
        {
          //printf("%04X   DCR  E\n",CPU->PC);
          setflags(0xD4,CPU->reg_E, '-', 0x01, CPU);
          CPU->reg_E = CPU->reg_E - 0x01;
          cycles_performed = 5;
          break; // decr. E by one
        }

      case 0x1E:
        {
          //printf("%04X   MVI  E   #$%02X\n",CPU->PC, OP_CODE_1);
          CPU->reg_E = OP_CODE_1;
          inc_PC_with = 0x02;
           cycles_performed = 7;
          break; // puts the value of the next bit into E
        }

      case 0x1F:
        {
          //printf("%04X   RAR\n",CPU->PC);
          //printf("A before: %02X\n", CPU->reg_A);
          if (CPU->reg_A % 0x02 != 0) {
            CPU->reg_A = (CPU->reg_A >> 1) + (old_carry * 0x80);
            CPU->carry_flag = 0x01;
          } else {
            CPU->reg_A = (CPU->reg_A >> 1) + (old_carry * 0x80);
            CPU->carry_flag = 0x00;
          }

          //printf("A after: %02X\n", CPU->reg_A);
          //printf("C flag: %02X\n", CPU->carry_flag);

           break;
        }



          /*****************************************************************/




      case 0x20: /*printf("%04X   NOP\n",CPU->PC);*/ break;

      case 0x21:
        {
          //printf("%04X   LXI  H   #$%02X%02X\n", CPU->PC, OP_CODE_2, OP_CODE_1);
          CPU->reg_H = OP_CODE_2;
          CPU->reg_L = OP_CODE_1;
          //printf("H: %02X L: %02X\n",CPU->reg_H,CPU->reg_L);
          inc_PC_with = 3;
          cycles_performed = 10;
          break; // store data inCPU->reg_pair_DE
        }

      case 0x22:
        {
          //printf("%04X   SHLD     #$%02X%02X\n", CPU->PC, OP_CODE_2, OP_CODE_1);
          Memory[((OP_CODE_2 << 8) + OP_CODE_1)] = CPU->reg_L;
          Memory[((OP_CODE_2 << 8) + OP_CODE_1) + 0x0001] = CPU->reg_H;
          //printf("H: %02X L: %02X  Memory[low]: %02X Memory[high]: %02X\n",CPU->reg_H,CPU->reg_L,Memory[((OP_CODE_2 << 8) + OP_CODE_1)],Memory[((OP_CODE_2 << 8) + OP_CODE_1) + 0x0001]);
          inc_PC_with = 3;
          cycles_performed = 16;
          break; // store L and H direct
        }

      case 0x23:
        {
          //printf("%04X   INX  H\n",CPU->PC);
          if (CPU->reg_L == 0xFF) {
            CPU->reg_H = CPU->reg_H + 0x01;
          }
          CPU->reg_L = CPU->reg_L + 0x01;
          cycles_performed = 5;
          //printf("H:%02x L:%02X\n",CPU->reg_H,CPU->reg_L);
          break; // incr. CPU->reg_pair_HL by one
        }

      case 0x24:
        {
          //printf("%04X   INR  H\n",CPU->PC);
          setflags(0xD4,CPU->reg_H, '+', 0x01, CPU);
          CPU->reg_H = CPU->reg_H + 0x01;
          cycles_performed = 5;
          break; // incr. H by one
        }

      case 0x25:
        {
          //printf("%04X   DCR  H\n",CPU->PC);
          setflags(0xD4,CPU->reg_H, '+', 0x01, CPU);
          CPU->reg_H = CPU->reg_H - 0x01;
          cycles_performed = 5;
          break; // decr. H by one
        }

      case 0x26:
        {
          //printf("%04X   MVI  H   #$%02X\n",CPU->PC, OP_CODE_1);
          CPU->reg_H = OP_CODE_1;
          inc_PC_with = 0x02;
           cycles_performed = 7;
          break; // puts the value of the next bit into H
        }

      case 0x27:
        {
          printf("%04X   DAA\n",CPU->PC);
          if ( ( (CPU->reg_A & 0x0F) > 0x09)) { //possible ERROR
            CPU->reg_A = CPU->reg_A + 6;
            CPU->aux_carry_flag = 0x01;
          } else {
            CPU->aux_carry_flag = 0x00;
          }

          if ( (CPU->reg_A > 0x9F) || old_carry) {
            CPU->reg_A = CPU->reg_A + 0x60;
              CPU->carry_flag = 0x01;
          } else {
            CPU->carry_flag = 0x00;
          }
          break;
        }

      case 0x28: /*printf("%04X   NOP\n",CPU->PC);*/ break;

      case 0x29:
        {

          //printf("CPU->reg_pair_HL: %04X  CPU->reg_pair_HL: %04X  H: %02X  L: %02X\n",CPU->reg_pair_HL,CPU->reg_pair_HL,CPU->reg_H,CPU->reg_L);
          //printf("%04X   DAD  H\n",CPU->PC);
          if ((CPU->reg_pair_HL + CPU->reg_pair_HL) > 0xFFFF) {
            CPU->carry_flag = 0x01;
          }else {
            CPU->carry_flag = 0x00;
          }
          CPU->reg_pair_HL = CPU->reg_pair_HL + CPU->reg_pair_HL;
          CPU->reg_H = (CPU->reg_pair_HL & 0xFF00) >> 8;
          CPU->reg_L = (CPU->reg_pair_HL & 0x00FF);
          //printf("CPU->reg_pair_HL: %04X  H: %02X  CARRY: %02X\n",CPU->reg_pair_HL,CPU->reg_H,CPU->carry_flag);

          cycles_performed = 10;
          break;  //adds two 16 bit numbers and loads them in CPU->reg_pair_HL, CPU->reg_pair_HL = CPU->reg_pair_HL + CPU->reg_pair_HL, carry flag is set if overflow
        }

      case 0x2A:
        {
          //printf("%04X   LHLD     #$%02X%02X\n", CPU->PC, OP_CODE_2, OP_CODE_1);
          CPU->reg_L = Memory[((OP_CODE_2 << 8) + OP_CODE_1)];
          CPU->reg_H = Memory[((OP_CODE_2 << 8) + OP_CODE_1) + 0x0001];
          //printf("H: %02X L: %02X  Memory[low]: %02X Memory[high]: %02X\n",CPU->reg_H,CPU->reg_L,Memory[((OP_CODE_2 << 8) + OP_CODE_1)],Memory[((OP_CODE_2 << 8) + OP_CODE_1) + 0x0001]);
          inc_PC_with = 3;
          cycles_performed = 16;
          break;
        }   //loads L and H with data at Memory[data and data+1]

      case 0x2B:
        {
          //printf("%04X   DCX  H\n",CPU->PC);
          if (CPU->reg_L == 0x00) {
            CPU->reg_H = CPU->reg_H - 0x01;
          }
          CPU->reg_L= CPU->reg_L - 0x01;
          cycles_performed = 5;
          //printf("H:%02x L:%02X\n",CPU->reg_H,CPU->reg_L);
           break;
        } //decr. CPU->reg_pair_HL by one

      case 0x2C:
        {
          //printf("%04X   INR  L\n",CPU->PC);
          setflags(0xD4,CPU->reg_L, '+', 0x01, CPU);
          CPU->reg_L = CPU->reg_L + 0x01;
          cycles_performed = 5;
          break;
        }  // incr. L by one

      case 0x2D:
        {
          //printf("%04X   DCR  L\n",CPU->PC);
          setflags(0xD4,CPU->reg_L, '-', 0x01, CPU);
          CPU->reg_L = CPU->reg_L - 0x01;
          cycles_performed = 5;
          break; // decr. L by one
        }

      case 0x2E:
        {
          //printf("%04X   MVI  L   #$%02X\n",CPU->PC, OP_CODE_1);
          CPU->reg_L = OP_CODE_1;
          inc_PC_with = 0x02;
           cycles_performed = 7;
          break; // puts the value of the next bit into L
        }

      case 0x2F:
        {
          //printf("%04X   CMA\n",CPU->PC);
          //printf("A before: %02X\n", CPU->reg_A);
          CPU->reg_A = CPU->reg_A ^ 0xFF;
          //printf("A after: %02X\n", CPU->reg_A);


           break; //producess 1 complement of A
        }



          /********************************************************************/



      case 0x30: /*printf("%04X   NOP\n",CPU->PC);*/ break;

      case 0x31:
        {
          //printf("%04X   LXI  SP   #$%02X%02X\n", CPU->PC, OP_CODE_2, OP_CODE_1);
          CPU->SP = (OP_CODE_2 << 8) + OP_CODE_1;
          //printf("SP: %04X\n",CPU->SP);
          inc_PC_with = 0x03;
          cycles_performed = 10;
          break; // store data in SP
        }

      case 0x32:
        {
          //printf("%04X   STA         $%02X%02X\n",CPU->PC, OP_CODE_2, OP_CODE_1); //ERROR
          Memory[(OP_CODE_2 << 8) + OP_CODE_1] = CPU->reg_A;
          inc_PC_with = 0x03;
          cycles_performed = 13;
          //printf("Memory[DATA]: %02X A: %02X\n",Memory[(OP_CODE_2 << 8) + OP_CODE_1],CPU->reg_A);
          break; // store the value of A in memorycell$DATA
        }

      case 0x33:
        {
          //printf("%04X   INX  SP\n",CPU->PC);
          CPU->SP = CPU->SP + 0x01;
          cycles_performed = 5;
          //printf("SP: %04X\n",CPU->SP);
          break; // incr. SP by one
        }

      case 0x34:
        {
          //printf("%04X   INR  M\n",CPU->PC);
          setflags(0xD4, Memory[CPU->reg_pair_HL], '+', 0x01, CPU);
          Memory[CPU->reg_pair_HL] = Memory[CPU->reg_pair_HL] + 0x01;
          cycles_performed = 10;
          break; // incr. Memory[CPU->reg_pair_HL] by one
        }

      case 0x35:
        {
          //printf("%04X   DCR  M\n",CPU->PC);
          setflags(0xD4, Memory[CPU->reg_pair_HL], '-', 0x01, CPU);
          Memory[CPU->reg_pair_HL] = Memory[CPU->reg_pair_HL] - 0x01;
          cycles_performed = 10;
          break; // decr. Memory[CPU->reg_pair_HL] by one
        }

      case 0x36:
        {
          //printf("%04X   MVI  M   #$%02X\n",CPU->PC, OP_CODE_1);
          Memory[CPU->reg_pair_HL] = OP_CODE_1;
          inc_PC_with = 0x02;
           cycles_performed = 10;
          break; // puts the value of the next bit into Memory[CPU->reg_pair_HL]
        }

      case 0x37:
        {
          //printf("%04X   STC\n",CPU->PC);

            CPU->carry_flag = 0x01;
          break;  //Set carry
        }

      case 0x38: /*printf("%04X   NOP\n",CPU->PC);*/ break;

      case 0x39:
        {

          //printf("SP: %04X  CPU->reg_pair_HL: %04X  H: %02X  L: %02X\n",CPU->SP,CPU->reg_pair_HL,CPU->reg_H,CPU->reg_L);
          //printf("%04X   DAD  SP\n",CPU->PC);
          if ((CPU->reg_pair_HL + CPU->SP) > 0xFFFF) {
            CPU->carry_flag = 0x01;
          }else {
            CPU->carry_flag = 0x00;
          }
          CPU->reg_pair_HL = CPU->reg_pair_HL + CPU->SP;
          CPU->reg_H = (CPU->reg_pair_HL & 0xFF00) >> 8;
          CPU->reg_L = (CPU->reg_pair_HL & 0x00FF);
          //printf("CPU->reg_pair_HL: %04X  H: %02X  CARRY: %02X\n",CPU->reg_pair_HL,CPU->reg_H,CPU->carry_flag);

          cycles_performed = 10;
          break;  //adds two 16 bit numbers and loads them in CPU->reg_pair_HL, CPU->reg_pair_HL = CPU->reg_pair_HL + SP, carry flag is set if overflow
        }

      case 0x3A:
        {
          //printf("%04X   LDA    $%02X%02X\n",CPU->PC,OP_CODE_2,OP_CODE_1);
          CPU->reg_A = Memory[(OP_CODE_2 << 8) + OP_CODE_1];
          inc_PC_with = 0x03;
          cycles_performed = 13;

          break;
        }   //loads A with data at Memory[CPU->reg_pair_HL]

      case 0x3B:
        {
          //printf("%04X   DCX  SP\n",CPU->PC);
          CPU->SP = CPU->SP - 0x01;
          cycles_performed = 5;
          //printf("D:%02x E:%02X\n",CPU->reg_D,CPU->reg_E);
           break;
        } //decr. SP by one

      case 0x3C:
        {
          //printf("%04X   INR  A\n",CPU->PC);
          setflags(0xD4,CPU->reg_A, '+', 0x01, CPU);
          CPU->reg_A = CPU->reg_A + 0x01;
          cycles_performed = 5;
          break;
        }  // incr. A by one

      case 0x3D:
        {
          //printf("%04X   DCR  A\n",CPU->PC);
          setflags(0xD4,CPU->reg_A, '-', 0x01, CPU);
          CPU->reg_A = CPU->reg_A - 0x01;
          cycles_performed = 5;
          break; // decr. A by one
        }

      case 0x3E:
        {
          //printf("%04X   MVI  A   #$%02X\n",CPU->PC, OP_CODE_1);
          CPU->reg_A = OP_CODE_1;
          inc_PC_with = 0x02;
           cycles_performed = 7;
          break; // puts the value of the next bit into A
        }

      case 0x3F:
        {
          //printf("%04X   CMC\n",CPU->PC);
          CPU->carry_flag = (old_carry ^ 0x01);
           break;
        }





        /*******************************************************************/



      // A bunch of move instructions
      case 0x40: /*printf("%04X   MOV  B,B\n",CPU->PC);*/ CPU->reg_B = CPU->reg_B ; cycles_performed = 5; break;
      case 0x41: /*printf("%04X   MOV  B,C\n",CPU->PC);*/ CPU->reg_B = CPU->reg_C ; cycles_performed = 5; break;
      case 0x42: /*printf("%04X   MOV  B,D\n",CPU->PC);*/ CPU->reg_B = CPU->reg_D ; cycles_performed = 5; break;
      case 0x43: /*printf("%04X   MOV  B,E\n",CPU->PC);*/ CPU->reg_B = CPU->reg_E ; cycles_performed = 5; break;
      case 0x44: /*printf("%04X   MOV  B,H\n",CPU->PC);*/ CPU->reg_B = CPU->reg_H ; cycles_performed = 5; break;
      case 0x45: /*printf("%04X   MOV  B,L\n",CPU->PC);*/ CPU->reg_B = CPU->reg_L ; cycles_performed = 5; break;
      case 0x46: /*printf("%04X   MOV  B,M\n",CPU->PC);*/ CPU->reg_B = Memory[CPU->reg_pair_HL]; cycles_performed = 7; break;
      case 0x47: /*printf("%04X   MOV  B,A\n",CPU->PC);*/ CPU->reg_B = CPU->reg_A ; cycles_performed = 5; break;
      case 0x48: /*printf("%04X   MOV  C,B\n",CPU->PC);*/ CPU->reg_C = CPU->reg_B ; cycles_performed = 5; break;
      case 0x49: /*printf("%04X   MOV  C,C\n",CPU->PC);*/ CPU->reg_C = CPU->reg_C ; cycles_performed = 5; break;
      case 0x4A: /*printf("%04X   MOV  C,D\n",CPU->PC);*/ CPU->reg_C = CPU->reg_D ; cycles_performed = 5; break;
      case 0x4B: /*printf("%04X   MOV  C,E\n",CPU->PC);*/ CPU->reg_C = CPU->reg_E ; cycles_performed = 5; break;
      case 0x4C: /*printf("%04X   MOV  C,H\n",CPU->PC);*/ CPU->reg_C = CPU->reg_H ; cycles_performed = 5; break;
      case 0x4D: /*printf("%04X   MOV  C,L\n",CPU->PC);*/ CPU->reg_C = CPU->reg_L ; cycles_performed = 5; break;
      case 0x4E: /*printf("%04X   MOV  C,M\n",CPU->PC);*/ CPU->reg_C = Memory[CPU->reg_pair_HL]; cycles_performed = 7; break;
      case 0x4F: /*printf("%04X   MOV  C,A\n",CPU->PC);*/ CPU->reg_C = CPU->reg_A ; cycles_performed = 5; break;

      case 0x50: /*printf("%04X   MOV  D,B\n",CPU->PC);*/ CPU->reg_D = CPU->reg_B ; cycles_performed = 5; break;
      case 0x51: /*printf("%04X   MOV  D,C\n",CPU->PC);*/ CPU->reg_D = CPU->reg_C ; cycles_performed = 5; break;
      case 0x52: /*printf("%04X   MOV  D,D\n",CPU->PC);*/ CPU->reg_D = CPU->reg_D ; cycles_performed = 5; break;
      case 0x53: /*printf("%04X   MOV  D,E\n",CPU->PC);*/ CPU->reg_D = CPU->reg_E ; cycles_performed = 5; break;
      case 0x54: /*printf("%04X   MOV  D,H\n",CPU->PC);*/ CPU->reg_D = CPU->reg_H ; cycles_performed = 5; break;
      case 0x55: /*printf("%04X   MOV  D,L\n",CPU->PC);*/ CPU->reg_D = CPU->reg_L ; cycles_performed = 5; break;
      case 0x56: /*printf("%04X   MOV  D,M\n",CPU->PC);*/ CPU->reg_D = Memory[CPU->reg_pair_HL] ; cycles_performed = 7; break;
      case 0x57: /*printf("%04X   MOV  D,A\n",CPU->PC);*/ CPU->reg_D = CPU->reg_A ; cycles_performed = 5; break;
      case 0x58: /*printf("%04X   MOV  E,B\n",CPU->PC);*/ CPU->reg_E = CPU->reg_B ; cycles_performed = 5; break;
      case 0x59: /*printf("%04X   MOV  E,C\n",CPU->PC);*/ CPU->reg_E = CPU->reg_C ; cycles_performed = 5; break;
      case 0x5A: /*printf("%04X   MOV  E,D\n",CPU->PC);*/ CPU->reg_E = CPU->reg_D ; cycles_performed = 5; break;
      case 0x5B: /*printf("%04X   MOV  E,E\n",CPU->PC);*/ CPU->reg_E = CPU->reg_E ; cycles_performed = 5; break;
      case 0x5C: /*printf("%04X   MOV  E,H\n",CPU->PC);*/ CPU->reg_E = CPU->reg_H ; cycles_performed = 5; break;
      case 0x5D: /*printf("%04X   MOV  E,L\n",CPU->PC);*/ CPU->reg_E = CPU->reg_L ; cycles_performed = 5; break;
      case 0x5E: /*printf("%04X   MOV  E,M\n",CPU->PC);*/ CPU->reg_E = Memory[CPU->reg_pair_HL] ; cycles_performed = 7; break;
      case 0x5F: /*printf("%04X   MOV  E,A\n",CPU->PC);*/ CPU->reg_E = CPU->reg_A ; cycles_performed = 5; break;

      case 0x60: /*printf("%04X   MOV  H,B\n",CPU->PC);*/ CPU->reg_H = CPU->reg_B ; cycles_performed = 5; break;
      case 0x61: /*printf("%04X   MOV  H,C\n",CPU->PC);*/ CPU->reg_H = CPU->reg_C ; cycles_performed = 5; break;
      case 0x62: /*printf("%04X   MOV  H,D\n",CPU->PC);*/ CPU->reg_H = CPU->reg_D ; cycles_performed = 5; break;
      case 0x63: /*printf("%04X   MOV  H,E\n",CPU->PC);*/ CPU->reg_H = CPU->reg_E ; cycles_performed = 5; break;
      case 0x64: /*printf("%04X   MOV  H,H\n",CPU->PC);*/ CPU->reg_H = CPU->reg_H ; cycles_performed = 5; break;
      case 0x65: /*printf("%04X   MOV  H,L\n",CPU->PC);*/ CPU->reg_H = CPU->reg_L ; cycles_performed = 5; break;
      case 0x66: /*printf("%04X   MOV  H,M\n",CPU->PC);*/ CPU->reg_H = Memory[CPU->reg_pair_HL] ; cycles_performed = 7; break;
      case 0x67: /*printf("%04X   MOV  H,A\n",CPU->PC);*/ CPU->reg_H = CPU->reg_A ; cycles_performed = 5; break;
      case 0x68: /*printf("%04X   MOV  L,B\n",CPU->PC);*/ CPU->reg_L = CPU->reg_B ; cycles_performed = 5; break;
      case 0x69: /*printf("%04X   MOV  L,C\n",CPU->PC);*/ CPU->reg_L = CPU->reg_C ; cycles_performed = 5; break;
      case 0x6A: /*printf("%04X   MOV  L,D\n",CPU->PC);*/ CPU->reg_L = CPU->reg_D ; cycles_performed = 5; break;
      case 0x6B: /*printf("%04X   MOV  L,E\n",CPU->PC);*/ CPU->reg_L = CPU->reg_E ; cycles_performed = 5; break;
      case 0x6C: /*printf("%04X   MOV  L,H\n",CPU->PC);*/ CPU->reg_L = CPU->reg_H ; cycles_performed = 5; break;
      case 0x6D: /*printf("%04X   MOV  L,L\n",CPU->PC);*/ CPU->reg_L = CPU->reg_L ; cycles_performed = 5; break;
      case 0x6E: /*printf("%04X   MOV  L,M\n",CPU->PC);*/ CPU->reg_L = Memory[CPU->reg_pair_HL] ; cycles_performed = 7; break;
      case 0x6F: /*printf("%04X   MOV  L,A\n",CPU->PC);*/ CPU->reg_L = CPU->reg_A ; cycles_performed = 5; break;

      case 0x70: /*printf("%04X   MOV  M,B\n",CPU->PC);*/ Memory[CPU->reg_pair_HL] = CPU->reg_B ; cycles_performed = 7; break;
      case 0x71: /*printf("%04X   MOV  M,C\n",CPU->PC);*/ Memory[CPU->reg_pair_HL] = CPU->reg_C ; cycles_performed = 7; break;
      case 0x72: /*printf("%04X   MOV  M,D\n",CPU->PC);*/ Memory[CPU->reg_pair_HL] = CPU->reg_D ; cycles_performed = 7; break;
      case 0x73: /*printf("%04X   MOV  M,E\n",CPU->PC);*/ Memory[CPU->reg_pair_HL] = CPU->reg_E ; cycles_performed = 7; break;
      case 0x74: /*printf("%04X   MOV  M,H\n",CPU->PC);*/ Memory[CPU->reg_pair_HL] = CPU->reg_H ; cycles_performed = 7; break;
      case 0x75: /*printf("%04X   MOV  M,L\n",CPU->PC);*/ Memory[CPU->reg_pair_HL] = CPU->reg_L ; cycles_performed = 7; break;
      case 0x76: printf("%04X   CPU->reg_pair_HLT\n",CPU->PC); CPU->running = 0x00; break; //possible need to switch order to incr. PC before the stopped state
      case 0x77: /*printf("%04X   MOV  M,A\n",CPU->PC);*/ Memory[CPU->reg_pair_HL] = CPU->reg_A ; cycles_performed = 7; break;
      case 0x78: /*printf("%04X   MOV  A,B\n",CPU->PC);*/ CPU->reg_A = CPU->reg_B ; cycles_performed = 5; break;
      case 0x79: /*printf("%04X   MOV  A,C\n",CPU->PC);*/ CPU->reg_A = CPU->reg_C ; cycles_performed = 5; break;
      case 0x7A: /*printf("%04X   MOV  A,D\n",CPU->PC);*/ CPU->reg_A = CPU->reg_D ; cycles_performed = 5; break;
      case 0x7B: /*printf("%04X   MOV  A,E\n",CPU->PC);*/ CPU->reg_A = CPU->reg_E ; cycles_performed = 5; break;
      case 0x7C: /*printf("%04X   MOV  A,H\n",CPU->PC);*/ CPU->reg_A = CPU->reg_H ; cycles_performed = 5; break;
      case 0x7D: /*printf("%04X   MOV  A,L\n",CPU->PC);*/ CPU->reg_A = CPU->reg_L ; cycles_performed = 5; break;
      case 0x7E: /*printf("%04X   MOV  A,M\n",CPU->PC);*/ CPU->reg_A = Memory[CPU->reg_pair_HL]; cycles_performed = 7; break;
      case 0x7F: /*printf("%04X   MOV  A,A\n",CPU->PC);*/ CPU->reg_A = CPU->reg_A ; cycles_performed = 5; break;




      /********************************************************************/




      // A bunch of addition instructions


      case 0x80: /*printf("%04X   ADD  B\n",CPU->PC);*/ setflags(0xD5, CPU->reg_A, '+', CPU->reg_B, CPU); CPU->reg_A = CPU->reg_A + CPU->reg_B; break;
      case 0x81: /*printf("%04X   ADD  C\n",CPU->PC);*/ setflags(0xD5, CPU->reg_A, '+', CPU->reg_C, CPU); CPU->reg_A = CPU->reg_A + CPU->reg_C; break;
      case 0x82: /*printf("%04X   ADD  D\n",CPU->PC);*/ setflags(0xD5, CPU->reg_A, '+', CPU->reg_D, CPU); CPU->reg_A = CPU->reg_A + CPU->reg_D; break;
      case 0x83: /*printf("%04X   ADD  E\n",CPU->PC);*/ setflags(0xD5, CPU->reg_A, '+', CPU->reg_E, CPU); CPU->reg_A = CPU->reg_A + CPU->reg_E; break;
      case 0x84: /*printf("%04X   ADD  H\n",CPU->PC);*/ setflags(0xD5, CPU->reg_A, '+', CPU->reg_H, CPU); CPU->reg_A = CPU->reg_A + CPU->reg_H; break;
      case 0x85: /*printf("%04X   ADD  L\n",CPU->PC);*/ setflags(0xD5, CPU->reg_A, '+', CPU->reg_L, CPU); CPU->reg_A = CPU->reg_A + CPU->reg_L; break;
      case 0x86: /*printf("%04X   ADD  M\n",CPU->PC);*/ setflags(0xD5, CPU->reg_A, '+', Memory[CPU->reg_pair_HL], CPU); CPU->reg_A = CPU->reg_A + Memory[CPU->reg_pair_HL]; cycles_performed = 7; break;
      case 0x87: /*printf("%04X   ADD  A\n",CPU->PC);*/ setflags(0xD5, CPU->reg_A, '+', CPU->reg_A, CPU); CPU->reg_A = CPU->reg_A + CPU->reg_A; break;
      case 0x88: /*printf("%04X   ADC  B\n",CPU->PC);*/ setflags(0xD5, CPU->reg_A, '+', CPU->reg_B + old_carry, CPU); CPU->reg_A = CPU->reg_A + CPU->reg_B + old_carry; break;
      case 0x89: /*printf("%04X   ADC  C\n",CPU->PC);*/ setflags(0xD5, CPU->reg_A, '+', CPU->reg_C + old_carry, CPU); CPU->reg_A = CPU->reg_A + CPU->reg_C + old_carry; break;
      case 0x8A: /*printf("%04X   ADC  D\n",CPU->PC);*/ setflags(0xD5, CPU->reg_A, '+', CPU->reg_D + old_carry, CPU); CPU->reg_A = CPU->reg_A + CPU->reg_D + old_carry; break;
      case 0x8B: /*printf("%04X   ADC  E\n",CPU->PC);*/ setflags(0xD5, CPU->reg_A, '+', CPU->reg_E + old_carry, CPU); CPU->reg_A = CPU->reg_A + CPU->reg_E + old_carry; break;
      case 0x8C: /*printf("%04X   ADC  H\n",CPU->PC);*/ setflags(0xD5, CPU->reg_A, '+', CPU->reg_H + old_carry, CPU); CPU->reg_A = CPU->reg_A + CPU->reg_H + old_carry; break;
      case 0x8D: /*printf("%04X   ADC  L\n",CPU->PC);*/ setflags(0xD5, CPU->reg_A, '+', CPU->reg_L + old_carry, CPU); CPU->reg_A = CPU->reg_A + CPU->reg_L + old_carry; break;
      case 0x8E: /*printf("%04X   ADC  M\n",CPU->PC);*/ setflags(0xD5, CPU->reg_A, '+', Memory[CPU->reg_pair_HL] + old_carry, CPU); CPU->reg_A = CPU->reg_A + Memory[CPU->reg_pair_HL] + old_carry;cycles_performed = 7; break;
      case 0x8F: /*printf("%04X   ADC  A\n",CPU->PC);*/ setflags(0xD5, CPU->reg_A, '+', CPU->reg_A + old_carry, CPU); CPU->reg_A = CPU->reg_A + CPU->reg_A + old_carry; break;



      // A bunch os subtraction instructions
      case 0x90: /*printf("%04X   SUB  B\n",CPU->PC);*/ setflags(0xD5, CPU->reg_A, '-', CPU->reg_B, CPU); CPU->reg_A = CPU->reg_A - CPU->reg_B; break;
      case 0x91: /*printf("%04X   SUB  C\n",CPU->PC);*/ setflags(0xD5, CPU->reg_A, '-', CPU->reg_C, CPU); CPU->reg_A = CPU->reg_A - CPU->reg_C; break;
      case 0x92: /*printf("%04X   SUB  D\n",CPU->PC);*/ setflags(0xD5, CPU->reg_A, '-', CPU->reg_D, CPU); CPU->reg_A = CPU->reg_A - CPU->reg_D; break;
      case 0x93: /*printf("%04X   SUB  E\n",CPU->PC);*/ setflags(0xD5, CPU->reg_A, '-', CPU->reg_E, CPU); CPU->reg_A = CPU->reg_A - CPU->reg_E; break;
      case 0x94: /*printf("%04X   SUB  H\n",CPU->PC);*/ setflags(0xD5, CPU->reg_A, '-', CPU->reg_H, CPU); CPU->reg_A = CPU->reg_A - CPU->reg_H; break;
      case 0x95: /*printf("%04X   SUB  L\n",CPU->PC);*/ setflags(0xD5, CPU->reg_A, '-', CPU->reg_L, CPU); CPU->reg_A = CPU->reg_A - CPU->reg_L; break;
      case 0x96: /*printf("%04X   SUB  M\n",CPU->PC);*/ setflags(0xD5, CPU->reg_A, '-', Memory[CPU->reg_pair_HL], CPU); CPU->reg_A = CPU->reg_A - Memory[CPU->reg_pair_HL]; cycles_performed = 7;break;
      case 0x97: /*printf("%04X   SUB  A\n",CPU->PC);*/ setflags(0xD5, CPU->reg_A, '-', CPU->reg_A, CPU); CPU->reg_A = CPU->reg_A - CPU->reg_A; break;
      case 0x98: /*printf("%04X   SBB  B\n",CPU->PC);*/ setflags(0xD5, CPU->reg_A, '-', CPU->reg_B + old_carry, CPU); CPU->reg_A = CPU->reg_A - CPU->reg_B - old_carry; break;
      case 0x99: /*printf("%04X   SBB  C\n",CPU->PC);*/ setflags(0xD5, CPU->reg_A, '-', CPU->reg_C + old_carry, CPU); CPU->reg_A = CPU->reg_A - CPU->reg_C - old_carry; break;
      case 0x9A: /*printf("%04X   SBB  D\n",CPU->PC);*/ setflags(0xD5, CPU->reg_A, '-', CPU->reg_D + old_carry, CPU); CPU->reg_A = CPU->reg_A - CPU->reg_D - old_carry; break;
      case 0x9B: /*printf("%04X   SBB  E\n",CPU->PC);*/ setflags(0xD5, CPU->reg_A, '-', CPU->reg_E + old_carry, CPU); CPU->reg_A = CPU->reg_A - CPU->reg_E - old_carry; break;
      case 0x9C: /*printf("%04X   SBB  H\n",CPU->PC);*/ setflags(0xD5, CPU->reg_A, '-', CPU->reg_H + old_carry, CPU); CPU->reg_A = CPU->reg_A - CPU->reg_H - old_carry; break;
      case 0x9D: /*printf("%04X   SBB  L\n",CPU->PC);*/ setflags(0xD5, CPU->reg_A, '-', CPU->reg_L + old_carry, CPU); CPU->reg_A = CPU->reg_A - CPU->reg_L - old_carry; break;
      case 0x9E: /*printf("%04X   SBB  M\n",CPU->PC);*/ setflags(0xD5, CPU->reg_A, '-', Memory[CPU->reg_pair_HL] + old_carry, CPU); CPU->reg_A = CPU->reg_A - Memory[CPU->reg_pair_HL] - old_carry; cycles_performed = 7;break;
      case 0x9F: /*printf("%04X   SBB  A\n",CPU->PC);*/ setflags(0xD5, CPU->reg_A, '-', CPU->reg_A + old_carry, CPU); CPU->reg_A = CPU->reg_A - CPU->reg_A - old_carry; break;





      // A bunch of logical instructions



      case 0xA0: /*printf("%04X   ANA  B\n",CPU->PC);*/ setflags(0xD4, CPU->reg_A, '&', CPU->reg_B, CPU); CPU->reg_A = CPU->reg_A & CPU->reg_B; CPU->carry_flag = 0x00; break;
      case 0xA1: /*printf("%04X   ANA  C\n",CPU->PC);*/ setflags(0xD4, CPU->reg_A, '&', CPU->reg_C, CPU); CPU->reg_A = CPU->reg_A & CPU->reg_C; CPU->carry_flag = 0x00; break;
      case 0xA2: /*printf("%04X   ANA  D\n",CPU->PC);*/ setflags(0xD4, CPU->reg_A, '&', CPU->reg_D, CPU); CPU->reg_A = CPU->reg_A & CPU->reg_D; CPU->carry_flag = 0x00; break;
      case 0xA3: /*printf("%04X   ANA  E\n",CPU->PC);*/ setflags(0xD4, CPU->reg_A, '&', CPU->reg_E, CPU); CPU->reg_A = CPU->reg_A & CPU->reg_E; CPU->carry_flag = 0x00; break;
      case 0xA4: /*printf("%04X   ANA  H\n",CPU->PC);*/ setflags(0xD4, CPU->reg_A, '&', CPU->reg_H, CPU); CPU->reg_A = CPU->reg_A & CPU->reg_H; CPU->carry_flag = 0x00; break;
      case 0xA5: /*printf("%04X   ANA  L\n",CPU->PC);*/ setflags(0xD4, CPU->reg_A, '&', CPU->reg_L, CPU); CPU->reg_A = CPU->reg_A & CPU->reg_L; CPU->carry_flag = 0x00; break;
      case 0xA6: /*printf("%04X   ANA  M\n",CPU->PC);*/ setflags(0xD4, CPU->reg_A, '&', Memory[CPU->reg_pair_HL], CPU); CPU->reg_A = CPU->reg_A & Memory[CPU->reg_pair_HL]; CPU->carry_flag = 0x00; cycles_performed = 7;break;
      case 0xA7: /*printf("%04X   ANA  A\n",CPU->PC);*/ setflags(0xD4, CPU->reg_A, '&', CPU->reg_A, CPU); CPU->reg_A = CPU->reg_A & CPU->reg_A; CPU->carry_flag = 0x00; break;
      case 0xA8: /*printf("%04X   XRA  B\n",CPU->PC);*/ setflags(0xD4, CPU->reg_A, '^', CPU->reg_B, CPU); CPU->reg_A = CPU->reg_A ^ CPU->reg_B; CPU->carry_flag = 0x00; break;
      case 0xA9: /*printf("%04X   XRA  C\n",CPU->PC);*/ setflags(0xD4, CPU->reg_A, '^', CPU->reg_C, CPU); CPU->reg_A = CPU->reg_A ^ CPU->reg_C; CPU->carry_flag = 0x00; break;
      case 0xAA: /*printf("%04X   XRA  D\n",CPU->PC);*/ setflags(0xD4, CPU->reg_A, '^', CPU->reg_D, CPU); CPU->reg_A = CPU->reg_A ^ CPU->reg_D; CPU->carry_flag = 0x00; break;
      case 0xAB: /*printf("%04X   XRA  E\n",CPU->PC);*/ setflags(0xD4, CPU->reg_A, '^', CPU->reg_E, CPU); CPU->reg_A = CPU->reg_A ^ CPU->reg_E; CPU->carry_flag = 0x00; break;
      case 0xAC: /*printf("%04X   XRA  H\n",CPU->PC);*/ setflags(0xD4, CPU->reg_A, '^', CPU->reg_H, CPU); CPU->reg_A = CPU->reg_A ^ CPU->reg_H; CPU->carry_flag = 0x00; break;
      case 0xAD: /*printf("%04X   XRA  L\n",CPU->PC);*/ setflags(0xD4, CPU->reg_A, '^', CPU->reg_L, CPU); CPU->reg_A = CPU->reg_A ^ CPU->reg_L; CPU->carry_flag = 0x00; break;
      case 0xAE: /*printf("%04X   XRA  M\n",CPU->PC);*/ setflags(0xD4, CPU->reg_A, '^', Memory[CPU->reg_pair_HL], CPU); CPU->reg_A = CPU->reg_A ^ Memory[CPU->reg_pair_HL]; CPU->carry_flag = 0x00; cycles_performed = 7;break;
      case 0xAF: /*printf("%04X   XRA  A\n",CPU->PC);*/ setflags(0xD4, CPU->reg_A, '^', CPU->reg_A, CPU); CPU->reg_A = CPU->reg_A ^ CPU->reg_A; CPU->carry_flag = 0x00; break;


      case 0xB0: /*printf("%04X   ORA  B\n",CPU->PC);*/ setflags(0xD4, CPU->reg_A, '|', CPU->reg_B, CPU); CPU->reg_A = CPU->reg_A | CPU->reg_B; CPU->carry_flag = 0x00; break;
      case 0xB1: /*printf("%04X   ORA  C\n",CPU->PC);*/ setflags(0xD4, CPU->reg_A, '|', CPU->reg_C, CPU); CPU->reg_A = CPU->reg_A | CPU->reg_C; CPU->carry_flag = 0x00; break;
      case 0xB2: /*printf("%04X   ORA  D\n",CPU->PC);*/ setflags(0xD4, CPU->reg_A, '|', CPU->reg_D, CPU); CPU->reg_A = CPU->reg_A | CPU->reg_D; CPU->carry_flag = 0x00; break;
      case 0xB3: /*printf("%04X   ORA  E\n",CPU->PC);*/ setflags(0xD4, CPU->reg_A, '|', CPU->reg_E, CPU); CPU->reg_A = CPU->reg_A | CPU->reg_E; CPU->carry_flag = 0x00; break;
      case 0xB4: /*printf("%04X   ORA  H\n",CPU->PC);*/ setflags(0xD4, CPU->reg_A, '|', CPU->reg_H, CPU); CPU->reg_A = CPU->reg_A | CPU->reg_H; CPU->carry_flag = 0x00; break;
      case 0xB5: /*printf("%04X   ORA  L\n",CPU->PC);*/ setflags(0xD4, CPU->reg_A, '|', CPU->reg_L, CPU); CPU->reg_A = CPU->reg_A | CPU->reg_L; CPU->carry_flag = 0x00; break;
      case 0xB6: /*printf("%04X   ORA  M\n",CPU->PC);*/ setflags(0xD4, CPU->reg_A, '|', Memory[CPU->reg_pair_HL], CPU); CPU->reg_A = CPU->reg_A | Memory[CPU->reg_pair_HL]; CPU->carry_flag = 0x00; cycles_performed = 7;break;
      case 0xB7: /*printf("%04X   ORA  A\n",CPU->PC);*/ setflags(0xD4, CPU->reg_A, '|', CPU->reg_A, CPU); break;
      case 0xB8: /*printf("%04X   CMP  B\n",CPU->PC);*/ setflags(0xD4, CPU->reg_A, '-', CPU->reg_B, CPU); break;
      case 0xB9: /*printf("%04X   CMP  C\n",CPU->PC);*/ setflags(0xD4, CPU->reg_A, '-', CPU->reg_C, CPU); break;
      case 0xBA: /*printf("%04X   CMP  D\n",CPU->PC);*/ setflags(0xD4, CPU->reg_A, '-', CPU->reg_D, CPU); break;
      case 0xBB: /*printf("%04X   CMP  E\n",CPU->PC);*/ setflags(0xD4, CPU->reg_A, '-', CPU->reg_E, CPU); break;
      case 0xBC: /*printf("%04X   CMP  H\n",CPU->PC);*/ setflags(0xD4, CPU->reg_A, '-', CPU->reg_H, CPU); break;
      case 0xBD: /*printf("%04X   CMP  L\n",CPU->PC);*/ setflags(0xD4, CPU->reg_A, '-', CPU->reg_L, CPU); break;
      case 0xBE: /*printf("%04X   CMP  M\n",CPU->PC);*/ setflags(0xD4, CPU->reg_A, '-', Memory[CPU->reg_pair_HL], CPU);cycles_performed = 7; break;
      case 0xBF: /*printf("%04X   CMP  A\n",CPU->PC);*/ setflags(0xD4, CPU->reg_A, '-', CPU->reg_A, CPU); break;


      /************************************************************************/



      case 0xC0: //return on no zero
      {
        //printf("%04X   RNZ\n",CPU->PC);
        if (!CPU->zero_flag) {
          inc_PC_with = 0x03;
          CPU->PC = (Memory[CPU->SP + 0x01] << 8) + Memory[CPU->SP];
          CPU->SP = CPU->SP + 0x02;
          cycles_performed = 11;
        } else {
          cycles_performed = 5;
          //printf("No return, z flag is 1\n");
        }
        break;
      }

      case 0xC1: // pop 2 bytes of the stack and put them into B and C registers
        {
          //printf("%04X   POP  B\n",CPU->PC);
          CPU->reg_B = Memory[CPU->SP + 0x01];
          CPU->reg_C = Memory[CPU->SP];
          CPU->SP = CPU->SP + 0x02;
          cycles_performed = 10;
          break;
        }

      case 0xC2: // Jump on no zero
        {
          //printf("%04X   JNZ       $%02X%02X\n", CPU->PC, OP_CODE_2, OP_CODE_1);
          if (!CPU->zero_flag) {
            inc_PC_with = 0x00;
            CPU->PC = (OP_CODE_2 << 8) + OP_CODE_1;
          } else {
            //printf("No jump, z flag is 1\n");
            inc_PC_with = 0x03;
          }
          cycles_performed = 10;
          break;
        }

      case 0xC3: //Jump unconditionally
        {
          //printf("%04X   JMP       $%02X%02X\n", CPU->PC, OP_CODE_2, OP_CODE_1);
          inc_PC_with = 0x00;
          CPU->PC = (OP_CODE_2 << 8) + OP_CODE_1;
          cycles_performed = 10;
          break;

        }
      case 0xC4: // Call subroutine on no zero
        {
          //printf("%04X   CNZ       $%02X%02X\n", CPU->PC, OP_CODE_2, OP_CODE_1);
          if (!CPU->zero_flag) {
            inc_PC_with = 0x00;
            Memory[CPU->SP - 0x01] = (CPU->PC & 0xFF00) >> 8;
            Memory[CPU->SP - 0x02] = CPU->PC & 0x00FF;
            CPU->SP = CPU->SP - 0x02;
            CPU->PC = (OP_CODE_2 << 8) + OP_CODE_1;
            cycles_performed = 17;
          } else {
            //printf("No Call, z flag is 1\n");
            inc_PC_with = 0x03;
            cycles_performed = 11;
          }
          break;
        }

      case 0xC5: // push contents of B and C registers onto the stack
        {
          //printf("%04X   PUSH B\n",CPU->PC);
          Memory[CPU->SP - 0x01] = CPU->reg_B;
          Memory[CPU->SP - 0x02] = CPU->reg_C;
          CPU->SP = CPU->SP - 0x02;
          cycles_performed = 11;
          break;
        }

      case 0xC6: //add data to A register
        {
          //printf("%04X   ADI      #$%02X\n",CPU->PC, OP_CODE_1);
          inc_PC_with = 0x02;
          setflags(0xD5, CPU->reg_A, '+', OP_CODE_1, CPU);
          CPU->reg_A = CPU->reg_A + OP_CODE_1;
          cycles_performed = 7;
          break;
        }

      case 0xC7: //reset to adress 0x0000
        {
          //printf("%04X   RST  0\n",CPU->PC);
          inc_PC_with = 0x00;
          Memory[CPU->SP - 0x01] = (CPU->PC & 0xFF00) >> 8;
          Memory[CPU->SP - 0x02] = CPU->PC & 0x00FF;
          CPU->SP = CPU->SP - 0x02;
          CPU->PC = 0x0000;
          cycles_performed = 11;
          break;
        }

      case 0xC8: //return on zero
        {
          //printf("%04X   RZ\n",CPU->PC);
          if (CPU->zero_flag) {
            inc_PC_with = 0x03;
            CPU->PC = (Memory[CPU->SP + 0x01] << 8) + Memory[CPU->SP];
            CPU->SP = CPU->SP + 0x02;
            cycles_performed = 11;
          } else {
            cycles_performed = 5;
            //printf("No return, z flag is 0\n");
          }
          break;
        }

      case 0xC9: // return unconditionally
        {
          //printf("%04X   RET\n",CPU->PC);
            inc_PC_with = 0x03;
            CPU->PC = (Memory[CPU->SP + 0x01] << 8) + Memory[CPU->SP];
            CPU->SP = CPU->SP + 0x02;
            cycles_performed = 10;
          break;
        }

      case 0xCA: // jump on zero
        {
          //printf("%04X   JZ        $%02X%02X\n", CPU->PC, OP_CODE_2, OP_CODE_1);
          if (CPU->zero_flag) {
            inc_PC_with = 0x00;
            CPU->PC = (OP_CODE_2 << 8) + OP_CODE_1;
          } else {
            //printf("No jump, z flag is 0\n");
            inc_PC_with = 0x03;
          }
          cycles_performed = 10;
          break;
        }

      case 0xCB: // Jump unconditionally
      {
        //printf("%04X   JMP*      $%02X%02X\n", CPU->PC, OP_CODE_2, OP_CODE_1);
        inc_PC_with = 0x00;
        CPU->PC = (OP_CODE_2 << 8) + OP_CODE_1;
        cycles_performed = 10;
        break;
      }

      case 0xCC:
      {
          //printf("%04X   CZ       $%02X%02X\n", CPU->PC, OP_CODE_2, OP_CODE_1);
          if (CPU->zero_flag) {
            inc_PC_with = 0x00;
            Memory[CPU->SP - 0x01] = (CPU->PC & 0xFF00) >> 8;
            Memory[CPU->SP - 0x02] = CPU->PC & 0x00FF;
            CPU->SP = CPU->SP - 0x02;
            CPU->PC = (OP_CODE_2 << 8) + OP_CODE_1;
            cycles_performed = 17;
          } else {
            //printf("No Call, z flag is 0\n");
            inc_PC_with = 0x03;
            cycles_performed = 11;
          }
          break;
        }

      case 0xCD: // Call unconditionally
        {


            //printf("%04X   CALL       $%02X%02X\n", CPU->PC, OP_CODE_2, OP_CODE_1);
            inc_PC_with = 0x00;
            Memory[CPU->SP - 0x01] = (CPU->PC & 0xFF00) >> 8;
            Memory[CPU->SP - 0x02] = CPU->PC & 0x00FF;
            CPU->SP = CPU->SP - 0x02;
            CPU->PC = (OP_CODE_2 << 8) + OP_CODE_1;
            cycles_performed = 17;
            break;
        }

      case 0xCE:
        {
          //printf("%04X   ACI      #$%02X\n",CPU->PC, OP_CODE_1);
          inc_PC_with = 0x02;
          setflags(0xD5, CPU->reg_A, '+', OP_CODE_1 + old_carry, CPU);
          CPU->reg_A = CPU->reg_A + OP_CODE_1 + old_carry;
          cycles_performed = 7;
          break;
        }

      case 0xCF: // resets PC to address 0x0008
        {
          //printf("%04X   RST  1\n",CPU->PC);
          inc_PC_with = 0x00;
          Memory[CPU->SP - 0x01] = (CPU->PC & 0xFF00) >> 8;
          Memory[CPU->SP - 0x02] = CPU->PC & 0x00FF;
          CPU->SP = CPU->SP - 0x02;
          CPU->PC = 0x0008;
          cycles_performed = 11;
          break;
        }


        /*******************************************************************/



      case 0xD0: //return on no carry
      {
        //printf("%04X   RNC\n",CPU->PC);
        if (!old_carry) {
          inc_PC_with = 0x03;
          CPU->PC = (Memory[CPU->SP + 0x01] << 8) + Memory[CPU->SP];
          CPU->SP = CPU->SP + 0x02;
          cycles_performed = 11;
        } else {
          cycles_performed = 5;
          //printf("No return, c flag is 1\n");
        }
        break;
      }

      case 0xD1: // pop 2 bytes of the stack and put them into D and E registers
        {
          //printf("%04X   POP  D\n",CPU->PC);
          CPU->reg_D = Memory[CPU->SP + 0x01];
          CPU->reg_E = Memory[CPU->SP];
          CPU->SP = CPU->SP + 0x02;
          cycles_performed = 10;
          break;
        }

      case 0xD2: // Jump on no carry
        {
          //printf("%04X   JNC       $%02X%02X\n", CPU->PC, OP_CODE_2, OP_CODE_1);
          if (!old_carry) {
            inc_PC_with = 0x00;
            CPU->PC = (OP_CODE_2 << 8) + OP_CODE_1;
          } else {
            //printf("No jump, c flag is 1\n");
            inc_PC_with = 0x03;
          }
          cycles_performed = 10;
          break;
        }

      case 0xD3: //Set Output device nr DATA to the same value as register A
        {
          //printf("%04X   OUT       $%02X\n", CPU->PC, OP_CODE_1);
          if (OP_CODE_1 == 0x04) {
            CPU->OUT[OP_CODE_1] = (CPU->OUT[OP_CODE_1] >> 8);
            CPU->OUT[OP_CODE_1] = (CPU->reg_A << 8);
          } else if (OP_CODE_1 == 0x02){
            CPU->OUT[OP_CODE_1] = CPU->reg_A;
            CPU->IN[0x03] = ((CPU->OUT[0x04] >> (0x08 - CPU->OUT[0x02])) & 0x00FF);
            }else{
              CPU->OUT[OP_CODE_1] = CPU->reg_A;
          }
          inc_PC_with = 0x02;
          cycles_performed = 10;
          break;

        }
      case 0xD4: // Call subroutine on no carry
        {
          //printf("%04X   CNC       $%02X%02X\n", CPU->PC, OP_CODE_2, OP_CODE_1);
          if (!old_carry) {
            inc_PC_with = 0x00;
            Memory[CPU->SP - 0x01] = (CPU->PC & 0xFF00) >> 8;
            Memory[CPU->SP - 0x02] = CPU->PC & 0x00FF;
            CPU->SP = CPU->SP - 0x02;
            CPU->PC = (OP_CODE_2 << 8) + OP_CODE_1;
            cycles_performed = 17;
          } else {
            //printf("No Call, c flag is 1\n");
            inc_PC_with = 0x03;
            cycles_performed = 11;
          }
          break;
        }

      case 0xD5: // push contents of D and E registers onto the stack
        {
          //printf("%04X   PUSH D\n",CPU->PC);
          Memory[CPU->SP - 0x01] = CPU->reg_D;
          Memory[CPU->SP - 0x02] = CPU->reg_E;
          CPU->SP = CPU->SP - 0x02;
          cycles_performed = 11;
          break;
        }

      case 0xD6: //subtract data from A register
        {
          //printf("%04X   SUI      #$%02X\n",CPU->PC, OP_CODE_1);
          inc_PC_with = 0x02;
          setflags(0xD5, CPU->reg_A, '-', OP_CODE_1, CPU);
          CPU->reg_A = CPU->reg_A - OP_CODE_1;
          cycles_performed = 7;
          break;
        }

      case 0xD7: //reset to adress 0x0010
        {
          //printf("%04X   RST  2\n",CPU->PC);
          inc_PC_with = 0x00;
          Memory[CPU->SP - 0x01] = (CPU->PC & 0xFF00) >> 8;
          Memory[CPU->SP - 0x02] = CPU->PC & 0x00FF;
          CPU->SP = CPU->SP - 0x02;
          CPU->PC = 0x0010;
          cycles_performed = 11;
          break;
        }

      case 0xD8: //return on carry
        {
          //printf("%04X   RC\n",CPU->PC);
          if (old_carry) {
            inc_PC_with = 0x03;
            CPU->PC = (Memory[CPU->SP + 0x01] << 8) + Memory[CPU->SP];
            CPU->SP = CPU->SP + 0x02;
            cycles_performed = 11;
          } else {
            cycles_performed = 5;
            //printf("No return, c flag is 0\n");
          }
          break;
        }

      case 0xD9: // return unconditionally
        {
          //printf("%04X   RET*\n",CPU->PC);
            inc_PC_with = 0x03;
            CPU->PC = (Memory[CPU->SP + 0x01] << 8) + Memory[CPU->SP];
            CPU->SP = CPU->SP + 0x02;
            cycles_performed = 10;
          break;
        }

      case 0xDA: // jump on carry
        {
          //printf("%04X   JC        $%02X%02X\n", CPU->PC, OP_CODE_2, OP_CODE_1);
          if (old_carry) {
            inc_PC_with = 0x00;
            CPU->PC = (OP_CODE_2 << 8) + OP_CODE_1;
          } else {
            //printf("No jump, c flag is 0\n");
            inc_PC_with = 0x03;
          }
          cycles_performed = 10;
          break;
        }

      case 0xDB: // Read contents of IN[DATA] into A register
      {
      //printf("%04X   IN        $%02X\n", CPU->PC, OP_CODE_1);


      CPU->reg_A = CPU->IN[OP_CODE_1];
      inc_PC_with = 0x02;
      cycles_performed = 10;
      break;
      }

      case 0xDC:
      {
          //printf("%04X   CC       $%02X%02X\n", CPU->PC, OP_CODE_2, OP_CODE_1);
          if (old_carry) {
            inc_PC_with = 0x00;
            Memory[CPU->SP - 0x01] = (CPU->PC & 0xFF00) >> 8;
            Memory[CPU->SP - 0x02] = CPU->PC & 0x00FF;
            CPU->SP = CPU->SP - 0x02;
            CPU->PC = (OP_CODE_2 << 8) + OP_CODE_1;
            cycles_performed = 17;
          } else {
            //printf("No Call, c flag is 0\n");
            inc_PC_with = 0x03;
            cycles_performed = 11;
          }
          break;
        }

      case 0xDD: // Call unconditionally
        {
            //printf("%04X   CALL*      $%02X%02X\n", CPU->PC, OP_CODE_2, OP_CODE_1);
            inc_PC_with = 0x00;
            Memory[CPU->SP - 0x01] = (CPU->PC & 0xFF00) >> 8;
            Memory[CPU->SP - 0x02] = CPU->PC & 0x00FF;
            CPU->SP = CPU->SP - 0x02;
            CPU->PC = (OP_CODE_2 << 8) + OP_CODE_1;
            break;
        }

      case 0xDE: // subtract with borrow
        {
          //printf("%04X   SBI      #$%02X\n",CPU->PC, OP_CODE_1);
          inc_PC_with = 0x02;
          setflags(0xD5, CPU->reg_A, '-', OP_CODE_1 + old_carry, CPU);
          CPU->reg_A = CPU->reg_A - (OP_CODE_1 + old_carry);
          cycles_performed = 7;
          break;
        }

      case 0xDF: // resets PC to address 0x0018
        {
          //printf("%04X   RST  3\n",CPU->PC);
          inc_PC_with = 0x00;
          Memory[CPU->SP - 0x01] = (CPU->PC & 0xFF00) >> 8;
          Memory[CPU->SP - 0x02] = CPU->PC & 0x00FF;
          CPU->SP = CPU->SP - 0x02;
          CPU->PC = 0x0018;
          cycles_performed = 11;
          break;
        }



        /***********************************************************************/


      case 0xE0: //return on parity odd
      {
        //printf("%04X   RPO\n",CPU->PC);
        if (!CPU->parity_flag) {
          inc_PC_with = 0x03;
          CPU->PC = (Memory[CPU->SP + 0x01] << 8) + Memory[CPU->SP];
          CPU->SP = CPU->SP + 0x02;
          cycles_performed = 11;
        } else {
          cycles_performed = 5;
          //printf("No return, p flag is 1\n");
        }
        break;
      }

      case 0xE1: // pop 2 bytes of the stack and put them into H and L registers
        {
          //printf("%04X   POP  H\n",CPU->PC);
          CPU->reg_H = Memory[CPU->SP + 0x01];
          CPU->reg_L = Memory[CPU->SP];
          CPU->SP = CPU->SP + 0x02;
          cycles_performed = 10;
          break;
        }

      case 0xE2: // Jump on parity odd
        {
          //printf("%04X   JPO       $%02X%02X\n", CPU->PC, OP_CODE_2, OP_CODE_1);
          if (!CPU->parity_flag) {
            inc_PC_with = 0x00;
            CPU->PC = (OP_CODE_2 << 8) + OP_CODE_1;
          } else {
            //printf("No jump, p flag is 1\n");
            inc_PC_with = 0x03;
          }
          cycles_performed = 10;
          break;
        }

      case 0xE3: //Exchange contents of SP and SP + 1 with H and L registers
        {
          //printf("%04X   XTHL \n", CPU->PC, OP_CODE_1);
          swap = CPU->reg_L;
          CPU->reg_L = Memory[CPU->SP];
          Memory[CPU->SP] = swap;
          swap = CPU->reg_H;
          CPU->reg_H = Memory[CPU->SP + 1];
          Memory[CPU->SP + 1] = swap;
          cycles_performed = 18;
          break;
        }

      case 0xE4: // Call subroutine on parity odd
        {
          //printf("%04X   CPO       $%02X%02X\n", CPU->PC, OP_CODE_2, OP_CODE_1);
          if (!CPU->parity_flag) {
            inc_PC_with = 0x00;
            Memory[CPU->SP - 0x01] = (CPU->PC & 0xFF00) >> 8;
            Memory[CPU->SP - 0x02] = CPU->PC & 0x00FF;
            CPU->SP = CPU->SP - 0x02;
            CPU->PC = (OP_CODE_2 << 8) + OP_CODE_1;
            cycles_performed = 17;
          } else {
            //printf("No Call, c flag is 1\n");
            inc_PC_with = 0x03;
            cycles_performed = 11;
          }
          break;
        }

      case 0xE5: // push contents of H and L registers onto the stack
        {
          //printf("%04X   PUSH H\n",CPU->PC);
          Memory[CPU->SP - 0x01] = CPU->reg_H;
          Memory[CPU->SP - 0x02] = CPU->reg_L;
          CPU->SP = CPU->SP - 0x02;
          cycles_performed = 11;
          break;
        }

      case 0xE6: //logicaly & data and A register
        {
          //printf("%04X   ANI      #$%02X\n",CPU->PC, OP_CODE_1);
          inc_PC_with = 0x02;
          setflags(0xD4, CPU->reg_A, '&', OP_CODE_1, CPU);
          CPU->carry_flag = 0x00;
          CPU->reg_A = (CPU->reg_A & OP_CODE_1);
          cycles_performed = 7;
          break;
        }

      case 0xE7: //reset to adress 0x0020
        {
          //printf("%04X   RST  4\n",CPU->PC);
          inc_PC_with = 0x00;
          Memory[CPU->SP - 0x01] = (CPU->PC & 0xFF00) >> 8;
          Memory[CPU->SP - 0x02] = CPU->PC & 0x00FF;
          CPU->SP = CPU->SP - 0x02;
          CPU->PC = 0x0020;
          cycles_performed = 11;
          break;
        }

      case 0xE8: //return on parity even
        {
          //printf("%04X   RPE\n",CPU->PC);
          if (CPU->parity_flag) {
            inc_PC_with = 0x03;
            CPU->PC = (Memory[CPU->SP + 0x01] << 8) + Memory[CPU->SP];
            CPU->SP = CPU->SP + 0x02;
            cycles_performed = 11;
          } else {
            cycles_performed = 5;
            //printf("No return, p flag is 0\n");
          }
          break;
        }

      case 0xE9: // put value of CPU->reg_pair_HL into PC
        {
          //printf("%04X   PCHL\n",CPU->PC);
          CPU->PC = CPU->reg_pair_HL;
          inc_PC_with = 0x00;
          cycles_performed = 5;
          break;
        }

      case 0xEA: // jump on parity even
        {
          //printf("%04X   JPE       $%02X%02X\n", CPU->PC, OP_CODE_2, OP_CODE_1);
          if (CPU->parity_flag) {
            inc_PC_with = 0x00;
            CPU->PC = (OP_CODE_2 << 8) + OP_CODE_1;
          } else {
            //printf("No jump, p flag is 0\n");
            inc_PC_with = 0x03;
          }
          cycles_performed = 10;
          break;
        }

      case 0xEB: // Swap CPU->reg_pair_HL andCPU->reg_pair_DE
      {
      //printf("%04X   XCHG\n", CPU->PC);
      swap = CPU->reg_H;
      CPU->reg_H = CPU->reg_D;
      CPU->reg_D = swap;
      swap = CPU->reg_L;
      CPU->reg_L = CPU->reg_E;
      CPU->reg_E = swap;
      cycles_performed = 4;
      break;
      }

      case 0xEC: // call on parity even
      {
          //printf("%04X   CPE      $%02X%02X\n", CPU->PC, OP_CODE_2, OP_CODE_1);
          if (CPU->parity_flag) {
            inc_PC_with = 0x00;
            Memory[CPU->SP - 0x01] = (CPU->PC & 0xFF00) >> 8;
            Memory[CPU->SP - 0x02] = CPU->PC & 0x00FF;
            CPU->SP = CPU->SP - 0x02;
            CPU->PC = (OP_CODE_2 << 8) + OP_CODE_1;
            cycles_performed = 17;
          } else {
            //printf("No Call, p flag is 0\n");
            inc_PC_with = 0x03;
            cycles_performed = 11;
          }
          break;
        }

      case 0xED: // Call unconditionally
        {
            //printf("%04X   CALL*      $%02X%02X\n", CPU->PC, OP_CODE_2, OP_CODE_1);
            inc_PC_with = 0x00;
            Memory[CPU->SP - 0x01] = (CPU->PC & 0xFF00) >> 8;
            Memory[CPU->SP - 0x02] = CPU->PC & 0x00FF;
            CPU->SP = CPU->SP - 0x02;
            CPU->PC = (OP_CODE_2 << 8) + OP_CODE_1;
            break;
        }

      case 0xEE: // xor data and A register
        {
          //printf("%04X   XRI      #$%02X\n",CPU->PC, OP_CODE_1);
          inc_PC_with = 0x02;
          setflags(0xD5, CPU->reg_A, '^', OP_CODE_1, CPU);
          CPU->reg_A = CPU->reg_A ^ OP_CODE_1;
          cycles_performed = 7;
          break;
        }

      case 0xEF: // resets PC to address 0x0028
        {
          //printf("%04X   RST  5\n",CPU->PC);
          inc_PC_with = 0x00;
          Memory[CPU->SP - 0x01] = (CPU->PC & 0xFF00) >> 8;
          Memory[CPU->SP - 0x02] = CPU->PC & 0x00FF;
          CPU->SP = CPU->SP - 0x02;
          CPU->PC = 0x0028;
          cycles_performed = 11;
          break;
        }


      /*************************************************************************/


      case 0xF0: //return on plus
      {
        //printf("%04X   RP\n",CPU->PC);
        if (!CPU->sign_flag) {
          inc_PC_with = 0x03;
          CPU->PC = (Memory[CPU->SP + 0x01] << 8) + Memory[CPU->SP];
          CPU->SP = CPU->SP + 0x02;
          cycles_performed = 11;
        } else {
          cycles_performed = 5;
          //printf("No return, s flag is 1\n");
        }
        break;
      }

      case 0xF1: // pop 2 bytes of the stack and set flags and register A accordingly
        {
          //printf("%04X   POP  PSW\n",CPU->PC);

          CPU->sign_flag = (Memory[CPU->SP + 0x01] & 0x80) >> 7;
          CPU->zero_flag = (Memory[CPU->SP + 0x01] & 0x40) >> 6;
          CPU->aux_carry_flag = (Memory[CPU->SP + 0x01] & 0x10) >> 4;
          CPU->parity_flag = (Memory[CPU->SP + 0x01] & 0x04) >> 2;
          CPU->carry_flag = (Memory[CPU->SP + 0x01] & 0x01);
          CPU->reg_A = Memory[CPU->SP];
          CPU->SP = CPU->SP + 0x02;
          cycles_performed = 10;
          break;
        }

      case 0xF2: // Jump on plus
        {
          //printf("%04X   JP       $%02X%02X\n", CPU->PC, OP_CODE_2, OP_CODE_1);
          if (!CPU->sign_flag) {
            inc_PC_with = 0x00;
            CPU->PC = (OP_CODE_2 << 8) + OP_CODE_1;
          } else {
            //printf("No jump, s flag is 1\n");
            inc_PC_with = 0x03;
          }
          break;
        }

      case 0xF3: //Disable interupts
        {
          //printf("%04X   DI\n", CPU->PC);
          CPU->interupts_enabled = false;
          break;
        }

      case 0xF4: // Call subroutine on plus
        {
          //printf("%04X   CP       $%02X%02X\n", CPU->PC, OP_CODE_2, OP_CODE_1);
          if (!CPU->sign_flag) {
            inc_PC_with = 0x00;
            Memory[CPU->SP - 0x01] = (CPU->PC & 0xFF00) >> 8;
            Memory[CPU->SP - 0x02] = CPU->PC & 0x00FF;
            CPU->SP = CPU->SP - 0x02;
            CPU->PC = (OP_CODE_2 << 8) + OP_CODE_1;
            cycles_performed = 17;
          } else {
            //printf("No Call, s flag is 1\n");
            inc_PC_with = 0x03;
            cycles_performed = 11;
          }
          break;
        }

      case 0xF5: // push current vallue of A and flags to stack
        {
          //printf("%04X   PUSH PSW\n",CPU->PC);
          uint8_t flagbyte = (CPU->sign_flag << 7) + (CPU->zero_flag << 6) + (CPU->aux_carry_flag << 4) + (CPU->parity_flag << 2) + CPU->sign_flag;
          Memory[CPU->SP - 0x01] = flagbyte;
          Memory[CPU->SP - 0x02] = CPU->reg_A;
          CPU->SP = CPU->SP - 0x02;
          cycles_performed = 11;
          break;
        }

      case 0xF6: //logicaly or data and A register
        {
          //printf("%04X   ORI      #$%02X\n",CPU->PC, OP_CODE_1);
          inc_PC_with = 0x02;
          setflags(0xD5, CPU->reg_A, '|', OP_CODE_1, CPU);
          CPU->reg_A = CPU->reg_A | OP_CODE_1;
          cycles_performed = 7;
          break;
        }

      case 0xF7: //reset to adress 0x0030
        {
          //printf("%04X   RST  6\n",CPU->PC);
          inc_PC_with = 0x00;
          Memory[CPU->SP - 0x01] = (CPU->PC & 0xFF00) >> 8;
          Memory[CPU->SP - 0x02] = CPU->PC & 0x00FF;
          CPU->SP = CPU->SP - 0x02;
          CPU->PC = 0x0030;
          cycles_performed = 11;
          break;
        }

      case 0xF8: //return on minus
        {
          //printf("%04X   RM\n",CPU->PC);
          if (CPU->sign_flag) {
            inc_PC_with = 0x03;
            CPU->PC = (Memory[CPU->SP + 0x01] << 8) + Memory[CPU->SP];
            CPU->SP = CPU->SP + 0x02;
            cycles_performed = 11;
          } else {
            cycles_performed = 5;
            //printf("No return, s flag is 0\n");
          }
          break;
        }

      case 0xF9: // put value of CPU->reg_pair_HL into SP
        {
          //printf("%04X   SPHL\n",CPU->PC);
          CPU->SP = CPU->reg_pair_HL;
          cycles_performed = 5;
          break;
        }

      case 0xFA: // jump on minus
        {
          //printf("%04X   JM        $%02X%02X\n", CPU->PC, OP_CODE_2, OP_CODE_1);
          if (CPU->sign_flag) {
            inc_PC_with = 0x00;
            CPU->PC = (OP_CODE_2 << 8) + OP_CODE_1;
          } else {
            //printf("No jump, p flag is 0\n");
            inc_PC_with = 0x03;
          }
          cycles_performed = 10;
          break;
        }

      case 0xFB: // Enable interupts
      {
      //printf("%04X   EI\n", CPU->PC);
      CPU->interupts_enabled = true;
      break;
      }

      case 0xFC: // call on minus
      {
          //printf("%04X   CM       $%02X%02X\n", CPU->PC, OP_CODE_2, OP_CODE_1);
          if (CPU->sign_flag) {
            inc_PC_with = 0x00;
            Memory[CPU->SP - 0x01] = (CPU->PC & 0xFF00) >> 8;
            Memory[CPU->SP - 0x02] = CPU->PC & 0x00FF;
            CPU->SP = CPU->SP - 0x02;
            CPU->PC = (OP_CODE_2 << 8) + OP_CODE_1;
            cycles_performed = 17;
          } else {
            //printf("No Call, s flag is 0\n");
            inc_PC_with = 0x03;
            cycles_performed = 11;
          }
          break;
        }

      case 0xFD: // Call unconditionally
        {
            //printf("%04X   CALL*      $%02X%02X\n", CPU->PC, OP_CODE_2, OP_CODE_1);
            inc_PC_with = 0x00;
            Memory[CPU->SP - 0x01] = (CPU->PC & 0xFF00) >> 8;
            Memory[CPU->SP - 0x02] = CPU->PC & 0x00FF;
            CPU->SP = CPU->SP - 0x02;
            CPU->PC = (OP_CODE_2 << 8) + OP_CODE_1;
            break;
        }

      case 0xFE: // compare data and A register
        {
          //printf("%04X   CPI      #$%02X\n",CPU->PC, OP_CODE_1);
          inc_PC_with = 0x02;
          setflags(0xD5, CPU->reg_A, '-', OP_CODE_1, CPU);
          cycles_performed = 7;
          break;
        }

      case 0xFF: // resets PC to address 0x0038
        {
          //printf("%04X   RST  7\n",CPU->PC);
          inc_PC_with = 0x00;
          Memory[CPU->SP - 0x01] = (CPU->PC & 0xFF00) >> 8;
          Memory[CPU->SP - 0x02] = CPU->PC & 0x00FF;
          CPU->SP = CPU->SP - 0x02;
          CPU->PC = 0x0038;
          cycles_performed = 11;
          break;
        }



    }
    CPU->reg_pair_BC = (CPU->reg_B << 8) + CPU->reg_C;
    CPU->reg_pair_DE = (CPU->reg_D << 8) + CPU->reg_E;
    CPU->reg_pair_HL = (CPU->reg_H << 8) + CPU->reg_L;
    CPU->PC = CPU->PC + inc_PC_with;
    return cycles_performed;

}

#ifndef _8080_CORE_SDL
#define _8080_CORE_SDL


typedef struct e8080_cpu
{
  //running
  uint8_t running;
  //registers
  uint8_t reg_A;
  uint8_t reg_B;
  uint8_t reg_C;
  uint8_t reg_D;
  uint8_t reg_E;
  uint8_t reg_H;
  uint8_t reg_L;
  //flagbyte register
  uint8_t reg_flagbyte;
  //register pairs
  uint16_t reg_pair_HL;
  uint16_t reg_pair_BC;
  uint16_t reg_pair_DE;
  uint16_t reg_pair_PSW;

  //FLAGS
  uint8_t sign_flag;
  uint8_t zero_flag;
  uint8_t aux_carry_flag;
  uint8_t parity_flag;
  uint8_t carry_flag;
  bool interupts_enabled;
  //program counter and stack pointer
  uint16_t SP;
  uint16_t PC;

  //IO
  uint8_t OUT[0xFF]; // output
  uint8_t IN[0xFF];



}e8080_cpu;
#endif

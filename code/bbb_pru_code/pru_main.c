#include <stdint.h>
#include "pru_hal.h"

int main(void)
{
   volatile float x = 3.1415;
   volatile float y;
   volatile uint32_t z[4];
   int i, j;
   x *= 12.345;
   y = x * 1.2345;

  ocp_init();
  shm_init();
 
  /* i = 0; */
  while (1) 
  {

//      __asm__ __volatile__                                                   
//      (                                                                      
//      "  ADC_WAIT_DATA:    MOV r1, 0x44e0d0e4 \n" //ADC_MIO_ADDR + ADC_REG_FIFO0COUNT \n"                                                        
//      "  LBBO r14, r1, 0, 4 \n"                                           
//      "  AND r14, r14, (1 << 7) - 1 \n"                                  
//      "  QBEQ ADC_WAIT_DATA, r14, 0 \n"                                  
//      "  LDI32 r1, 0x44e0d100 \n" //ADC_MIO_ADDR + ADC_REG_FIFO0DATA \n"     
//      "  LBBO &r14, r1, 0, 16 \n"                                            
//      "  LDI32 r1, 0x00000fff \n"                                            
//      "  AND r14, r14, r1 \n"                                                
//      "  AND r15, r15, r1 \n"                                                
//      "  AND r16, r16, r1 \n"                                                
//      "  AND r17, r17, r1 \n"                                                
//      "  JMP R3.w2 \n"                                                       
//      );                                                                     


//    for(i=0;i<1000000;i++)
//    { 
       z[0] = adc_read(0, 0, 0);  
       z[1] = adc_read1(0, 0, 0);
       z[2] = adc_read2(0, 0, 0);
       z[3] = adc_read3(0, 0, 0);
//       z[1] = adc_read(0, 1, 0);  
       for(j=0;j<10;j++);
//    }
//    z[1]= z[0] * 0.000439453125; // Result in Volt
    shm_write_uint32(0, 0xdeadbeef);
//    shm_write_uint32(4, 0x2b2b2c2d);
//    shm_write_float(8, x);
//    shm_write_float(12, y);
    shm_write_uint32(12, 0xffffffff);
    shm_write_uint32(16, z[0]);
    shm_write_uint32(20, z[1]);
    shm_write_uint32(24, z[2]);
    shm_write_uint32(28, z[3]);
//    shm_write_float(20, z[1]);
//    shm_write_uint32(20, 0x12345678);
//    shm_write_uint32(24, 0x23456789);
//    shm_write_uint32(28, 0x34567890);
//    z= 0.0;

  }

  /* for (i = 0; i != 8; ++i) */
  /* { */
  /*   shm_write(i * 4, i); */
  /* } */

  __halt();

  return 0;
}


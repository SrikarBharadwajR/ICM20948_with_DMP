# 0 "/home/srikar/Projects/DMP_ICM20948/nrf/nRF/Source/nrf52_Vectors.s"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "/home/srikar/Projects/DMP_ICM20948/nrf/nRF/Source/nrf52_Vectors.s"
# 61 "/home/srikar/Projects/DMP_ICM20948/nrf/nRF/Source/nrf52_Vectors.s"
        .syntax unified
# 73 "/home/srikar/Projects/DMP_ICM20948/nrf/nRF/Source/nrf52_Vectors.s"
.macro VECTOR Name=
        .section .vectors, "ax"
        .code 16
        .word \Name
.endm




.macro EXC_HANDLER Name=



        .section .vectors, "ax"
        .word \Name



        .section .init.\Name, "ax"
        .thumb_func
        .weak \Name
        .balign 2
\Name:
        1: b 1b
.endm




.macro ISR_HANDLER Name=



        .section .vectors, "ax"
        .word \Name
# 116 "/home/srikar/Projects/DMP_ICM20948/nrf/nRF/Source/nrf52_Vectors.s"
        .section .init.\Name, "ax"
        .thumb_func
        .weak \Name
        .balign 2
\Name:
        1: b 1b

.endm




.macro ISR_RESERVED
        .section .vectors, "ax"
        .word 0
.endm




.macro ISR_RESERVED_DUMMY
        .section .vectors, "ax"
        .word Dummy_Handler
.endm







        .extern __stack_end__
        .extern Reset_Handler
        .extern HardFault_Handler
# 163 "/home/srikar/Projects/DMP_ICM20948/nrf/nRF/Source/nrf52_Vectors.s"
        .section .vectors, "ax"
        .code 16
        .balign 256
        .global _vectors
_vectors:



        VECTOR __stack_end__
        VECTOR Reset_Handler
        EXC_HANDLER NMI_Handler
        VECTOR HardFault_Handler





        EXC_HANDLER MemManage_Handler
        EXC_HANDLER BusFault_Handler
        EXC_HANDLER UsageFault_Handler

        ISR_RESERVED
        ISR_RESERVED
        ISR_RESERVED
        ISR_RESERVED
        EXC_HANDLER SVC_Handler



        EXC_HANDLER DebugMon_Handler

        ISR_RESERVED
        EXC_HANDLER PendSV_Handler
        EXC_HANDLER SysTick_Handler




        ISR_HANDLER POWER_CLOCK_IRQHandler
        ISR_HANDLER RADIO_IRQHandler
        ISR_HANDLER UARTE0_UART0_IRQHandler
        ISR_HANDLER SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQHandler
        ISR_HANDLER SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQHandler
        ISR_HANDLER NFCT_IRQHandler
        ISR_HANDLER GPIOTE_IRQHandler
        ISR_HANDLER SAADC_IRQHandler
        ISR_HANDLER TIMER0_IRQHandler
        ISR_HANDLER TIMER1_IRQHandler
        ISR_HANDLER TIMER2_IRQHandler
        ISR_HANDLER RTC0_IRQHandler
        ISR_HANDLER TEMP_IRQHandler
        ISR_HANDLER RNG_IRQHandler
        ISR_HANDLER ECB_IRQHandler
        ISR_HANDLER CCM_AAR_IRQHandler
        ISR_HANDLER WDT_IRQHandler
        ISR_HANDLER RTC1_IRQHandler
        ISR_HANDLER QDEC_IRQHandler
        ISR_HANDLER COMP_LPCOMP_IRQHandler
        ISR_HANDLER SWI0_EGU0_IRQHandler
        ISR_HANDLER SWI1_EGU1_IRQHandler
        ISR_HANDLER SWI2_EGU2_IRQHandler
        ISR_HANDLER SWI3_EGU3_IRQHandler
        ISR_HANDLER SWI4_EGU4_IRQHandler
        ISR_HANDLER SWI5_EGU5_IRQHandler
        ISR_HANDLER TIMER3_IRQHandler
        ISR_HANDLER TIMER4_IRQHandler
        ISR_HANDLER PWM0_IRQHandler
        ISR_HANDLER PDM_IRQHandler
        ISR_RESERVED
        ISR_RESERVED
        ISR_HANDLER MWU_IRQHandler
        ISR_HANDLER PWM1_IRQHandler
        ISR_HANDLER PWM2_IRQHandler
        ISR_HANDLER SPIM2_SPIS2_SPI2_IRQHandler
        ISR_HANDLER RTC2_IRQHandler
        ISR_HANDLER I2S_IRQHandler
        ISR_HANDLER FPU_IRQHandler


        .section .vectors, "ax"
_vectors_end:
# 264 "/home/srikar/Projects/DMP_ICM20948/nrf/nRF/Source/nrf52_Vectors.s"
        .section .init.Dummy_Handler, "ax"
        .thumb_func
        .weak Dummy_Handler
        .balign 2
Dummy_Handler:
        1: b 1b

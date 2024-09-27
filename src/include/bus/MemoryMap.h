#pragma once

#include <cstdint>

// Memory hardware constants

#define MEM0_START ((uint32_t)0x08000000)
#define MEM0_END   ((uint32_t)0x082E0000)
#define MEM0_SIZE  ((uint32_t)MEM0_END - MEM0_START)

#define MEM1_START ((uint32_t)0x00000000)
#define MEM1_END   ((uint32_t)0x02000000)
#define MEM1_SIZE  ((uint32_t)MEM1_END - MEM1_START)

#define MEM2_START ((uint32_t)0x10000000)
#define MEM2_END   ((uint32_t)0xD0000000)
#define MEM2_SIZE  ((uint32_t)MEM2_END - MEM2_START)

#define SRAM0_MIRROR_START ((uint32_t)0x0D400000)
#define SRAM0_MIRROR_END   ((uint32_t)0x0D7FFFFF)
#define SRAM0_MIRROR_SIZE  ((uint32_t)(SRAM0_MIRROR_END - SRAM0_MIRROR_START + 1))

#define SRAM0_START ((uint32_t)0xFFF00000)
#define SRAM0_END   ((uint32_t)0xFFFFFFFF)
#define SRAM0_SIZE  ((uint32_t)(SRAM0_END - SRAM0_START + 1))

#define OHCI_00_START ((uint32_t)0x0D050000)
#define OHCI_00_END   ((uint32_t)0x0D060000)
#define OHCI_00_SIZE  ((uint32_t)OHCI_00_END - OHCI_00_START)

#define OHCI_01_START ((uint32_t)0x0D060000)
#define OHCI_01_END   ((uint32_t)0x0D070000)
#define OHCI_01_SIZE  ((uint32_t)OHCI_01_END - OHCI_01_START)

#define OHCI_10_START ((uint32_t)0x0D130000)
#define OHCI_10_END   ((uint32_t)0x0D140000)
#define OHCI_10_SIZE  ((uint32_t)OHCI_10_END - OHCI_10_START)

#define OHCI_20_START ((uint32_t)0x0D150000)
#define OHCI_20_END   ((uint32_t)0x0D160000)
#define OHCI_20_SIZE  ((uint32_t)OHCI_20_END - OHCI_20_START)

#define EHCI_0_START ((uint32_t)0x0D040000)
#define EHCI_0_END   ((uint32_t)0x0D050000)
#define EHCI_0_SIZE  ((uint32_t)EHCI_0_END - EHCI_0_START)

#define EHCI_1_START ((uint32_t)0x0D120000)
#define EHCI_1_END   ((uint32_t)0x0D130000)
#define EHCI_1_SIZE  ((uint32_t)EHCI_1_END - EHCI_1_START)

#define EHCI_2_START ((uint32_t)0x0D140000)
#define EHCI_2_END   ((uint32_t)0x0D150000)
#define EHCI_2_SIZE  ((uint32_t)EHCI_2_END - EHCI_2_START)

#define HW_SRNPROT ((uint32_t)0x0d800060)
#define HW_AIPPROT ((uint32_t)0x0d800070)
#define HW_AIPCTRL ((uint32_t)0x0d800074)
#define HW_SPARE1 ((uint32_t)0x0d80018c)
#define LT_SYSCFG1 ((uint32_t)0x0d8005a4)

#define AHMN_RDBI    ((uint32_t)0x0d8b080C)
#define AHMN_TRSFSTS ((uint32_t)0x0d8b0850)

#define MEM_AHMFLUSH ((uint32_t)0x0d8b4228)

// Offsets to important things

#define STARBUCK_KERNEL_START_OFFS (0x01000300)
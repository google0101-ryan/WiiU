#include <bus/Bus.h>
#include <fstream>
#include <util/debug.h>
#include <sys/mman.h>

uint16_t mem0_cfg = 0;
uint16_t mem1_cfg = 0;
uint16_t mem2_cfg = 0;
uint32_t ahmn_mem0_cfg = 0;
uint32_t ahmn_mem1_cfg = 0;
uint32_t ahmn_mem2_cfg = 0;
uint32_t ahmn_mem0[0x20] = {0};
uint32_t ahmn_mem1[0x80] = {0};
uint32_t ahmn_mem2[0x100] = {0};

void ReadFile(const char* path, uint8_t* buf)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    file.read((char*)buf, size);
    file.close();
}

Bus::Bus(std::string starbuckKernel)
{
    // mem0 = new uint8_t[MEM0_SIZE];
    mem0 = (uint8_t*)mmap(NULL, MEM0_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    mem1 = (uint8_t*)mmap(NULL, MEM1_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    mem2 = (uint8_t*)mmap(NULL, MEM2_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    sram0 = new uint8_t[SRAM0_SIZE];

    for (int i = 0; i < 4; i++)
        ochi[i] = new OHCI();
    
    for (int i = 0; i < 3; i++)
        ehci[i] = new EHCI();
    
    latte = new Latte();

    // IOSU loader is position-independent within MEM1, so we load it at 0x01000000
    ReadFile(starbuckKernel.c_str(), mem1+0x01000000);
}

void Bus::Update()
{
    latte->Update();
}

void Bus::Dump()
{
    std::ofstream out("mem1.bin");
    out.write((char*)mem1, MEM1_SIZE);
    out.close();
    out.open("sram0.bin");
    out.write((char*)sram0, SRAM0_SIZE);
    out.close();
    out.open("mem0.bin");
    out.write((char*)mem0, MEM0_SIZE);
    out.close();
}

uint32_t Bus::SRead32(uint32_t addr)
{   
    if (addr == 0x8129a24)
        throw std::runtime_error("iosPanic()!");

    if (addr >= MEM0_START && addr < MEM0_END)
        return __builtin_bswap32(*(uint32_t*)&mem0[addr - MEM0_START]);
    if (addr >= MEM1_START && addr < MEM1_END)
        return __builtin_bswap32(*(uint32_t*)&mem1[addr - MEM1_START]);
    if (addr >= MEM2_START && addr < MEM2_END)
        return __builtin_bswap32(*(uint32_t*)&mem2[addr - MEM2_START]);
    if (addr >= SRAM0_START && addr < SRAM0_END)
        return __builtin_bswap32(*(uint32_t*)&sram0[addr - SRAM0_START]);

    switch (addr)
    {
    case HW_SRNPROT:
    case HW_AIPPROT:
    case HW_SPARE1:
    case HW_AIPCTRL:
        return 0;
    case AHMN_TRSFSTS:
    case LT_SYSCFG1:
    case 0x0d80004c:
    case 0x0d800194:
    case 0x0d8005e0:
        return 0;
    case 0x0d8b0800:
        return ahmn_mem0_cfg;
    case 0x0d8b0804:
        return ahmn_mem1_cfg;
    case 0x0d8b0808:
        return ahmn_mem2_cfg;
    case 0x0d8b0900 ... 0x0d8b0980:
        return ahmn_mem0[(addr - 0x0d8b0900) / 4];
    case 0x0d8b0a00 ... 0x0d8b0bff:
        return ahmn_mem1[(addr - 0x0d8b0a00) / 4];
    case 0x0d8b0c00 ... 0x0d8b1000:
        return ahmn_mem2[(addr - 0x0d8b0c00) / 4];
    }

    if (addr >= 0x0d800000 && addr < 0x0d80FFFF)
        return latte->Read32(addr);
    if (addr >= EHCI_0_START && addr < EHCI_0_END)
        return ehci[0]->Read32(addr - EHCI_0_START);
    if (addr >= EHCI_1_START && addr < EHCI_1_END)
        return ehci[1]->Read32(addr - EHCI_1_START);
    if (addr >= EHCI_2_START && addr < EHCI_2_END)
        return ehci[2]->Read32(addr - EHCI_2_START);

    printf("[BUS]: SRead32 from unknown address 0x%08x\n", addr);
    throw std::runtime_error("READ FROM UNKNOWN ADDRESS!\n");
}

uint16_t Bus::SRead16(uint32_t addr)
{
    if (addr >= MEM0_START && addr < MEM0_END)
        return __builtin_bswap16(*(uint16_t*)&mem0[addr - MEM0_START]);
    if (addr >= MEM1_START && addr < MEM1_END)
        return __builtin_bswap16(*(uint16_t*)&mem1[addr - MEM1_START]);
    if (addr >= MEM2_START && addr < MEM2_END)
        return __builtin_bswap16(*(uint16_t*)&mem2[addr - MEM2_START]);
    if (addr >= SRAM0_START && addr < SRAM0_END)
        return __builtin_bswap16(*(uint16_t*)&sram0[addr - SRAM0_START]);

    switch (addr)
    {
    case MEM_AHMFLUSH:
        return 0;
    case 0xD8B4400:
        return mem0_cfg;
    case 0xD8B4402:
        return mem1_cfg;
    case 0xD8B4404:
        return mem2_cfg;
    }

    printf("[BUS]: SRead16 from unknown address 0x%08x\n", addr);
    throw std::runtime_error("READ FROM UNKNOWN ADDRESS!\n");
}

uint8_t Bus::SRead8(uint32_t addr)
{
    if (addr >= MEM0_START && addr < MEM0_END)
        return mem0[addr - MEM0_START];
    if (addr >= MEM1_START && addr < MEM1_END)
        return mem1[addr - MEM1_START];
    if (addr >= MEM2_START && addr < MEM2_END)
        return mem2[addr - MEM2_START];
    if (addr >= SRAM0_START && addr < SRAM0_END)
        return sram0[addr - SRAM0_START];

    printf("[BUS]: SRead8 from unknown address 0x%08x\n", addr);
    throw std::runtime_error("READ FROM UNKNOWN ADDRESS!\n");
}

void Bus::SWrite32(uint32_t addr, uint32_t data)
{
    if (addr >= MEM0_START && addr < MEM0_END)
    {
        *(uint32_t*)&mem0[addr - MEM0_START] = __builtin_bswap32(data);
        return;
    }
    if (addr >= MEM1_START && addr < MEM1_END)
    {
        *(uint32_t*)&mem1[addr - MEM1_START] = __builtin_bswap32(data);
        return;
    }
    if (addr >= MEM2_START && addr < MEM2_END)
    {
        *(uint32_t*)&mem2[addr - MEM2_START] = __builtin_bswap32(data);
        return;
    }
    if (addr >= SRAM0_START && addr <= SRAM0_END)
    {
        *(uint32_t*)&sram0[addr - SRAM0_START] = __builtin_bswap32(data);
        return;
    }
    if (addr >= OHCI_00_START && addr < OHCI_00_END)
        return ochi[0]->Write32(addr - OHCI_00_START, data);
    if (addr >= OHCI_01_START && addr < OHCI_01_END)
        return ochi[1]->Write32(addr - OHCI_01_START, data);
    if (addr >= OHCI_10_START && addr < OHCI_10_END)
        return ochi[2]->Write32(addr - OHCI_10_START, data);
    if (addr >= OHCI_20_START && addr < OHCI_20_END)
        return ochi[3]->Write32(addr - OHCI_20_START, data);
    if (addr >= EHCI_0_START && addr < EHCI_0_END)
        return ehci[0]->Write32(addr - EHCI_0_START, data);
    if (addr >= EHCI_1_START && addr < EHCI_1_END)
        return ehci[1]->Write32(addr - EHCI_1_START, data);
    if (addr >= EHCI_2_START && addr < EHCI_2_END)
        return ehci[2]->Write32(addr - EHCI_2_START, data);
    
    switch (addr)
    {
    case HW_SRNPROT:
        printf("0x%08x -> HW_SRNPROT\n", data);
        return;
    case HW_AIPPROT:
        printf("0x%08x -> HW_AIPPROT\n", data);
        return;
    case HW_AIPCTRL:
    case HW_SPARE1:
    case AHMN_RDBI:
    case LT_SYSCFG1:
    case 0x0d800058:
    case 0x0d80005c:
    case 0x0d800194:
    case 0x0d8b0820:
    case 0x0d8b0824:
    case 0x0d8b0840:
    case 0x0d8b0844:
    case 0x0d80004c:
    case 0x0d800510:
    case 0x0d8005e0:
        return;
    case 0x0d8b0900 ... 0x0d8b0980:
        ahmn_mem0[(addr - 0x0d8b0900) / 4] = data ^ 0x80000000;
        return;
    case 0x0d8b0a00 ... 0x0d8b0bff:
        ahmn_mem1[(addr - 0x0d8b0a00) / 4] = data ^ 0x80000000;
        return;
    case 0x0d8b0c00 ... 0x0d8b1000:
        ahmn_mem2[(addr - 0x0d8b0c00) / 4] = data ^ 0x80000000;
        return;
    case 0x0d8b0800:
        ahmn_mem0_cfg = data;
        return;
    case 0x0d8b0804:
        ahmn_mem1_cfg = data;
        return;
    case 0x0d8b0808:
        ahmn_mem2_cfg = data;
        return;
    }
    

    if (addr >= 0x0d800000 && addr < 0x0d80FFFF)
        return latte->Write32(addr, data);

    printf("[BUS]: SWrite32 to unknown address 0x%08x\n", addr);
    throw std::runtime_error("WRITE TO UNKNOWN ADDRESS!\n");
}

void Bus::SWrite16(uint32_t addr, uint16_t data)
{
    if (addr >= MEM0_START && addr < MEM0_END)
    {
        *(uint16_t*)&mem0[addr - MEM0_START] = __builtin_bswap16(data);
        return;
    }
    if (addr >= MEM1_START && addr < MEM1_END)
    {
        *(uint16_t*)&mem1[addr - MEM1_START] = __builtin_bswap16(data);
        return;
    }
    if (addr >= MEM2_START && addr < MEM2_END)
    {
        *(uint16_t*)&mem2[addr - MEM2_START] = __builtin_bswap16(data);
        return;
    }
    if (addr >= SRAM0_START && addr <= SRAM0_END)
    {
        *(uint16_t*)&sram0[addr - SRAM0_START] = __builtin_bswap16(data);
        return;
    }

    switch (addr)
    {
    case 0x0d8b4228:
    case 0x0d8b4406 ... 0x0d8b44FF:
        return;
    case 0x0d8b4400:
        mem0_cfg = data;
        return;
    case 0x0d8b4402:
        mem1_cfg = data;
        return;
    case 0x0d8b4404:
        mem2_cfg = data;
        return;
    }

    printf("[BUS]: SWrite16 to unknown address 0x%08x\n", addr);
    throw std::runtime_error("WRITE TO UNKNOWN ADDRESS!\n");
}

void Bus::SWrite8(uint32_t addr, uint8_t data)
{
    if (addr >= MEM0_START && addr < MEM0_END)
    {
        mem0[addr - MEM0_START] = data;
        return;
    }
    if (addr >= MEM1_START && addr < MEM1_END)
    {
        mem1[addr - MEM1_START] = data;
        return;
    }
    if (addr >= MEM2_START && addr < MEM2_END)
    {
        mem2[addr - MEM2_START] = data;
        return;
    }
    if (addr >= SRAM0_START && addr < SRAM0_END)
    {
        sram0[addr - SRAM0_START] = data;
        return;
    }

    printf("[BUS]: SWrite8 to unknown address 0x%08x\n", addr);
    throw std::runtime_error("WRITE TO UNKNOWN ADDRESS!\n");
}

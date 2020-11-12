#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "EeeP.h"

#ifndef TRUE
#define TRUE    (1)
#endif 
#ifndef FALSE
#define FALSE   (0)
#endif 
#ifndef BOOL
#define BOOL    int
#endif 

#ifndef ARRAYSIZE
#define ARRAYSIZE(a)    (sizeof(a)/sizeof((a)[0]))
#endif // !ARRAYSIZE

#ifndef FIELD_OFFSET
#define FIELD_OFFSET(type,field)    (((size_t)(void *)&(((type *)0)->field)))
#endif

void PrintUdid(EeePUDId_t *udid)
{
	printf("Vendor: %02X%02X, Device: %02X%02X, Flavor: %02X, Rev: %02X",
		udid->VendId[0], udid->VendId[1], udid->DeviceId[0], 
		udid->DeviceId[1], udid->DeviceFlav, udid->RevId);
}

void ParseCarrier(COM0R20_CB_t *cbp)
{
	const char *misc_io1_str[] = 
		{ "WAKE0", "WAKE1", "SUS", "BATLOW", "THRMP", "EBROM", "WDT", "AC97" };
 	const char *misc_io2_str[] = 
		{ "SSC", "SDIO", "LID_SW", "Sleep", "FAN0", "SER0", "SER1", "Reserved" };
	const char *ddi_port_str[] =
		{ "eDisplay Port", "Display Port", "HDMI/DVI", "SDVO" };
	int count;
    printf("Carrier board EEPROM\n");
    printf("   "); PrintUdid(&cbp->DevId); printf("\n");
    printf("   Carrier Type: %d\n", cbp->CBType);
    printf("   Spec Revision: %d.%d\n", cbp->SpecRev >> 4 & 0xF, cbp->SpecRev & 0xF);
    printf("   USB Port Count: Super Speed: %d, High Speed: %d\n",
		cbp->UsbDesc >> 4 & 7, cbp->UsbDesc & 0xF);
	printf("   SAS ports: ");
	for(int i = 0; i < 4; i++) {
		printf("%d:%s ", i + 1, 
			(cbp->SasDesc >> (i * 2) & 1) ?
				((cbp->SasDesc >> (i * 2 + 1) & 1) ? "SAS" : "SATA") : "None"); 
	}
	printf("\n");
	printf("   LAN: ");
	for(int i = 0; i < 3; i++) {
		if(cbp->LanDesc & (1 << i)) {
			printf("GBE%d ", i);
		}
	}
	printf("\n");
	printf("   Misc IO1: ");
	for(int i = 0; i < 8; i++) {
		if(cbp->MiscIo1 & (1 << i)) {
			printf("%s ", misc_io1_str[i]);
		}
	}
	printf("\n");
	printf("   Misc IO2: ");
	for(int i = 0; i < 8; i++) {
		if(cbp->MiscIo2 & (1 << i)) {
			printf("%s ", misc_io2_str[i]);
		}
	}
	printf("\n");
	printf("   DDI: ");
	for(int i = 0; i < 4; i++) {
		uint8_t ddi_desc = cbp->DDIDesc[i >> 1];
		if(i & 1)
			ddi_desc >>= 4;
		ddi_desc &= 7;
		if(ddi_desc >= 1 && ddi_desc <= 4) {
			printf("%d:%s ", i, ddi_port_str[ddi_desc - 1]);
		}
	}
	printf("\n");
	printf("   PCI Express Lane Gen:");
	count = 0;
	for(int i = 0; i < 32; i++) {
		uint8_t gen = cbp->PCIeGen[i >> 2];
		gen >>= (i & 3) << 1;
		gen &= 3;
		if(gen != 3) {
			if(count % 16 == 0) {
				printf("\n      ");
			}
			printf("%d:Gen%d ", i, gen + 1);
			count ++;
		}
	}
	printf("\n");
	printf("   PCI Express Lane Map:");
	count = 0;
	for(int i = 0; i < 32; i++) {
		uint8_t map = cbp->LaneMap[i >> 1];
		if(i & 1)
			map >>= 4;
		map &= 7;
		if(map != 0 && map != 7) {
			if(count % 16 == 0) {
				printf("\n      ");
			}
			printf("%d:x%d ", i, 1 << (map-1));
			count ++;
		}
	}
	printf("\n");
}

void ParseModule(COM0R20_M_t *mp)
{
    printf("Module EEPROM\n");
    printf("   "); PrintUdid(&mp->DevId); printf("\n");
    printf("   Module Type: %d\n", mp->MType);
    printf("   Spec Revision: %d.%d\n", mp->SpecRev >> 4 & 0xF, mp->SpecRev & 0xF);
}

static void PrintPortMask(const char *ptype, uint8_t field, int start_mask, int max_count)
{
    printf("   %s Ports In Use: ", ptype);
    for(int i = 0; i < max_count; i++) {
        if(field & (start_mask << i)) {
            printf("%d ", i);
        }
    }
    printf("\n");
}

void ParseComHpcCarrier(COMHR10_CB_t *cbp)
{
	const char *misc_io1_str[] = 
		{ "Wake0", "Wake1", "Suspend", "BatteryLow", "ThermalProtect", "Fan", "Watchdog" };
 	const char *misc_io2_str[] = 
		{ "RapidHutdown", "TamperDetect", "LidSwitch", "Sleep" };
	const char *ddi_port_str[] =
		{ "None", "Display Port", "HDMI/DVI" };
	const char *csi_port_str[] =
		{ "None", "CSI-2", "CSI-3" };
    const char *uart_str[] = 
        { "None", "RS232", "RS232 with RTS/CTS", "RS485" };
    const char *i2c_str[] = 
        { "None", "100 KHz", "400 KHz", "No speed limit" };
    const char *mgmt_str[] =
        { "eSPI", "Boot SPI", "IPMB", "SM Bus", "BIOS Select", "PCIe BMC" };
    const char *pcie_gens[] = 
        { "Gen1/2", "Gen3", "Gen4", "Gen5" };
        
        
	int count;
    printf("COM_HPC Carrier board EEPROM\n");
    printf("   "); PrintUdid(&cbp->DevId); printf("\n");
    printf("   COM-HPC Carrier Type: %s, MaxSize: %c%s\n", 
        (cbp->CBType >> 6) & 1 ? "C.M" : "C.U",
        'A' + ((cbp->CBType >> 3) & 7),
        cbp->CBType & 4 ? "Multi-Module," : "");
    printf("   Spec Revision: %d.%d\n", cbp->SpecRev >> 4 & 0xF, cbp->SpecRev & 0xF);
    PrintPortMask("USB 2.0", cbp->UsbDesc1, 1, 8);
    PrintPortMask("USB 3.x/4.x", cbp->UsbDesc2, 1, 4);
    PrintPortMask("SATA", cbp->SataBaseTDesc, 1, 2);
    PrintPortMask("NBASE-T", cbp->SataBaseTDesc, 0x10, 2);
    PrintPortMask("Ethernet", cbp->KrKxDesc, 1, 8);
    printf("   DDI: DDI0: %s; DDI1: %s; DDI2: %s%s%s\n",
        ddi_port_str[cbp->DDIDesc & 3],
        ddi_port_str[(cbp->DDIDesc >> 2) & 3],
        ddi_port_str[(cbp->DDIDesc >> 4)  & 3],
        cbp->DDIDesc & 0x40 ? "; eDP" : "",
        cbp->DDIDesc & 0x80 ? "; MIPI DSI" : "");
     printf("   CSI/Audio: CSI0: %s; CSI1: %s%s%s\n",
        csi_port_str[cbp->CSIAudioDesc & 3],
        csi_port_str[(cbp->CSIAudioDesc >> 2) & 3],
        cbp->CSIAudioDesc & 0x10 ? "; SoundWire" : "",
        (cbp->CSIAudioDesc & 0x60) == 0x20 ? "; I2S" :
            (cbp->CSIAudioDesc & 0x60) == 0x40 ? "; SoundWire 2" : "");
    printf("   UART/I2C: UART0: %s; UART1: %s; I2C0: %s; I2C1: %s\n",
        uart_str[cbp->UartI2CDesc & 3], uart_str[(cbp->UartI2CDesc >> 2) & 3],
        i2c_str[(cbp->UartI2CDesc >> 4) & 3], i2c_str[(cbp->UartI2CDesc >> 6) & 3]);
        
    printf("   Management interfaces: ");
	for(int i = 0; i < 6; i++) {
		if(cbp->MgmtDesc & (1 << i)) {
			printf("%s ", mgmt_str[i]);
		}
	}
	printf("\n");
    PrintPortMask("SPI Chip Select", cbp->SpiDesc, 1, 4);
	printf("   Misc IO1: ");
	for(int i = 0; i < 7; i++) {
		if(cbp->MiscIo1 & (1 << i)) {
			printf("%s ", misc_io1_str[i]);
		}
	}
	printf("\n");
	printf("   Misc IO2: ");
	for(int i = 0; i < 4; i++) {
		if(cbp->MiscIo2 & (1 << i)) {
			printf("%s ", misc_io2_str[i]);
		}
	}
	printf("\n");
	printf("   PCI Express Lane Gen:");
	count = 0;
	for(int i = 0; i < 64; i++) {
		uint8_t gen = cbp->PCIeGen[i >> 2];
		gen >>= (i & 3) << 1;
		gen &= 3;
		if(gen != 3) {
			if(count % 16 == 0) {
				printf("\n      ");
			}
			printf("%d:%s ", i, pcie_gens[gen]);
			count ++;
		}
	}
	printf("\n");
	printf("   PCI Express Lane Map:");
	count = 0;
	for(int i = 0; i < 64; i++) {
		uint8_t map = cbp->LaneMap[i >> 1];
		if(i & 1)
			map >>= 4;
		map &= 7;
		if(map != 0) {
			if(count % 16 == 0) {
				printf("\n      ");
			}
			printf("%d:x%d ", i, 1 << (map-1));
			count ++;
		}
	}
	printf("\n");
    for(int i = 0; i < 2; i++) {
        if(cbp->PCIeTarget[i] & 0x80) {
            printf("   PCIe Target %d: Lane %d\n", i+1, cbp->PCIeTarget[i] & 0x3F);
        }
    }
}

void ParseComHpcModule(COMHR10_M_t *mp)
{
	const char *arch_str[] = 
		{ "X86", "ARM", "RISC-V", "Reserved", "Other", "GPU", "FPGA" };
	const char *arch64_str[] = 
		{ "X64", "Aarch64", "64-bit RISC-V", "Reserved", "64-bit Other", "64-bit GPU", "64-bit FPGA" };
    const char *mgmt_str[] =
        { "eSPI", "Boot SPI", "IPMB", "SM Bus", "BIOS Select", "PCIe BMC" };
        
    printf("COM-HPC Module EEPROM\n");
    printf("   "); PrintUdid(&mp->DevId); printf("\n");
    printf("   COM-HPC Module Type: %s, Pins: %d, MaxSize: %c\n", 
        (mp->MType >> 6) & 2 ? "M.F" : (mp->MType >> 6) & 1 ? "M.B" : "M.U",
        mp->MType & 7,
        'A' + ((mp->MType >> 3) & 7));
    printf("   Spec Revision: %d.%d\n", mp->SpecRev >> 4 & 0xF, mp->SpecRev & 0xF);
    printf("   Architecture: %s\n", mp->Architecture & 0x80 ? arch_str[mp->Architecture & 0x7F] : arch64_str[mp->Architecture & 0x7F]);
    printf("   CPU Count: %d, Cores/CPU: %d, Threads/Core: %d\n", 
        mp->NumCpus & 0xF, mp->NumCores, (mp->NumCpus >> 4) & 0xF);
    printf("   Max Memory Size: %d Gb, Memory Slots: %d\n", 
        1 << (mp->MaxMem & 0xF), (mp->MaxMem >> 4) & 0xF);
    printf("   USB Port Counts: USB 2: %d, USB 3.2 Gen1/2: %d, USB 3.2 Gen 2x2: %d, USB 4: %d\n", 
        mp->UsbDesc1 & 0xF, (mp->UsbDesc1 >> 4) & 3, mp->UsbDesc2 & 7, (mp->UsbDesc2 >> 4) & 7);
    printf("   Ethernet Port Counts: NBASE-T: %d, KR/KX: %d\n", 
        mp->EthSataDesc & 3, (mp->EthSataDesc >> 2) & 0xF);
    printf("   SATA/SAS Port Count: %d\n", (mp->EthSataDesc >> 6) & 3);
    printf("   PCI Express: Lanes: %d, Target Ports: %d\n", mp->PciELanes & 0x7F, mp->PciEDesc & 3);
    printf("   Management interfaces: ");
	for(int i = 0; i < 6; i++) {
		if(mp->MgmtDesc & (1 << i)) {
			printf("%s ", mgmt_str[i]);
		}
	}
	printf("\n");
    printf("   Video: DDI Ports: %d; CSI Ports: %d%s%s%s\n",
        mp->VideoDesc & 3,
        (mp->VideoDesc >> 3) & 3,
        mp->VideoDesc & 4 ? "; eDP/DSI" : "",
        mp->VideoDesc & 0x20 ? "; Soundwire Audio" : "",
        mp->VideoDesc & 0x40 ? "; I2S Audio" : "");
    printf("   Serial Port Count: %d\n", mp->GPBusDesc & 3);
    printf("   I2C Port Count: %d\n", (mp->GPBusDesc >> 2) & 3);
    if(mp->GPBusDesc & 0x10) {
        printf("   General Purpose SPI Implemented\n");
    }
}

void ParseExpansion(Exp_EEP_t *exp)
{
    printf("Expansion EEPROM \n");
    printf("   "); PrintUdid(&exp->DevId); printf("\n");
}

struct {
    uint8_t code;
    const char *text;
} DynamicBlockTexts[] = {
    { 0xF0, "VENDOR SPECIFIC"},
    { 0xF1, "EXPANSION EEPROM DESCRIPTOR" },
    { 0xD0, "SMBIOS INFORMATION" },
    { 0xD1, "LFP DISPLAY" },
    { 0xF2, "CRC" },
    { 0xFF, "IGNORED" },
    { 0xE0, "EXPRESS CARD TOPOLOGY" },
    { 0xE1, "SERIAL PORT DESCRIPTOR" },
    { 0xE2, "NETWORK PORT BLOCK" },
    { 0xE3, "PCI-E BIFURCATION BLOCK" },
    { 0xE4, "USB PORT BLOCK" },
    { 0xE5, "DISPLAY BLOCK" },
    { 0, "END" }
};

struct {
    enum SMBIOS_BlockId_e code;
    const char *text;
} SmbiosBlockTexts[] = {
    { SMBIOS_TypeBIOS_INFORMATION, "BIOS" },
    { SMBIOS_TypeSYSTEM_INFORMATION, "SYSTEM" },
    { SMBIOS_TypeBASE_BOARD_MODULE_INFORMATION, "BOARD MODULE" },
    { SMBIOS_TypeSYSTEM_ENCLOSURE_OR_CHASSIS, "ENCLOSURE OR CHASSIS" },
    { SMBIOS_TypePROCESSOR_INFORMATION, "PROCESSOR" },
    { SMBIOS_TypeMEMORY_CONTROLLER_INFORMATION, "MEMORY CONTROLLER" },
    { SMBIOS_TypeMEMORY_MODULE_INFORMATION, "MEMORY MODULE" },
    { SMBIOS_TypeEND_OF_TABLE, "END OF TABLE" }
};

struct {
    uint8_t code;
    const char *text;
} BoardTypeTexts[] = {
    { SMBIOS_BoardType_Unknown, "Unknown" },
    { SMBIOS_BoardType_Other, "Other" },
    { SMBIOS_BoardType_ServerBlade, "Server Blade" },
    { SMBIOS_BoardType_ConnectivitySwitch, "Connectivity Switch" },
    { SMBIOS_BoardType_SystemManagementModule, "System Management Module" },
    { SMBIOS_BoardType_ProcessorModule, "Processor Module" },
    { SMBIOS_BoardType_IO_Module, "IO Module" },
    { SMBIOS_BoardType_Memory_Module, "Memory Module" },
    { SMBIOS_BoardType_DaughterBoard, "Daughter Board" },
    { SMBIOS_BoardType_Motherboard, "Motherboard" },
    { SMBIOS_BoardType_ProcessorMemory_Module, "Processor Memory Module" },
    { SMBIOS_BoardType_ProcessorIO_Module, "Processor IO Module" },
    { SMBIOS_BoardType_Interconnect_Board, "Interconnect Board" }
};

struct {
    uint8_t code;
    const char *text;
} ChassisTypeTexts[] = {
    { SMBIOS_ChassisType_Unknown, "Unknown" },
    { SMBIOS_ChassisType_Other, "Other" },
    { SMBIOS_ChassisType_Desktop, "Desktop" },
    { SMBIOS_ChassisType_Low_Profile_Desktop, "Low Profile Desktop" },
    { SMBIOS_ChassisType_Pizza_Box, "Pizza Box" },
    { SMBIOS_ChassisType_Mini_Tower, "Mini Tower" },
    { SMBIOS_ChassisType_Tower, "Tower" },
    { SMBIOS_ChassisType_Portable, "Portable" },
    { SMBIOS_ChassisType_Laptop, "Laptop" },
    { SMBIOS_ChassisType_Notebook, "Notebook" },
    { SMBIOS_ChassisType_Hand_Held, "Hand Held" },
    { SMBIOS_ChassisType_Docking_Station, "Docking Station" },
    { SMBIOS_ChassisType_All_In_One, "All In One" },
    { SMBIOS_ChassisType_Sub_Notebook, "Sub Notebook" },
    { SMBIOS_ChassisType_Space_Saving, "Space Saving" },
    { SMBIOS_ChassisType_Lunch_Box, "Lunch Box" },
    { SMBIOS_ChassisType_Main_Server_Chassis, "Main Server Chassis" },
    { SMBIOS_ChassisType_Expansion_Chassis, "Expansion Chassis" },
    { SMBIOS_ChassisType_SubChassis, "Sub Chassis" },
    { SMBIOS_ChassisType_Bus_Expansion_Chassis, "Bus Expansion Chassis" },
    { SMBIOS_ChassisType_Peripheral_Chassis, "Peripheral Chassis" },
    { SMBIOS_ChassisType_RAID_Chassis, "RAID Chassis" },
    { SMBIOS_ChassisType_Rack_Mount_Chassis, "Rack Mount Chassis" },
    { SMBIOS_ChassisType_Sealed_case_PC, "Sealed Case PC" },
    { SMBIOS_ChassisType_Multi_system_chassis, "Multi-System Chassis" }
};

const char *FindDynamicBlockText(uint8_t blockId)
{
    for (int i = 0; i < ARRAYSIZE(DynamicBlockTexts); i++) {
        if (blockId == DynamicBlockTexts[i].code) {
            return DynamicBlockTexts[i].text;
        }
    }
    return "";
}

const char *FindSmbiosBlockText(enum SMBIOS_BlockId_e blockId)
{
    for (int i = 0; i < ARRAYSIZE(SmbiosBlockTexts); i++) {
        if (blockId == SmbiosBlockTexts[i].code) {
            return SmbiosBlockTexts[i].text;
        }
    }
    return "";
}

const char *FindBoardTypeText(uint8_t blockId)
{
    for (int i = 0; i < ARRAYSIZE(BoardTypeTexts); i++) {
        if (blockId == BoardTypeTexts[i].code) {
            return BoardTypeTexts[i].text;
        }
    }
    return "";
}

const char *FindChassisTypeText(uint8_t blockId)
{
    for (int i = 0; i < ARRAYSIZE(ChassisTypeTexts); i++) {
        if (blockId == ChassisTypeTexts[i].code) {
            return ChassisTypeTexts[i].text;
        }
    }
    return "";
}

const char *GetSmbiosString(uint8_t index, void *base)
{
    const char *strbase = (const char *)base;
    //printf("strbase=%s, index=%d\n", strbase, index);
    if (index == 0)
        return NULL;
    for (--index; index; --index) {
        strbase += strlen(strbase) + 1;
    }
    return strbase;
}

void ParseSystemInformation(EeePSystemInfo_t *info)
{
    const char *str;
    if (str = GetSmbiosString(info->Manufacturer, info+1)) {
        printf("      Manufacturer: %s\n", str);
    }
    if (str = GetSmbiosString(info->ProductName, info+1)) {
        printf("      Product Name: %s\n", str);
    }
    if (str = GetSmbiosString(info->Version, info+1)) {
        printf("      Version: %s\n", str);
    }
    if (str = GetSmbiosString(info->SerialNumber, info + 1)) {
        printf("      Serial Number: %s\n", str);
    }
    printf("      UUID: %08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X\n",
        info->UUID.a.n, info->UUID.b[0].n, info->UUID.b[1].n, 
        info->UUID.c[0], info->UUID.c[1], info->UUID.c[2], info->UUID.c[3], 
        info->UUID.c[4], info->UUID.c[5], info->UUID.c[6], info->UUID.c[7]);
    if (str = GetSmbiosString(info->SKU_Number, info + 1)) {
        printf("      SKU Number: %s\n", str);
    }
    if (str = GetSmbiosString(info->Family, info + 1)) {
        printf("      Family: %s\n", str);
    }
}

void ParseBoardInformation(EeePModuleInfo_t *info)
{
    void *strstart = info->Handles + info->ContainedHndls;
    const char *str;
    if (str = GetSmbiosString(info->Manufacturer, strstart)) {
        printf("      Manufacturer: %s\n", str);
    }
    if (str = GetSmbiosString(info->Product, strstart)) {
        printf("      Product: %s\n", str);
    }
    if (str = GetSmbiosString(info->Version, strstart)) {
        printf("      Version: %s\n", str);
    }
    if (str = GetSmbiosString(info->SerialNumber, strstart)) {
        printf("      Serial Number: %s\n", str);
    }
    if (str = GetSmbiosString(info->AssetTag, strstart)) {
        printf("      Asset Tag: %s\n", str);
    }
    if (info->FeatureFlag) {
        printf("      Features: ");
        if (info->FeatureFlag & SMBIOS_IS_Motherboard) {
            printf("Motherboard ");
        }
        if (info->FeatureFlag & SMBIOS_REQ_DAUGHTER) {
            printf("ReqDaughterBoard ");
        }
        if (info->FeatureFlag & SMBIOS_REMOVABLE) {
            printf("Removable ");
        }
        if (info->FeatureFlag & SMBIOS_REPLACEABLE) {
            printf("Replaceable ");
        }
        if (info->FeatureFlag & SMBIOS_HOT_SWAP_CAP) {
            printf("HotSwapCapable ");
        }
        printf("\n");
    }
    if (str = GetSmbiosString(info->Location, strstart)) {
        printf("      Location: %s\n", str);
    }
    printf("      Board Type: %d (%s)\n", 
        info->BoardType, FindBoardTypeText(info->BoardType));
}   

void ParseChassisInformation(EeePChassisInfo_t *info)
{
    void *strstart = (unsigned char *)info->CElement + info->CElementCnt * info->CElementSize;
    const char *str;
    if (str = GetSmbiosString(info->Manufacturer, strstart)) {
        printf("      Manufacturer: %s\n", str);
    }
    printf("      Chassis Type: %d (%s)\n",
        info->Type, FindChassisTypeText(info->Type));
    if (str = GetSmbiosString(info->Version, strstart)) {
        printf("      Version: %s\n", str);
    }
    if (str = GetSmbiosString(info->SerialNumber, strstart)) {
        printf("      Serial Number: %s\n", str);
    }
    if (str = GetSmbiosString(info->AssetTagNumber, strstart)) {
        printf("      Asset Tag: %s\n", str);
    }
    printf("      Height: %dU\n", info->Height);
    printf("      Number of Power Cords: %d\n", info->NumPowerCords);
}

void ParseLFP(EeePLFPDataBlock_t *dblock, int size)
{
	int raw_size = size - FIELD_OFFSET(EeePLFPDataBlock_t, RawData);
	printf("   LFP block:\n      Interface: %d\n", dblock->Interface);
	printf("      Raw Data:\n      ");
	for( int i = 0; i < raw_size; i++ ) {
		if( i%16 == 0) {
			printf("\n      ");
		}
		printf("%02X ", dblock->RawData[i]);
	}
	printf("\n");
}

void ParseExpressCard(ExpCardBlock_t *dblock, int size)
{
	int addr_array_size = size - FIELD_OFFSET(ExpCardBlock_t, SwitchDevFuncAddr);
	printf("   Express Card block:\n      Card: %d, USB Port: %d, PCIExpress Port: %d\n", 
		dblock->ExpressCardNumber, dblock->ComExpressPort >> 5, dblock->ComExpressPort & 0x1F);
	printf("      Switch Addresses: ");
	for( int i = 0; i < addr_array_size; i++ ) {
		uint8_t dev_func = dblock->SwitchDevFuncAddr[i];
		if(dev_func == 0xFF)
			break;
		if(i != 0) {
			printf(", ");
		}
		printf("%d:%d", dev_func >> 3, dev_func & 7);
	}
	printf("\n");
}

void PrintSerialPortIRQ(uint8_t irq)
{
	switch(irq)
	{
	case 0:
		printf("Auto");
		break;
	case 1:
		printf("No IRQ");
		break;
	case 2:
		printf("Reserved");
		break;
	default:
		printf("IRQ %d", irq);
		break;
	}
}

void ParseSerialPort(SerPortCfgBlock_t *dblock)
{
	printf("   Serial Port COM block:\n");
	printf("      Port 0: Base: %x, IRQ: ", (unsigned int)dblock->Ser0BaseAddr[0] << 8 | dblock->Ser0BaseAddr[1]);
	PrintSerialPortIRQ(dblock->SerIRQ & 0x0F);
	printf("\n");
	printf("      Port 1: Base: %x, IRQ: ", (unsigned int)dblock->Ser1BaseAddr[0] << 8 | dblock->Ser1BaseAddr[1]);
	PrintSerialPortIRQ((dblock->SerIRQ >> 4) & 0x0F);
	printf("\n");
}

//  COM-HPC Network block

void ParseNetworkBlock(NetworkPortBlock_t *dblock)
{
    const char *port_types[] = { "NBASE-T", "KR/KX" };
    const char *speeds[] =
        { "10MB", "100MB", "1GB", "2.5GB", "5GB", "10GB", "25GB", "40GB" };
    const char *options[] =
        { "FullDuplex", "AutoNeg", "SyncE", "Backplane", "PHY" };
	printf("   COM-HPC Network Port block:\n");
	printf("      Port %d: %s\n", dblock->PortNr & 0xF, 
            (dblock->PortNr >> 4) < ARRAYSIZE(port_types) ? port_types[dblock->PortNr >> 4] : "?");
    printf("      Supported speeds: ");
    for(int i = 0; i < 8; i++) {
        if(dblock->SupportedSpeeds & (1 << i)) {
            printf("%s ", speeds[i]);
        }
    }
    printf("\n");
    printf("      Supported options: ");
    for(int i = 0; i < 5; i++) {
        if(dblock->Options & (1 << i)) {
            printf("%s ", options[i]);
        }
    }
    printf("\n");
    printf("      MAC Address: %02X-%02X-%02X-%02X-%02X-%02X\n",
        dblock->MfgMacAddress[0], dblock->MfgMacAddress[1],
        dblock->MfgMacAddress[2], dblock->MfgMacAddress[3],
        dblock->MfgMacAddress[4], dblock->MfgMacAddress[5]);
}

//  COM-HPC PCI-E Bifurcation block

void ParsePciEBifurcationBlock(PciEBifurcationBlock_t *dblock)
{
    int idx1, idx2;
    PciELane_t *laneDescr = dblock->LaneDescriptors;
	printf("   COM-HPC PCI-E Bifurcation block:\n");
    for(idx1 = 0; idx1 < dblock->NrOfLaneDescriptors; idx1++) {
        int count2 = laneDescr->NrOfConfigurations;
        printf("      Lane %d: Gen%d%s%s%s\n", 
            laneDescr->LaneNumber, 
            laneDescr->LaneCapabilities & 0xF,
            laneDescr->LaneCapabilities & 0x20 ? "; NVME" : "",
            laneDescr->LaneCapabilities & 0x40 ? "; PEG" : "",
            laneDescr->LaneCapabilities & 0x80 ? "; Target" : "");
        printf("         Configurations: ");
        for(idx2 = 0; idx2 < count2; idx2++) {
            printf("%d%s ", 
                laneDescr->Configurations[idx2] & 0x3F,
                laneDescr->Configurations[idx2] & 0x80 ? "(R)" : ""); 
        }
        printf("\n");
        laneDescr = (PciELane_t *)((const char *)laneDescr + FIELD_OFFSET(PciELane_t, Configurations) + count2);
    }
}

//  COM-HPC USB Port block

void ParseUsbPortBlock(USBPortBlock_t *dblock)
{
    const char *generations[] =
        { "USB 2.0", "USB 3.2 Gen 1", "USB 3.2 Gen 2", "USB 3.2 Gen 2x2", "USB 4.0 Gen 2x2", "USB 4.0 Gen 3x2" };
    const char *flags[] =
        { "DisplayPort", "PCIe", "USB-C-Connector", "USB-C-LaneMuxing", "Port80", "ClientMode" };
	printf("   COM-HPC USB Port block:\n");
	printf("      Port %d: Generation: %s\n", dblock->PortNr, generations[dblock->UsbGeneration & 7] );
    if((dblock->UsbGeneration >> 4) & 7) {
        printf("      Paired with port %d\n", (dblock->UsbGeneration >> 4) & 7);
    }
    printf("      Supported features: ");
    for(int i = 0; i < 6; i++) {
        if(dblock->Flags & (1 << i)) {
            printf("%s ", flags[i]);
        }
    }
    printf("\n");
}

//  COM-HPC Display Port block

void ParseDisplayBlock(DisplayBlock_t *dblock)
{
    const char *interfaces[] =
        { "DisplayPort", "HDMI/DVI", "DSI", "eDP" };
    const char *port_types[] = { "DDI", "eDP/DSI" };
    const char *suffixes[] =
        { "", ".a", ".b", ".c" };
	printf("   COM-HPC Display Port block:\n");
	printf("      Port %d: %s; Supported interfaces: ", dblock->PortNr & 0xF, 
        (dblock->PortNr >> 4) < ARRAYSIZE(port_types) ? port_types[dblock->PortNr >> 4] : "?");
    for(int i = 0; i < 4; i++) {
        if(dblock->Interfaces & (1 << i)) {
            printf("%s ", interfaces[i]);
        }
    }
    printf("\n");
    printf("      DisplayPort version: %d.%d\n", 
        dblock->DPVersion & 7, (dblock->DPVersion >> 3) & 7);
    printf("      HDMI version: %d.%d%s\n", 
        dblock->HDMIVersion & 7, (dblock->HDMIVersion >> 3) & 7,
        suffixes[(dblock->HDMIVersion >> 6) & 3]);
    printf("      Maximum size: %dx%d\n", dblock->MaxWidth, dblock->MaxHeight);
}


const char *buses_str[] = 
	{ "I2C", "SMBus", "LVDS", "DDI1", "DDI2", "DDI3", "SDVOB", "SDVOC", "CRT" };
	
void ParseExtDeviceDesc(EeePExtI2CDeviceDesc_t *dblock)
{
	printf("   Extension EEPROM Device:\n");
	printf("      Bus: %s, Device Address: %x\n", buses_str[dblock->DeviceBus], 
		(unsigned)dblock->DeviceAddr[0] | (unsigned)dblock->DeviceAddr[1] << 8);
	printf("      Size: %d bytes, Index: %s\n", 256 << (dblock->DeviceDesc & 0xF), 
		dblock->DeviceDesc & 0x10 ? "Extended" : "Standard");
}

BOOL ParseDynamic(unsigned char *buf, unsigned int size, unsigned int *offset)
{
    DBlockIdHdr_t *dblock = (DBlockIdHdr_t *)(buf + (*offset));
    int next_size;
    if (size < (*offset) + 3) {
        fprintf(stderr, "Inconsistency in the list of dynamic blocks at %d\n", *offset);
        return FALSE;
    }
    next_size = ((unsigned int)(dblock->DBlockLength[0]) | (dblock->DBlockLength[1] << 8)) << 1;
    printf("Dynamic block at offset %d, type %d (%s), length %d bytes\n", *offset, dblock->DBlockId, 
        FindDynamicBlockText(dblock->DBlockId), next_size);
    if (next_size == 0) {
        return FALSE;
    }
    if (dblock->DBlockId == EEEP_BLOCK_ID_SMBIOS) {
        //  Parse further
        EeePSmbiosHdr_t *smblock = (EeePSmbiosHdr_t *)dblock;
        printf("   SMBIOS block type: %d (%s)\n", smblock->Type, FindSmbiosBlockText(smblock->Type));
        switch (smblock->Type) {
        case SMBIOS_TypeSYSTEM_INFORMATION:
            ParseSystemInformation((EeePSystemInfo_t *)smblock);
            break;
        case SMBIOS_TypeBASE_BOARD_MODULE_INFORMATION:
            ParseBoardInformation((EeePModuleInfo_t *)smblock);
            break;
        case SMBIOS_TypeSYSTEM_ENCLOSURE_OR_CHASSIS:
            ParseChassisInformation((EeePChassisInfo_t *)smblock);
            break;
        }
    } else if(dblock->DBlockId == EEEP_BLOCK_ID_LFP) {
		//	Parse the LFP record
		ParseLFP((EeePLFPDataBlock_t *)dblock, next_size);
    } else if(dblock->DBlockId == COM0R20_BLOCK_ID_EXP_CARD_DESC) {
		//	Parse the Express Card record
		ParseExpressCard((ExpCardBlock_t *)dblock, next_size);
    } else if(dblock->DBlockId == COM0R20_BLOCK_ID_SERIO_DESC) {
		//	Parse the Serial Block record
		ParseSerialPort((SerPortCfgBlock_t *)dblock);
    } else if(dblock->DBlockId == COMHR10_BLOCK_ID_NETWORK_PORT) {
		//	Parse the Network Block record
		ParseNetworkBlock((NetworkPortBlock_t *)dblock);
    } else if(dblock->DBlockId == COMHR10_BLOCK_ID_PCI_E_BIFURCATION) {
		//	Parse the PCI-E Bifurcation Block record
		ParsePciEBifurcationBlock((PciEBifurcationBlock_t *)dblock);
    } else if(dblock->DBlockId == COMHR10_BLOCK_ID_USB_PORT) {
		//	Parse the USB Port Block record
		ParseUsbPortBlock((USBPortBlock_t *)dblock);
    } else if(dblock->DBlockId == COMHR10_BLOCK_ID_DISPLAY_PORT) {
		//	Parse the Display Block record
		ParseDisplayBlock((DisplayBlock_t *)dblock);
    } else if(dblock->DBlockId == EEEP_BLOCK_ID_EXP_EEPROM) {
		//	Parse the Ext EEPROM Device record
		ParseExtDeviceDesc((EeePExtI2CDeviceDesc_t *)dblock);
	}
    (*offset) += next_size;
    return TRUE;
}

#define CHUNK_SIZE      4096

int main(int argc, char **argv)
{
    FILE *ifp;
    unsigned char *buf = NULL;
    unsigned char local_buf[CHUNK_SIZE];
    unsigned int curr_size = 0;
    unsigned int dynamic_offset = 0;
    int next_size;
    Exp_EEP_t *exp_eep;
    EeePCmn_t *pcommon;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s infile\n", argv[0]);
        return -1;
    }
    if ((ifp = fopen(argv[1], "rb")) == NULL) {
        fprintf(stderr, "%s: cannot open %s\n", argv[0], argv[1]);
        return -1;
    }
    //  Read the file into memory
    while((next_size = fread(local_buf, 1, CHUNK_SIZE, ifp)) > 0) {
        if (curr_size) {
            buf = (unsigned char *)realloc(buf, curr_size + next_size);
        } else {
            buf = (unsigned char *)malloc(next_size);
        }
        memcpy(buf + curr_size, local_buf, next_size);
        curr_size += next_size;
    }
    //  Now parse the memory
    if (curr_size < sizeof(EeePCmn_t)) {
        fprintf(stderr, "%s: file %s is too short (%d bytes, expected at least %d bytes)\n", 
            argv[0], argv[1], curr_size, sizeof(EeePCmn_t));
        return -1;
    }
    pcommon = (EeePCmn_t *)buf;
    if (pcommon->EepId[0] != '3' && pcommon->EepId[0] != 'P') {
        fprintf(stderr, "%s: EeeP signatiure does not match: %02X %02X, expected %02X %02X\n",
            argv[0], pcommon->EepId[0], pcommon->EepId[1], '3', 'P');
        return -1;
    }
    dynamic_offset = pcommon->BlkOffset * 2;
    exp_eep = (Exp_EEP_t *)buf;
    if (memcmp(exp_eep->GenId, "EXP1", 4) == 0) {
        //  Expansion ROM
        ParseExpansion(exp_eep);
    } else if (memcmp(exp_eep->GenId, "Com0", 4) == 0) {
        //  Carrier Board
        COM0R20_CB_t *cbp = (COM0R20_CB_t *)buf;
        if (curr_size < sizeof(COM0R20_CB_t)) {
            fprintf(stderr, "%s: carrier file %s is too short (%d bytes, expected at least %d bytes)\n",
                argv[0], argv[1], curr_size, sizeof(COM0R20_CB_t));
            return -1;
        }
        ParseCarrier(cbp);
    } else if (memcmp(exp_eep->GenId, "coM0", 4) == 0) {
        //  Module board
        COM0R20_M_t *mp = (COM0R20_M_t *)buf;
        if (curr_size < sizeof(COM0R20_M_t)) {
            fprintf(stderr, "%s: module file %s is too short (%d bytes, expected at least %d bytes)\n",
                argv[0], argv[1], curr_size, sizeof(COM0R20_M_t));
            return -1;
        }
        ParseModule(mp);
    } else if (memcmp(exp_eep->GenId, "ComH", 4) == 0) {
        //  COM-HPC Carrier Board
        COMHR10_CB_t *cbp = (COMHR10_CB_t *)buf;
        if (curr_size < sizeof(COMHR10_CB_t)) {
            fprintf(stderr, "%s: carrier file %s is too short (%d bytes, expected at least %d bytes)\n",
                argv[0], argv[1], curr_size, sizeof(COMHR10_CB_t));
            return -1;
        }
        ParseComHpcCarrier(cbp);
    } else if (memcmp(exp_eep->GenId, "coMH", 4) == 0) {
        //  Module board
        COMHR10_M_t *mp = (COMHR10_M_t *)buf;
        if (curr_size < sizeof(COMHR10_M_t)) {
            fprintf(stderr, "%s: module file %s is too short (%d bytes, expected at least %d bytes)\n",
                argv[0], argv[1], curr_size, sizeof(COMHR10_M_t));
            return -1;
        }
        ParseComHpcModule(mp);
    } else {
        fprintf(stderr, "%s: Unknown GenID in the header: \"%c%c%c%c\" (%02X %02X %02X %02X)\n",
            argv[0],
            exp_eep->GenId[0], exp_eep->GenId[1], exp_eep->GenId[2], exp_eep->GenId[3],
            exp_eep->GenId[0], exp_eep->GenId[1], exp_eep->GenId[2], exp_eep->GenId[3]);
    }
    while (dynamic_offset < curr_size && ParseDynamic(buf, curr_size, &dynamic_offset))
        ;
    return 0;
}

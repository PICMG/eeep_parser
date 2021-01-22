/*
*<KHeader>
*+=========================================================================
*I Project Name: EApiDK Embedded Application Development Kit
*+=========================================================================
*I $HeadURL: https://eapidk.svn.sourceforge.net/svnroot/eapidk/trunk/include/COM0EEP.h $
*+=========================================================================
*I Copyright: Copyright (c) 2020, PICMG
*I Author: Stefan Krupop, stefan.krupop@christmann.info
*I Author: Serge Zhukov, serge.zhukov@nvent.com
*I
*I License: All rights reserved. This program and the accompanying
*I materials are licensed and made available under the
*I terms and conditions of the BSD License which
*I accompanies this distribution. The full text of the
*I license may be found at
*I http://opensource.org/licenses/bsd-license.php
*I
*I THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN
*I "AS IS" BASIS, WITHOUT WARRANTIES OR REPRESENTATIONS OF
*I ANY KIND, EITHER EXPRESS OR IMPLIED.
*I
*I Description: COMH R1.0 Specific Structures and Data
*I
*+-------------------------------------------------------------------------
*I
*I File Name : COMHEEP.h
*I File Location : include
*I Last committed : $Revision$
*I Last changed by : $Author$
*I Last changed date : $Date$
*I ID : $Id$
*I
*+=========================================================================
*</KHeader>
*/
/* Structures for COMH STDEEP */
#ifndef __COMHEEP_H__
#define __COMHEEP_H__

#define COMHR10_M_EEP_DEV_ADDR 0xA0
#define COMHR10_CB_EEP_DEV_ADDR 0xAE
/*
* Detecting COM0 R1.0 EEPROM
*
* High Level Check
* if(!memcmp(
* &COM0EEP[0xE0] ,
* "COMExpressConfig" ,
* 0x10
* )
* )
* {
* // Found COM0R10 EEPROM
* }
*
* Sample I2C Transfer
* Device Address : 0xAE(0x57)
* Index Type : Standard
* Start<0x57><W>Ack<0xE0>Ack
* Start<0x57><R>Ack<'C'>Ack<'O'>Ack<'M'>Ack<'E'>Ack<'x'>Ack<'p'>Ack
* <'r'>Ack<'e'>Ack<'s'>Ack<'s'>Ack<'C'>Ack<'o'>Ack
* <'n'>Ack<'f'>Ack<'i'>Ack<'g'>Nak
Stop
*
*/
/*
* Detecting COM0 R2.0 Carrier Board EEPROM
*
* High Level Check
* if(!memcmp(
* &COM0EEP[0x06] ,
* "Com0" ,
* 0x04
* )
* )
* {
* // Found COM0R20 Carrier Board EEPROM
* }
*
* Sample I2C Transfer
* Device Address : 0xAE(0x57)
* Index Type : Extended
* Start<0x57><W>Ack<0x00>Ack<0x06>Ack
* Start<0x57><R>Ack<'C'>Ack<'o'>Ack<'m'>Ack<'0'>Nak Stop
*
* Detecting COM0 R2.0 Module EEPROM
*
* High Level Check
* if(!memcmp(
* &COM0EEP[0x06] ,
* "coM0" ,
* 0x04
* )
* )
* {
* // Found COM0R20 Module EEPROM
* }
*
* Sample I2C Transfer
* Device Address : 0xAE(0x57)
* Index Type : Extended
* Start<0x57><W>Ack<0x00>Ack<0x06>Ack
* Start<0x57><R>Ack<'c'>Ack<'o'>Ack<'M'>Ack<'0'>Nak Stop
*
*/
/* COMH R1.0 Standard Revision */

#define COMHR10_VER 1
#define COMHR10_REVISION 0
#define COMHR10_VERSION EEEP_VER_CREATE(COMHR10_VER, COMHR10_REVISION)

/*
 *  COM-HPC Module Descriptor
 * 
 */
 
typedef struct COMHR10_M_s {
     EeePCmn_t   EeePHdr;    /* 0x00 EeeP Common Header */
     uint8_t     GenId[4];   /* 0x06 "coMH" */
     EeePUDId_t  DevId;      /* 0x0A Unique Device Id */
     uint8_t     MType;      /* 0x10 Module Type
                                Bit 0-2: Same as TYPE pins
                                Bit 3-5: Module size
                                         0 = A, 1 = B, 2 = C, 3 = D, 4 = E
                                Bit 6-7: Manageability
                                         0 = M.U - Unmanaged
                                         1 = M.B - Basic management capabilities
                                         2 = M.F - Full management capabilities */
     uint8_t     SpecRev;    /* 0x11 COMH Specification Revision */
     uint8_t     NumCpus;    /* 0x12 Number of CPUs on module and number of threads per core
                                Bit 0-3: Number of CPUs
                                Bit 4-7: Number of threads per Core */
     uint8_t     NumCores;   /* 0x13 Number of cores per CPU
                                Bit 0-7: Number of Cores per CPU */
     uint8_t     MaxMem;     /* 0x14 Maximum amount of memory supported per slot and number of 
                                     memory slots
                                Bit 0-3: Maximum amount of memory supported as 2 ^ (value) GB
                                Bit 4-7: Number of memory slots */
     uint8_t     Architecture;/* 0x15 Architecture of the processing elements on the module
                                Bit 0: x86/x64
                                Bit 1: ARM/AArch64
                                Bit 2: RISC-V
                                Bit 3: Reserved
                                Bit 4: Other, e.g. PowerPC, SPARC, MIPS, ASIP, ...
                                Bit 5: GPU
                                Bit 6: FPGA
                                Bit 7: 64 bit architecture */
     uint8_t     UsbDesc1;   /* 0x16 USB Descriptor Byte 1
                                Bit 0-3: USB 2 Port Count
                                Bit 4-5: USB 3.2 Gen 1/Gen 2 Port Count
                                Bit 6-7: Reserved */
     uint8_t     UsbDesc2;   /* 0x17 USB Descriptor Byte 2
                                Bit 0-2: USB 3.2 Gen 2x2 Port Count
                                Bit 3:   Reserved
                                Bit 4-6: USB 4 Port Count
                                Bit 7:   Reserved */
     uint8_t     EthSataDesc;/* 0x18 ETH/SATA Descriptor Byte
                                Bit 0-1: NBASE-T Port Count
                                Bit 2-5: KR/KX Port Count
                                Bit 6-7: SATA/SAS Port Count */
     uint8_t     PciELanes;  /* 0x19 PCIe Lanes Byte
                                Bit 0-6: PCIe Lane Count
                                Bit 7:   Reserved */
     uint8_t     PciEDesc;   /* 0x1A PCIe Descriptor Byte
                                Bit 0-1: PCIe Target Port Count
                                Bit 2-7: Reserved */
     uint8_t     MgmtDesc;   /* 0x1B Management Descriptor Byte
                                Bit 0:   eSPI implemented
                                Bit 1:   Boot SPI implemented
                                Bit 2:   IPMB implemented
                                Bit 3:   System management bus implemented
                                Bit 4:   BIOS Select implemented
                                Bit 5:   PCIe BMC interface implemented
                                Bit 6-7: Reserved */
     uint8_t     VideoDesc;  /* 0x1C Video Descriptor Byte
                                Bit 0-1: DDI Port Count
                                Bit 2:   eDP/DSI implemented
                                Bit 3-4: CSI Port Count
                                Bit 5:   Soundwire Audio implemented
                                Bit 6:   I2S Audio implemented
                                Bit 7:   Reserved */
     uint8_t     GPBusDesc;  /* 0x1D General Purpose Buses Descriptor Byte
                                Bit 0-1: Serial Port Count
                                Bit 2-3: I2C Port Count
                                Bit 4  : General Purpose SPI implemented
                                Bit 5-7: Reserved */
} COMHR10_M_t;
 
 /*
 *  COM-HPC Cariier Descriptor
 * 
 */
 
typedef struct COMHR10_CB_s{
    EeePCmn_t   EeePHdr;       /* 0x00 EeeP Common Header */
    uint8_t     GenId[4];      /* 0x06 "ComH" */
    EeePUDId_t  DevId;         /* 0x0C Unique Device Id */
    uint8_t     CBType;        /* 0x10 Carrier Board Type
                                  Bit 0-1: Reserved
                                  Bit 2:   Multi-Module carrier
                                  Bit 3-5: Maximum module size
                                           0 = A, 1 = B, 2 = C, 3 = D, 4 = E
                                  Bit 6-7: Manageability
                                           0 = C.U - Unmanaged
                                           1 = C.M - Managed via BMC */
    uint8_t     SpecRev;       /* 0x11 COMH Specification Revision */
    uint8_t     UsbDesc1;      /* 0x12 USB 2.0 Ports in use
                                  Bit 0-7: 1 = Port in use */
    uint8_t     UsbDesc2;      /* 0x13 USB 3/4 Ports in use
                                  Bit 0-3: 1 = Port in use
                                  Bit 4-7: Reserved */
    uint8_t     SataBaseTDesc; /* 0x14 SATA and NBASE-T Descriptor Byte
                                  Bit 0-1: SATA port in use
                                  Bit 2-3: Reserved
                                  Bit 4-5: NBASE-T port in use
                                  Bit 6-7: Reserved */
    uint8_t     KrKxDesc;      /* 0x15 Ethernet KR/KX Descriptor Byte
                                  Bit 0-7: Ethernet port in use */
    uint8_t     DDIDesc;        /* 0x16 DDI Descriptor
                                  Bit 0-1: DDI 0 implementation
                                           0 = Not implemented
                                           1 = Display Port implemented
                                           2 = HDMI/DVI implemented
                                  Bit 2-3: DDI 1 implementation
                                  Bit 4-5: DDI 2 implementation
                                  Bit 6:   eDP implemented
                                  Bit 7:   MIPI DSI implemented */
    uint8_t     CSIAudioDesc;  /* 0x17 CSI and Audio Descriptor
                                  Bit 0-1: CSI 0 implementation
                                           0 = Not implemented
                                           1 = CSI-2 implemented
                                           2 = CSI-3 implemented
                                  Bit 2-3: CSI 1 implementation
                                  Bit 4:   Soundwire implemented
                                  Bit 5-6: I2S
                                           0 = Not implemented
                                           1 = I2S implemented
                                           2 = Soundwire 2 implemented
                                  Bit 7:   Reserved */
    uint8_t     UartI2CDesc;   /* 0x18 UART Descriptor
                                  Bit 0-1: UART 0 implementation
                                           0 = Not implemented
                                           1 = RS232 implemented
                                           2 = RS232 with RTS/CTS implemented
                                           3 = RS485 implemented
                                  Bit 2-3: UART 1 implementation
                                  Bit 4-5: I2C 0 implementation
                                           0 = Not implemented
                                           1 = Limit to 100 kHz
                                           2 = Limit to 400 kHz
                                           3 = No speed limit
                                  Bit 6-7: I2C 1 implementation */
    uint8_t     MgmtDesc;      /* 0x19 Management Descriptor
                                  Bit 0:   eSPI implemented
                                  Bit 1:   Boot SPI implemented
                                  Bit 2:   IPMB implemented
                                  Bit 3:   System management bus implemented
                                  Bit 4:   BIOS Select implemented
                                  Bit 5:   PCIe BMC interface implemented
                                  Bit 6-7: Reserved */
    uint8_t     SpiDesc;       /* 0x1A General purpose SPI Descriptor
                                  Bit 0-3: SPI chip select implemented
                                  Bit 4-7: Reserved */
    uint8_t     MiscIo1;       /* 0x1B Miscellaneous I/O Descriptor
                                  Bit 0:   Wake 0 implemented
                                  Bit 1:   Wake 1 implemented
                                  Bit 2:   Suspend implemented
                                  Bit 3:   Battery low implemented
                                  Bit 4:   Thermal protection implemented
                                  Bit 5:   Fan implemented
                                  Bit 6:   Watchdog implemented
                                  Bit 7:   Reserved */
    uint8_t     MiscIo2;       /* 0x1C Miscellaneous I/O Descriptor
                                  Bit 0:   Rapid shutdown implemented
                                  Bit 1:   Tamper detection implemented
                                  Bit 2:   Lid switch implemented
                                  Bit 3:   Sleep button implemented
                                  Bit 4-7: Reserved */
    uint8_t     PCIeGen[16];   /* 0x1D PCI Express Lane Generation, 2 Bits per Lane
                                  Bit 0-1: Lane 0 generation
                                           0 = Generation 1/2
                                           1 = Generation 3
                                           2 = Generation 4
                                           3 = Generation 5
                                  Bit 2-3: Lane 1 generation
                                  [...] */
    uint8_t     LaneMap[32];   /* 0x2D PCI Express Lane Information, 4 Bits per Lane
                                  Bit 0-2: Lane 0 link width
                                           0 = Not implemented
                                           1 = x1
                                           2 = x2
                                           3 = x4
                                           4 = x8
                                           5 = x16
                                           6 = x32
                                           7 = x64
                                  Bit 3:   Reserved
                                  Bit 4-6: Lane 1 link width
                                  Bit 7:   Reserved */
    uint8_t     PCIeTarget[2]; /* 0x4D PCI Express Target Descriptor
                                  Bit 0-5: PCIe Target starting lane
                                  Bit 6:   Reserved
                                  Bit 7:   PCIe Target implemented */
} COMHR10_CB_t;

#define COMHR10_BLOCK_ID_NETWORK_PORT EEEP_UINT8_C(0xE2)

/*
* COM-HPC Network Port Block (To be used on modules)
*
*/
typedef struct NetworkPortBlock_s{
    DBlockIdHdr_t DBHdr;          /* 0x00 Dynamic Block Header */
    uint8_t PortNr;               /* 0x03 Number and category of the network port this block describes
                                     Bit 0-3: Port number, starting at 0
                                     Bit 4-7: Category
                                              0 = NBASE-T
                                              1 = KR/KX */
    uint8_t SupportedSpeeds;      /* 0x04 Supported speeds
                                     Bit 0:   10 MBit
                                     Bit 1:   100 MBit
                                     Bit 2:   1 GBit
                                     Bit 3:   2.5 GBit
                                     Bit 4:   5 GBit
                                     Bit 5:   10 GBit
                                     Bit 6:   25 GBit
                                     Bit 7:   40 GBit */
    uint8_t Options;              /* 0x05 Options
                                     Bit 0:   Half-/Full-Duplex (1 = Full-Duplex) supported
                                     Bit 1:   Autonegotiation supported
                                     Bit 2:   SyncE supported
                                     Bit 3:   Backplane ethernet suppported
                                     Bit 4:   PHY module suppported
                                     Bit 5-7: Reserved */
    uint8_t MfgMacAddress[6];     /* 0x06 MAC address of this interface assigned by the manufacturer */
} NetworkPortBlock_t;

#define COMHR10_BLOCK_ID_PCI_E_BIFURCATION EEEP_UINT8_C(0xE3)

/*
* COM-HPC PCI-E Bifurcation Block (To be used on modules)
*
*/

typedef struct PciELane_s{
    uint8_t LaneNumber;           /* 0x00 Lane number */
    uint8_t LaneCapabilities;     /* 0x01 Lane capabilities
                                     Bit 0-3: PCIe generation supported
                                     Bit 4:   Reserved
                                     Bit 5:   NVMe supported on this lane
                                     Bit 6:   PEG supported on this lane
                                     Bit 7:   PCIe target supported on this lane */
    uint8_t NrOfConfigurations;   /* 0x02 Number of configurations to follow */
    uint8_t Configurations[1];    /* 0x03 Configurations
                                     Bit 0-5: Last lane number in this configuration
                                     Bit 6:   Reserved
                                     Bit 7:   Lane reversal supported in this configuration */
} PciELane_t;

typedef struct PciEBifurcationBlock_s {
    DBlockIdHdr_t DBHdr;          /* Dynamic Block Header */
    uint8_t NrOfLaneDescriptors;  /* 0x03 Number of lane descriptors to follow */
    PciELane_t LaneDescriptors[1];/* 0x04 Lanes */
} PciEBifurcationBlock_t;

#define COMHR10_BLOCK_ID_USB_PORT EEEP_UINT8_C(0xE4)

/*
* COM-HPC USB Port Block (To be used on modules)
*
*/

typedef struct UsbPortBlock_s{
    DBlockIdHdr_t DBHdr;          /* 0x00 Dynamic Block Header */
    uint8_t PortNr;               /* 0x03 Number of the port this block describes */
    uint8_t UsbGeneration;        /* 0x04 USB Generation supported
                                     Bit 0-2: Generation.
                                              0 = USB 2.0
                                              1 = USB 3.2 Gen 1
                                              2 = USB 3.2 Gen 2
                                              3 = USB 3.2 Gen 2x2
                                              4 = USB 4.0 Gen 2x2
                                              5 = USB 4.0 Gen 3x2
                                     Bit 3:   Reserved
                                     Bit 4-6: USB 2.0 channel this port is paired with
                                     Bit 7:   Reserved */
    uint8_t Flags;                /* 0x05 Features supported
                                     Bit 0:   DisplayPort supported
                                     Bit 1:   PCIe supported
                                     Bit 2:   USB C connector supported
                                     Bit 3:   USB C lane muxing supported
                                     Bit 4:   Port 80 support on USB_PD
                                     Bit 5:   USB Client mode supported (Only for USB0)
                                     Bit 6-7: Reserved */
} USBPortBlock_t;

#define COMHR10_BLOCK_ID_DISPLAY_PORT EEEP_UINT8_C(0xE5)

/*
* COM-HPC Display Port Block (To be used on modules)
*
*/

typedef struct DisplayBlock_s{
    DBlockIdHdr_t DBHdr;          /* 0x00 Dynamic Block Header */
    uint8_t PortNr;               /* 0x03 Number and category of the display port this block describes
                                     Bit 0-3: Port number, starting at 0
                                     Bit 4-7: Category
                                              0 = DDI
                                              1 = eDP/DSI */
    uint8_t Interfaces;           /* 0x04 Supported interfaces
                                     Bit 0:   DisplayPort supported
                                     Bit 1:   HDMI/DVI supported
                                     Bit 2:   DSI supported
                                     Bit 3:   eDP supported
                                     Bit 4-7: Reserved */
    uint8_t DPVersion;            /* 0x05 DisplayPort version supported
                                     Bit 0-2: Major version
                                     Bit 3-5: Minor version
                                     Bit 6-7: Suffix
                                              0 = no suffix */
    uint8_t HDMIVersion;          /* 0x06 HDMI version supported
                                     Bit 0-2: Major version
                                     Bit 3-5: Minor version
                                     Bit 6-7: Suffix
                                              0 = no suffix
                                              1 = a
                                              2 = b
                                              3 = c  */
    uint16_t MaxWidth;         /* 0x07 Maximum display width in pixels */
    uint16_t MaxHeight;        /* 0x09 Maximum display height in pixels */
} DisplayBlock_t;

#endif /* __COMHEEP_H__ */



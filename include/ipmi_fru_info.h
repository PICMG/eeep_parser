/*
*<KHeader>
*+=========================================================================
*I Project Name: EApiDK Embedded Application Development Kit
*+=========================================================================
*I $HeadURL: https://eapidk.svn.sourceforge.net/svnroot/eapidk/trunk/include/EeeP.h $
*+=========================================================================
*I Copyright: Copyright (c) 2009, PICMG
*I Author: John Kearney, John.Kearney@kontron.com
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
*I Description: Embedded EEPROM
*I
*+-------------------------------------------------------------------------
*I
*I File Name : EeeP.h
*I File Location : include
*I Last committed : $Revision: 51 $
*I Last changed by : $Author: dethrophes $
*I Last changed date : $Date: 2010-02-01 08:48:20 +0100 (Mo, 01 Feb 2010) $
*I ID : $Id: EeeP.h 51 2010-02-01 07:48:20Z dethrophes $
*I
*+=========================================================================
*</KHeader>
*/
/* Structures for IPMI FRU Information */

#ifndef __IPMI_FRU_INFO_H__
#define __IPMI_FRU_INFO_H__

#pragma pack(push, 1) /* push current alignment to stack */
#pragma pack(1) /* set alignment to 1 byte boundary */

//  Common Header

typedef struct {
    uint8_t format_version;
    uint8_t internal_use_area_offset;
    uint8_t chassis_info_area_offset;
    uint8_t board_info_area_offset;
    uint8_t product_info_area_offset;
    uint8_t multirecord_area_offset;
    uint8_t pad;
    uint8_t checksum;
} ipmi_fru_common_header_t;

//  Chassis info area 

typedef struct {
    uint8_t format_version;
    uint8_t area_length;
    uint8_t chassis_type;
    uint8_t variable_part[1];
} ipmi_fru_chassis_info_area_t;

//  Chassis type

typedef enum {
    IpmiFruChassisTypeMin = 1,
    IpmiFruChassisTypeOther = IpmiFruChassisTypeMin,
    IpmiFruChassisTypeUnknown,
    IpmiFruChassisTypeDesktop,
    IpmiFruChassisTypeLowProfileDesktop,
    IpmiFruChassisTypePizzaBox,
    IpmiFruChassisTypeMiniTower,
    IpmiFruChassisTypeTower,
    IpmiFruChassisTypePortable,
    IpmiFruChassisTypeLaptop,
    IpmiFruChassisTypeNotebook,
    IpmiFruChassisTypeHandheld,
    IpmiFruChassisTypeDockingStation,
    IpmiFruChassisTypeAllInOne,
    IpmiFruChassisTypeSubNotebook,
    IpmiFruChassisTypeSpaceSaving,
    IpmiFruChassisTypeLunchBox,
    IpmiFruChassisTypeMainServerChassis,
    IpmiFruChassisTypeExpansionChassis,
    IpmiFruChassisTypeSubChassis,
    IpmiFruChassisTypeBusExpansionChassis,
    IpmiFruChassisTypePeripheralChassis,
    IpmiFruChassisTypeRAIDChassis,
    IpmiFruChassisTypeRackMountChassis,
    IpmiFruChassisTypeMax = IpmiFruChassisTypeRackMountChassis
} ipmi_fru_chassis_type_t;
    
//  Board info area 

typedef struct {
    uint8_t format_version;
    uint8_t area_length;
    uint8_t language_code;
    uint8_t mfg_date_time[3];
    uint8_t variable_part[1];
} ipmi_fru_board_info_area_t;

//  Product info area 

typedef struct {
    uint8_t format_version;
    uint8_t area_length;
    uint8_t language_code;
    uint8_t variable_part[1];
} ipmi_fru_product_info_area_t;

//  Format version

#define IPMI_FRUINFO_FORMAT_VERSION                     1

#define IPMI_FRUINFO_MAX_AREA_SIZE                      (256*8)
#define IPMI_FRUINFO_MAX_MULTIRECORD_SIZE               (255+sizeof(ipmi_fru_multirecord_record_header_t))


//  Language code

#define IPMI_FRUINFO_LANGUAGE_CODE_ENGLISH              25

//  Multirecord-related definitions

#define IPMI_FRUINFO_MULTIRECORD_OEM_TYPE_ID            0xC0
#define IPMI_FRUINFO_MULTIRECORD_FORMAT_VERSION         2
#define IPMI_FRUINFO_LAST_MULTIRECORD                   0x80


//  PICMG record IDs for carrier header/module header/dynamic record

#define PICMG_RECORD_ID_EEEP_CARRIER_HEADER             0x70
#define PICMG_RECORD_ID_EEEP_MODULE_HEADER              0x71
#define PICMG_RECORD_ID_EEEP_DYNAMIC_BLOCK              0x72

//  Common header for a record in multi-record area

typedef struct {
    uint8_t type_id;
    uint8_t format_version;
    uint8_t length;
    uint8_t record_checksum;
    uint8_t header_checksum;
} ipmi_fru_multirecord_record_header_t;


//  PICMG-specific header

typedef struct {
    ipmi_fru_multirecord_record_header_t common_header;
    uint8_t manufacturer_id[3];
    uint8_t picmg_record_id;
    uint8_t picmg_format_version;
} ipmi_fru_picmg_record_header_t;

//  Dynamic EEEP record header

typedef struct {
    ipmi_fru_picmg_record_header_t header;
    uint8_t instance_id[2];
} ipmi_fru_picmg_dynamic_eeep_record_header_t;

unsigned int ipmi_fru_finalize_area(uint8_t *area, unsigned int size);

#pragma pack(pop) /* restore original alignment from stack */
#endif /* __IPMI_FRU_INFO_H__ */


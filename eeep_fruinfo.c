#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "EeeP.h"
#include "ipmi_fru_info.h"

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

//  Static data structures for IPMI FRU Information

#pragma pack(push, 1) /* push current alignment to stack */
#pragma pack(1) /* set alignment to 1 byte boundary */

static ipmi_fru_common_header_t common_header;

static struct {
    ipmi_fru_chassis_info_area_t area;
    uint8_t pad[IPMI_FRUINFO_MAX_AREA_SIZE - sizeof(ipmi_fru_chassis_info_area_t)];
} chassis_info_area;

static struct {
    ipmi_fru_board_info_area_t area;
    uint8_t pad[IPMI_FRUINFO_MAX_AREA_SIZE - sizeof(ipmi_fru_board_info_area_t)];
} board_info_area;

static struct {
    ipmi_fru_product_info_area_t area;
    uint8_t pad[IPMI_FRUINFO_MAX_AREA_SIZE - sizeof(ipmi_fru_product_info_area_t)];
} product_info_area;
    
static struct {
    ipmi_fru_picmg_record_header_t header;
    uint8_t data[IPMI_FRUINFO_MAX_MULTIRECORD_SIZE - sizeof(ipmi_fru_picmg_record_header_t)];
} multirecord;

#pragma pack(pop)

static unsigned int chassis_info_area_size = 0;
static unsigned int board_info_area_size = 0;
static unsigned int product_info_area_size = 0;

static BOOL verbose = FALSE;

//  Finalize chassis, board, product area

unsigned int IpmiFruFinalizeArea(uint8_t *area, unsigned int size)
{
    area[size++] = 0xC1;
    //  Pad to 8 bytes
    do {
        area[size++] = 0;
    } while(size % 8);
    area[0] = IPMI_FRUINFO_FORMAT_VERSION;
    area[1] = size / 8;
    //  Calculate checksum
    uint8_t checksum = 0;
    for (unsigned int index = 0; index < size; index++) {
        checksum += area[index];
    }
    area[size-1] = -checksum;
    return size;
}

//  Write common header and areas to the file: use static areas and sizes

size_t IpmiFruWriteCommonAreas(FILE *ofd)
{
    //  First, populate the common header
    memset(&common_header, 0, sizeof(common_header));
    common_header.format_version = IPMI_FRUINFO_FORMAT_VERSION;
    uint8_t running_offset = 1;
    if (chassis_info_area_size) {
        common_header.chassis_info_area_offset = running_offset;
        running_offset += chassis_info_area_size / 8;
    }
    if (board_info_area_size) {
        common_header.board_info_area_offset = running_offset;
        running_offset += board_info_area_size / 8;
    }
    if (product_info_area_size) {
        common_header.product_info_area_offset = running_offset;
        running_offset += product_info_area_size / 8;
    }
    common_header.multirecord_area_offset = running_offset;
    uint8_t checksum = 
        common_header.format_version +
        common_header.chassis_info_area_offset +
        common_header.board_info_area_offset +
        common_header.product_info_area_offset +
        common_header.multirecord_area_offset;
    common_header.checksum = -checksum;
    //  Write the common header
    if (fwrite(&common_header, 1, sizeof(common_header), ofd) <= 0) {
        fprintf(stderr, "Failed to write common header to the output file\n");
        return -1;
    }
    //  Write chassis info area
    if (chassis_info_area_size) {
        if (fwrite(&chassis_info_area, 1, chassis_info_area_size, ofd) <= 0) {
            fprintf(stderr, "Failed to write chassis info to the output file\n");
            return -1;
        }
    }
    //  Write board info area
    if (board_info_area_size) {
        if (fwrite(&board_info_area, 1, board_info_area_size, ofd) <= 0) {
            fprintf(stderr, "Failed to write board info to the output file\n");
            return -1;
        }
    }
    //  Write product info area
    if (product_info_area_size) {
        if (fwrite(&product_info_area, 1, product_info_area_size, ofd) <= 0) {
            fprintf(stderr, "Failed to write product info to the output file\n");
            return -1;
        }
    }
    return (size_t)running_offset * 8;
} 

//  Fill in the PICMG header of a multirecord

void IpmiFillPICMGHeader(uint8_t picmg_record_id, size_t size)
{
    multirecord.header.common_header.type_id = IPMI_FRUINFO_MULTIRECORD_OEM_TYPE_ID;
    multirecord.header.common_header.format_version = IPMI_FRUINFO_MULTIRECORD_FORMAT_VERSION;
    multirecord.header.common_header.length = size + 
        sizeof(ipmi_fru_picmg_record_header_t) - 
        sizeof(ipmi_fru_multirecord_record_header_t);
    multirecord.header.common_header.header_checksum = 
        multirecord.header.common_header.record_checksum = 0;
    multirecord.header.manufacturer_id[0] = 0x5A;
    multirecord.header.manufacturer_id[1] = 0x31;
    multirecord.header.manufacturer_id[2] = 0;
    multirecord.header.picmg_record_id = picmg_record_id;
    multirecord.header.picmg_format_version = 0;
}

//  Encode a carrier record as a multirecord: use static multirecord buffer

size_t IpmiFruEncodeCarrier(void *buffer, size_t size)
{
    memcpy(&multirecord.data, buffer, size);
    IpmiFillPICMGHeader(PICMG_RECORD_ID_EEEP_CARRIER_HEADER, size);
    return size + sizeof(ipmi_fru_picmg_record_header_t);
}

//  Encode a module record as a multirecord: use static multirecord buffer

size_t IpmiFruEncodeModule(void *buffer, size_t size)
{
    memcpy(&multirecord.data, buffer, size);
    IpmiFillPICMGHeader(PICMG_RECORD_ID_EEEP_MODULE_HEADER, size);
    return size + sizeof(ipmi_fru_picmg_record_header_t);
}

//  Encode a dynamic block as a multirecord: use static multirecord buffer
//  Only part of a dynamic block may be encoded

BOOL IpmiFruEncodeDynamic(void *buffer, unsigned int offset, size_t size, uint16_t instance_id, int *piece_offset)
{
    const int MAX_PIECE_SIZE = IPMI_FRUINFO_MAX_MULTIRECORD_SIZE - 
                            sizeof(ipmi_fru_picmg_dynamic_eeep_record_header_t);
    size_t piece_size = size - (*piece_offset);
    BOOL last_piece = TRUE;
    if (piece_size > MAX_PIECE_SIZE) {
        piece_size = MAX_PIECE_SIZE;
        last_piece = FALSE;
    }
    multirecord.data[0] = instance_id & 0xFF;
    multirecord.data[1] = (instance_id >> 8) & 0xFF;
    memcpy(multirecord.data + 2, (uint8_t *)buffer + offset + (*piece_offset), piece_size);
    IpmiFillPICMGHeader(PICMG_RECORD_ID_EEEP_DYNAMIC_BLOCK, piece_size + 2);
    (*piece_offset) += piece_size;
    return last_piece;
}

//  Write a prepared multirecord to the file: use static multirecord buffer

size_t IpmiFruWritePreparedMultirecord(FILE *ofd, BOOL last)
{
    //  Finsihing touches
    if (last) {
        multirecord.header.common_header.format_version |= IPMI_FRUINFO_LAST_MULTIRECORD;
    }
    //  Calculate data and header checksums
    uint8_t *start = (uint8_t *)(&multirecord.header.common_header + 1);
    uint8_t checksum = 0;
    for (int index = 0; index < multirecord.header.common_header.length; index++) {
        checksum += start[index];
    }
    multirecord.header.common_header.record_checksum = -checksum;
    checksum = 
        multirecord.header.common_header.type_id +
        multirecord.header.common_header.format_version +
        multirecord.header.common_header.length +
        multirecord.header.common_header.record_checksum;
    multirecord.header.common_header.header_checksum = -checksum;
    //  Now write the record to the file
    size_t res = fwrite(&multirecord, 1,
        multirecord.header.common_header.length + sizeof(ipmi_fru_multirecord_record_header_t), ofd);
    if (res <= 0) {
        fprintf(stderr, "Failed to write a multirecord to the output file\n");
    }
    return res;
}

//  Parse a dynamic record, return next offset (or -1 on failure)

size_t ParseDynamic(void *buf, unsigned int offset, size_t size, uint8_t *block_id)
{
    DBlockIdHdr_t *dblock = (DBlockIdHdr_t *)((unsigned char *)buf + offset);
    unsigned int next_size;
    if (size < offset + 3) {
        fprintf(stderr, "Inconsistency in the list of dynamic blocks at %d\n", offset);
        return -1;
    }
    next_size = ((unsigned int)(dblock->DBlockLength[0]) | (dblock->DBlockLength[1] << 8)) << 1;
    if (next_size == 0) {
        if (dblock->DBlockId == EEEP_BLOCK_ID_UNUSED) {
            //  End of data
            next_size = size - offset;
        } else {
            //  Error
            fprintf(stderr, "Dynamic block at %d has size 0!\n", offset);
            return -1;
        }
    }
    *block_id = dblock->DBlockId;
    return next_size;
}

//  Write multirecords to the file: use static multirecord buffer

size_t IpmiFruWriteMultirecords(FILE *ofd, Exp_EEP_t *exp_eep, unsigned int dynamic_offset, size_t size)
{
    size_t acc_size = 0;
    int res;
    BOOL multirecord_present = FALSE;
    
    if (memcmp(exp_eep->GenId, "Com0", 4) == 0) {
        //  Encode Com0 carrier record
        if (verbose) printf("Encoding COM-Express Carrier description\n");
        IpmiFruEncodeCarrier(exp_eep, sizeof(COM0R20_CB_t));
        multirecord_present = TRUE;
    } else if (memcmp(exp_eep->GenId, "ComH", 4) == 0) {
        //  Encode ComH carrier record
        if (verbose) printf("Encoding COM-HPC Carrier description\n");
        IpmiFruEncodeCarrier(exp_eep, sizeof(COMHR10_CB_t));
        multirecord_present = TRUE;
    } else if (memcmp(exp_eep->GenId, "coM0", 4) == 0) {
        //  Encode Com0 module record
        if (verbose) printf("Encoding COM-Express Module description\n");
        IpmiFruEncodeModule(exp_eep, sizeof(COM0R20_M_t));
        multirecord_present = TRUE;
    } else if (memcmp(exp_eep->GenId, "coMH", 4) == 0) {
        //  Encode ComH module record
        if (verbose) printf("Encoding COM-HPC Module description\n");
        IpmiFruEncodeModule(exp_eep, sizeof(COMHR10_M_t));
        multirecord_present = TRUE;
    }
    //  Now encode dynamic records
    uint16_t instance_id = 0;
    while (dynamic_offset < size) {
        uint8_t block_id;
        int next_size = ParseDynamic(exp_eep, dynamic_offset, size, &block_id);
        if (next_size <= 0) {
            fprintf(stderr, "Failed to parse a dynamic record at offset %d\n", dynamic_offset);
            return -1;
        }
        if(block_id != EEEP_BLOCK_ID_UNUSED && block_id != EEEP_BLOCK_ID_CRC_CHK) {
            int piece_offset = 0;
            BOOL last_piece;
            do {
                if (multirecord_present) {
                    if ((res = IpmiFruWritePreparedMultirecord(ofd, FALSE)) <= 0) {                
                        return -1;
                    }
                    acc_size += res;
                }
                last_piece = IpmiFruEncodeDynamic(exp_eep, dynamic_offset, next_size, 
                    instance_id, &piece_offset); 
                if (verbose) printf("Encoding dynamic block %x, size=%d, offset=%d, instance=%d, last piece=%d\n", 
                        block_id, next_size, dynamic_offset, instance_id, last_piece);
                multirecord_present = TRUE;
            } while (!last_piece);
            instance_id ++;
        }
        dynamic_offset += next_size;
    }
    //  Write last record 
    if (multirecord_present) {
        if ((res = IpmiFruWritePreparedMultirecord(ofd, TRUE)) <= 0) {                
            return -1;
        }
        acc_size += res;
    }
    return acc_size;
}

struct {
    ChassisTypes_t eeep_chassis_type;
    ipmi_fru_chassis_type_t fru_chassis_type;
} chassis_type_convs[] = {
    { SMBIOS_ChassisType_Unknown, IpmiFruChassisTypeUnknown },
    { SMBIOS_ChassisType_Other, IpmiFruChassisTypeOther },
    { SMBIOS_ChassisType_Desktop, IpmiFruChassisTypeDesktop },
    { SMBIOS_ChassisType_Low_Profile_Desktop, IpmiFruChassisTypeLowProfileDesktop },
    { SMBIOS_ChassisType_Pizza_Box, IpmiFruChassisTypePizzaBox },
    { SMBIOS_ChassisType_Mini_Tower, IpmiFruChassisTypeMiniTower },
    { SMBIOS_ChassisType_Tower, IpmiFruChassisTypeTower },
    { SMBIOS_ChassisType_Portable, IpmiFruChassisTypePortable },
    { SMBIOS_ChassisType_Laptop, IpmiFruChassisTypeLaptop },
    { SMBIOS_ChassisType_Notebook, IpmiFruChassisTypeNotebook },
    { SMBIOS_ChassisType_Hand_Held, IpmiFruChassisTypeHandheld },
    { SMBIOS_ChassisType_Docking_Station, IpmiFruChassisTypeDockingStation },
    { SMBIOS_ChassisType_All_In_One, IpmiFruChassisTypeAllInOne },
    { SMBIOS_ChassisType_Sub_Notebook, IpmiFruChassisTypeSubNotebook },
    { SMBIOS_ChassisType_Space_Saving, IpmiFruChassisTypeSpaceSaving },
    { SMBIOS_ChassisType_Lunch_Box, IpmiFruChassisTypeLunchBox },
    { SMBIOS_ChassisType_Main_Server_Chassis, IpmiFruChassisTypeMainServerChassis },
    { SMBIOS_ChassisType_Expansion_Chassis, IpmiFruChassisTypeExpansionChassis },
    { SMBIOS_ChassisType_Sub_Chassis, IpmiFruChassisTypeSubChassis },
    { SMBIOS_ChassisType_Bus_Expansion_Chassis, IpmiFruChassisTypeBusExpansionChassis },
    { SMBIOS_ChassisType_Peripheral_Chassis, IpmiFruChassisTypePeripheralChassis },
    { SMBIOS_ChassisType_RAID_Chassis, IpmiFruChassisTypeRAIDChassis },
    { SMBIOS_ChassisType_Rack_Mount_Chassis, IpmiFruChassisTypeRackMountChassis },
    { SMBIOS_ChassisType_Sealed_case_PC, IpmiFruChassisTypeOther },
    { SMBIOS_ChassisType_Multi_system_chassis, IpmiFruChassisTypeOther }
};

const char *GetSmbiosString(uint8_t index, void *base)
{
    const char *strbase = (const char *)base;
    if (index == 0)
        return NULL;
    for (--index; index; --index) {
        strbase += strlen(strbase) + 1;
    }
    return strbase;
}

//  Add a string to the variable part of an area

uint8_t *IpmiFruAddString(uint8_t *pos, const char *str)
{
    int len = strlen(str);
    if (len == 1) {
        len ++;
    } else if (len > 63) {
        len = 63;
    }
    *pos++ = 0xC0 + len;
    if (len) {
        memcpy(pos, str, len);
        pos += len;
    }
    return pos;
}

//  Add a custom string to an area in the format "key=value"

uint8_t *IpmiFruAddCustomString(uint8_t *pos, const char *key, const char *value)
{
    int len1 = strlen(key);
    int len2 = strlen(value);
    if ((len1 == 0 && len2 == 0) || len1 + 1 > 63) {
        return pos;
    }
    if (len1 + 1 + len2 > 63) {
        len2 = 63 - len1 - 1;
    }
    *pos++ = 0xC0 + len1 + 1 + len2;
    memcpy(pos, key, len1);
    pos += len1;
    *pos++ = (uint8_t)'=';
    memcpy(pos, value, len2);
    pos += len2;
    return pos;
}

//  Convert chassis type from EEEP to IPMI FRU

ipmi_fru_chassis_type_t ConvertChassisType(ChassisTypes_t type) 
{ 
    for (int idx = 0; idx < ARRAYSIZE(chassis_type_convs); idx++ ) {
        if (chassis_type_convs[idx].eeep_chassis_type == type) {
            return chassis_type_convs[idx].fru_chassis_type;
        }
    }
    return IpmiFruChassisTypeOther; 
}

//  Build chassis info area

void IpmiFruBuildChassisInfoArea(EeePChassisInfo_t *info)
{
    chassis_info_area.area.format_version = IPMI_FRUINFO_FORMAT_VERSION;
    chassis_info_area.area.chassis_type = ConvertChassisType(info->Type);
    uint8_t *varp = chassis_info_area.area.variable_part;

    void *strstart = (unsigned char *)info->CElement + info->CElementCnt * info->CElementSize;
    const char *str;
   //  Part number - not specified
    *varp++ = 0xC0;
    //  Chassis serial number
    if (str = GetSmbiosString(info->SerialNumber, strstart)) {
        varp = IpmiFruAddString(varp, str);
    } else {
        *varp++ = 0xC0;
    }
    //  Manufacturer as custom string
    if (str = GetSmbiosString(info->Manufacturer, strstart)) {
        varp = IpmiFruAddCustomString(varp, "MANUFACTURER", str);
    }
    //  Version as custom string
     if (str = GetSmbiosString(info->Version, strstart)) {
        varp = IpmiFruAddCustomString(varp, "VERSION", str);
    }
    //  Asset tag as custom string
    if (str = GetSmbiosString(info->AssetTagNumber, strstart)) {
        varp = IpmiFruAddCustomString(varp, "ASSET_TAG", str);
    }
    uint8_t *chassis_area_ptr = (uint8_t *)&chassis_info_area;
    chassis_info_area_size = IpmiFruFinalizeArea(chassis_area_ptr, varp-chassis_area_ptr);
}

//  Build board info area

void IpmiFruBuildBoardInfoArea(EeePModuleInfo_t *info)
{
    board_info_area.area.format_version = IPMI_FRUINFO_FORMAT_VERSION;
    board_info_area.area.language_code = IPMI_FRUINFO_LANGUAGE_CODE_ENGLISH;
    board_info_area.area.mfg_date_time[0] = 0;
    board_info_area.area.mfg_date_time[1] = 0;
    board_info_area.area.mfg_date_time[2] = 0;
    uint8_t *varp = board_info_area.area.variable_part;

    void *strstart = info->Handles + info->ContainedHndls;
    const char *str;
    //  Manufacturer
    if (str = GetSmbiosString(info->Manufacturer, strstart)) {
        varp = IpmiFruAddString(varp, str);
    } else {
        *varp++ = 0xC0;
    }
    //  Product name
    if (str = GetSmbiosString(info->Product, strstart)) {
        varp = IpmiFruAddString(varp, str);
    } else {
        *varp++ = 0xC0;
    }
    //  Serial number
    if (str = GetSmbiosString(info->SerialNumber, strstart)) {
        varp = IpmiFruAddString(varp, str);
    } else {
        *varp++ = 0xC0;
    }
    //  Part number - not specified
    *varp++ = 0xC0;
    //  FRU File ID - not specified
    *varp++ = 0xC0;
    //  Version as custom string
    if (str = GetSmbiosString(info->Version, strstart)) {
        varp = IpmiFruAddCustomString(varp, "VERSION", str);
    }
    //  Location as custom string
    if (str = GetSmbiosString(info->Location, strstart)) {
        varp = IpmiFruAddCustomString(varp, "LOCATION", str);
    }
    //  Asset tag as custom string
    if (str = GetSmbiosString(info->AssetTag, strstart)) {
        varp = IpmiFruAddCustomString(varp, "ASSET_TAG", str);
    }
    uint8_t *board_area_ptr = (uint8_t *)&board_info_area;
    board_info_area_size = IpmiFruFinalizeArea(board_area_ptr, varp-board_area_ptr);
}

//  Build product info area

void IpmiFruBuildProductInfoArea(EeePSystemInfo_t *info)
{
    product_info_area.area.format_version = IPMI_FRUINFO_FORMAT_VERSION;
    product_info_area.area.language_code = IPMI_FRUINFO_LANGUAGE_CODE_ENGLISH;
    uint8_t *varp = product_info_area.area.variable_part;

    void *strstart = info + 1;
    const char *str;
    //  Manufacturer
    if (str = GetSmbiosString(info->Manufacturer, strstart)) {
        varp = IpmiFruAddString(varp, str);
    } else {
        *varp++ = 0xC0;
    }
    //  Product name
    if (str = GetSmbiosString(info->ProductName, strstart)) {
        varp = IpmiFruAddString(varp, str);
    } else {
        *varp++ = 0xC0;
    }
    //  Model/Part number - not specified
    *varp++ = 0xC0;
    //  Version
    if (str = GetSmbiosString(info->Version, strstart)) {
        varp = IpmiFruAddString(varp, str);
    } else {
        *varp++ = 0xC0;
    }
    //  Serial number
    if (str = GetSmbiosString(info->SerialNumber, strstart)) {
        varp = IpmiFruAddString(varp, str);
    } else {
        *varp++ = 0xC0;
    }
    //  Asset tag - use SKU number
    if (str = GetSmbiosString(info->SKU_Number, strstart)) {
        varp = IpmiFruAddString(varp, str);
    } else {
        *varp++ = 0xC0;
    }
    //  FRU File ID - not specified
    *varp++ = 0xC0;
    //  Family as custom string
    if (str = GetSmbiosString(info->Family, strstart)) {
        varp = IpmiFruAddCustomString(varp, "FAMILY", str);
    }
    uint8_t *product_area_ptr = (uint8_t *)&product_info_area;
    product_info_area_size = IpmiFruFinalizeArea(product_area_ptr, varp-product_area_ptr);
}

//  Find the specific SMBIOS block

EeePSmbiosHdr_t *FindSmbiosInfoBlock(Exp_EEP_t *exp_eep, size_t size, SMBIOS_BlockId_t smbios_type)
{
    unsigned int dynamic_offset = exp_eep->EeePHdr.BlkOffset * 2;
    while (dynamic_offset < size) {
        DBlockIdHdr_t *dblock = (DBlockIdHdr_t *)(((unsigned char *)exp_eep) + dynamic_offset);
        int next_size;
        if (size < dynamic_offset + 3) {
            return NULL;
        }
        next_size = ((unsigned int)(dblock->DBlockLength[0]) | (dblock->DBlockLength[1] << 8)) << 1;
        if (next_size == 0) {
            return NULL;
        }
        if (dblock->DBlockId == EEEP_BLOCK_ID_SMBIOS) {
            EeePSmbiosHdr_t *smbios_hdr = (EeePSmbiosHdr_t *)dblock;
            if (smbios_hdr->Type == smbios_type) {
                return smbios_hdr;
            }
        }
        dynamic_offset += next_size;
    }
    return NULL;
}

//  Find the System SMBIOS block

EeePSystemInfo_t *FindSystemInfoBlock(Exp_EEP_t *exp_eep, size_t size)
{
    return (EeePSystemInfo_t *)FindSmbiosInfoBlock(exp_eep, size, SMBIOS_TypeSYSTEM_INFORMATION);
}

//  Find the Board/Module SMBIOS block

EeePModuleInfo_t *FindBoardInfoBlock(Exp_EEP_t *exp_eep, size_t size)
{
    return (EeePModuleInfo_t *)FindSmbiosInfoBlock(exp_eep, size, SMBIOS_TypeBASE_BOARD_MODULE_INFORMATION);
}

//  Find the Chassis SMBIOS block

EeePChassisInfo_t *FindChassisInfoBlock(Exp_EEP_t *exp_eep, size_t size)
{
    return (EeePChassisInfo_t *)FindSmbiosInfoBlock(exp_eep, size, SMBIOS_TypeSYSTEM_ENCLOSURE_OR_CHASSIS);
}

#define CHUNK_SIZE      4096

int main(int argc, char **argv)
{
    FILE *ifp, *ofp;
    unsigned char *buf = NULL;
    unsigned char local_buf[CHUNK_SIZE];
    unsigned int curr_size = 0;
    unsigned int dynamic_offset = 0;
    int next_size;
    Exp_EEP_t *exp_eep;
    EeePCmn_t *pcommon;

    if (argc > 2 && strcmp(argv[1], "-v") == 0) {
        verbose = TRUE;
    }
    if (argc < 3 || (verbose && argc < 4)) {
        fprintf(stderr, "Usage: %s [-v] infile outfile\n", argv[0]);
        return -1;
    }
    
    const char *ifname = verbose ? argv[2] : argv[1];
    const char *ofname = verbose ? argv[3] : argv[2];
    
    if ((ifp = fopen(ifname, "rb")) == NULL) {
        fprintf(stderr, "%s: cannot open %s\n", argv[0], ifname);
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
            argv[0], ifname, curr_size, sizeof(EeePCmn_t));
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
        //  Expansion ROM, OK
    } else if (memcmp(exp_eep->GenId, "Com0", 4) == 0) {
        //  Carrier Board
        COM0R20_CB_t *cbp = (COM0R20_CB_t *)buf;
        if (curr_size < sizeof(COM0R20_CB_t)) {
            fprintf(stderr, "%s: carrier file %s is too short (%d bytes, expected at least %d bytes)\n",
                argv[0], ifname, curr_size, sizeof(COM0R20_CB_t));
            return -1;
        }
    } else if (memcmp(exp_eep->GenId, "coM0", 4) == 0) {
        //  Module board
        COM0R20_M_t *mp = (COM0R20_M_t *)buf;
        if (curr_size < sizeof(COM0R20_M_t)) {
            fprintf(stderr, "%s: module file %s is too short (%d bytes, expected at least %d bytes)\n",
                argv[0], ifname, curr_size, sizeof(COM0R20_M_t));
            return -1;
        }
    } else if (memcmp(exp_eep->GenId, "ComH", 4) == 0) {
        //  COM-HPC Carrier Board
        COMHR10_CB_t *cbp = (COMHR10_CB_t *)buf;
        if (curr_size < sizeof(COMHR10_CB_t)) {
            fprintf(stderr, "%s: carrier file %s is too short (%d bytes, expected at least %d bytes)\n",
                argv[0], ifname, curr_size, sizeof(COMHR10_CB_t));
            return -1;
        }
    } else if (memcmp(exp_eep->GenId, "coMH", 4) == 0) {
        //  Module board
        COMHR10_M_t *mp = (COMHR10_M_t *)buf;
        if (curr_size < sizeof(COMHR10_M_t)) {
            fprintf(stderr, "%s: module file %s is too short (%d bytes, expected at least %d bytes)\n",
                argv[0], ifname, curr_size, sizeof(COMHR10_M_t));
            return -1;
        }
    } else {
        fprintf(stderr, "%s: Unknown GenID in the header: \"%c%c%c%c\" (%02X %02X %02X %02X)\n",
            argv[0],
            exp_eep->GenId[0], exp_eep->GenId[1], exp_eep->GenId[2], exp_eep->GenId[3],
            exp_eep->GenId[0], exp_eep->GenId[1], exp_eep->GenId[2], exp_eep->GenId[3]);
    }
    
    if ((ofp = fopen(ofname, "wb")) == NULL) {
        fprintf(stderr, "%s: cannot open output file %s\n", argv[0], ofname);
        return -1;
    }
    
    EeePChassisInfo_t *eeep_chassis = FindChassisInfoBlock(exp_eep, curr_size);
    EeePModuleInfo_t *eeep_board = FindBoardInfoBlock(exp_eep, curr_size);
    EeePSystemInfo_t *eeep_system = FindSystemInfoBlock(exp_eep, curr_size);
    chassis_info_area_size = board_info_area_size = product_info_area_size = 0;
    
    if (eeep_chassis) {
        IpmiFruBuildChassisInfoArea(eeep_chassis);
    }
    
    if (eeep_board) {
        IpmiFruBuildBoardInfoArea(eeep_board);
    }
    
    if (eeep_system) {
        IpmiFruBuildProductInfoArea(eeep_system);
    }
    
    if (IpmiFruWriteCommonAreas(ofp) < 0) {
        fclose(ofp);
        return -1;
    }
    
    if (IpmiFruWriteMultirecords(ofp, exp_eep, dynamic_offset, curr_size) < 0) {
        fclose(ofp);
        return -1;
    }
    
    fclose(ofp);
    return 0;
}

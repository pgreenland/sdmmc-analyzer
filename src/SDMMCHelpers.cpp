#include "SDMMCHelpers.h"

static struct MMCResponse responses[64] = {
	/*  0 */ {MMC_RSP_NONE,     0,   0, false, false, false},
	/*  1 */ {MMC_RSP_R3,      32,   5, false, false, false},
	/*  2 */ {MMC_RSP_R2_CID, 128,   5, false, false, false},
	/*  3 */ {MMC_RSP_R1,      32,  64, false, false, false},
	/*  4 */ {MMC_RSP_NONE,     0,   0, false, false, false},
	/*  5 */ {MMC_RSP_R1,      32,  64, true, false, false},
	/*  6 */ {MMC_RSP_R1,      32,  64, true, false, false},
	/*  7 */ {MMC_RSP_R1,      32,  64, true, false, false},
	/*  8 */ {MMC_RSP_R1,      32,  64, false, true, false},
	/*  9 */ {MMC_RSP_R2_CSD, 128,  64, false, false, false},
	/* 10 */ {MMC_RSP_R2_CID, 128,  64, false, false, false},
	/* 11 */ {MMC_RSP_R1,      32,  64, false, false, false},
	/* 12 */ {MMC_RSP_R1,      32,  64, false, false, false},
	/* 13 */ {MMC_RSP_R1,      32,  64, false, false, false},
	/* 14 */ {MMC_RSP_R1,      32,  64, false, false, false},
	/* 15 */ {MMC_RSP_NONE,     0,   0, false, false, false},
	/* 16 */ {MMC_RSP_R1,      32,  64, false, false, false},
	/* 17 */ {MMC_RSP_R1,      32,  64, false, true, false},
	/* 18 */ {MMC_RSP_R1,      32,  64, false, true, true},
	/* 19 */ {MMC_RSP_R1,      32,  64, false, false, false},
	/* 20 */ {MMC_RSP_R1,      32,  64, false, false, false},
	/* 21 */ {MMC_RSP_NONE,     0,   0, false, false, false}, /* reserved */
	/* 22 */ {MMC_RSP_NONE,     0,   0, false, false, false}, /* reserved */
	/* 23 */ {MMC_RSP_R1,      32,  64, false, false, false},
	/* 24 */ {MMC_RSP_R1,      32,  64, false, true, false},
	/* 25 */ {MMC_RSP_R1,      32,  64, false, true, true},
	/* 26 */ {MMC_RSP_R1,      32,  64, false, false, false},
	/* 27 */ {MMC_RSP_R1,      32,  64, false, false, false},
	/* 28 */ {MMC_RSP_R1,      32,  64, true, false, false},
	/* 29 */ {MMC_RSP_R1,      32,  64, true, false, false},
	/* 30 */ {MMC_RSP_R1,      32,  64, false, false, false},
	/* 31 */ {MMC_RSP_NONE,     0,   0, false, false, false}, /* reserved */
	/* 32 */ {MMC_RSP_NONE,     0,   0, false, false, false}, /* reserved */
	/* 33 */ {MMC_RSP_NONE,     0,   0, false, false, false}, /* reserved */
	/* 34 */ {MMC_RSP_NONE,     0,   0, false, false, false}, /* reserved */
	/* 35 */ {MMC_RSP_R1,      32,  64, false, false, false},
	/* 36 */ {MMC_RSP_R1,      32,  64, false, false, false},
	/* 37 */ {MMC_RSP_NONE,     0,   0, false, false, false}, /* reserved */
	/* 38 */ {MMC_RSP_R1,      32,  64, true, false, false},
	/* 39 */ {MMC_RSP_R4,      32,  64, false, false, false},
	/* 40 */ {MMC_RSP_R5,      32,  64, false, false, false},
	/* 41 */ {MMC_RSP_NONE,     0,   0, false, false, false}, /* reserved */
	/* 42 */ {MMC_RSP_R1,      32,  64, false, false, false},
	/* 43 */ {MMC_RSP_NONE,     0,   0, false, false, false}, /* reserved */
	/* 44 */ {MMC_RSP_NONE,     0,   0, false, false, false}, /* reserved */
	/* 45 */ {MMC_RSP_NONE,     0,   0, false, false, false}, /* reserved */
	/* 46 */ {MMC_RSP_NONE,     0,   0, false, false, false}, /* reserved */
	/* 47 */ {MMC_RSP_NONE,     0,   0, false, false, false}, /* reserved */
	/* 48 */ {MMC_RSP_NONE,     0,   0, false, false, false}, /* reserved */
	/* 49 */ {MMC_RSP_NONE,     0,   0, false, false, false}, /* reserved */
	/* 50 */ {MMC_RSP_NONE,     0,   0, false, false, false}, /* reserved */
	/* 51 */ {MMC_RSP_NONE,     0,   0, false, false, false}, /* reserved */
	/* 52 */ {MMC_RSP_NONE,     0,   0, false, false, false}, /* reserved */
	/* 53 */ {MMC_RSP_NONE,     0,   0, false, false, false}, /* reserved */
	/* 54 */ {MMC_RSP_NONE,     0,   0, false, false, false}, /* reserved */
	/* 55 */ {MMC_RSP_R1,      32,  64, false, false, false},
	/* 56 */ {MMC_RSP_R1,      32,  64, false, false, false},
	/* 57 */ {MMC_RSP_NONE,     0,   0, false, false, false}, /* reserved */
	/* 58 */ {MMC_RSP_NONE,     0,   0, false, false, false}, /* reserved */
	/* 59 */ {MMC_RSP_NONE,     0,   0, false, false, false}, /* reserved */
	/* 60 */ {MMC_RSP_NONE,     0,   0, false, false, false}, /* reserved */
	/* 61 */ {MMC_RSP_NONE,     0,   0, false, false, false}, /* reserved */
	/* 62 */ {MMC_RSP_NONE,     0,   0, false, false, false}, /* reserved */
	/* 63 */ {MMC_RSP_NONE,     0,   0, false, false, false}, /* reserved */
};

static struct MMCResponse invalid_response = {
	MMC_RSP_NONE,  0,   0, false, false
};

struct MMCResponse SDMMCHelpers::MMCCommandResponse(U64 index)
{
	if (index > 63)
		return invalid_response;

	return responses[index];
}

/* According to eMMC 4.51 specs */
static struct MMCCommand commands[64] = {
		"GO_IDLE_STATE",		/* CMD00	arg:00000000 => GO_IDLE_STATE
											arg:F0F0F0F0 => GO_PRE_IDLE_STATE
											arg:FFFFFFFA => BOOT_INITIATION */
		"SEND_OP_COND",			/* CMD01 */
		"ALL_SEND_CID",			/* CMD02 */
		"SET_RELATIVE_ADDR",	/* CMD03 */
		"SET_DSR",				/* CMD04 */
		"SLEEP_AWAKE",			/* CMD05 */
		"SWITCH",				/* CMD06 */
		"SELECT/DESELECT_CARD",	/* CMD07 */
		"SEND_EXT_CSD",			/* CMD08 */
		"SEND_CSD",				/* CMD09 */
		"SEND_CID",				/* CMD10 */
		"_Obsolete",			/* CMD11 */
		"STOP_TRANSMISSION",	/* CMD12 */
		"SEND_STATUS",			/* CMD13 */
		"BUSTEST_R",			/* CMD14 */
		"GO_INACTIVE_STATE",	/* CMD15 */
		"SET_BLOCKLEN",			/* CMD16 */
		"READ_SINGLE_BLOCK",	/* CMD17 */
		"READ_MULTIPLE_BLOCK",	/* CMD18 */
		"BUSTEST_W",			/* CMD19 */
		"_Obsolete",			/* CMD20 */
		"SEND_TUNING_BLOCK",	/* CMD21 */
		"_Reserved",			/* CMD22 */
		"SET_BLOCK_COUNT",		/* CMD23 */
		"WRITE_BLOCK",			/* CMD24 */
		"WRITE_MULTIPLE_BLOCK",	/* CMD25 */
		"PROGRAM_CID",			/* CMD26 */
		"PROGRAM_CSD",			/* CMD27 */
		"SET_WRITE_PROT",		/* CMD28 */
		"CLR_WRITE_PROT",		/* CMD29 */
		"SEND_WRITE_PROT",		/* CMD30 */
		"SEND_WRITE_PROT_TYPE",	/* CMD31 */
		"_Reserved",			/* CMD32 */
		"_Reserved",			/* CMD33 */
		"_Reserved",			/* CMD34 */
		"ERASE_GROUP_START",	/* CMD35 */
		"ERASE_GROUP_END",		/* CMD36 */
		"_Reserved",			/* CMD37 */
		"ERASE",				/* CMD38 */
		"FAST_IO",				/* CMD39 */
		"GO_IRQ_STATE",			/* CMD40 */
		"_Reserved",			/* CMD41 */
		"LOCK_UNLOCK",			/* CMD42 */
		"_Reserved",			/* CMD43 */
		"_Reserved",			/* CMD44 */
		"_Reserved",			/* CMD45 */
		"_Reserved",			/* CMD46 */
		"_Reserved",			/* CMD47 */
		"_Reserved",			/* CMD48 */
		"SET_TIME",				/* CMD49 */
		"_Reserved",			/* CMD50 */
		"_Reserved",			/* CMD51 */
		"_Reserved",			/* CMD52 */
		"PROTOCOL_RD",			/* CMD53 */
		"PROTOCOL_WR",			/* CMD54 */
		"APP_CMD",				/* CMD55 */
		"GEN_CMD",				/* CMD56 */
		"_Reserved",			/* CMD57 */
		"_Reserved",			/* CMD58 */
		"_Reserved",			/* CMD59 */
		"_ReservedMFR",			/* CMD60 */
		"_ReservedMFR",			/* CMD61 */
		"_ReservedMFR",			/* CMD62 */
		"_ReservedMFR",			/* CMD63 */
};

static struct MMCCommand invalid_response_cmd = {
	"_INVALID_"
};

const char * SDMMCHelpers::MMCCommandDescription(U64 index, U64 args)
{
	if (index > 63)
		return invalid_response_cmd.desc;

	if (index == 0) {
		if (args == 0x00000000)
			return "GO_IDLE_STATE";
		else if (args == 0xF0F0F0F0)
			return "GO_PRE_IDLE_STATE";
		else if (args == 0xFFFFFFFA)
			return "BOOT_INITIATION";
		else
			return "_INVALID_CMD01_";
	}

	return commands[index].desc;
}

/*
  Polynomial = 0x89 (2^7 + 2^3 + 1)
  width      = 7 bit
*/
static unsigned char crc7_table[256] = {
	0x00, 0x09, 0x12, 0x1b, 0x24, 0x2d, 0x36, 0x3f,
	0x48, 0x41, 0x5a, 0x53, 0x6c, 0x65, 0x7e, 0x77,
	0x19, 0x10, 0x0b, 0x02, 0x3d, 0x34, 0x2f, 0x26,
	0x51, 0x58, 0x43, 0x4a, 0x75, 0x7c, 0x67, 0x6e,
	0x32, 0x3b, 0x20, 0x29, 0x16, 0x1f, 0x04, 0x0d,
	0x7a, 0x73, 0x68, 0x61, 0x5e, 0x57, 0x4c, 0x45,
	0x2b, 0x22, 0x39, 0x30, 0x0f, 0x06, 0x1d, 0x14,
	0x63, 0x6a, 0x71, 0x78, 0x47, 0x4e, 0x55, 0x5c,
	0x64, 0x6d, 0x76, 0x7f, 0x40, 0x49, 0x52, 0x5b,
	0x2c, 0x25, 0x3e, 0x37, 0x08, 0x01, 0x1a, 0x13,
	0x7d, 0x74, 0x6f, 0x66, 0x59, 0x50, 0x4b, 0x42,
	0x35, 0x3c, 0x27, 0x2e, 0x11, 0x18, 0x03, 0x0a,
	0x56, 0x5f, 0x44, 0x4d, 0x72, 0x7b, 0x60, 0x69,
	0x1e, 0x17, 0x0c, 0x05, 0x3a, 0x33, 0x28, 0x21,
	0x4f, 0x46, 0x5d, 0x54, 0x6b, 0x62, 0x79, 0x70,
	0x07, 0x0e, 0x15, 0x1c, 0x23, 0x2a, 0x31, 0x38,
	0x41, 0x48, 0x53, 0x5a, 0x65, 0x6c, 0x77, 0x7e,
	0x09, 0x00, 0x1b, 0x12, 0x2d, 0x24, 0x3f, 0x36,
	0x58, 0x51, 0x4a, 0x43, 0x7c, 0x75, 0x6e, 0x67,
	0x10, 0x19, 0x02, 0x0b, 0x34, 0x3d, 0x26, 0x2f,
	0x73, 0x7a, 0x61, 0x68, 0x57, 0x5e, 0x45, 0x4c,
	0x3b, 0x32, 0x29, 0x20, 0x1f, 0x16, 0x0d, 0x04,
	0x6a, 0x63, 0x78, 0x71, 0x4e, 0x47, 0x5c, 0x55,
	0x22, 0x2b, 0x30, 0x39, 0x06, 0x0f, 0x14, 0x1d,
	0x25, 0x2c, 0x37, 0x3e, 0x01, 0x08, 0x13, 0x1a,
	0x6d, 0x64, 0x7f, 0x76, 0x49, 0x40, 0x5b, 0x52,
	0x3c, 0x35, 0x2e, 0x27, 0x18, 0x11, 0x0a, 0x03,
	0x74, 0x7d, 0x66, 0x6f, 0x50, 0x59, 0x42, 0x4b,
	0x17, 0x1e, 0x05, 0x0c, 0x33, 0x3a, 0x21, 0x28,
	0x5f, 0x56, 0x4d, 0x44, 0x7b, 0x72, 0x69, 0x60,
	0x0e, 0x07, 0x1c, 0x15, 0x2a, 0x23, 0x38, 0x31,
	0x46, 0x4f, 0x54, 0x5d, 0x62, 0x6b, 0x70, 0x79
};

U8 SDMMCHelpers::crc7(const U8 *data, unsigned int size)
{
	U8 crc7_accum = 0;

	for (unsigned int i = 0; i < size; i++) {
		crc7_accum = crc7_table[(crc7_accum << 1) ^ data[i]];
	}
	return crc7_accum;
}

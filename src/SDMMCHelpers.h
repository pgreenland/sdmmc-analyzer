#ifndef __SDMMC_HELPERS
#define __SDMMC_HELPERS

#include <Analyzer.h>

enum MMCResponseType {
	MMC_RSP_NONE,
	MMC_RSP_R1,
	MMC_RSP_R2_CID,
	MMC_RSP_R2_CSD,
	MMC_RSP_R3,
	MMC_RSP_R4,
	MMC_RSP_R5,
};

enum CommandReadPhase {
	CMD_INIT, // Command is still at 1
	CMD_DIR, // Direction bit being read
	CMD_IDX, // cmd index
	CMD_ARG,
	CMD_CRC,
	CMD_STOP,
	RESP_INIT, // Command is still at 1
	RESP_DIR, // Direction bit being read
	RESP_IGNORED, // 6 bits - "command index" or "check bits"
	RESP_DATA,
	RESP_CRC, // 0 or 7 bits
	RESP_STOP,
	CMD_ERROR,
	CMD_END,
	CMD_INTERRUPT // same as END, but indicates that data read must stop now
};

enum DataReadPhase {
	DATA_NOTSTARTED,
	DATA_INIT, // Before start bit
	DATA_DATA, // Data being read
	DATA_CRC,  // 16 bits
	DATA_STOP, // Stop bit
	DATA_CHECKCRC_INIT, // after write
	DATA_CHECKCRC_CRC,
	DATA_CHECKCRC_STOP,
	DATA_BUSY,
	DATA_BUSY_END,
	DATA_ERROR, // Decoding error
	DATA_END
};

struct CommandReadState {
	enum CommandReadPhase phase;
	U8 cmd_idx_cnt;
	U8 cmd_arg_cnt;
	U8 cmd_crc_cnt;
	enum MMCResponseType responseType;
	int timeout;
	int cmdindex;
	U8 resp_data_bits; // data length, depends only on command index
	U8 resp_ignore_cnt;
	U8 resp_data_cnt;
	U8 resp_crc_cnt;
};

struct DataReadState { // FIXME: take "block len" into account
	enum DataReadPhase phase;
	int cmdindex;
	U16 data_cnt;
	U8 crc_cnt;
	U8 checkcrc_cnt;
	bool hasSeveralDataBlocks;
};

struct MMCResponse
{
	enum MMCResponseType mType;
	unsigned int mBits;
	int mTimeout;
	bool mBusy;
	bool hasDataBlock;
	bool hasSeveralDataBlocks;
};

struct MMCCommand
{
	const char *desc;
};

class SDMMCHelpers
{
public:

public:
	static U8 crc7(const U8 *data, unsigned int size);
	static struct MMCResponse MMCCommandResponse(unsigned int index);
	static const char * MMCCommandDescription(unsigned int index, unsigned int args);

private:
	static U8 __crc7(U8 crc, U8 data);
	static U8 __crc7_finalize(U8 crc);
};

#endif

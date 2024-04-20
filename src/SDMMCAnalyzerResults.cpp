#include "SDMMCHelpers.h"
#include "SDMMCAnalyzerResults.h"
#include "SDMMCAnalyzer.h"
#include "SDMMCAnalyzerSettings.h"

#include <AnalyzerHelpers.h>
#include <iostream>
#include <fstream>

SDMMCAnalyzerResults::SDMMCAnalyzerResults(SDMMCAnalyzer* analyzer, SDMMCAnalyzerSettings* settings)
:	mSettings(settings),
	mAnalyzer(analyzer)
{
}

SDMMCAnalyzerResults::~SDMMCAnalyzerResults()
{
}

static const char* cmd_abbrev_from_number(U64 cmd_number)
{
	if (cmd_number >= 43 && cmd_number <= 47) return "[reserved]";
	switch (cmd_number) {
		case 0: return "GO_IDLE_STATE";
		case 1: return "[reserved]";
		case 2: return "ALL_SEND_CID";
		case 3: return "SEND_RELATIVE_ADDR";
		case 4: return "SET_DSR";
		case 5: return "[reserved for SDIO]";
		case 6: return "SWITCH_FUNC";
		case 7: return "SELECT/DESELECT_CARD";
		case 8: return "SEND_EXT_CSD";
		case 9: return "SEND_CSD";
		case 10: return "SEND_CID";
		case 11: return "VOLTAGE_SWITCH";
		case 12: return "STOP_TRANSMISSION";
		case 13: return "SEND_STATUS";
		case 14: return "[reserved]";
		case 15: return "GO_INACTIVE_STATE";
		case 16: return "SET_BLOCKLEN";
		case 17: return "READ_SINGLE_BLOCK";
		case 18: return "READ_MULTIPLE_BLOCK";
		case 19: return "SEND_TUNING_BLOCK";
		case 20: return "SPEED_CLASS_CONTROL";
		case 21: return "[reserved for DPS]";
		case 22: return "[reserved]";
		case 23: return "SET_BLOCK_COUNT";
		case 24: return "WRITE_BLOCK";
		case 25: return "WRITE_MULTIPLE_BLOCK";
		case 26: return "[reserved for manufacturer]";
		case 27: return "PROGRAM_CSD";
		case 28: return "SWT_WRITE_PROT";
		case 29: return "CLR_WRITE_PROT";
		case 30: return "SEND_WRITE_PROT";
		case 31: return "[reserved]";
		case 32: return "ERASE_WR_BLK_START";
		case 33: return "ERASE_WR_BLK_END";
		case 34: return "[function dependent]";
		case 35: return "[function dependent]";
		case 36: return "[function dependent]";
		case 37: return "[function dependent]";
		case 38: return "ERASE";
		case 39: return "[reserved]";
		case 40: return "[defined by DPS]";
		case 41: return "[reserved]";
		case 42: return "LOCK_UNLOCK";
		case 48: return "READ_EXTR_SINGLE";
		case 49: return "WRITE_EXTR_SINGLE";
		case 50: return "[function dependent]";
		case 51: return "[reserved]";
		case 52: return "[sdio]";
		case 53: return "[sdio]";
		case 54: return "[sdio]";
		case 55: return "APP_CMD";
		case 56: return "GEN_CMD";
		case 57: return "[function dependent]";
		case 58: return "READ_EXTR_MULTI";
		case 59: return "WRITE_EXTR_MULTI";
		default: return "[invalid command number]";
	}
}

void SDMMCAnalyzerResults::GenerateBubbleText(U64 frame_index, Channel& channel, DisplayBase display_base)
{
	ClearResultStrings();
	Frame frame = GetFrame(frame_index);

	switch (frame.mType) {
	case FRAMETYPE_HEADER:
		if (channel != mSettings->mCommandChannel)
			break;
		if (frame.mData1 == 1)
			AddResultString("Host sending");
		else
			AddResultString("Card sending");
		break;

case FRAMETYPE_COMMAND:
	{
		if (channel != mSettings->mCommandChannel)
			break;
		char str_cmd[33];
		char str_arg[33];
		const char *str_desc;
		char str_buf[512];

		AnalyzerHelpers::GetNumberString(frame.mData1, Decimal, 6, str_cmd, sizeof(str_cmd));
		AnalyzerHelpers::GetNumberString(frame.mData2, display_base, 32, str_arg, sizeof(str_arg));

		str_desc = SDMMCHelpers::MMCCommandDescription(frame.mData1, frame.mData2);

		AddResultString("CMD");
		AddResultString("CMD", str_cmd);
		AddResultString("CMD", str_cmd, ", arg=", str_arg, "  ", str_desc);
		AddResultString("CMD", str_cmd, " (", cmd_abbrev_from_number(frame.mData1), "), arg=", str_arg);

		switch (frame.mData1) {
		case 3: //SET_RELATIVE_ADDR
		case 7: //SELECT/DESELECT_CARD
		case 9: //SEND_CSD
		case 10: //SEND_CID
		case 15: //GO_INACTIVE_STATE
		case 55: //APP_CMD
        {
			char str_rca[33];
			char str_stuff[33];

			int rca = (frame.mData2 >> 16) & 0xFF;			//[31:16] RCA
			int stuff = frame.mData2 & 0xFF;				//[15:0] stuff bits

			AnalyzerHelpers::GetNumberString(rca, display_base, 32, str_rca, sizeof(str_rca));
			AnalyzerHelpers::GetNumberString(stuff, display_base, 32, str_stuff, sizeof(str_stuff));

			sprintf_s(str_buf, ", arg=%s, rca=%s, stuff=%s", str_arg, str_rca, str_stuff);
			break;
        }
		case 4: //SET_DSR
            {
			char str_dsr[33];
			char str_stuff[33];

			int dsr = (frame.mData2 >> 16) & 0xFF;			//[31:16] DSR
			int stuff = frame.mData2 & 0xFF;				//[15:0] stuff bits

			AnalyzerHelpers::GetNumberString(dsr, display_base, 32, str_dsr, sizeof(str_dsr));
			AnalyzerHelpers::GetNumberString(stuff, display_base, 32, str_stuff, sizeof(str_stuff));

			sprintf_s(str_buf, ", arg=%s, dsr=%s, stuff=%s", str_arg, str_dsr, str_stuff);
			break;}
		case 5: //SLEEP_AWAKE
            {
			char str_rca[33];
			char str_sleepawake[33];
			char str_stuff[33];

			int rca = (frame.mData2 >> 16) & 0xFF;			//[31:16] RCA
			int sleepawake = (frame.mData2 >> 15) & 0xF;	//[15]Sleep/Awake
			int stuff = frame.mData2 & 0xFF;				//[14:0] stuff bits

			AnalyzerHelpers::GetNumberString(rca, display_base, 32, str_rca, sizeof(str_rca));
			AnalyzerHelpers::GetNumberString(sleepawake, display_base, 4, str_sleepawake, sizeof(str_sleepawake));
			AnalyzerHelpers::GetNumberString(stuff, display_base, 32, str_stuff, sizeof(str_stuff));

			sprintf_s(str_buf, ", arg=%s, rca=%s, sleepawake=%s, stuff=%s", str_arg, str_rca, str_sleepawake, str_stuff);
			break;}
		case 6: //SWITCH
            {
			char str_access[33];
			char str_index[33];
			char str_value[33];
			char str_cmd2[33];

			int access = (frame.mData2 >> 24) & 0xF;   		//[25:24] Access 0=cmd_set 1=set_bits 2=clear_bits 3=write_byte
			int index = (frame.mData2 >> 16) & 0xFF; 		//[23:16] Index
			int value = (frame.mData2 >> 8) & 0xFF;  		//[15:8] Value
			int cmd = frame.mData2 & 0xF;              		//[2:0] Cmd Set

			AnalyzerHelpers::GetNumberString(access, display_base, 4, str_access, sizeof(str_access));
			AnalyzerHelpers::GetNumberString(index, display_base, 32, str_index, sizeof(str_index));
			AnalyzerHelpers::GetNumberString(value, display_base, 32, str_value, sizeof(str_value));
			AnalyzerHelpers::GetNumberString(cmd, display_base, 4, str_cmd2, sizeof(str_cmd2));

			sprintf_s(str_buf, ", arg=%s, access=%s, index=%s, value=%s, cmd=%s", str_arg, str_access, str_index, str_value, str_cmd2);

			AddResultString("CMD", str_cmd, str_buf);
			break;}
		case 12: //STOP_TRANSMISSION
		case 13: //SEND_STATUS
            {
			char str_rca[33];
			char str_stuff[33];
			char str_hpi[33];

			int rca = (frame.mData2 >> 16) & 0xFF;	//[31:16] RCA
			int stuff = (frame.mData2 >> 1) & 0xFF; //[15:1] stuff bits
			int hpi = frame.mData2 & 0xF;			//[0] HPI

			AnalyzerHelpers::GetNumberString(rca, display_base, 32, str_rca, sizeof(str_rca));
			AnalyzerHelpers::GetNumberString(stuff, display_base, 32, str_stuff, sizeof(str_stuff));
			AnalyzerHelpers::GetNumberString(hpi, display_base, 4, str_hpi, sizeof(str_hpi));

			sprintf_s(str_buf, ", arg=%s, rca=%s, stuff=%s, hpi=%s", str_arg, str_rca, str_stuff, str_hpi);
			break;}
		case 23: //SET_BLOCK_COUNT
            {
			char str_rwritereq[33];
			char str_tagreq[33];
			char str_ctx_id[33];
			char str_forced[33];
			char str_num_blocks[33];

		// default
			int rwritereq = (frame.mData2 >> 31) & 0xF;	//[31] Reliable Write Request
														//[30] ‘0’ non- packed
			int tagreq = (frame.mData2 >> 29) & 0xF;	//[29] tag request
			int ctx_id = (frame.mData2 >> 25) & 0xFF;	//[28:25] context ID
			int forced = (frame.mData2 >> 24) & 0xF;	//[24]: forced programming
														//[23:16] set to 0
			int num_blocks = frame.mData2 & 0xFF;		//[15:0] number of blocks
		// packed
			int p_zero = (frame.mData2 >> 31) & 0xF;	//[31] set to 0
			int p_one = (frame.mData2 >> 30) & 0xF;		//[30] ‘1’ packed
			int p_zero2 = (frame.mData2 >> 16) & 0xFF;	//[29:16] set to 0
			int num_blocks2 = frame.mData2 & 0xFF;		//[15:0] number of blocks

			if (p_zero == 0 && p_one == 1 && p_zero2 == 0) { // seems packed
				AnalyzerHelpers::GetNumberString(num_blocks2, display_base, 32, str_num_blocks, sizeof(str_num_blocks));

				sprintf_s(str_buf, ", arg=%s, num_blocks=%s", str_arg, str_num_blocks);

			} else { // has to be default then
				AnalyzerHelpers::GetNumberString(rwritereq, display_base, 4, str_rwritereq, sizeof(str_rwritereq));
				AnalyzerHelpers::GetNumberString(tagreq, display_base, 32, str_tagreq, sizeof(str_tagreq));
				AnalyzerHelpers::GetNumberString(ctx_id, display_base, 32, str_ctx_id, sizeof(str_ctx_id));
				AnalyzerHelpers::GetNumberString(forced, display_base, 4, str_forced, sizeof(str_forced));
				AnalyzerHelpers::GetNumberString(num_blocks, display_base, 4, str_num_blocks, sizeof(str_num_blocks));

				sprintf_s(str_buf, ", arg=%s, rwritereq=%s, tagreq=%s, ctx_id=%s, forced=%s, num_blocks=%i", str_arg, str_rwritereq, str_tagreq, str_ctx_id, str_forced, num_blocks);
			}
			break;}
		case 38: //ERASE
            {
			char str_securereq[33];
			char str_forcegarb[33];
			char str_discard_en[33];
			char str_ident_writeblocks[33];

			int securereq = (frame.mData2 >> 31) & 0xF;	//[31] Secure request
			//[30:16] set to 0
			int forcegarb = (frame.mData2 >> 15) & 0xF;	//[15] Force Garbage Collect request
			//[14:2] set to 0
			int discard_en = (frame.mData2 >> 1) & 0xF;	//[1] Discard Enable
			int ident_writeblocks = frame.mData2 & 0xF;	//[0] Identify Write Blocks for Erase (or TRIM Enable)

			AnalyzerHelpers::GetNumberString(securereq, display_base, 4, str_securereq, sizeof(str_securereq));
			AnalyzerHelpers::GetNumberString(forcegarb, display_base, 4, str_forcegarb, sizeof(str_forcegarb));
			AnalyzerHelpers::GetNumberString(discard_en, display_base, 4, str_discard_en, sizeof(str_discard_en));
			AnalyzerHelpers::GetNumberString(ident_writeblocks, display_base, 4, str_ident_writeblocks, sizeof(str_ident_writeblocks));

			sprintf_s(str_buf, ", arg=%s, securereq=%s, forcegarbage=%s, discard_en=%s, ident_writeblocks=%s", str_arg, str_securereq, str_forcegarb, str_discard_en, str_ident_writeblocks);
			break;}
		case 39: //FAST_IO
            {
			char str_rca[33];
			char str_reg_wflag[33];
			char str_reg_addr[33];
			char str_reg_data[33];

			int rca = (frame.mData2 >> 16) & 0xFF;		//[31:16] RCA
			int reg_wflag = (frame.mData2 >> 15) & 0xF;	//[15:15] register write flag
			int reg_addr = (frame.mData2 >> 8) & 0xFF;	//[14:8] register address
			int reg_data = frame.mData2 & 0xFF;			//[7:0] register data

			AnalyzerHelpers::GetNumberString(rca, display_base, 32, str_rca, sizeof(str_rca));
			AnalyzerHelpers::GetNumberString(reg_wflag, display_base, 4, str_reg_wflag, sizeof(str_reg_wflag));
			AnalyzerHelpers::GetNumberString(reg_addr, display_base, 32, str_reg_addr, sizeof(str_reg_addr));
			AnalyzerHelpers::GetNumberString(reg_data, display_base, 32, str_reg_data, sizeof(str_reg_data));

			sprintf_s(str_buf, ", arg=%s, rca=%s, reg_wflag=%s, reg_addr=%s, reg_data=%s", str_arg, str_rca, str_reg_wflag, str_reg_addr, str_reg_data);
			break;}
		case 53: //PROTOCOL_RD
		case 54: //PROTOCOL_WR
            {
			char str_sec_spec[33];
			char str_sec_prot[33];
			char str_reserv[33];

			int sec_spec = (frame.mData2 >> 16) & 0xFF;	//[16:31] Security Protocol Specific
			int sec_prot = (frame.mData2 >> 8) & 0xFF;	//[15:8] Security Protocol
			int reserv = frame.mData2 & 0xFF;			//[7:0] reserved

			AnalyzerHelpers::GetNumberString(sec_spec, display_base, 32, str_sec_spec, sizeof(str_sec_spec));
			AnalyzerHelpers::GetNumberString(sec_prot, display_base, 32, str_sec_prot, sizeof(str_sec_prot));
			AnalyzerHelpers::GetNumberString(reserv, display_base, 32, str_reserv, sizeof(str_reserv));

			sprintf_s(str_buf, ", arg=%s, sec_specific=%s, sec_protocol=%s, reserv=%s", str_arg, str_sec_spec, str_sec_prot, str_reserv);
			break;}
		case 56: //GEN_CMD
            {
			char str_stuff[33];
			char str_rd_wr[33];

			int stuff = (frame.mData2 >> 1) & 0xFF;	//[31:1] stuff bits.
			int rd_wr = frame.mData2 & 0xF;			//[0]: RD/WR1

			AnalyzerHelpers::GetNumberString(stuff, display_base, 32, str_stuff, sizeof(str_stuff));
			AnalyzerHelpers::GetNumberString(rd_wr, display_base, 4, str_rd_wr, sizeof(str_rd_wr));

			sprintf_s(str_buf, ", arg=%s, stuff=%s, rd_wr1=%s", str_arg, str_stuff, str_rd_wr);
			break;}
		}
	}

	case FRAMETYPE_RESPONSE:
	{
		if (channel != mSettings->mCommandChannel)
			break;
		char str_32[33];

		switch (frame.mFlags) {
		case MMC_RSP_R1:
		{
			const char *str_state = "reserved";
			std::string str_flags("");

			switch ((frame.mData1 >> 9) & 0xf) {
			case 0: str_state = "Idle"; break;
			case 1: str_state = "Ready"; break;
			case 2: str_state = "Ident"; break;
			case 3: str_state = "Stby"; break;
			case 4: str_state = "Tran"; break;
			case 5: str_state = "Data"; break;
			case 6: str_state = "Rcv"; break;
			case 7: str_state = "Prg"; break;
			case 8: str_state = "Dis"; break;
			case 9: str_state = "Btst"; break;
			case 10: str_state = "Slp "; break;
			}

			AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 32, str_32, sizeof(str_32));

			AddResultString("R1");
			AddResultString("R1, ", str_state);
			AddResultString("R1, ", str_state, ", rsp=", str_32);

			if (frame.mData1 & (1 << 31))
					str_flags += " ADDRESS_OUT_OF_RANGE";
			if (frame.mData1 & (1 << 30))
					str_flags += " ADDRESS_MISALIGN";
			if (frame.mData1 & (1 << 29))
					str_flags += " BLOCK_LEN_ERROR";
			if (frame.mData1 & (1 << 28))
					str_flags += " ERASE_SEQ_ERROR";
			if (frame.mData1 & (1 << 27))
					str_flags += " ERASE_PARAM";
			if (frame.mData1 & (1 << 26))
					str_flags += " WP_VIOLATION";
			if (frame.mData1 & (1 << 25))
					str_flags += " CARD_IS_LOCKED";
			if (frame.mData1 & (1 << 24))
					str_flags += " LOCK_UNLOCK_FAILED";
			if (frame.mData1 & (1 << 23))
					str_flags += " COM_CRC_ERROR";
			if (frame.mData1 & (1 << 22))
					str_flags += " ILLEGAL_COMMAND";
			if (frame.mData1 & (1 << 21))
					str_flags += " CARD_ECC_FAILED";
			if (frame.mData1 & (1 << 20))
					str_flags += " CC_ERROR";
			if (frame.mData1 & (1 << 19))
					str_flags += " ERROR";
			if (frame.mData1 & (1 << 18))
					str_flags += " UNDERRUN";
			if (frame.mData1 & (1 << 17))
					str_flags += " OVERRUN";
			if (frame.mData1 & (1 << 16))
					str_flags += " CID/CSD_OVERWRITE";
			if (frame.mData1 & (1 << 15))
					str_flags += " WP_ERASE_SKIP";
			if (frame.mData1 & (1 << 13))
					str_flags += " ERASE_RESET";
			if (frame.mData1 & (1 << 8))
					str_flags += " READY_FOR_DATA";
			if (frame.mData1 & (1 << 7))
					str_flags += " SWITCH_ERROR";
			if (frame.mData1 & (1 << 6))
					str_flags += " EXCEPTION_EVENT";
			if (frame.mData1 & (1 << 5))
					str_flags += " APP_CMD";
			if (frame.mData1 & (1 << 4))
					str_flags += " RESERVED";
			if (frame.mData1 & (1 << 3) || frame.mData1 & (1 << 2))
					str_flags += " APP_SPECIFIC";
			if (frame.mData1 & (1 << 1) || frame.mData1 & 1)
					str_flags += " MANUF_TEST";

			if (str_flags.length() > 0)
				AddResultString("R1, ", str_state, ", rsp=", str_32, str_flags.c_str());

			break;
		}
		case MMC_RSP_R2_CID:
		{
			std::string res("R2");
			char pname[7], prv_str[4], psn_str[12];
			char rsp_str[64];

			AddResultString(res.c_str());

			res += " [CID]";
			AddResultString(res.c_str());

			res += " rsp=";
			AnalyzerHelpers::GetNumberString(frame.mData1 >> 32, display_base, 32, rsp_str, sizeof(rsp_str));
			res += rsp_str;
			res += " ";
			AnalyzerHelpers::GetNumberString(frame.mData1 & 0xffffffffull, display_base, 32, rsp_str, sizeof(rsp_str));
			res += rsp_str;
			res += " ";
			AnalyzerHelpers::GetNumberString(frame.mData2 >> 32, display_base, 32, rsp_str, sizeof(rsp_str));
			res += rsp_str;
			res += " ";
			AnalyzerHelpers::GetNumberString(frame.mData2 & 0xffffffffull, display_base, 32, rsp_str, sizeof(rsp_str));
			res += rsp_str;
			AddResultString(res.c_str());

			pname[0] = (frame.mData1 >> 32) & 0xff;
			pname[1] = (frame.mData1 >> 24) & 0xff;
			pname[2] = (frame.mData1 >> 16) & 0xff;
			pname[3] = (frame.mData1 >>  8) & 0xff;
			pname[4] = (frame.mData1 >>  0) & 0xff;
			pname[5] = (frame.mData2 >> 56) & 0xff;
			pname[6] = 0;

			unsigned prv = (unsigned)((frame.mData2 >> 48) & 0xff);
			prv_str[0] = '0' + ((prv >> 4) & 0x0f);
			prv_str[1] = '.';
			prv_str[2] = '0' + (prv & 0x0f);
			prv_str[3] = 0;

			unsigned psn = (unsigned)((frame.mData2 >> 16) & 0xfffffffful);
			AnalyzerHelpers::GetNumberString(psn, Decimal, 32, psn_str, sizeof(psn_str));

			res += " pnm='";
			res += pname;
			res += "' prv=";
			res += prv_str;
			res += " psn=";
			res += psn_str;
			AddResultString(res.c_str());

			break;
		}
		case MMC_RSP_R2_CSD:
		{
			std::string res("R2");
			char rsp_str[64];

			AddResultString(res.c_str());

			res += " [CSD]";
			AddResultString(res.c_str());

			res += " rsp=";
			AnalyzerHelpers::GetNumberString(frame.mData1 >> 32, display_base, 32, rsp_str, sizeof(rsp_str));
			res += rsp_str;
			res += " ";
			AnalyzerHelpers::GetNumberString(frame.mData1 & 0xffffffffull, display_base, 32, rsp_str, sizeof(rsp_str));
			res += rsp_str;
			res += " ";
			AnalyzerHelpers::GetNumberString(frame.mData2 >> 32, display_base, 32, rsp_str, sizeof(rsp_str));
			res += rsp_str;
			res += " ";
			AnalyzerHelpers::GetNumberString(frame.mData2 & 0xffffffffull, display_base, 32, rsp_str, sizeof(rsp_str));
			res += rsp_str;
			AddResultString(res.c_str());

			break;
		}
		case MMC_RSP_R3:
			AnalyzerHelpers::GetNumberString(frame.mData1, Hexadecimal, 32, str_32, sizeof(str_32));
			AddResultString("R3");
			AddResultString("R3, ocr=", str_32);
			break;
		case MMC_RSP_R4:
			AddResultString("R4");
			break;
		case MMC_RSP_R5:
			AddResultString("R5");
			break;
		}
		break;
	}

	case FRAMETYPE_CRC:
	{
		if (channel != mSettings->mCommandChannel)
			break;
		char str_crc[8];

		AnalyzerHelpers::GetNumberString(frame.mData1, Hexadecimal, 7, str_crc, sizeof(str_crc));

		AddResultString("CRC");
		AddResultString("CRC=", str_crc);
		break;
	}

	case FRAMETYPE_DATA_CONTENTS:
	{
		if (channel != mSettings->mDataChannel0)
			break;

		char str_crc[8];
		AnalyzerHelpers::GetNumberString(frame.mData1, Hexadecimal, 8,
				str_crc, sizeof(str_crc));
		AddResultString(str_crc);
		break;
	}
	case FRAMETYPE_DATA_CRC:
	{
		if (channel != mSettings->mDataChannel0)
			break;

		char str_crc[8];
		AnalyzerHelpers::GetNumberString(frame.mData1, Hexadecimal, 8,
				str_crc, sizeof(str_crc));
		AddResultString(str_crc);
		break;
	}

	case FRAMETYPE_DATA_CRC_CHECK:
	{
		if (channel != mSettings->mDataChannel0)
			break;
		if (frame.mData1 == 5) {
			AddResultString("BAD CRC");
		} else {
			if (frame.mData1 == 2) {
				AddResultString("GOOD CRC");
			} else {
				AddResultString("UNEXPECTED CRC");
			}
		}
		break;
	}
	default:
		AddResultString("error");
	}
}

void SDMMCAnalyzerResults::GenerateExportFile(const char* file, DisplayBase display_base, U32 export_type_user_id)
{
	std::ofstream file_stream(file, std::ios::out);

	U64 trigger_sample = mAnalyzer->GetTriggerSample();
	U32 sample_rate = mAnalyzer->GetSampleRate();

	U8 lastCmd = 0;

	U64 num_frames = GetNumFrames();
	for  (U32 i = 0; i < num_frames; i++)
	{
		Frame frame = GetFrame(i);

		switch (frame.mType) {
		case FRAMETYPE_COMMAND:
		{
			char str_cmd[4];
			char str_arg[33];

			AnalyzerHelpers::GetNumberString(frame.mData1, Decimal, 6, str_cmd, sizeof(str_cmd));
			AnalyzerHelpers::GetNumberString(frame.mData2, display_base, 32, str_arg, sizeof(str_arg));

			file_stream << "CMD" << str_cmd << "(" << cmd_abbrev_from_number(frame.mData1) << "), arg=" << str_arg << " ";
			break;
		}

		case FRAMETYPE_RESPONSE:
		{
			char str_32[33];

			switch (frame.mFlags) {
			case MMC_RSP_R1:
			{
				const char *str_state = "reserved";
				std::string str_flags("");

				switch ((frame.mData1 >> 9) & 0xf) {
				case 0: str_state = "Idle"; break;
				case 1: str_state = "Ready"; break;
				case 2: str_state = "Ident"; break;
				case 3: str_state = "Stby"; break;
				case 4: str_state = "Tran"; break;
				case 5: str_state = "Data"; break;
				case 6: str_state = "Rcv"; break;
				case 7: str_state = "Prg"; break;
				case 8: str_state = "Dis"; break;
				case 9: str_state = "Btst"; break;
				case 10: str_state = "Slp "; break;
				}

				AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 32, str_32, sizeof(str_32));

				if (frame.mData1 & (1 << 31))
						str_flags += " ADDRESS_OUT_OF_RANGE";
				if (frame.mData1 & (1 << 30))
						str_flags += " ADDRESS_MISALIGN";
				if (frame.mData1 & (1 << 29))
						str_flags += " BLOCK_LEN_ERROR";
				if (frame.mData1 & (1 << 28))
						str_flags += " ERASE_SEQ_ERROR";
				if (frame.mData1 & (1 << 27))
						str_flags += " ERASE_PARAM";
				if (frame.mData1 & (1 << 26))
						str_flags += " WP_VIOLATION";
				if (frame.mData1 & (1 << 25))
						str_flags += " CARD_IS_LOCKED";
				if (frame.mData1 & (1 << 24))
						str_flags += " LOCK_UNLOCK_FAILED";
				if (frame.mData1 & (1 << 23))
						str_flags += " COM_CRC_ERROR";
				if (frame.mData1 & (1 << 22))
						str_flags += " ILLEGAL_COMMAND";
				if (frame.mData1 & (1 << 21))
						str_flags += " CARD_ECC_FAILED";
				if (frame.mData1 & (1 << 20))
						str_flags += " CC_ERROR";
				if (frame.mData1 & (1 << 19))
						str_flags += " ERROR";
				if (frame.mData1 & (1 << 18))
						str_flags += " UNDERRUN";
				if (frame.mData1 & (1 << 17))
						str_flags += " OVERRUN";
				if (frame.mData1 & (1 << 16))
						str_flags += " CID/CSD_OVERWRITE";
				if (frame.mData1 & (1 << 15))
						str_flags += " WP_ERASE_SKIP";
				if (frame.mData1 & (1 << 13))
						str_flags += " ERASE_RESET";
				if (frame.mData1 & (1 << 8))
						str_flags += " READY_FOR_DATA";
				if (frame.mData1 & (1 << 7))
						str_flags += " SWITCH_ERROR";
				if (frame.mData1 & (1 << 5))
						str_flags += " APP_CMD";

				file_stream << "R1, " << str_state << ", rsp=" << str_32;

				if (str_flags.length() > 0)
					file_stream << str_flags.c_str();

				file_stream << " ";

				break;
			}
			case MMC_RSP_R2_CID:
			{
				std::string res("R2");
				char pname[7], prv_str[4], psn_str[12];
				char rsp_str[64];

				res += " [CID]";
				res += " rsp=";
				AnalyzerHelpers::GetNumberString(frame.mData1 >> 32, display_base, 32, rsp_str, sizeof(rsp_str));
				res += rsp_str;
				res += " ";
				AnalyzerHelpers::GetNumberString(frame.mData1 & 0xffffffffull, display_base, 32, rsp_str, sizeof(rsp_str));
				res += rsp_str;
				res += " ";
				AnalyzerHelpers::GetNumberString(frame.mData2 >> 32, display_base, 32, rsp_str, sizeof(rsp_str));
				res += rsp_str;
				res += " ";
				AnalyzerHelpers::GetNumberString(frame.mData2 & 0xffffffffull, display_base, 32, rsp_str, sizeof(rsp_str));
				res += rsp_str;

				pname[0] = (frame.mData1 >> 32) & 0xff;
				pname[1] = (frame.mData1 >> 24) & 0xff;
				pname[2] = (frame.mData1 >> 16) & 0xff;
				pname[3] = (frame.mData1 >>  8) & 0xff;
				pname[4] = (frame.mData1 >>  0) & 0xff;
				pname[5] = (frame.mData2 >> 56) & 0xff;
				pname[6] = 0;

				unsigned prv = (unsigned)((frame.mData2 >> 48) & 0xff);
				prv_str[0] = '0' + ((prv >> 4) & 0x0f);
				prv_str[1] = '.';
				prv_str[2] = '0' + (prv & 0x0f);
				prv_str[3] = 0;

				unsigned psn = (unsigned)((frame.mData2 >> 16) & 0xfffffffful);
				AnalyzerHelpers::GetNumberString(psn, Decimal, 32, psn_str, sizeof(psn_str));

				res += " pnm='";
				res += pname;
				res += "' prv=";
				res += prv_str;
				res += " psn=";
				res += psn_str;
				file_stream << res.c_str() << std::endl;

				break;
			}
			case MMC_RSP_R2_CSD:
			{
				std::string res("R2");
				char rsp_str[64];

				res += " [CSD]";

				res += " rsp=";
				AnalyzerHelpers::GetNumberString(frame.mData1 >> 32, display_base, 32, rsp_str, sizeof(rsp_str));
				res += rsp_str;
				res += " ";
				AnalyzerHelpers::GetNumberString(frame.mData1 & 0xffffffffull, display_base, 32, rsp_str, sizeof(rsp_str));
				res += rsp_str;
				res += " ";
				AnalyzerHelpers::GetNumberString(frame.mData2 >> 32, display_base, 32, rsp_str, sizeof(rsp_str));
				res += rsp_str;
				res += " ";
				AnalyzerHelpers::GetNumberString(frame.mData2 & 0xffffffffull, display_base, 32, rsp_str, sizeof(rsp_str));
				res += rsp_str;
				file_stream << res.c_str() << std::endl;

				break;
			}
			case MMC_RSP_R3:
				AnalyzerHelpers::GetNumberString(frame.mData1, Hexadecimal, 32, str_32, sizeof(str_32));
				file_stream << "R3, ocr=" << str_32 << std::endl;
				break;
			case MMC_RSP_R4:
				file_stream << "R4 ";
				break;
			case MMC_RSP_R5:
				file_stream << "R5 ";
				break;
			}
			break;
		}

		case FRAMETYPE_CRC:
		{
			if (i == 0)
			{
				file_stream << "CRC Error" << std::endl;
				break;
			}

			Frame dFrame = GetFrame(i-1);
			char str_crc[8];

			U8 data[6];
			U8 calcCrc;
			bool crc = 1;

			switch (dFrame.mType) {
			case FRAMETYPE_COMMAND:
			{
				lastCmd = (U8)(dFrame.mData1);
				data[0] = 0x40 + (U8)(dFrame.mData1);
				data[1] = (U8)(dFrame.mData2 >> 24);
				data[2] = (U8)(dFrame.mData2 >> 16);
				data[3] = (U8)(dFrame.mData2 >>  8);
				data[4] = (U8)(dFrame.mData2);

				calcCrc = SDMMCHelpers::crc7(data, 5);
				break;
			}

			case FRAMETYPE_RESPONSE:
			{
				switch (frame.mFlags) {
				case MMC_RSP_R1:
				case MMC_RSP_R4:
				case MMC_RSP_R5:
					data[0] = (U8)(lastCmd);
					data[1] = (U8)(dFrame.mData1 >> 24);
					data[2] = (U8)(dFrame.mData1 >> 16);
					data[3] = (U8)(dFrame.mData1 >>  8);
					data[4] = (U8)(dFrame.mData1);

					calcCrc = SDMMCHelpers::crc7(data, 5);
					break;
				case MMC_RSP_R2_CID:
				case MMC_RSP_R2_CSD:
				case MMC_RSP_R3:
					crc = 0;
					break;
				}

				break;
			}
			}

			if (!crc) break;

			AnalyzerHelpers::GetNumberString(frame.mData1, Hexadecimal, 8, str_crc, sizeof(str_crc));

			file_stream << "CRC=" << str_crc;

			if (calcCrc != (U8)(frame.mData1)) {
				AnalyzerHelpers::GetNumberString(calcCrc, Hexadecimal, 8, str_crc, sizeof(str_crc));
				file_stream << ", ERROR=" << str_crc;
			}
			file_stream << std::endl;
			break;
		}

		case FRAMETYPE_DATA_CONTENTS:
		{
			char str_byte[5];
			AnalyzerHelpers::GetNumberString(frame.mData1,
					Hexadecimal, 8, str_byte,
					sizeof(str_byte));
			file_stream << "DATA_BYTE, " << str_byte << std::endl;
			break;
		}
		case FRAMETYPE_DATA_CRC:
		{
			break;
			// XXX does not work correctly
			char str_crc[20];
			char str_byte8[20];
			char str_byte16[20];
			AnalyzerHelpers::GetNumberString(frame.mData1,
					Hexadecimal, 8, str_byte8,
					sizeof(str_byte8));
			file_stream << "CRC8, " << str_byte8 << std::endl;
			AnalyzerHelpers::GetNumberString(frame.mData1,
					Hexadecimal, 16, str_byte16,
					sizeof(str_byte16));
			file_stream << "CRC16, " << str_byte16 << std::endl;
			AnalyzerHelpers::GetNumberString(frame.mData1,
					Hexadecimal, 64, str_crc,
					sizeof(str_crc));
			file_stream << "DATA_CRC, " << str_crc << std::endl;
			break;
		}
		default:
			file_stream << "error";
		}

		//file_stream << ""; //std::endl;


		// check for cancel flag
		if (UpdateExportProgressAndCheckForCancel(i, num_frames) == true)
		{
			file_stream.close();
			return;
		}
	}
	file_stream.close();
}

void SDMMCAnalyzerResults::GenerateFrameTabularText(U64 frame_index, DisplayBase display_base)
{
	ClearTabularText();
	Frame frame = GetFrame(frame_index);
	if (frame.mType != FRAMETYPE_COMMAND)
		return;
	char str_cmd[33];
	char str_arg[33];
	const char *str_desc;
	AnalyzerHelpers::GetNumberString(frame.mData1, Decimal, 6, str_cmd, sizeof(str_cmd));
	AnalyzerHelpers::GetNumberString(frame.mData2, display_base, 32, str_arg, sizeof(str_arg));
	str_desc = SDMMCHelpers::MMCCommandDescription(frame.mData1, frame.mData2);
	AddTabularText("CMD", str_cmd, ", arg=", str_arg, " ", str_desc);
}

void SDMMCAnalyzerResults::GeneratePacketTabularText(U64 packet_id, DisplayBase display_base)
{
	ClearResultStrings();
	AddResultString("not supported");
}

void SDMMCAnalyzerResults::GenerateTransactionTabularText(U64 transaction_id, DisplayBase display_base)
{
	ClearResultStrings();
	AddResultString("not supported");
}

#include <AnalyzerChannelData.h>
#include "SDMMCAnalyzer.h"
#include "SDMMCAnalyzerResults.h"
#include "SDMMCHelpers.h"

SDMMCAnalyzer::SDMMCAnalyzer()
:	mSimulationInitialized(false)
{
	SetAnalyzerSettings(&mSettings);
}

SDMMCAnalyzer::~SDMMCAnalyzer()
{
	KillThread();
}

const char* SDMMCAnalyzer::GetAnalyzerName() const
{
	return ::GetAnalyzerName();
}

const char* GetAnalyzerName()
{
	return "SDMMC";
}

void SDMMCAnalyzer::WorkerThread()
{
	mClock = GetAnalyzerChannelData(mSettings.mClockChannel);
	mCommand = GetAnalyzerChannelData(mSettings.mCommandChannel);
	if (mSettings.mBusWidth != BUS_WIDTH_0)
		mData0 = GetAnalyzerChannelData(mSettings.mDataChannel0);
	if (mSettings.mBusWidth == BUS_WIDTH_4 ||
			mSettings.mBusWidth == BUS_WIDTH_8) {
		mData1 = GetAnalyzerChannelData(mSettings.mDataChannel1);
		mData2 = GetAnalyzerChannelData(mSettings.mDataChannel2);
		mData3 = GetAnalyzerChannelData(mSettings.mDataChannel3);
	}
	if (mSettings.mBusWidth == BUS_WIDTH_8) {
		mData4 = GetAnalyzerChannelData(mSettings.mDataChannel4);
		mData5 = GetAnalyzerChannelData(mSettings.mDataChannel5);
		mData6 = GetAnalyzerChannelData(mSettings.mDataChannel6);
		mData7 = GetAnalyzerChannelData(mSettings.mDataChannel7);
	}

	while (true) {

		ReportProgress(mClock->GetSampleNumber());
		CheckIfThreadShouldExit();
		// Skip to next command
		AdvanceToNextCommand();
		AdvanceToNextClock();

		// Frame objects used by state machine functions - can't be created
		// there (lifetime)
		Frame respFrame;
		Frame dataFrame;

		// Init CMD & DATA state machines
		CommandReadState cmdState = {
			CMD_INIT, 0, 0, 0, // CMD counters & state
			// RESP counters & state (will be set after CMD has been read)
			MMC_RSP_NONE, 0, 0, 0, 0, 0, 0};
		DataReadState dataState = {DATA_NOTSTARTED, 0, 0, 0, 0, false};

		while (cmdState.phase != CMD_ERROR && dataState.phase != DATA_ERROR &&
				cmdState.phase != CMD_INTERRUPT &&
				!(cmdState.phase == CMD_END &&
					(dataState.phase == DATA_END ||
					 dataState.phase == DATA_NOTSTARTED))) {
			// CMD state machine may start DATA state machine => ref needed
			ReadCommandBit(&cmdState, &dataState, &respFrame);
			ReadDataBit(&dataState, &dataFrame);
			AdvanceToNextClock();
		}
		mResults->CommitResults();
	}
}

bool SDMMCAnalyzer::NeedsRerun()
{
	return false;
}

U32 SDMMCAnalyzer::GenerateSimulationData(U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels)
{
	if (!mSimulationInitialized) {
	mDataGenerator.Initialize(GetSimulationSampleRate(), &mSettings);
	mSimulationInitialized = true;
	}

	return mDataGenerator.GenerateSimulationData(newest_sample_requested, sample_rate, simulation_channels);
}

U32 SDMMCAnalyzer::GetMinimumSampleRateHz()
{
	return 400000 * 4;
}

void SDMMCAnalyzer::SetupResults()
{
	mResults.reset(new SDMMCAnalyzerResults(this, &mSettings));
	SetAnalyzerResults(mResults.get());

	// set which channels will carry bubbles
	mResults->AddChannelBubblesWillAppearOn(mSettings.mCommandChannel);
	if (mSettings.mBusWidth != BUS_WIDTH_0)
		mResults->AddChannelBubblesWillAppearOn(mSettings.mDataChannel0);
}

void SDMMCAnalyzer::AdvanceToNextClock()
{
	enum BitState search = mSettings.mSampleEdge == SAMPLE_EDGE_RISING ? BIT_HIGH : BIT_LOW;

	do {
		mClock->AdvanceToNextEdge();
	} while (mClock->GetBitState() != search);

	mCommand->AdvanceToAbsPosition(mClock->GetSampleNumber());
	if (mSettings.mBusWidth != BUS_WIDTH_0)
		mData0->AdvanceToAbsPosition(mClock->GetSampleNumber());
	if (mSettings.mBusWidth == BUS_WIDTH_4 ||
			mSettings.mBusWidth == BUS_WIDTH_8) {
		mData1->AdvanceToAbsPosition(mClock->GetSampleNumber());
		mData2->AdvanceToAbsPosition(mClock->GetSampleNumber());
		mData3->AdvanceToAbsPosition(mClock->GetSampleNumber());
	}
	if (mSettings.mBusWidth == BUS_WIDTH_8) {
		mData4->AdvanceToAbsPosition(mClock->GetSampleNumber());
		mData5->AdvanceToAbsPosition(mClock->GetSampleNumber());
		mData6->AdvanceToAbsPosition(mClock->GetSampleNumber());
		mData7->AdvanceToAbsPosition(mClock->GetSampleNumber());
	}
}

void SDMMCAnalyzer::AdvanceToNextCommand() {
	mCommand->AdvanceToNextEdge();
	mClock->AdvanceToAbsPosition(mCommand->GetSampleNumber());
	if (mSettings.mBusWidth != BUS_WIDTH_0)
		mData0->AdvanceToAbsPosition(mCommand->GetSampleNumber());
	if (mSettings.mBusWidth == BUS_WIDTH_4 ||
			mSettings.mBusWidth == BUS_WIDTH_8) {
		mData1->AdvanceToAbsPosition(mCommand->GetSampleNumber());
		mData2->AdvanceToAbsPosition(mCommand->GetSampleNumber());
		mData3->AdvanceToAbsPosition(mCommand->GetSampleNumber());
	}
	if (mSettings.mBusWidth == BUS_WIDTH_8) {
		mData4->AdvanceToAbsPosition(mCommand->GetSampleNumber());
		mData5->AdvanceToAbsPosition(mCommand->GetSampleNumber());
		mData6->AdvanceToAbsPosition(mCommand->GetSampleNumber());
		mData7->AdvanceToAbsPosition(mCommand->GetSampleNumber());
	}
	return;
}

void SDMMCAnalyzer::ReadCommandBit(CommandReadState *state, DataReadState
		*dataState, struct Frame *frame) {
	switch(state->phase) {
		case CMD_INIT:
			if (mCommand->GetBitState() == BIT_LOW) {
				mResults->AddMarker(mClock->GetSampleNumber(),
						AnalyzerResults::Start, mSettings.mCommandChannel);
				state->phase = CMD_DIR;
			}
			return;
		case CMD_DIR:
			if (mCommand->GetBitState() != BIT_HIGH) {
				mResults->AddMarker(mClock->GetSampleNumber(),
						AnalyzerResults::X, mSettings.mCommandChannel);
				state->phase = CMD_ERROR;
			} else {
				state->phase = CMD_IDX;
			}
			return;
		case CMD_IDX:
			if (state->cmd_idx_cnt == 0) {
				frame->mStartingSampleInclusive = mClock->GetSampleNumber();
				frame->mData1 = 0;
				frame->mData2 = 0;
				frame->mType = SDMMCAnalyzerResults::FRAMETYPE_COMMAND;
				frame->mFlags = 0;
			}
			frame->mData1 = (frame->mData1 << 1) | mCommand->GetBitState();
			if (state->cmd_idx_cnt == 5)
				state->phase = CMD_ARG;
			state->cmd_idx_cnt++;
			return;
		case CMD_ARG:
			frame->mData2 = (frame->mData2 << 1) | mCommand->GetBitState();
			if (state->cmd_arg_cnt == 31) {
				state->phase = CMD_CRC;
				frame->mEndingSampleInclusive = mClock->GetSampleNumber();
				mResults->AddFrame(*frame);
				mResults->CommitResults();
				state->cmdindex = (int)frame->mData1;
				dataState->cmdindex = (int)frame->mData1;
			}
			state->cmd_arg_cnt++;
			return;
		case CMD_CRC:
			if (state->cmd_crc_cnt == 0) {
				frame->mStartingSampleInclusive = mClock->GetSampleNumber();
				frame->mData1 = 0;
				frame->mData2 = 0;
				frame->mType = SDMMCAnalyzerResults::FRAMETYPE_CRC;
				frame->mFlags = 0;
			}
			frame->mData1 = (frame->mData1 << 1) | mCommand->GetBitState();
			if (state->cmd_crc_cnt == 6) {
				state->phase = CMD_STOP;
				frame->mEndingSampleInclusive = mClock->GetSampleNumber();
				mResults->AddFrame(*frame);
				mResults->CommitResults();
			}
			state->cmd_crc_cnt++;
			return;
		case CMD_STOP:
			{
                        	mResults->AddMarker(mClock->GetSampleNumber(),
                                              AnalyzerResults::Stop, mSettings.mCommandChannel);
				struct MMCResponse response = SDMMCHelpers::MMCCommandResponse(state->cmdindex);
				if (response.mType != MMC_RSP_NONE) {
					state->phase = RESP_INIT;
					// prepare RESP parsing
					state->timeout = response.mTimeout + 3; // add some slack time
					state->resp_data_bits = response.mBits;
					state->responseType = response.mType;
					if (response.hasDataBlock &&
							mSettings.mBusWidth != BUS_WIDTH_0) {
						// init DATA state machine
						dataState->phase = DATA_INIT;
						dataState->hasSeveralDataBlocks = response.hasSeveralDataBlocks;
					}
				} else {
					state->phase = CMD_END;
				}
			}
			return;
		case RESP_INIT:
			if (mCommand->GetBitState() != BIT_LOW) {
				// Waiting for init
				if (state->timeout == 0) {
					state->phase = CMD_ERROR;
					mResults->AddMarker(mClock->GetSampleNumber(),
							AnalyzerResults::X, mSettings.mCommandChannel);
				} else {
					state->timeout--;
				}
			} else {
				mResults->AddMarker(mClock->GetSampleNumber(),
						AnalyzerResults::Start, mSettings.mCommandChannel);
				state->phase = RESP_DIR;
			}
			return;
		case RESP_DIR:
			if (mCommand->GetBitState() != BIT_LOW) {
				/* if card is not transferring this is no response */
				mResults->AddMarker(mClock->GetSampleNumber(),
						AnalyzerResults::X, mSettings.mCommandChannel);
				state->phase = CMD_ERROR;
			} else {
				state->phase = RESP_IGNORED;
			}
			return;
		case RESP_IGNORED:
			if (state->resp_ignore_cnt == 0) {
				frame->mStartingSampleInclusive = mClock->GetSampleNumber();
				frame->mData1 = 0;
				frame->mData2 = 0;
				frame->mType = SDMMCAnalyzerResults::FRAMETYPE_RESPONSE;
				frame->mFlags = state->responseType;
			}
			if (state->resp_ignore_cnt == 5)
				state->phase = RESP_DATA;
			state->resp_ignore_cnt++;
			return;
		case RESP_DATA:
			if (state->resp_data_cnt < 64) {
				frame->mData1 = (frame->mData1 << 1) | mCommand->GetBitState();
				/* bits 0 to 63 */
			} else {
				frame->mData2 = (frame->mData2 << 1) | mCommand->GetBitState();
				/* bits 64 to 127 */
			}
			/* last data bit only */
			if (state->resp_data_cnt == state->resp_data_bits) {
				// observed: no data block following CMD8 R1 response,
				// EXCEPTION_EVENT bit (6), RESERVED (4) and APP-specific (2)
				// bits set - why?
				if (state->responseType == MMC_RSP_R1 &&
						((frame->mData1 & (1 << 6)) != 0)) {
					dataState->phase = DATA_NOTSTARTED;
				}
				frame->mEndingSampleInclusive = mClock->GetSampleNumber();
				mResults->AddFrame(*frame);
				mResults->CommitResults();
				if (state->responseType != MMC_RSP_R2_CID &&
						state->responseType != MMC_RSP_R2_CSD) {
					state->phase = RESP_CRC;
					state->resp_crc_cnt = 0;
				} else {
					state->phase = RESP_STOP;
				}
			}
			state->resp_data_cnt++;
			return;
		case RESP_CRC:
			/* first crc bit only */
			if (state->resp_crc_cnt == 0) {
				frame->mStartingSampleInclusive = mClock->GetSampleNumber();
				frame->mData1 = 0;
				frame->mData2 = 0;
				frame->mType = SDMMCAnalyzerResults::FRAMETYPE_CRC;
			}
			frame->mData1 = (frame->mData1 << 1) | mCommand->GetBitState();

			/* last crc bit only */
			if (state->resp_crc_cnt == 6) {
				frame->mEndingSampleInclusive = mClock->GetSampleNumber();
				mResults->AddFrame(*frame);
				mResults->CommitResults();
				ReportProgress(frame->mEndingSampleInclusive);
				state->phase = RESP_STOP;
			}
			state->resp_crc_cnt++;
			return;
		case RESP_STOP:
			/* stop bit */
			mResults->AddMarker(mClock->GetSampleNumber(), AnalyzerResults::Stop, mSettings.mCommandChannel);
			if (state->cmdindex == 12) {  // STOP_TRANSMISSION
				state->phase = CMD_INTERRUPT;
				return;
			}
			if (dataState->hasSeveralDataBlocks) {
				// READ MULTIPLE BLOCK / WRITE MULTIPLE BLOCK: wait for another
				// command while data is being transmitted
				state->phase = CMD_INIT;
				// Reset state
				state->cmd_idx_cnt = 0;
				state->cmd_arg_cnt = 0;
				state->cmd_crc_cnt = 0;
				state->responseType = MMC_RSP_NONE;
				state->timeout = 0;
				state->cmdindex = 0;
				state->resp_data_bits = 0;
				state->resp_ignore_cnt = 0;
				state->resp_data_cnt = 0;
			} else {
				state->phase = CMD_END;
			}
			return;
		/* this method does not gets called for END and ERROR phases */
		case CMD_ERROR:
		case CMD_END:
			return;
	}

}

void SDMMCAnalyzer::ReadDataBit(DataReadState *state, struct Frame *frame) {
	switch(state->phase) {
		case DATA_INIT:
			if (mData0->GetBitState() == BIT_LOW) {
				/* other data lines must be at 0, too
				 * (depending on bus width (?)) */
				if (	/* BusWidth=4 or 8 */
						mSettings.mBusWidth != BUS_WIDTH_1 &&
						(mData1->GetBitState() != BIT_LOW ||
						 mData2->GetBitState() != BIT_LOW ||
						 mData3->GetBitState() != BIT_LOW) ||
						/* BusWidth=8 */
						mSettings.mBusWidth == BUS_WIDTH_8 &&
						(mData4->GetBitState() != BIT_LOW ||
						 mData5->GetBitState() != BIT_LOW ||
						 mData6->GetBitState() != BIT_LOW ||
						 mData7->GetBitState() != BIT_LOW)) {
					mResults->AddMarker(mClock->GetSampleNumber(),
							AnalyzerResults::X, mSettings.mDataChannel0);
					state->phase = DATA_ERROR;
				} else {
					mResults->AddMarker(mClock->GetSampleNumber(),
							AnalyzerResults::Start, mSettings.mDataChannel0);
					state->phase = DATA_DATA;
				}
			}
			return;
		case DATA_DATA:
			 /* Start frames if necessary*/
			if (mSettings.mBusWidth == BUS_WIDTH_8 ||
					(mSettings.mBusWidth == BUS_WIDTH_4 && state->data_cnt % 2 == 0) ||
					(mSettings.mBusWidth == BUS_WIDTH_1 && state->data_cnt % 8 == 0)) {
				frame->mType = SDMMCAnalyzerResults::FRAMETYPE_DATA_CONTENTS;
				frame->mStartingSampleInclusive = mClock->GetSampleNumber();
				frame->mData1 = 0;
				frame->mData2 = 0;
				frame->mFlags = 0;
			}
			/* Store data */
			if (mSettings.mBusWidth == BUS_WIDTH_8) {
				frame->mData1 = (frame->mData1 << 1) | mData7->GetBitState();
				frame->mData1 = (frame->mData1 << 1) | mData6->GetBitState();
				frame->mData1 = (frame->mData1 << 1) | mData5->GetBitState();
				frame->mData1 = (frame->mData1 << 1) | mData4->GetBitState();
			}
			if (mSettings.mBusWidth == BUS_WIDTH_8 ||
					mSettings.mBusWidth == BUS_WIDTH_4) {
				frame->mData1 = (frame->mData1 << 1) | mData3->GetBitState();
				frame->mData1 = (frame->mData1 << 1) | mData2->GetBitState();
				frame->mData1 = (frame->mData1 << 1) | mData1->GetBitState();
			}
			frame->mData1 = (frame->mData1 << 1) | mData0->GetBitState();
			/* Commit results if necessary */
			if (mSettings.mBusWidth == BUS_WIDTH_8 ||
					(mSettings.mBusWidth == BUS_WIDTH_4 && state->data_cnt % 2 == 1) ||
					(mSettings.mBusWidth == BUS_WIDTH_1 && state->data_cnt % 8 == 7)
					) {
				frame->mEndingSampleInclusive = mClock->GetSampleNumber();
				mResults->AddFrame(*frame);
				mResults->CommitResults();
			}
			/* last data bit only - change state */
			if (state->data_cnt == 1023) {
				/* FIXME do not used fixed block width */
				state->phase = DATA_CRC;
			}
			state->data_cnt++;
			return;
		case DATA_CRC:
			if (state->crc_cnt == 0) {
				frame->mType = SDMMCAnalyzerResults::FRAMETYPE_DATA_CRC;
				frame->mStartingSampleInclusive = mClock->GetSampleNumber();
				frame->mData1 = 0;
				frame->mData2 = 0;
				frame->mFlags = 0;
			}
			frame->mData1 <<= 1;
			frame->mData1 |= (mData0->GetBitState());
			if (mSettings.mBusWidth != BUS_WIDTH_1) { /* BusWidth=4 or 8 */
				frame->mData1 |= (static_cast<U64>(mData1->GetBitState()) << 16);
				frame->mData1 |= (static_cast<U64>(mData2->GetBitState()) << 32);
				frame->mData1 |= (static_cast<U64>(mData3->GetBitState()) << 48);
			}
			if (mSettings.mBusWidth == BUS_WIDTH_8) { /* BusWidth=8 */
				frame->mData2 |= (static_cast<U64>(mData4->GetBitState()) << 16);
				frame->mData2 |= (static_cast<U64>(mData5->GetBitState()) << 16);
				frame->mData2 |= (static_cast<U64>(mData6->GetBitState()) << 32);
				frame->mData2 |= (static_cast<U64>(mData7->GetBitState()) << 48);
			}
			if (state->crc_cnt == 15) {
				frame->mEndingSampleInclusive = mClock->GetSampleNumber();
				mResults->AddFrame(*frame);
				mResults->CommitResults();
				state->phase = DATA_STOP;
			}
			state->crc_cnt++;
			return;
		case DATA_STOP:
			if (	/* BusWidth=4 or 8 */
					mSettings.mBusWidth != BUS_WIDTH_1 &&
					(mData0->GetBitState() == BIT_LOW ||
					 mData1->GetBitState() == BIT_LOW ||
					 mData2->GetBitState() == BIT_LOW ||
					 mData3->GetBitState() == BIT_LOW) ||
					/* BusWidth=8 */
					mSettings.mBusWidth == BUS_WIDTH_8 &&
					(mData4->GetBitState() == BIT_LOW ||
					 mData5->GetBitState() == BIT_LOW ||
					 mData6->GetBitState() == BIT_LOW ||
					 mData7->GetBitState() == BIT_LOW)) {
				mResults->AddMarker(mClock->GetSampleNumber(),
						AnalyzerResults::X, mSettings.mDataChannel0);
				state->phase = DATA_ERROR;
			} else {
				mResults->AddMarker(mClock->GetSampleNumber(),
						AnalyzerResults::Stop, mSettings.mDataChannel0);
				if (state->cmdindex == 24 || state->cmdindex == 25) {
					// Write commands
					state->phase = DATA_CHECKCRC_INIT;
				} else {
					if (state->hasSeveralDataBlocks) {
						// FIXME take CMD23 (SET BLOCK COUNT) into account, stop if
						// necessary
						state->data_cnt = 0;
						state->crc_cnt = 0;
						state->checkcrc_cnt = 0;
						state->phase = DATA_INIT;
					} else {
						state->phase = DATA_END;
					}
				}
			}
		case DATA_CHECKCRC_INIT:
			if (mData0->GetBitState() == BIT_LOW) {
				state->phase = DATA_CHECKCRC_CRC;
				mResults->AddMarker(mClock->GetSampleNumber(),
						AnalyzerResults::Start, mSettings.mDataChannel0);
			}
			return;
		case DATA_CHECKCRC_CRC:
			// 010 -> OK; 101 -> NOK
			if (state->checkcrc_cnt == 0) {
				frame->mStartingSampleInclusive = mClock->GetSampleNumber();
				frame->mData1 = 0;
				frame->mData2 = 0;
				frame->mType = SDMMCAnalyzerResults::FRAMETYPE_DATA_CRC_CHECK;
			}
			frame->mData1 = (frame->mData1 << 1) | mData0->GetBitState();
			if (state->checkcrc_cnt == 2) {
				state->phase = DATA_CHECKCRC_STOP;
				frame->mEndingSampleInclusive = mClock->GetSampleNumber();
				mResults->AddFrame(*frame);
				mResults->CommitResults();
				ReportProgress(frame->mEndingSampleInclusive);
			}
			state->checkcrc_cnt++;
			return;
		case DATA_CHECKCRC_STOP:
			mResults->AddMarker(mClock->GetSampleNumber(),
					AnalyzerResults::Stop, mSettings.mDataChannel0);
			state->phase = DATA_BUSY;
			return;
		case DATA_BUSY:
			if (mData0->GetBitState() == BIT_HIGH) {
				state->phase = DATA_BUSY_END;
			}
			return;
		case DATA_BUSY_END:
			if (state->hasSeveralDataBlocks) {
				state->phase = DATA_INIT;
			} else {
				state->phase = DATA_END;
			}
			return;
			/* this method does not gets called for NOTSTARTED, END and ERROR
			 * phases */
		case DATA_NOTSTARTED:
		case DATA_ERROR:
		case DATA_END:
			return;
	}
}
/*
 * loader hooks
 */

Analyzer* CreateAnalyzer()
{
	return new SDMMCAnalyzer();
}

void DestroyAnalyzer(Analyzer* analyzer)
{
	delete analyzer;
}

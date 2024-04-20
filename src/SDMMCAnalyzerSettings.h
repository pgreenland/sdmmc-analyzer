#ifndef __SDMMC_ANALYZER_SETTINGS
#define __SDMMC_ANALYZER_SETTINGS

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

enum SDMMCProtocol {
	PROTOCOL_MMC,
	PROTOCOL_SD,
};

enum SDMMCBusWidth {
    BUS_WIDTH_0 = 0,
    BUS_WIDTH_1 = 1,
    BUS_WIDTH_4 = 4,
    BUS_WIDTH_8 = 8,
};

enum SDMMCSampleEdge {
	SAMPLE_EDGE_RISING,
	SAMPLE_EDGE_FALLING
};

class SDMMCAnalyzerSettings : public AnalyzerSettings
{
public:
	SDMMCAnalyzerSettings();
	virtual ~SDMMCAnalyzerSettings();

	virtual bool SetSettingsFromInterfaces();
	void UpdateInterfacesFromSettings();

	virtual void LoadSettings(const char *settings);
	virtual const char *SaveSettings();

	Channel mClockChannel;
	Channel mCommandChannel;
	Channel mDataChannel0;
	Channel mDataChannel1;
	Channel mDataChannel2;
	Channel mDataChannel3;
	Channel mDataChannel4;
	Channel mDataChannel5;
	Channel mDataChannel6;
	Channel mDataChannel7;
	enum SDMMCProtocol mProtocol;
	enum SDMMCBusWidth mBusWidth;
	enum SDMMCSampleEdge mSampleEdge;

protected:
	std::unique_ptr<AnalyzerSettingInterfaceChannel> mClockChannelInterface;
	std::unique_ptr<AnalyzerSettingInterfaceChannel> mCommandChannelInterface;
	std::unique_ptr<AnalyzerSettingInterfaceChannel> mDataChannelInterface0;
	std::unique_ptr<AnalyzerSettingInterfaceChannel> mDataChannelInterface1;
	std::unique_ptr<AnalyzerSettingInterfaceChannel> mDataChannelInterface2;
	std::unique_ptr<AnalyzerSettingInterfaceChannel> mDataChannelInterface3;
	std::unique_ptr<AnalyzerSettingInterfaceChannel> mDataChannelInterface4;
	std::unique_ptr<AnalyzerSettingInterfaceChannel> mDataChannelInterface5;
	std::unique_ptr<AnalyzerSettingInterfaceChannel> mDataChannelInterface6;
	std::unique_ptr<AnalyzerSettingInterfaceChannel> mDataChannelInterface7;
	std::unique_ptr<AnalyzerSettingInterfaceNumberList> mProtocolInterface;
	std::unique_ptr<AnalyzerSettingInterfaceNumberList> mBusWidthInterface;
	std::unique_ptr<AnalyzerSettingInterfaceNumberList> mSampleEdgeInterface;
};

#endif

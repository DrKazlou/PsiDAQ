#ifndef FUNCMAIN_H
#define FUNCMAIN_H

#include <vector>
#include <TH1D.h>
#include <TH2D.h>
#include <TCanvas.h>
#include <TStopwatch.h>
#include <TGButton.h>
#include <TGNumberEntry.h>
#include <TGLabel.h>
#include <TRandom3.h>

#include <TStyle.h>

#include "N957Lib.h"
#include "N957oslib.h"
#include "N957types.h"

#include "CAENDPPLib.h"
#include "CAENDPPLibTypes.h"

#include "CAENDigitizer.h"
#include "CAENDigitizerType.h"

#define MAX_CH 16
#define BIN_WIDTH_DT5770 6.4
#define BIN_WIDTH_DT5720 4
#define BIN_WIDTH_V1730 2

//DT5770 part
// The maximum number of bits for the histograms
// 14 bits ->from bin 0 to bin 16383 (2^14-1)
#define MAX_HISTOBITS (14)

// The maximum number of bins of the histograms
// (depending on their maximum number of bits)
#define MAX_HISTO_NBINS (0x1 << (MAX_HISTOBITS-0))

#define AUTOSET_VALUE       (-1)    // The value used for Autoset of parameters


enum class DeviceType{Simulated, N957, DT5770, DT5720, V1730};


struct DAC_Calibration_data{
	float cal[2];
	float offset[2];
};


struct N957MCAConfig{
	int name;
	short int port;
	N957_BYTE thr;
	N957_UINT16 BLDIM;
	N957ControlModes ControlMode;
		
	N957MCAConfig() : name(957), port(0), thr(30), BLDIM(32768), ControlMode (N957ControlModeAuto){}
};


struct DT5770DigiConfig{
	int name;
	CAENDPP_ConnectionParams_t connParam;
	CAENDPP_ConnectionType type; // it's correct; they mistype in own headers
	CAENDPP_AcqMode_t acqMode;
	CAENDPP_DgtzParams_t DParams;
	CAENDPP_InputRange_t InputRange;
	
	uint32_t ns; // number of samples
	//double temp;
	int32_t BID; // board ID
	
	
	CAENDPP_Info_t info;
	
	DT5770DigiConfig() : name(5770) {}
};	

struct DT5720DigiConfig{
	int name;
	int Nch;
	int Nbit;
	int port;
	uint32_t RecordLength[2];
	int PostTrigger;
	int NumEvents = 1; // for SetMaxNumEventsBLT()
	
	CAEN_DGTZ_IOLevel_t FPIOtype;
    CAEN_DGTZ_TriggerMode_t ExtTriggerMode;
    uint16_t EnableMask;
	CAEN_DGTZ_TriggerMode_t ChannelTriggerMode[2];
	CAEN_DGTZ_TriggerPolarity_t PulsePolarity[2];
	uint32_t DCOffset[2];
	//uint32_t Threshold[2];// what is that threshold??
	uint32_t cal_thr[2]; // calibrated threshold
	uint32_t thr[2]; // threshold from gui
	
	uint32_t FTDCOffset[2];
	uint32_t FTThreshold[2];
	CAEN_DGTZ_TriggerMode_t	FastTriggerMode;
	CAEN_DGTZ_EnaDis_t	 FastTriggerEnabled;
	
	DAC_Calibration_data DAC_Calib; // some shite for calibration but i will try finally to cut it
	
	DT5720DigiConfig() : name(5720), Nch(2), Nbit(12), port(0), PostTrigger(80), NumEvents(1),
                         FPIOtype(CAEN_DGTZ_IOLevel_NIM), ExtTriggerMode(CAEN_DGTZ_TRGMODE_ACQ_ONLY),
                         EnableMask(0x3), FastTriggerMode(CAEN_DGTZ_TRGMODE_ACQ_ONLY),
                         FastTriggerEnabled(CAEN_DGTZ_DISABLE) {
        RecordLength[0] = 100;
        RecordLength[1] = 100;
        ChannelTriggerMode[0] = CAEN_DGTZ_TRGMODE_ACQ_ONLY;
        ChannelTriggerMode[1] = CAEN_DGTZ_TRGMODE_ACQ_ONLY;
        PulsePolarity[0] = CAEN_DGTZ_TriggerOnRisingEdge;
        PulsePolarity[1] = CAEN_DGTZ_TriggerOnRisingEdge;
        DCOffset[0] = 0;
        DCOffset[1] = 0;
        //Threshold[0] = 200;
        //Threshold[1] = 200;
        cal_thr[0] = 0;
        cal_thr[1] = 0;
        thr[0] = 200;
        thr[1] = 200;
        FTDCOffset[0] = 0;
        FTDCOffset[1] = 0;
        FTThreshold[0] = 0;
        FTThreshold[1] = 0;
    }
};

typedef enum{
	InputRange_2Vpp = 0,
	InputRange_0_5Vpp = 1
}CAEN_DGTZ_InputRange_t;

struct V1730DigiConfig{
    
	int name;
	uint32_t PID;
    int Nch;
	int NB;
	bool IsMaster;
	
	uint32_t ChannelMask; // Bitmask for enabled channels (b15:0)
	int EventAggr;
	CAEN_DGTZ_DPP_AcqMode_t AcqMode;
	CAEN_DGTZ_IOLevel_t IOlev;
	
    std::vector<CAEN_DGTZ_InputRange_t> InputRange; // Input range per channel
    std::vector<CAEN_DGTZ_PulsePolarity_t> PulsePolarity; // Polarity per channel
    std::vector<uint32_t> RecordLength; // Samples per channel
    std::vector<uint32_t> PreTrigger; // Pre-trigger samples
    std::vector<uint32_t> DCOffset; // DC offset (0x0000 to 0xFFFF)
    std::vector<uint32_t> thr; // Threshold
    std::vector<uint32_t> nsbl; // Baseline samples (0, 16, 64, 256, 1024)
    std::vector<uint32_t> lgate; // Long gate width (samples)
    std::vector<uint32_t> sgate; // Short gate width (samples)
    std::vector<uint32_t> pgate; // Pre-gate offset (samples)
    std::vector<uint32_t> selft; // Self-trigger enable (0 or 1)
	std::vector<CAEN_DGTZ_DPP_TriggerConfig_t>trgc; // Smth that was switch off
    std::vector<CAEN_DGTZ_DPP_Discrimination_t> discr; // Discriminator mode (LED or CFD) but it was uint32_t
    std::vector<uint32_t> cfdd; // CFD delay (samples)
    std::vector<uint32_t> cfdf; // CFD fraction (0: 25%, 1: 50%, 2: 75%, 3: 100%)
    std::vector<uint32_t> tvaw; // Trigger validation window (samples)
    std::vector<uint32_t> csens; // Charge sensitivity (0: 5fC, 1: 20fC, etc.)
	
	//Pile-Up rejection Mode
    //CAEN_DGTZ_DPP_PSD_PUR_DetectOnly -> Only Detect Pile-Up
    //CAEN_DGTZ_DPP_PSD_PUR_Enabled -> Reject Pile-Up 
	CAEN_DGTZ_DPP_PUR_t purh; 
	int purgap; // Purity Gap in LSB units (1LSB = 0.12 mV for 2Vpp Input Range, 1LSB = 0.03 mV for 0.5 Vpp Input Range )
	int blthr;  // Baseline Threshold
	int  bltmo; //not used in example but exist in structure of CAEN_DGTZ_DPP_PSD_Params_t
	int trgho;  //in 8  ns steps - !!!BULLSHIT here value already in ns!!! Trigger HoldOff
	
    uint32_t SerialNumber; // Board serial number
	
	V1730DigiConfig() : name(1730), PID(15103), Nch(16), NB(1), IsMaster(true), ChannelMask(0xFFFF), EventAggr(1), AcqMode(CAEN_DGTZ_DPP_ACQ_MODE_Mixed), IOlev(CAEN_DGTZ_IOLevel_NIM),
						purh(CAEN_DGTZ_DPP_PSD_PUR_DetectOnly), 
						purgap(100), blthr(3), trgho(400), SerialNumber(0) {
        InputRange.resize(16, InputRange_0_5Vpp);
        PulsePolarity.resize(16, CAEN_DGTZ_PulsePolarityNegative);
        RecordLength.resize(16, 500);
        PreTrigger.resize(16, 50);
        DCOffset.resize(16, 0x199A);
        thr.resize(16, 100);
        nsbl.resize(16, 1); // number samples for the baseline averaging: {0 - absolute, 1 - 16 sampl, 2 - 64, 3 - 256, 4 - 1024}
        lgate.resize(16, 500); // in bins
        sgate.resize(16, 124); // in bins
        pgate.resize(16, 8);   // in bins
        selft.resize(16, 1);
        trgc.resize(16, CAEN_DGTZ_DPP_TriggerConfig_Threshold); // Threshold | Peak, have no idea what is it
        discr.resize(16, CAEN_DGTZ_DPP_DISCR_MODE_LED);
        cfdd.resize(16, 4); 
        cfdf.resize(16, 1);
        tvaw.resize(16, 16);
        csens.resize(16, 0);
    }
};


struct SignalParameters {
    Int_t polarity;        // Polarity: +1 or -1
    Double_t sampleWidth;  // Width of one sample (ns)
    Int_t nSamples;        // Number of samples
    Double_t tauRise;      // Rising edge time constant (ns)
    Double_t tauDecay;     // Falling edge time constant (ns)
    Int_t distMode;        // Distribution mode (0-6)
    Int_t frequency;       // Signal generation frequency (Hz)
};

// event structure for Queue

struct ProcessedEvent { 
	int channel;
	int board;
    std::vector<Double_t> trace;
// Mainly for DT5770 VirtualProbe
	std::vector<int16_t> probe[3];
// only for V1730S
	int16_t charge; 
// for DT5770 internal MCA histogram
	std::vector<uint32_t> det; 
    uint32_t timestamp;
    uint32_t eventCounter;
// 1 for positive, -1 for negative	
    int polarity; 
};

class WindowMain;



///////////////////
void SetIcon(TGMainFrame *fMain, TString filename);
bool ParseConfigFile(const std::string& filename, DeviceType AcqMode, N957MCAConfig *Ncfg, DT5770DigiConfig *Scfg, DT5720DigiConfig *Dcfg, V1730DigiConfig *Vcfg, Bool_t debugEnabled);

void CalcParams(std::vector<Double_t>& signal, Int_t polarity, 
                Int_t psdBin, TH1D* h_ampl, TH1D* h_integral, TH1D* h_psd,
                TH2D* h_psd_ampl, TH2D* h_psd_int);

void FillHistogram(const std::vector<Double_t>& data, TH1D* h_ampl);
void SetHistogram(const std::vector<Double_t>& data, TH1D* h_ampl);

//void SetDefaultConfiguration(DT5720DigiConfig* Dcfg);

CAEN_DGTZ_ErrorCode QuitMain(int handle, char* buffer);

CAEN_DGTZ_ErrorCode Set_relative_Threshold(int handle, DT5720DigiConfig* Dcfg);

void BaseLineCut(TH1D* hist, int BL_CUT);

void FillHitPattern(const std::vector<ProcessedEvent>& events, TH2D* h_xy, uint64_t timeWindow);

// !! Finally change that fucking bullshite to some structure !!
void UpdateCanvas(TCanvas* canvas, TStopwatch& stopwatch, TGLabel* TimingLabel[], double TimingValue[], Bool_t checkVP[4],
                  Bool_t checkStates[12], Bool_t radioStates[12], Bool_t byChannel,
                  TH1D* h_trace[2][4][MAX_CH], TH1D* h_ampl[2][MAX_CH], TH1D* h_integral[2][MAX_CH], TH1D* h_charge[2][MAX_CH], TH1D* h_psd[2][MAX_CH],
                  TH2D* h_psd_ampl[2][MAX_CH], TH2D* h_psd_int[2][MAX_CH], TH1D* h_det, TH1D* h_counts, TH1D* h_layers, TH2D* h_xy, int Board, int Ch2D, Bool_t debugEnabled, Bool_t showStats, Bool_t& canvasNeedsUpdate,
                  std::vector<bool>& drawChannels);
void GenerateAndQueueSignal(WindowMain* gui, TRandom3& rand);

CAEN_DGTZ_ErrorCode  SwitchOffLogic(int handle, int N_CH);
CAEN_DGTZ_ErrorCode  SetLogic(int32_t handle, uint32_t reg_val[2][8], int N_CH);


#endif
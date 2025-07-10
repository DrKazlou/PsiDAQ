#ifndef DT5770Params_H
#define DT5770Params_H

#include "FuncMain.h"
#include "DigitizerClass.h"
#include <TGFrame.h>
#include <TGTab.h>
#include <TGButton.h>
#include <TGTextEntry.h>
#include <TGComboBox.h>
#include <TGLabel.h>
#include <TGNumberEntry.h>
#include <RQ_OBJECT.h>

//#include <stdlib.h>

class WindowMain;

class DT5770Params : public TGMainFrame{
	
private:
	TGMainFrame     *fMain;
	WindowMain* fParent; 
	TGCompositeFrame     *f1;
   
   //TGTextButton      *fSetButton;
   
	
   TGGroupFrame *fGF[16];	
	
	TGVerticalFrame    *fVF0, *fVF[16];
	
	TGHorizontalFrame    *fHF_WF[8], *fHF_StD[3], *fHF_DPP[15], *fHF_Reset[5], *fHF_Extra[8];
	
	TGCheckButton *fC[16], *fCselft[16];	
	TGComboBox *fProbe[4], *fProbeTrigger, *fPolarity, *fInputRange, *fRDMode, *fImpedance;
	
   TGLabel *fWFLabel[8], *fStDLabel[3], *fDPPLabel[15], *fResetLabel[5], *fExtraLabel[8];
   
   TGNumberEntry        *fNumericEntries[5][15];
   
	TGTextEntry        *fTEntries[16][16];
	TGTextBuffer *tbuf[16][16];
	
	Digitizer *fDigitizer; 
    DT5770DigiConfig *fScfg; 
	
	const char titlelabel[5][15] = {"WaveFormParams", "Standart", "DPP Params", "Reset Detector", "Extra Params"};
	const char wf_label[8][20] = {"vp1", "vp2", "dp1", "dp2", "WFLength", "PreTrigger", "probeTrigger", "probeSelfTrigger"};		
	const char std_label[3][13] = {"DCOffset", "Polarity", "InputRange"};		
	const char dpp_label[15][45] = {"Signal Decay Time Constant", "Trapezoid Flat Top", "Trapezoid Rise Time", "Flat Top Delay", "Trigger Filter smoothing factor", "Input Signal Rise time", "Trigger Threshold", 
														"number of samples for baseline mean", "number of samples for peak mean calculation", "peak hold off", "base line hold off", "digital probe gain", "Energy normalization factor", "Decimation of input signal", "Trigger hold off"};	
	const char reset_label[5][20] = {"ResetDetectionMode", "Enabled", "thrshold", "reslenmin", "reslength"};	
	const char extra_label[8][40] = {"InputImpedance", "Continuous reset analog gain", "Pulsed reset analog gain", "SaturationHoldoff", "energyFilterMode", "trigger fast trapezoid rising time", "trigger fast trapezoid flat top", "trigMODE"};	
	
	const char List_vp1[9][20] = {"Input", "Delta", "Delta2", "Trapezoid", "FastTrap", "TrapBaseLine", "EnergyOut", "TrapBLCorr", "None"};
	const char List_vp2[10][20] = {"Input", "S3", "TrapBLCorr", "TrapBaseLine",  "None", "Delta", "FastTrap", "Delta2", "Trapezoid", "EnergyOut"};
	const char List_dp1[21][20] = {"TrgWin", "Armed", "PkRun", "PURFlag", "Peaking", "TVAW", "BLHoldoff", "TRGHoldoff", "TGVal", "ACQVeto", "BFMVeto", "ExtTRG", "Trigger", "None", "EnergyAccepted", "Saturation", "Reset", "BLFreeze", "Busy", "PrgVeto", "Inhibit"};
	const char List_dp2[8][20] = {"Trigger", "None", "Peaking", "BLHoldoff",  "PURFlag", "EnergyAccepted", "Saturation", "Reset"};
	const char List_probeTrigger[8][30] = {"MainTrig", "MainTriggerDelayCompensated", "MainTriggerAcceptedPulse", "SelfTriggerMUX1",  "SelfTriggerMUX2", "BaselineRestorer", "ResetDetector", "FreeRunning"};
	
	CAENDPP_InputRange_t InputRange_val[4] = {CAENDPP_InputRange_1_25Vpp, CAENDPP_InputRange_2_5Vpp, CAENDPP_InputRange_5_0Vpp, CAENDPP_InputRange_10_0Vpp};
	const char List_inputRange[4][10] = {"1.25 Vpp", "2.5 Vpp", "5 Vpp", "10 Vpp"};
	
public:
	DT5770Params(const TGWindow *p, WindowMain *parent, Digitizer *digi, DT5770DigiConfig *Scfg);
	virtual ~DT5770Params();
	
	const char *sFont = "-Ubuntu-bold-r-*-*-16-*-*-*-*-*-iso8859-1";
	const char *gfFont = "-Ubuntu-bold-r-*-*-14-*-*-*-*-*-iso8859-1";
	const char *labelFont = "-Ubuntu-bold-r-*-*-13-*-*-*-*-*-iso8859-1";
	const char *paramFont = "-adobe-courier-bold-r-*-*-15-*-*-*-*-*-iso8859-1";
	
	
   // slots
   void TryToClose();
   void CloseWindow();
   void DoClose();
   void DoSetVal();
   void DoCheckBox();
   void DoComboBox();
   
	ClassDef(DT5770Params, 1);	
};

#endif
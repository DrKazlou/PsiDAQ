#ifndef WINDOWMAIN_H
#define WINDOWMAIN_H

#include "FuncMain.h"
#include "DigitizerClass.h"
#include "ParamsMenu.h"
#include "DT5770Params.h"
#include "LogicMenu.h"

#include <TObject.h>
#include <TFile.h>
#include <TTree.h>
#include <TGFileDialog.h>
#include <TArrayL.h>

#include <TGFrame.h>
#include <TRootEmbeddedCanvas.h>
#include <TGButton.h>
#include <TGTab.h>
#include <TGNumberEntry.h>
#include <TGComboBox.h>
#include <TGLabel.h>
#include <TGMenu.h>
#include <TGMsgBox.h>
#include <TGFont.h>
#include <TTimer.h>
#include <TStopwatch.h>
#include <TThread.h>
#include <TRandom3.h>
#include <TH1D.h>
#include <TH2D.h>
#include <Rtypes.h>
#include <TSystem.h>
#include <vector>
#include <mutex>
#include <queue>
#include <thread>
#include <condition_variable>
#include <atomic>


class WindowGenerator;


class WindowMain : public TObject {
private:
    friend class WindowGenerator;
    TGMainFrame* fMain;          
	TGHorizontalFrame* hFrame;
	TGHorizontalFrame *fHFrameEnableCH, *fHFrameSelfTrg;
	TGHorizontalFrame* fHFrameThr;
	TGHorizontalFrame* fHFrameDCOffset;
	
	TGVerticalFrame* vFrameRight;
	TGVerticalFrame* vFrameLeft;
	TGVerticalFrame* vFrameDigi;
    TRootEmbeddedCanvas* fEmbCanvas; 
    TCanvas* fCanvas;           
    TH1D* h_trace[2][4][MAX_CH];               
    TH1D* h_ampl[2][MAX_CH];     
    TH1D* h_integral[2][MAX_CH]; 
	TH1D* h_charge[2][MAX_CH];   
    TH1D* h_psd[2][MAX_CH];      
    TH2D* h_psd_ampl[2][MAX_CH]; 
    TH2D* h_psd_int[2][MAX_CH];  
	TH1D* h_det;               
	TH1D* h_counts;            
	TH1D* h_layers;            
	TH2D* h_xy;               
	
	TGNumberEntry *fNEMain[5];//, *fNEAxisRange[2][2];
	TGLabel* fNEMainLabel[5];

		   
	TGLabel* fLabelThr;              // Label for DT5720, N957 threshold 
	TGLabel* fLabelDCOffset;		 // Label for DT5720 DCOffset
	
	TGCheckButton *fCheckSelfTrg[2];
	TGCheckButton *fCheckCoincidence;
	//TGCheckButton *fCheckLog[3];
	TGNumberEntry* fNumEntryThr[2];    // Number entry for DT5720 / N957 thresholds 
	TGNumberEntry* fNumEntryDCOffset[2];    // Number entry for DT5720 DCOffset
	TGCheckButton* fCheckEnableCH[MAX_CH];  //Enable channel checkboxes
	TGCheckButton* fCVP[4];  //checkboxes for DT5770 Virtual Probes
	
	TGLabel *fLabelV1730AcqMode, *fTempLabel[2][MAX_CH/2];
	TGComboBox* fComboV1730AcqMode;
		
    TGCheckButton* fCBHistoTypes[12];
	TGRadioButton* fRBHistoTypes[12];
	Bool_t fCheckVP[4], fCheckStates[12], fRadioStates[12];
	TGTab*	fTab;
	Bool_t fDrawingStyle;
	
	std::vector<bool> fDrawChannels;
	
    //TGLabel* fLabelTime;         // Label for UpdateCanvas timing
    TGMenuBar* fMenuBar;         
    TGPopupMenu* fMenuFile;      
    TGPopupMenu* fMenuOptions;   
    TTimer	*fDrawTimer, *fDAQTimer;              // Timer for periodic updates
    TStopwatch fStopwatch;       // Stopwatch for timing UpdateCanvas
    WindowGenerator* fSignalDialog; // Pointer to signal config dialog
	
    Bool_t fIsRunning;           // Flag to control drawing
    Bool_t fIsClosing;           // Flag to prevent multiple CloseWindow calls
	Bool_t fInit;                // Flag to track initialization
	Bool_t fCanvasNeedsUpdate;	 //Flag to track if canvas layout needs updating
	
	TGLabel* fTrgRateLabel[2];    
    TTimer* fCountRateTimer;     // Timer for updating count rate label
	Int_t fSignalCount[2][MAX_CH], fAbsCount[2][MAX_CH];          // Count of signals for count rate
	uint32_t fDataCount[2]; 		 // for data transfer rate
    TStopwatch fCountRateStopwatch; // Stopwatch for count rate timing
		
	SignalParameters fParams;    // Signal parameters
    Int_t fUpdateInterval;            // Drawing interval in ms
    Int_t fPSDbin;               // PSD bin offset
	Int_t fBoard;               // board to draw histograms
	Int_t fCh2D;               // channel to draw TH2D histogram
	Int_t fTimerValue;               // channel to draw TH2D histogram
	TGCheckButton *fCTimer;
	
	int fSelectedType;
    int fSelectedChan;
	
    ParamsMenu *fParamsDialog;
	DT5770Params *fDT5770ParamsDialog;
	LogicMenu *fLogicDialog;
	TGTransientFrame* SHTdialog;
	uint32_t fLogVal[2][2][8]; // Array to store logic values
    	
    TRandom3 fRand;              // Random number generator for signal generation
	TStopwatch fElapsedTimer;    // Stopwatch for elapsed time since Start
    TGLabel* fLabelElapsedTime;  // Label for elapsed time
	TString fStatistics;
	
	double fTimingValue[5];
	TGLabel* fTimingLabel[5];
	
	
	
    TGCheckButton* fCheckDebug;  // Checkbox for enabling/disabling debug output
	
    //Bool_t fCanvasInteractive; // State of Canvas Interaction checkbox
    TGCheckButton* fCheckShowStats;    // Checkbox for showing stats
    TGCheckButton* fCheckCanvasInteractive; // Checkbox for canvas interaction
			
	Digitizer* fDigitizer[2];   // one class to rule them all
		
	DeviceType fDeviceType;
    TGComboBox* fCBDeviceType; // Select Simulated or N957
	
    TGTextButton* fButtonInit, *fButtonStart, *fButtonStop, *fButtonClear, *fButtonTemp;  
	
	//ROOT TTree data saving
	TGLabel* fLabelSaveRawData; 
	TGTextEntry* fTextEntryFileName; 
    std::string fOutputFileName; 
    TGCheckButton* fCheckSaveRawData;
    Bool_t fSaveRawData;
	    
        
    TFile* fRootFile;
    TTree* fTree;

    // Data structure for TTree branches
    struct RawDataEvent {
        uint32_t eventCounter;
        int channel;
		int board;
        uint32_t timestamp;
        std::vector<uint16_t> trace;
		int16_t charge;
    };
		
	// Thread-safe queue for signals
    
    mutable std::mutex fQueueMutex; // Mutex for thread-safe queue access
	std::mutex fDataQueueMutex;
	std::mutex fRootFileMutex; // New mutex to protect fRootFile and fTree
	std::mutex fHistMutex;       // Mutex for thread-safe histogram access
	std::mutex fAnalysisQueueMutex; 
	std::mutex fCoincidenceQueueMutex; 
	std::mutex fDigitizerMutex;
    
	std::atomic<size_t> fDroppedSignals; // Counter for dropped signals
	std::atomic<bool> fSignalRunning; // Flag to control signal generation thread
	std::atomic<bool> fStopWriting;
	
    std::thread fSignalThread;   // Thread for signal generation
    std::thread fDataThread;     // Thread for data acquisition
	std::thread fWriteThread;
	std::thread fAcquisitionThread; 
    std::thread fAnalysisThread;	
	std::thread fCoincidenceThread;	
	
	std::queue<std::vector<Double_t>> fSignalQueue; // Signal and peak_pos
    std::queue<RawDataEvent> fDataQueue;
	//std::queue<ProcessedEvent> fAnalysisQueue; 
	std::queue<std::vector<ProcessedEvent>> fAnalysisQueue; 
	std::queue<std::vector<ProcessedEvent>> fCoincidenceQueue; 
    
	
    std::condition_variable fDataQueueCond;
    std::condition_variable fAnalysisQueueCond;
	std::condition_variable fCoincidenceQueueCond;
	
    // TTree branch variables
    uint32_t fEventCounter;
    int fChannel;
    uint32_t fTimeStamp;
    std::vector<uint16_t>* fTrace;

    void WriteToRootFile(); // Thread function for writing to ROOT file

    enum EMenuIds {
        M_FILE_LOAD_CONFIG = 1,
		M_FILE_SAVE_CONFIG,
		M_FILE_SAVE_HISTOS,
		M_FILE_SAVE_HISTO_TXT,
        M_FILE_EXIT,
        M_OPTIONS_SIGNAL,
		M_OPTIONS_LOGIC,
		M_OPTIONS_DIGI_PARAMS,
		M_OPTIONS_DT5770_PARAMS
    };
	
	const char *filetypes[20] = { "All files",     "*",
                            "ROOT files",    "*.root",
                            "ROOT macros",   "*.C",
                            "Text files",    "*.[tT][xX][tT]",
                            0,               0 }; 
	char DigiName[5][13] = {"Simulate", "N957", "DT5770", "DT5720", "V1730"}; 
	Pixel_t p_color[16] = {0x0, 0xff, 0xFF0000, 0x8000, 0xFF00FF, 0x00FFFF, 0xFFA500, 0xEE82EE, // 1 - blue, 2 - red, 3- green, 5 -orange
							   0xFFC1CC, 0x8080, 0xFFD700, 0x800080, 0x808000, 0xFF7F7F, 0x80, 0xA52A2A};
	Int_t color[16] = {kBlack, kBlue, kRed, kGreen, kMagenta, kCyan, kOrange, kViolet, kPink, TColor::GetColor(0, 128, 128), TColor::GetColor(255, 215, 0),
						  TColor::GetColor(128, 0, 128), TColor::GetColor(128, 128, 0), TColor::GetColor(255, 127, 127), TColor::GetColor(0, 0, 128), TColor::GetColor(165, 42, 42)};
	
public:
    WindowMain();
    virtual ~WindowMain();
    void CloseWindow();
    void HandleMenu(Int_t id);
	void HandleAcqMode(); // Handle acquisition mode selection
	void HandleInit(); // Handle Init button
	void EnableControlButtons(Bool_t enable); //Enable/disable Start/Stop/Clear
	
	void SaveHistoTxt();
	void SetSelectedType(Int_t type);
	void SetSelectedChan(Int_t chan);
	
	void ModifyHistograms(int b, int ch);
	void UpdateHistograms();
    void UpdateSignalParameters(const SignalParameters& params);
	void UpdateCanvasPeriodically();
	void UpdateCountRateLabel();
		
	void HandleNumEntryPort(Long_t);    
    void HandleNumEntryThr(Long_t);
	void HandleNumEntryDCOffset(Long_t);
	
	void HandleCheckButton();
	void HandleRadioButton();
	void HandleTab(Int_t id);
	
	void DoCheckBox();
	void DoSetVal();
	void DoTextEntry();
	void DoComboBox();
	void BlockCanvas();
	
    void UpdateData();
	void TemperatureCheck();
    void ClearDrawing();
    void StartDrawing();
    void StopDrawing();
    void SignalDialogClosed();
    
    void StartSignalGeneration();
    void StopSignalGeneration();
	
	const char *TrgRateFont = "-adobe-courier-bold-r*-*-12-*-*-*-*-*-iso8859-1";
	const char *ButtonsFont = "-Ubuntu-bold-r-*-*-15-*-*-*-*-*-iso8859-1";
	const char *TempFont = "-Ubuntu-bold-r-*-*-12-*-*-*-*-*-iso8859-1";
	
	
	N957MCAConfig Ncfg;
	DT5770DigiConfig Scfg;
	
	DT5720DigiConfig Dcfg;
	V1730DigiConfig Vcfg[2];
	
    // Method to add signal to queue
    void AddSignalToQueue(const std::vector<Double_t>& signal);
	//void DataAcquisitionThread();
	void AcquisitionThread();
	void AnalysisThread();
	void CoincidenceThread();
	
	size_t GetQueueSize() const { std::lock_guard<std::mutex> lock(fQueueMutex); return fSignalQueue.size(); }
		
	const SignalParameters& GetSignalParameters() const { return fParams; }
    Bool_t IsSignalRunning() const { return fSignalRunning; }
	Bool_t IsRunning() const { return fIsRunning; }
	void SetRunning(const Bool_t &value) { fIsRunning = value; }
	
	Bool_t IsInit() const { return fInit; }
    Bool_t IsDebugEnabled() const { return fCheckDebug ? fCheckDebug->GetState() == kButtonDown : kFALSE; }
	Bool_t IsShowStats() const { return fCheckShowStats ? fCheckShowStats->GetState() == kButtonDown : kFALSE; }
	
	
	TString GetStatistic() const { return fStatistics; }
    void SetStatistic(const TString& value) { fStatistics = value; }
	    	
	std::mutex& GetHistMutex() { return fHistMutex; }
	std::mutex& GetDigitizerMutex() { return fDigitizerMutex; } 
	
    void IncrementSignalCount(int board, int channel, uint32_t count) {
        std::lock_guard<std::mutex> lock(fHistMutex);
        if (channel >= 0 && channel < MAX_CH && board<2) {
            fSignalCount[board][channel] += count;
			fAbsCount[board][channel] += count;
        }
    }
    Int_t GetSignalCount(int board, int channel) const {
        std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(fHistMutex));
        if (channel >= 0 && channel < MAX_CH && board<2) {
            return fSignalCount[board][channel];
        }
       return 0;
    }
	
	Int_t GetAbsCount(int board, int channel) const {
        std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(fHistMutex));
        if (channel >= 0 && channel < MAX_CH && board<2) {
            return fAbsCount[board][channel];
        }
        return 0;
    }
	
    void IncrementDataCount(int board, uint32_t count) {
        std::lock_guard<std::mutex> lock(fHistMutex);
		if (board<2) 
        	fDataCount[board] += count;
    }
	
	TGMainFrame* GetMainFrame() const{return fMain;}  
	
	//for raw data saving
	void InitRootFile();
	void ParamsDialogClosed() {fParamsDialog = nullptr;}
	void DT5770ParamsDialogClosed() {fDT5770ParamsDialog = nullptr;}
	void LogicDialogClosed() {fLogicDialog = nullptr;}
	uint32_t (*GetLogVal())[2][8] { return fLogVal; }
	
    ClassDef(WindowMain, 1)
};

#endif
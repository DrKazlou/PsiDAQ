#include "WindowMain.h"
#include "WindowGenerator.h"

#include "FuncGenerator.h"
#include <TGClient.h>
#include <TGFrame.h>
#include <TRootEmbeddedCanvas.h>
#include <TGButton.h>
#include <TGNumberEntry.h>
#include <TGLabel.h>
#include <TGMenu.h>
#include <TTimer.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TString.h>
#include <TApplication.h>

#include <X11/Xlib.h> 
#include <X11/Xutil.h> // Added for XWMHints
#include <TVirtualX.h> // Added for gVirtualX
#include <X11/Xatom.h> // Added for EWMH properties

#include <mutex>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <algorithm>



WindowMain::WindowMain() : fMain(nullptr), fEmbCanvas(nullptr), fCanvas(nullptr),
                          fMenuBar(nullptr), fMenuFile(nullptr),
                          fMenuOptions(nullptr), fDrawTimer(nullptr), fDAQTimer(nullptr), fStopwatch(),
                          fSignalDialog(nullptr), fIsRunning(kFALSE),
						  fIsClosing(kFALSE),fInit(kFALSE), 
						  fParams{-1, 1.0, 1000, 10.0, 100.0, 0, 10},
                          fUpdateInterval(500), fPSDbin(10), fBoard(0), fCh2D(0), fTimerValue(300),
                          fRand(0), fLabelElapsedTime(nullptr),
                          fCheckDebug(nullptr), fCBDeviceType(nullptr), 
						  fButtonInit(nullptr), fButtonStart(nullptr), fButtonStop(nullptr),
                          fButtonClear(nullptr), fButtonTemp(nullptr) {
				  
		
	fDigitizer[0] = nullptr;
	fDigitizer[1] = nullptr;
							  
	fParamsDialog = nullptr;
	fDT5770ParamsDialog = nullptr;
	fLogicDialog = nullptr;
	memset(fLogVal, 0, sizeof(fLogVal));
	
	fDroppedSignals = 0;							  
	fSignalRunning = false;
	fCheckSaveRawData = nullptr;
    fSaveRawData = kFALSE;
	fLabelSaveRawData = nullptr;
    fTextEntryFileName = nullptr;
    fOutputFileName = "output.root"; 						  
							  
	fDrawingStyle = kTRUE;//by channels						  
	fDrawChannels.clear();				  

    fRootFile = nullptr;
    fTree = nullptr;
    fTrace = nullptr;
    fStopWriting = false;
    							  
    fMain = new TGMainFrame(gClient->GetRoot(), 1600, 880);
    fMain->SetWindowName("PsiDAQ");
	fMain->SetClassHints("PsiDAQ", "PsiDAQ");						  
	
  	SetIcon(fMain, "icon_data.txt"); // in FuncMain.cpp 
							  
    fMain->SetCleanup(kDeepCleanup);
    fMain->Connect("CloseWindow()", "WindowMain", this, "CloseWindow()");

    fMenuBar = new TGMenuBar(fMain, 800, 20);
    fMenuFile = new TGPopupMenu(gClient->GetRoot());
    fMenuFile->AddEntry("L&oad config", M_FILE_LOAD_CONFIG);
	fMenuFile->AddEntry("Save &config", M_FILE_SAVE_CONFIG);
	fMenuFile->AddSeparator();
	fMenuFile->AddEntry("Save &histos", M_FILE_SAVE_HISTOS);
	fMenuFile->AddEntry("Save histogram as &txt", M_FILE_SAVE_HISTO_TXT);
	fMenuFile->AddSeparator();						  
    fMenuFile->AddEntry("E&xit", M_FILE_EXIT);
    fMenuFile->Connect("Activated(Int_t)", "WindowMain", this, "HandleMenu(Int_t)");
    fMenuOptions = new TGPopupMenu(gClient->GetRoot());
    fMenuOptions->AddEntry("&Generate Signal", M_OPTIONS_SIGNAL);
	fMenuOptions->AddEntry("&Logic Menu", M_OPTIONS_LOGIC);						  						  
	fMenuOptions->AddEntry("&Digitizer Parameters", M_OPTIONS_DIGI_PARAMS);
	fMenuOptions->AddSeparator();						  
	fMenuOptions->AddEntry("DT57&70 Parameters", M_OPTIONS_DT5770_PARAMS);						  
							  
    fMenuOptions->Connect("Activated(Int_t)", "WindowMain", this, "HandleMenu(Int_t)");
    fMenuBar->AddPopup("&File", fMenuFile, new TGLayoutHints(kLHintsTop | kLHintsLeft));
    fMenuBar->AddPopup("&Options", fMenuOptions, new TGLayoutHints(kLHintsTop | kLHintsLeft));
    
    fMain->AddFrame(fMenuBar, new TGLayoutHints(kLHintsTop | kLHintsExpandX));

    hFrame = new TGHorizontalFrame(fMain, 800, 600);
							  
	vFrameLeft = new TGVerticalFrame(hFrame, 200, 400);
    TGLabel* labelAcqMode = new TGLabel(vFrameLeft, "Choose Device:");
    vFrameLeft->AddFrame(labelAcqMode, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 0));
    fCBDeviceType = new TGComboBox(vFrameLeft);
    fCBDeviceType->AddEntry("Simulated", 0);
    fCBDeviceType->AddEntry("CAEN N957", 1);
	fCBDeviceType->AddEntry("CAEN DT5770", 2);						  
	fCBDeviceType->AddEntry("CAEN DT5720", 3);
	fCBDeviceType->AddEntry("CAEN V1730S", 4);
    fCBDeviceType->Select(0); // Default to Simulated
	fDeviceType = DeviceType::Simulated;						  
    fCBDeviceType->Connect("Selected(Int_t)", "WindowMain", this, "HandleAcqMode()");
    fCBDeviceType->Resize(120, 30);
    vFrameLeft->AddFrame(fCBDeviceType, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5, 5, 5, 5));
							  
    fButtonInit = new TGTextButton(vFrameLeft, "&Init");
    fButtonInit->Connect("Clicked()", "WindowMain", this, "HandleInit()");
	fButtonInit->SetFont(ButtonsFont);						  
	fButtonInit->Resize(60, 30);							  
    
    vFrameLeft->AddFrame(fButtonInit, new TGLayoutHints(kLHintsBottom | kLHintsCenterX, 5, 5, 5, 5));
    hFrame->AddFrame(vFrameLeft, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));		
							  
	vFrameDigi = new TGVerticalFrame(vFrameLeft, 200, 400);
	vFrameLeft->AddFrame(vFrameDigi, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));		
							  
    fEmbCanvas = new TRootEmbeddedCanvas("cData", hFrame, 600, 400);
    hFrame->AddFrame(fEmbCanvas, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
    fCanvas = fEmbCanvas->GetCanvas();
    fCanvas->SetFillColor(kWhite);
	fCanvasNeedsUpdate = kTRUE;

    vFrameRight = new TGVerticalFrame(hFrame, 200, 400);
							  
	const char *num_label[] = {"Update time, ms", "PDS bin", "Board", "CH_2D", "Timer, s"};	
	const Int_t num_init[] = {fUpdateInterval, fPSDbin, fBoard, fCh2D, fTimerValue};	
	TGHorizontalFrame *fHF[5];
	
   for (int i = 0; i < 5; i++) {
		fHF[i] = new TGHorizontalFrame(vFrameRight, 200, 30);
		vFrameRight->AddFrame(fHF[i], new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2));
	   	if (i != 2)
			fNEMain[i] = new TGNumberEntry(fHF[i], num_init[i], 8, i, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative);//TGNumberFormat::kNEANonNegative, TGNumberFormat::kNELLimitMinMax, 0, 1 
		else	   
			fNEMain[i] = new TGNumberEntry(fHF[i], num_init[i], 8, i, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative, TGNumberFormat::kNELLimitMinMax, 0, 1); 
	   
		fNEMain[i]->Connect("ValueSet(Long_t)", "WindowMain", this, "DoSetVal()");
		fHF[i]->AddFrame(fNEMain[i], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
		fNEMainLabel[i] = new TGLabel(fHF[i], num_label[i]);
		fHF[i]->AddFrame(fNEMainLabel[i], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
		if (i == 4) {
			fNEMain[i]->SetState(kFALSE);
			fCTimer = new TGCheckButton(fHF[i], new TGHotString(""), 66);	
			fCTimer->Connect("Clicked()", "WindowMain", this, "DoCheckBox()");
			fHF[i]->AddFrame(fCTimer, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
	  } 
   }						  
   						  

////////////////////////////							  
	
fTab = new TGTab(vFrameRight, 100, 300);
fTab->Connect("Selected(Int_t)", "WindowMain", this, "HandleTab(Int_t)");

// All channels on single pad
TGCompositeFrame* fCFChannels = fTab->AddTab("Channels");
const char* cbLabels[] = {"Trace", "Amplitude", "Integral", "PSD", "PSD vs Ampl", "PSD vs Int", "BL_CUT", "Det", "Ql", "Counts", "Layers", "XY"};
TGVerticalFrame* fLVF = new TGVerticalFrame(fCFChannels, 10, 10);
TGVerticalFrame* fRVF = new TGVerticalFrame(fCFChannels, 10, 10);							
							  
for (int i = 0; i < 12; i++) {
    fCBHistoTypes[i] = new TGCheckButton(i<6 ? fLVF: fRVF, cbLabels[i], i);
    fCBHistoTypes[i]->Connect("Toggled(Bool_t)", "WindowMain", this, "HandleCheckButton()");
    fCBHistoTypes[i]->SetEnabled(0); // Disabled until acquisition starts
	fCheckStates[i] = kFALSE; 
    (i<6 ? fLVF: fRVF)->AddFrame(fCBHistoTypes[i], new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));
}
//fCBHistoTypes[0]->SetState(kButtonDown); // Show Trace ON by default
//fCheckStates[0] = kTRUE;
fCFChannels->AddFrame(fLVF, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 5, 5, 5, 5));
fCFChannels->AddFrame(fRVF, new TGLayoutHints(kLHintsTop | kLHintsRight | kLHintsExpandX, 10, 5, 5, 5));
fCFChannels->SetLayoutManager(new TGHorizontalLayout(fCFChannels));
							  
TGCompositeFrame* fCFTypes = fTab->AddTab("Types");
const char* rbLabels[] = {"Trace", "Amplitude", "Integral", "Charge", "PSD", "PSD vs Amplitude", "PSD vs Integral"};

for (int i = 0; i < 7; i++) {
    fRBHistoTypes[i] = new TGRadioButton(fCFTypes, rbLabels[i], i);
    fRBHistoTypes[i]->Connect("Clicked()", "WindowMain", this, "HandleRadioButton()");
	fRadioStates[i] = kFALSE; 
    fCFTypes->AddFrame(fRBHistoTypes[i], new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));
}
fRBHistoTypes[0]->SetState(kButtonDown); // Trace ON by default
fRadioStates[0] = kTRUE;

vFrameRight->AddFrame(fTab, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));
fTab->Resize();							  
							  
/////////Timing Labels//////////							  
const char* tlLabels[] = {"UpdateCanvas Time", "AnalysQ Fill", "AnalysQ Unload", "CoincQ Fill", "CoincQ Unload"};
for (int i = 0; i < 5; i++) {							  
    fTimingLabel[i] = new TGLabel(vFrameRight, Form("%s: 0.00 ms",tlLabels[i]));
	fTimingValue[i] = 13;
	vFrameRight->AddFrame(fTimingLabel[i], new TGLayoutHints(kLHintsTop | kLHintsCenterX, 5, 5, 5, 5));
}	
/////////Timing Labels//////////							  	
							  
	TGHorizontalFrame *hf_buttons = new TGHorizontalFrame(vFrameRight, 200, 40);						  

	fButtonStart = new TGTextButton(hf_buttons, "&Start");
    fButtonStart->Connect("Clicked()", "WindowMain", this, "StartDrawing()");
    fButtonStart->SetEnabled(kFALSE);
	fButtonStart->SetFont(ButtonsFont);
	fButtonStart->Resize(60, 30);
	hf_buttons->AddFrame(fButtonStart, new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX, 4, 4, 4, 4));
							  
    fButtonClear = new TGTextButton(hf_buttons, "&Clear");
    fButtonClear->Connect("Clicked()", "WindowMain", this, "ClearDrawing()");
    fButtonClear->SetEnabled(kFALSE);
	fButtonClear->SetFont(ButtonsFont);						  
	fButtonClear->Resize(60, 30);
    hf_buttons->AddFrame(fButtonClear, new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX, 4, 4, 4, 4));
    
    fButtonStop = new TGTextButton(hf_buttons, "Sto&p");
    fButtonStop->Connect("Clicked()", "WindowMain", this, "StopDrawing()");
    fButtonStop->SetEnabled(kFALSE);
	fButtonStop->SetFont(ButtonsFont);						  
	fButtonStop->Resize(60, 30);							  
    hf_buttons->AddFrame(fButtonStop, new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX, 4, 4, 4, 4));
	vFrameRight->AddFrame(hf_buttons, new TGLayoutHints(kLHintsTop | kLHintsCenterX, 5, 5, 5, 5));
								  
	fLabelElapsedTime = new TGLabel(vFrameRight, "          Time: 0 s      ");
	fLabelElapsedTime->SetTextFont("-Ubuntu-bold-r-*-*-16-*-*-*-*-*-iso8859-1");
	fLabelElapsedTime->Resize(150, 30);
    vFrameRight->AddFrame(fLabelElapsedTime, new TGLayoutHints(kLHintsTop | kLHintsCenterX, 5, 5, 5, 5));	
							  
	fCheckDebug = new TGCheckButton(vFrameRight, "Debug");
    fCheckDebug->SetState(kButtonUp); // Debug off by default
    vFrameRight->AddFrame(fCheckDebug, new TGLayoutHints(kLHintsBottom | kLHintsLeft, 5, 5, 5, 5));	
							  
	fCheckShowStats = new TGCheckButton(vFrameRight, "Show Stats");
	fCheckShowStats->SetState(kButtonDown); // Stats off by default
	vFrameRight->AddFrame(fCheckShowStats, new TGLayoutHints(kLHintsBottom | kLHintsLeft, 5, 5, 5, 5));						  
							  
    fCheckCanvasInteractive = new TGCheckButton(vFrameRight, "Canvas Editable");
	fCheckCanvasInteractive->SetState(kButtonDown); // Interaction on by default
	fCheckCanvasInteractive->Connect("Clicked()", "WindowMain", this, "BlockCanvas()");
	vFrameRight->AddFrame(fCheckCanvasInteractive, new TGLayoutHints(kLHintsBottom | kLHintsLeft, 5, 5, 5, 5));							  

    hFrame->AddFrame(vFrameRight, new TGLayoutHints(kLHintsRight | kLHintsExpandY));
    fMain->AddFrame(hFrame, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
			
////Trg.Rate labels////
const char *trgr_label[] = {"                                                  A  r  e   y o u   s u r e    i n   r e a l i t y    o f                                       ",
						   "                                                        w h a t    a r e     y o u     s e e i n g       ?                                       "};								  
							  //"                                                B[%i] Reading @ 6.66 EB/s Trg. Rate: 6.66 Ezy                                              "};								  
							  
	for (int b = 2-1; b>=0; b--){						  
		fTrgRateLabel[b] = new TGLabel(fMain, trgr_label[b]);
		fTrgRateLabel[b]->SetTextFont(TrgRateFont);
		fTrgRateLabel[b]->Resize(800, 30);
    	fMain->AddFrame(fTrgRateLabel[b], new TGLayoutHints(kLHintsBottom | kLHintsCenterX | kLHintsExpandX, 5, 5, 2, 2));						  
	}	
////Trg.Rate labels////
							  
    Double_t total_time = fParams.nSamples * fParams.sampleWidth;
	
	for (int b = 0; b < 2; b++) { // put this part intp HandleAcqMode to reduce memory usage for "simplier" devices							   
 		for (int i = 0; i < MAX_CH; i++) {
			for (int n = 0; n < 4; n++) 
        		h_trace[b][n][i] = new TH1D(Form("h_trace%d_%d_%d", b, n, i), Form("B%i_N%d_CH[%d] Trace;Time (ns);Amplitude (arb. units)", b, n, i), fParams.nSamples, 0.0, total_time);
		       
			h_ampl[b][i] = new TH1D(Form("h_ampl%d_%d", b, i), Form("B[%i] CH[%d] Amplitude;Amplitude (arb. units);Counts", b, i), 1000, 0.0, 1.0);
        	h_integral[b][i] = new TH1D(Form("h_integral%d_%d", b, i), Form("B[%i] CH[%d] Integral;Integral (arb. units);Counts", b, i), 1000, 0.0, 1000.0);
			h_charge[b][i] = new TH1D(Form("h_charge%d_%d", b, i), Form("B[%i] CH[%d] Charge;Charge (arb. units);Counts", b, i), 1000, 0.0, 32768);
        	h_psd[b][i] = new TH1D(Form("h_psd%d_%d", b, i), Form("B[%i] CH[%d] PSD Histogram;PSD (arb. units);Counts", b, i), 1000, -1.0, 1.0);
			
			h_psd_ampl[b][i] = new TH2D(Form("h_psd_ampl%d_%d", b, i), Form("B[%i] CH[%d] PSD vs Amplitude;Amplitude (arb. units);PSD value", b, i), 100, 0.0, 1.0, 100, -1.0, 1.0);
    		h_psd_int[b][i] = new TH2D(Form("h_psd_int%d_%d", b, i), Form("B[%i] CH[%d] PSD vs Integral;Integral (arb. units);PSD value", b, i), 100, 0.0, 1000.0, 100, -1.0, 1.0);
    	}
	}

   
    h_det = new TH1D("h_det", "DT5770 internal MCA histogram;Channels, N;Counts, N", 16384, 0.0, 16384);
	h_counts = new TH1D("h_counts", "h_ampl entries;CH[0-31];Counts, N", 32, 0, 32);				
	h_counts->SetLineColor(kMagenta+3);					  
	h_counts->SetFillColor(kMagenta+3);					  
	h_counts->SetBarWidth(0.95);					  						  
							  
	h_layers = new TH1D("h_layers", "Mean h_ampl values;CH[0-31];Amplitude (arb. units)", 32, 0, 32);
	h_layers->SetLineColor(kMagenta+3);					  
	h_layers->SetFillColor(kMagenta+3);					  
	h_layers->SetBarWidth(0.95);					  						  
							  
	h_xy = new TH2D("h_xy", "Hits", 8, 0, 8, 8, 0, 8); // 8x8 hit pattern		
	//const char* xlabel[8] = {"8", "9", "10", "11", "12", "13", "14", "15"};
	//const char* ylabel[8] = {"0", "1", "2", "3", "4", "5", "6", "7"};	
							  
	//char *xlabel[16], *ylabel[16];
	//for (int i = 0; i<ch; i++){
	//	ylabel[i] = Form("%i",i);
	//	xlabel[i] = Form("%i",i+ch);
	//}
							  
	//for (Int_t i=0; i<8; i++)
	//		h_xy->Fill(xlabel[i], ylabel[i], 0);
							  
    fDrawTimer = new TTimer();
	fDAQTimer = new TTimer();							  
   							  
							  
    fMain->MapSubwindows();
    fMain->Resize(1600, 800);
    fMain->MapWindow();
	
	fCountRateTimer = new TTimer(1000); // 1-second timer for count rate
    fCountRateTimer->Connect("Timeout()", "WindowMain", this, "UpdateCountRateLabel()");
								  
}



WindowMain::~WindowMain() {
	StopDrawing(); // Ensure threads are stopped and joined
	// Stop the writing thread
	if (fParamsDialog) {
        delete fParamsDialog;
        fParamsDialog = nullptr;
    }
	
	if (fLogicDialog) {
        delete fLogicDialog;
        fLogicDialog = nullptr;
    }
	
    fStopWriting = true;
    fDataQueueCond.notify_all();
    if (fWriteThread.joinable()) {
        fWriteThread.join();
    }
	
	 // Close digitizer
    for (int n = 0; n<2; n++){
		if (fDigitizer[n]){
      		fDigitizer[n]->Close();
       		delete fDigitizer[n];
       		fDigitizer[n] = nullptr;
		}	
	}
	
    // Close the ROOT file
    if (fRootFile) {
        fRootFile->Write();
        fRootFile->Close();
        delete fRootFile;
        fRootFile = nullptr;
        fTree = nullptr;
    }

    delete fTrace;
	
	// Clean up histograms
	for (int b = 0; b < 2; b++) {
 		for (int i = 0; i < MAX_CH; i++) {
			for(int n = 0; n<(fDeviceType == DeviceType::DT5770 ? 4 : 1); n++){
        		if (h_trace[b][n][i]) {
            		delete h_trace[b][n][i];
            		h_trace[b][n][i] = nullptr;
        		}
			}
        	if (h_ampl[b][i]) {
            	delete h_ampl[b][i];
            	h_ampl[b][i] = nullptr;
        	}
        	if (h_integral[b][i]) {
            	delete h_integral[b][i];
            	h_integral[b][i] = nullptr;
        	}
	 		if (h_psd[b][i]) {
            	delete h_psd[b][i];
            	h_psd[b][i] = nullptr;
        	}
			if (h_psd_ampl[b][i]) {
            	delete h_psd_ampl[b][i];
            	h_psd_ampl[b][i] = nullptr;
        	}
			if (h_psd_int[b][i]) {
            	delete h_psd_int[b][i];
            	h_psd_int[b][i] = nullptr;
        	}
    	}
	}
		
	if (h_det) {
        delete h_det;
        h_det = nullptr;
    }
	
	if (h_counts) {
        delete h_counts;
        h_counts = nullptr;
    }

	if (h_layers) {
        delete h_layers;
        h_layers = nullptr;
    }
	
    if (fDrawTimer) {
        fDrawTimer->Stop();
        delete fDrawTimer;
        fDrawTimer = nullptr;
    }
	if (fDAQTimer) {
        fDAQTimer->Stop();
        delete fDAQTimer;
        fDAQTimer = nullptr;
    }
    if (fCountRateTimer) {
        fCountRateTimer->Stop();
        delete fCountRateTimer;
        fCountRateTimer = nullptr;
    }

    // Clean up GUI components
    if (fMain) {
        fMain->Cleanup();
        delete fMain;
        fMain = nullptr;
    }
	
	//delete fMain;

}

void WindowMain::CloseWindow() {

    gApplication->Terminate();
}

void WindowMain::BlockCanvas() {
	if (IsDebugEnabled()) printf("Canvas [%s] \n", fCheckCanvasInteractive->GetState() == kButtonDown ? "Editable" : "Blocked");
	
	//Bool_t isEditable = fCheckCanvasInteractive->GetState() == kButtonDown ? kTRUE : kFALSE;
	
	new TGMsgBox(gClient->GetRoot(), fMain, "Warning", Form("Does nothing \n but \n i hope \n it will"), kMBIconAsterisk, kMBOk);
   
    
}	

void WindowMain::HandleMenu(Int_t id) {
    if (IsDebugEnabled()) printf("HandleMenu: id=%d\n", id);
    switch (id) {
        case M_FILE_LOAD_CONFIG:
            {
                static TString dir(".");
                TGFileInfo fi;
                fi.fFileTypes = filetypes;
                fi.fIniDir = StrDup(dir.Data());
                if (IsDebugEnabled()) printf("HandleMenu: fIniDir set to %s\n", fi.fIniDir ? fi.fIniDir : "null");
                new TGFileDialog(gClient->GetRoot(), fMain, kFDOpen, &fi);
                if (fi.fFilename && fi.fFilename[0]) {
                    dir = fi.fIniDir;
                    TFile* file = new TFile(fi.fFilename, "READ");
                    if (file && !file->IsZombie()) {
                        if (IsDebugEnabled()) printf("Loading settings from %s\n", fi.fFilename);
                        TTree* tree = nullptr;
                        file->GetObject("ConfigTree", tree);
                        if (tree) {
							V1730DigiConfig *vcfgPtr[2] = {&Vcfg[0], &Vcfg[1]};
							uint32_t logVal[2][2][8];
                            tree->SetBranchAddress("fLogVal", logVal);
                            tree->SetBranchAddress("Vcfg0", &vcfgPtr[0]);
                            tree->SetBranchAddress("Vcfg1", &vcfgPtr[1]);
                            if (tree->GetEntry(0) > 0) {
                                for (int i = 0; i < 2; i++)
                                    for (int j = 0; j < 2; j++)
                                        for (int k = 0; k < 8; k++)
                                            fLogVal[i][j][k] = logVal[i][j][k];
                                if (IsDebugEnabled()) printf("Config loaded successfully Vcfg[0].thr[0]: %i Vcfg[0].thr[2]: %i\n", Vcfg[0].thr[0], Vcfg[0].thr[2]);
                            } else {
                                if (IsDebugEnabled()) printf("Failed to read tree entry\n");
                                new TGMsgBox(gClient->GetRoot(), fMain, "Error", "Failed to read config data", kMBIconExclamation, kMBOk);
                            }
                            delete tree;
                        } else {
                            if (IsDebugEnabled()) printf("Failed to load ConfigTree\n");
                            new TGMsgBox(gClient->GetRoot(), fMain, "Error", "No config tree found", kMBIconExclamation, kMBOk);
                        }
                        file->Close();
                        delete file;
                    } else {
                        new TGMsgBox(gClient->GetRoot(), fMain, "Error", "Failed to open ROOT file", kMBIconExclamation, kMBOk);
                    }
                } else if (IsDebugEnabled()) {
                    printf("HandleMenu: No filename selected for load\n");
                }
                
            }
            break;
		case M_FILE_SAVE_CONFIG:
            {
                static TString dir(".");
                TGFileInfo fi;
                fi.fFileTypes = filetypes;
                fi.fIniDir = StrDup(dir.Data());
                if (IsDebugEnabled()) printf("HandleMenu: fIniDir set to %s\n", fi.fIniDir ? fi.fIniDir : "null");
                new TGFileDialog(gClient->GetRoot(), fMain, kFDSave, &fi);
                if (fi.fFilename && fi.fFilename[0]) {
                    dir = fi.fIniDir;
                    TFile* file = new TFile(fi.fFilename, "RECREATE");
                    if (file && !file->IsZombie()) {
                        if (IsDebugEnabled()) printf("Saving config to %s\n", fi.fFilename);
                        TTree* tree = new TTree("ConfigTree", "Configuration Tree");
                                                
                        tree->Branch("fLogVal", fLogVal, "logVal[2][2][8]/i");
						tree->Branch("Vcfg0", &Vcfg[0]);
                        tree->Branch("Vcfg1", &Vcfg[1]);
                        tree->Fill();
                        tree->Write();
                        delete tree;
                        file->Close();
                        delete file;
                    } else {
                        new TGMsgBox(gClient->GetRoot(), fMain, "Error", "Failed to create ROOT file", kMBIconExclamation, kMBOk);
                    }
                } else if (IsDebugEnabled()) {
                    printf("HandleMenu: No filename selected for save config\n");
                }
            }
            break;	
		case M_FILE_SAVE_HISTOS:
            {			 
		 		static TString dir(".");
                TGFileInfo fi;
                fi.fFileTypes = filetypes;
                fi.fIniDir = StrDup(dir);
                new TGFileDialog(gClient->GetRoot(), fMain, kFDSave, &fi);
                if (fi.fFilename && fi.fFilename[0]) {
                    dir = fi.fIniDir;
                    TFile* file = new TFile(fi.fFilename, "RECREATE");
                    if (file && !file->IsZombie()) {
                        if (IsDebugEnabled()) printf("Saving all histograms and canvas to %s\n", fi.fFilename);
						
                        // Correctly rework it for different devices to do not create unnecessary empty histograms for simplier digitizers
						for (int b = 0; b < 2; b++) {
                        	for (int i = 0; i < MAX_CH; i++) {
                            	if (h_trace[b][0][i]) h_trace[b][0][i]->Write();   
    							if (h_ampl[b][i]) h_ampl[b][i]->Write();   
    							if (h_integral[b][i]) h_integral[b][i]->Write();   
								if (h_charge[b][i]) h_charge[b][i]->Write();   
    							if (h_psd[b][i]) h_psd[b][i]->Write();   
								if (h_psd_ampl[b][i]) h_psd_ampl[b][i]->Write();   
								if (h_psd_int[b][i]) h_psd_int[b][i]->Write();   
    						}
						}
						if (h_counts) h_counts->Write();
    					if (h_layers) h_layers->Write();  
						if (h_xy) h_xy->Write();  
						
                        fCanvas->Write();
                        file->Close();
                        delete file;
                    } else {
                        new TGMsgBox(gClient->GetRoot(), fMain, "Error", "Failed to create ROOT file", kMBIconExclamation, kMBOk);
                    }
                }
            }
            break;	
		case M_FILE_SAVE_HISTO_TXT:
            {	
				SHTdialog = new TGTransientFrame(gClient->GetRoot(), fMain, 200, 150);
				
                TGComboBox* typeCombo = new TGComboBox(SHTdialog);
                TGComboBox* chanCombo = new TGComboBox(SHTdialog);
                
                typeCombo->AddEntry("Trace", 0);
                typeCombo->AddEntry("Amplitude", 1);
                typeCombo->AddEntry("Integral", 2);
				typeCombo->AddEntry("Charge", 3);
                typeCombo->AddEntry("PSD", 4);
                typeCombo->Resize(150, 20);
                typeCombo->Select(1);
				fSelectedType = 1; 
                for (int i = 0; i < MAX_CH; i++) 
                    chanCombo->AddEntry(Form("Channel %d", i), i);
                
                chanCombo->Resize(150, 20);
                chanCombo->Select(0);
				fSelectedChan = 0;
                typeCombo->Connect("Selected(Int_t)", "WindowMain", this, "SetSelectedType(Int_t)");
                chanCombo->Connect("Selected(Int_t)", "WindowMain", this, "SetSelectedChan(Int_t)");
				
                SHTdialog->AddFrame(typeCombo, new TGLayoutHints(kLHintsCenterX, 5, 5, 5, 5));
                SHTdialog->AddFrame(chanCombo, new TGLayoutHints(kLHintsCenterX, 5, 5, 5, 5));
                TGTextButton* saveButton = new TGTextButton(SHTdialog, "Save");
                SHTdialog->AddFrame(saveButton, new TGLayoutHints(kLHintsCenterX, 5, 5, 5, 5));
                saveButton->Connect("Clicked()", "WindowMain", this, "SaveHistoTxt()");
				SHTdialog->Resize();
                SHTdialog->MapSubwindows();
                SHTdialog->MapRaised();
            }
            break;	
        case M_FILE_EXIT:
            CloseWindow();
            break;
        case M_OPTIONS_SIGNAL:
			if (fDeviceType != DeviceType::Simulated) {
				new TGMsgBox(gClient->GetRoot(), fMain, "Info", "Generate Signal \ndisabled in digi mode", kMBIconAsterisk, kMBOk);
                if (IsDebugEnabled()) printf("HandleMenu: Generate Signal disabled in digi mode\n");
                return;
            }
            if (!fSignalDialog) {
                if (IsDebugEnabled()) printf("Opening signal dialog\n");
                fSignalDialog = new WindowGenerator(gClient->GetRoot(), this);
            }
            break;
		case M_OPTIONS_LOGIC:
			if (fDeviceType == DeviceType::V1730){
				if (!fLogicDialog) {
					V1730DigiConfig *vcfgPtr[2] = {&Vcfg[0], &Vcfg[1]};
            		fLogicDialog = new LogicMenu(gClient->GetRoot(), this, fDigitizer, vcfgPtr);
        		}else 
	            	printf("Logic Menu window is already open!\n");
       		}else
				new TGMsgBox(gClient->GetRoot(), fMain, "Error", "Only for V1730 Mode", kMBIconExclamation, kMBOk);
			break;	
		case M_OPTIONS_DIGI_PARAMS:
			if (fDeviceType == DeviceType::V1730){
				if (!fParamsDialog) {
					V1730DigiConfig *vcfgPtr[2] = {&Vcfg[0], &Vcfg[1]};
            		fParamsDialog = new ParamsMenu(gClient->GetRoot(), this, fDigitizer, vcfgPtr);
        		} else 
            		printf("Digitizer Parameters window is already open!\n");
       		}else	
				new TGMsgBox(gClient->GetRoot(), fMain, "Error", "Only for V1730 Mode", kMBIconExclamation, kMBOk);
			break;
		case M_OPTIONS_DT5770_PARAMS:
			if (fDeviceType == DeviceType::DT5770){
				if (!fDT5770ParamsDialog) {
					fDT5770ParamsDialog = new DT5770Params(gClient->GetRoot(), this, fDigitizer[0], &Scfg);
        		} else 
            		printf("Digitizer Parameters window is already open!\n");
       		}else	
				new TGMsgBox(gClient->GetRoot(), fMain, "Error", "Only for DT5770 Mode", kMBIconExclamation, kMBOk);
			break;	
    }//switch id
}

void WindowMain::SetSelectedType(Int_t type) {
    fSelectedType = type;
    if (IsDebugEnabled()) printf("Selected type: %d\n", fSelectedType);
}

void WindowMain::SetSelectedChan(Int_t chan) {
    fSelectedChan = chan;
    if (IsDebugEnabled()) printf("Selected channel: %d\n", fSelectedChan);
}

void WindowMain::SaveHistoTxt() {
    if (fSelectedType >= 0 && fSelectedChan >= 0) {
        static TString dir(".");
        TGFileInfo fi;
        fi.fFileTypes = filetypes; //new const char*[]{"Text files", "*.txt", "All files", "*"};
        fi.fIniDir = StrDup(dir.Data());
        if (IsDebugEnabled()) printf("SaveHistoTxt: fIniDir set to %s\n", fi.fIniDir ? fi.fIniDir : "null");
        new TGFileDialog(gClient->GetRoot(), SHTdialog, kFDSave, &fi);
        if (fi.fFilename && fi.fFilename[0]) {
            dir = fi.fIniDir;
            std::ofstream out(fi.fFilename);
            if (out.is_open()) {
                if (IsDebugEnabled()) printf("Saving selected histogram to %s\n", fi.fFilename);
                TH1D* hist = nullptr;
                switch (fSelectedType) {
					// rework for multiboard case, curretly only for B[0]	
                    case 0: hist = h_trace[0][0][fSelectedChan]; break; // Trace
                    case 1: hist = h_ampl[0][fSelectedChan]; break;  // Amplitude
                    case 2: hist = h_integral[0][fSelectedChan]; break; // Integral
					case 3: hist = h_charge[0][fSelectedChan]; break; // Charge
                    case 4: hist = h_psd[0][fSelectedChan]; break;    // PSD
                }
                if (hist) {
                    for (int i = 1; i <= hist->GetNbinsX(); i++) {
                        out << hist->GetBinCenter(i) << " " << hist->GetBinContent(i) << std::endl;
                    }
                    if (IsDebugEnabled()) printf("Saved histogram for Channel %d, Type %d\n", fSelectedChan, fSelectedType);
                } else {
                    if (IsDebugEnabled()) printf("Failed to find histogram for Channel %d, Type %d\n", fSelectedChan, fSelectedType);
                }
                out.close();
            } else {
                new TGMsgBox(gClient->GetRoot(), SHTdialog, "Error", "Failed to open text file", kMBIconExclamation, kMBOk);
            }
        } else if (IsDebugEnabled()) {
            printf("SaveHistoTxt: No filename selected\n");
        }
        delete[] fi.fFileTypes;
    } else {
        new TGMsgBox(gClient->GetRoot(), SHTdialog, "Error", "Invalid selection", kMBIconExclamation, kMBOk);
    }
}

void WindowMain::HandleNumEntryThr(Long_t) {
	TGNumberEntry *ne = (TGNumberEntry *) gTQSender;
	Int_t id = ne->WidgetId();
	
    if (fInit && fDeviceType == DeviceType::N957) {
        N957_SetLLD(fDigitizer[0]->GetHandle(), static_cast<N957_BYTE>(ne->GetIntNumber()));
        if (IsDebugEnabled()) printf("HandleNumEntryThr: SetLLD to %ld\n", ne->GetIntNumber());
    }
	
    if (fDeviceType == DeviceType::DT5720) {
		int ch = id - 12; // id = 12 + n
		Dcfg.thr[ch] = ne->GetIntNumber();
		if (IsDebugEnabled())printf("Thr %i : %li \n",ch, ne->GetIntNumber());	
		
		Int_t p = Dcfg.PulsePolarity[ch] == CAEN_DGTZ_TriggerOnRisingEdge ? 1 : -1;  // to correctly handle both polarity
		if (IsDebugEnabled()) printf(" Trigger for CH[%i] will be: %i  (cal_thr[%i]: %d)\n", ch, Dcfg.cal_thr[ch] + p * Dcfg.thr[ch], ch, Dcfg.cal_thr[ch]); 
		
		std::lock_guard<std::mutex> lock(fDigitizerMutex);
		CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_SetChannelTriggerThreshold(fDigitizer[0]->GetHandle(), ch, Dcfg.cal_thr[ch] + p * Dcfg.thr[ch]);
		if (ret) {
			new TGMsgBox(gClient->GetRoot(), fMain, "Error", Form("SET_CH[%i]_TRIGGER_THRESHOLD_FAILURE \n ret = %i", ch, ret), kMBIconStop, kMBOk);
			fDigitizer[0]->Close();
		}
	}	
	
}

void WindowMain::HandleNumEntryDCOffset(Long_t){
	TGNumberEntry *ne = (TGNumberEntry *) gTQSender;
	Int_t id = ne->WidgetId();
	int ch = id - 14; // id = 14 + n
	Dcfg.DCOffset[ch] = ne->GetIntNumber(); 
	if (IsDebugEnabled())printf("DCOffset %i : %li \n", ch, ne->GetIntNumber());	
	
	std::lock_guard<std::mutex> lock(fDigitizerMutex);
	CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_SetChannelDCOffset(fDigitizer[0]->GetHandle(), ch, Dcfg.DCOffset[ch]);
	
	if (ret) {
		new TGMsgBox(gClient->GetRoot(), fMain, "Error", Form("SET_CH[%i]_DCOffset_FAILURE \n ret = %i", ch, ret), kMBIconStop, kMBOk);
		fDigitizer[0]->Close();
	}

}

void WindowMain::ModifyHistograms(int b, int ch) {
	std::lock_guard<std::mutex> lock(fHistMutex);
	
	h_trace[b][0][ch]->Reset();
    h_trace[b][0][ch]->SetBins(Vcfg[b].RecordLength[ch], 0.0, Vcfg[b].RecordLength[ch]*BIN_WIDTH_V1730);
	
	fCanvasNeedsUpdate = kTRUE;
}	

void WindowMain::UpdateHistograms() {
    if (IsDebugEnabled()) printf("UpdateHistograms: Updating histograms\n");
	
	int B_CH = fDeviceType == DeviceType::V1730 ? Vcfg[0].NB : 1;
	int H_CH = 1;
	int nBinsTrace[16], nBinsAmpl[16], nBinsIntegral[16], nBinsCharge[16], nBinsPSD[16], Ymin  = 0, Ymax = 20000;
	double TraceLength[16], AmplLimit[16], IntegralLimit[16], ChargeLimit[16];
	
	if (fDeviceType == DeviceType::Simulated){
		if (fParams.polarity == -1) 
			{Ymin = -0.1; Ymax = 1.1;}
		else
			{Ymin = -1.1; Ymax = -0.1;}
		
		nBinsTrace[0] = fParams.nSamples;
		TraceLength[0] = fParams.nSamples * fParams.sampleWidth;
		nBinsAmpl[0] = 1000;
		AmplLimit[0] = 1.0;
		nBinsIntegral[0] = 1000;
		IntegralLimit[0] = 1000;
	}else if (fDeviceType == DeviceType::N957){
		nBinsAmpl[0] = 8192;
		AmplLimit[0] = 8192;
	}else if (fDeviceType == DeviceType::DT5770){
		H_CH = 1;
		Ymin = 0; Ymax = 8192;
		for (int i = 0; i<H_CH; i++){
			nBinsTrace[i] = 2048;
			TraceLength[i] = 2048 * BIN_WIDTH_DT5770; 
			nBinsAmpl[i] = 16384;
			AmplLimit[i] = 16384;
			nBinsIntegral[i] = 1000;
			IntegralLimit[i] = 1000000;
			nBinsPSD[i] = 500;
		}
	}else if (fDeviceType == DeviceType::DT5720){
		H_CH = 2;
		Ymin = 0; Ymax = 8192;
		for (int i = 0; i<H_CH; i++){
			nBinsTrace[i] = Dcfg.RecordLength[i];
			TraceLength[i] = Dcfg.RecordLength[i] * BIN_WIDTH_DT5720;
			nBinsAmpl[i] = 1000;
			AmplLimit[i] = 8192;
			nBinsIntegral[i] = 1000;
			IntegralLimit[i] = 100000;
			nBinsPSD[i] = 500;
		}
	}else if (fDeviceType == DeviceType::V1730){
		Ymin = 0; Ymax = 20000;
		H_CH = Vcfg[0].Nch;
		for (int i = 0; i<H_CH; i++){
			nBinsTrace[i] = Vcfg[0].RecordLength[i];
			TraceLength[i] = Vcfg[0].RecordLength[i] * BIN_WIDTH_V1730;
			nBinsAmpl[i] = 1000;
			AmplLimit[i] = 16384;
			nBinsIntegral[i] = 1000;
			IntegralLimit[i] = 100000;
			nBinsCharge[i] = 1000;
			ChargeLimit[i] = 32768;
			nBinsPSD[i] = 500;
		}
	}	
	
    {
        std::lock_guard<std::mutex> lock(fHistMutex);
		
		for (int b = 0; b < B_CH; b++) {						
			for (int i = 0; i < H_CH; i++) {
					
        	    if (h_ampl[b][i]) {
           	    	h_ampl[b][i]->Reset();
                	h_ampl[b][i]->SetBins(nBinsAmpl[i],0.0, AmplLimit[i]);
            	} else {
               		h_ampl[b][i] = new TH1D(Form("h_ampl%d_%d", b, i), Form("B[%d] CH%d Amplitude Histogram;Amplitude (arb. units);Counts", b, i), nBinsAmpl[i],0.0, AmplLimit[i]);
				}   h_ampl[b][i]->SetLineColor(color[i]);
            
				if (fDeviceType != DeviceType::N957){
					for(int n = 0; n<(fDeviceType == DeviceType::DT5770 ? 4 : 1); n++){
            			if (h_trace[b][n][i]) {
            	   	 		h_trace[b][n][i]->Reset();
            		   		h_trace[b][n][i]->SetBins(nBinsTrace[i], 0.0, TraceLength[i]);
					
            			} else 
                			h_trace[b][n][i] = new TH1D(Form("h_trace%d_%d_%d", b, n, i), Form("B[%d] N%d CH%d Trace;Time (ns);Counts", b, n, i), nBinsTrace[i], 0.0, TraceLength[i]);
				
						h_trace[b][n][i]->SetLineColor(i==0 ? color[n] : color[i]);
						h_trace[b][n][i]->GetYaxis()->SetRangeUser(Ymin, Ymax);
           		     	h_trace[b][n][i]->GetXaxis()->SetRangeUser(0, TraceLength[i]);
					}

            		if (h_integral[b][i]) {
               		 	h_integral[b][i]->Reset();
            	    	h_integral[b][i]->SetBins(nBinsIntegral[i], 0.0, IntegralLimit[i]);
            		} else {
                		h_integral[b][i] = new TH1D(Form("h_integral%d_%d", b, i), Form("B[%d] CH%d Integral Histogram;Integral (arb. units);Counts", b, i), nBinsIntegral[i], 0.0, IntegralLimit[i]);
					}   	h_integral[b][i]->SetLineColor(color[i]);
				
					if (fDeviceType == DeviceType::V1730){
						if (h_charge[b][i]) {
                			h_charge[b][i]->Reset();
               	 			h_charge[b][i]->SetBins(nBinsCharge[i], 0.0, ChargeLimit[i]);
            			} else {
            	    		h_charge[b][i] = new TH1D(Form("h_charge%d_%d", b, i), Form("B[%d] CH%d Charge Histogram;Charge (arb. units);Counts", b, i), nBinsCharge[i], 0.0, ChargeLimit[i]);
						}   h_charge[b][i]->SetLineColor(color[i]);
						if (h_xy){
							int ch = Vcfg[0].NB*8;
							h_xy->SetBins(ch, 0, ch, ch, 0, ch);
							
							for (Int_t i=0; i<ch; i++)
								h_xy->Fill(Form("%i",i+ch), Form("%i",i), 0);
						}
							
					}		
					if (h_psd[b][i]) {
          		  		h_psd[b][i]->Reset();
           				h_psd[b][i]->SetBins(nBinsPSD[i], -1.0, 1.0);
        			} else {
            			h_psd[b][i] = new TH1D(Form("h_psd%d_%d", b, i), Form("B[%d] CH[%d] PSD Histogram;PSD (arb. units);Counts", b, i), nBinsPSD[i], -1.0, 1.0);
					}	h_psd[b][i]->SetLineColor(color[i]);
				
					if (h_psd_ampl[b][i]) {
            			h_psd_ampl[b][i]->Reset();
           		 		h_psd_ampl[b][i]->SetBins(500, 0.0, AmplLimit[i], 500, -1.0, 1.0);
        			} else {
            			h_psd_ampl[b][i] = new TH2D(Form("h_psd_ampl%d_%d", b, i), Form("B[%d] CH[%d] PSD vs Amplitude;Amplitude (arb. units);PSD (arb. units)", b, i), 500, 0.0, AmplLimit[i], 500, -1.0, 1.0);
        			}

        			if (h_psd_int[b][i]) {
            			h_psd_int[b][i]->Reset();
            			h_psd_int[b][i]->SetBins(500, 0.0, IntegralLimit[i], 500, -1.0, 1.0);
        			} else {
            			h_psd_int[b][i] = new TH2D(Form("h_psd_int%d_%d", b, i), Form("B[%d] CH[%d] PSD vs Integral;Integral (arb. units);PSD (arb. units)", b, i), 500, 0.0, IntegralLimit[i], 500, -1.0, 1.0);
        			}
				
				}	
			}	
		}	
			
    }

    if (IsDebugEnabled()) printf("UpdateHistograms: Set trace bins to %i, range 0 to %0.1f; amplitude bins to %d, range 0 to %0.1f\n",
                                nBinsTrace[0], TraceLength[0], nBinsAmpl[0], AmplLimit[0]);
}


void WindowMain::HandleAcqMode() {
	TGComboBox *cb = (TGComboBox *) gTQSender;
    fDeviceType = (DeviceType)cb->GetSelected();
    if (IsDebugEnabled()) printf("HandleAcqMode: Acquisition mode set to %s \n", DigiName[(int)fDeviceType]);
	
    ParseConfigFile("Config.txt", fDeviceType, &Ncfg, &Scfg, &Dcfg, &Vcfg[0], IsDebugEnabled());
	Vcfg[1] = Vcfg[0];
	if (Vcfg[0].NB==2) Vcfg[1].IsMaster = false;
	
	if (fDeviceType == DeviceType::N957) 
    	fDigitizer[0] = new N957Digitizer(this, &Ncfg);
	else if (fDeviceType == DeviceType::DT5770) 
            fDigitizer[0] = new DT5770Digitizer(this, &Scfg);
    else if (fDeviceType == DeviceType::DT5720) 
            fDigitizer[0] = new DT5720Digitizer(this, &Dcfg);
    else if (fDeviceType == DeviceType::V1730){ 
		for (int b = 0; b < Vcfg[0].NB; b++){
        	fDigitizer[b] = new V1730Digitizer(this, &Vcfg[b]);
			fDigitizer[b]->SetDebug(IsDebugEnabled());
		}		
	}	
    
	fDigitizer[0]->SetDebug(IsDebugEnabled());
  	
	for (int i = 0; i<12; i++){
		fCBHistoTypes[i]->SetEnabled(fDeviceType != DeviceType::N957 || (i==1 && fDeviceType == DeviceType::N957) || (i==7 && fDeviceType == DeviceType::DT5770));
		
		if(i==0){
			if (fDeviceType != DeviceType::N957)fCBHistoTypes[i]->SetState(kButtonDown);
			fCheckStates[i] = fDeviceType != DeviceType::N957 ? true : false;
		}
		else if(i==1){
			fCBHistoTypes[i]->SetState(fDeviceType == DeviceType::N957 ? kButtonDown : kButtonUp);
			fCheckStates[i] = fDeviceType == DeviceType::N957 ? true : false;
		}
		
		else if(i>1){
			if (fDeviceType != DeviceType::N957) fCBHistoTypes[i]->SetState(kButtonUp);
			fCheckStates[i] = false;
		}	
	}
	
	if (fDeviceType != DeviceType::DT5770){
		for (int i = 0; i<4; i++)
			fCheckVP[i] = i==0 ? true : false; 
	}
	
	if (fDeviceType == DeviceType::Simulated){ 							 	
		fDrawChannels.push_back(true); // only one
		fDrawChannels.push_back(true); // two two
    	UpdateSignalParameters(fParams);
		UpdateHistograms(); 
		return;
		
	}else if (fDeviceType == DeviceType::N957) {
				
		fDrawChannels.push_back(true); // only one
        fLabelThr = new TGLabel(vFrameDigi, "N957 Threshold:");
        vFrameDigi->AddFrame(fLabelThr, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));
        fNumEntryThr[0] = new TGNumberEntry(vFrameDigi, Ncfg.thr, 5, 1, TGNumberFormat::kNESInteger,// id = 1
                                         TGNumberFormat::kNEANonNegative, TGNumberFormat::kNELLimitMinMax, 0, 99);
        fNumEntryThr[0]->Connect("ValueSet(Long_t)", "WindowMain", this, "HandleNumEntryThr(Long_t)");
        vFrameDigi->AddFrame(fNumEntryThr[0], new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5, 5, 5, 5));
		
	}else if (fDeviceType == DeviceType::DT5770){
		fDrawChannels.push_back(true); // only one
		TGGroupFrame *gframeVP = new TGGroupFrame(vFrameDigi, "VirtualProbe", kVerticalFrame);
		gframeVP->SetTitlePos(TGGroupFrame::kCenter); 
		vFrameDigi->AddFrame(gframeVP, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 5, 5, 5, 5));//
     					
		const char *cb_label[] = {"AT1", "AT2",  "DT1", "DT2"};	

		for (int i = 0; i < 4; ++i) {
			fCVP[i] = new TGCheckButton(gframeVP, cb_label[i],	i);
			fCVP[i]->SetTextColor(p_color[i]);
			fCVP[i]->SetState(kButtonDown); 
			fCheckVP[i] = true; 
			fCVP[i]->Connect("Clicked()", "WindowMain", this, "DoCheckBox()");
			gframeVP->AddFrame(fCVP[i], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
		}
	 
		gframeVP->SetLayoutManager(new TGMatrixLayout(gframeVP, 0, 2, 2));
		gframeVP->Resize();
	}	
	else if (fDeviceType == DeviceType::DT5720){
		
		fLabelSaveRawData = new TGLabel(vFrameDigi, "Save Raw Data");
		vFrameDigi->AddFrame(fLabelSaveRawData, new TGLayoutHints(kLHintsTop | kLHintsCenterX, 2, 2, 2, 2));
		
		TGHorizontalFrame* hFrameSaveRaw = new TGHorizontalFrame(vFrameDigi);
		fTextEntryFileName = new TGTextEntry(hFrameSaveRaw, fOutputFileName.c_str());
        fTextEntryFileName->SetWidth(150); 
		fTextEntryFileName->SetEnabled(kFALSE); // Initially disabled
        fTextEntryFileName->Connect("ReturnPressed()", "WindowMain", this, "DoTextEntry()");
        hFrameSaveRaw->AddFrame(fTextEntryFileName, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 0, 2, 5));
		
		fCheckSaveRawData = new TGCheckButton(hFrameSaveRaw, "", 15);
        fCheckSaveRawData->SetState(kButtonUp); 
        fCheckSaveRawData->Connect("Clicked()", "WindowMain", this, "DoCheckBox()");
        hFrameSaveRaw->AddFrame(fCheckSaveRawData, new TGLayoutHints(kLHintsTop | kLHintsCenterX, 2, 2, 2, 2));
		vFrameDigi->AddFrame(hFrameSaveRaw, new TGLayoutHints(kLHintsTop | kLHintsCenterX, 5, 5, 5, 5));
				
        fHFrameEnableCH = new TGHorizontalFrame(vFrameDigi, 200, 40);
        for (int n = 0; n < 2; n++) {
            fCheckEnableCH[n] = new TGCheckButton(fHFrameEnableCH, Form("CH%d", n), 20 + n); 
            fCheckEnableCH[n]->SetTextColor(p_color[n]); // CH0 blue, CH1 red
			fCheckEnableCH[n]->SetState(kButtonDown); 
			fDrawChannels.push_back(true);
			fCheckEnableCH[n]->Connect("Clicked()", "WindowMain", this, "DoCheckBox()");
            fHFrameEnableCH->AddFrame(fCheckEnableCH[n], new TGLayoutHints(kLHintsLeft, 5, 5, 5, 5));
        }
        vFrameDigi->AddFrame(fHFrameEnableCH, new TGLayoutHints(kLHintsTop | kLHintsCenterX, 5, 5, 5, 5));

        // Self-trigger checkboxes
        fHFrameSelfTrg = new TGHorizontalFrame(vFrameDigi, 200, 40);
        for (int n = 0; n < 2; n++) {
            fCheckSelfTrg[n] = new TGCheckButton(fHFrameSelfTrg, Form("SelfTrg%d", n), 12 + n);
            fCheckSelfTrg[n]->SetTextColor(p_color[n]); // SelfTrg0 blue, SelfTrg1 red
			fCheckSelfTrg[n]->SetState(kButtonDown); 
			fCheckSelfTrg[n]->Connect("Clicked()", "WindowMain", this, "DoCheckBox()");
            fHFrameSelfTrg->AddFrame(fCheckSelfTrg[n], new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));
        }
        vFrameDigi->AddFrame(fHFrameSelfTrg, new TGLayoutHints(kLHintsTop | kLHintsCenterX, 5, 5, 5, 5));

        // Coincidence checkbox, centered
        fCheckCoincidence = new TGCheckButton(vFrameDigi, "Coincidence", 14);
		fCheckCoincidence->Connect("Clicked()", "WindowMain", this, "DoCheckBox()");
        vFrameDigi->AddFrame(fCheckCoincidence, new TGLayoutHints(kLHintsTop | kLHintsCenterX, 5, 5, 5, 5));
		
		fLabelThr = new TGLabel(vFrameDigi, "DT5720 Thresholds:");
        vFrameDigi->AddFrame(fLabelThr, new TGLayoutHints(kLHintsTop | kLHintsCenterX, 5, 5, 5, 5));
        fHFrameThr = new TGHorizontalFrame(vFrameDigi, 200, 40);
        for (int n = 0; n < 2; n++) {
            fNumEntryThr[n] = new TGNumberEntry(fHFrameThr, Dcfg.thr[n], 5, 2 + n + 10, TGNumberFormat::kNESInteger,
                                                TGNumberFormat::kNEANonNegative, TGNumberFormat::kNELLimitMinMax, 0, 1024);
            fNumEntryThr[n]->Connect("ValueSet(Long_t)", "WindowMain", this, "HandleNumEntryThr(Long_t)");
            fHFrameThr->AddFrame(fNumEntryThr[n], new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5, 5, 5, 5));
        }
        vFrameDigi->AddFrame(fHFrameThr, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5, 5, 5, 5));

        // DC offset label and entries in a horizontal frame
        fLabelDCOffset = new TGLabel(vFrameDigi, "DT5720 DC Offset:");
        vFrameDigi->AddFrame(fLabelDCOffset, new TGLayoutHints(kLHintsTop | kLHintsCenterX, 5, 5, 5, 5));
        fHFrameDCOffset = new TGHorizontalFrame(vFrameDigi, 200, 40);
        for (int n = 0; n < 2; n++) {
            fNumEntryDCOffset[n] = new TGNumberEntry(fHFrameDCOffset, Dcfg.DCOffset[n], 5, 4 + n + 10, TGNumberFormat::kNESInteger,
                                                     TGNumberFormat::kNEANonNegative, TGNumberFormat::kNELLimitMinMax, 0, 1000);
            fNumEntryDCOffset[n]->Connect("ValueSet(Long_t)", "WindowMain", this, "HandleNumEntryDCOffset(Long_t)");
            fHFrameDCOffset->AddFrame(fNumEntryDCOffset[n], new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5, 5, 5, 5));
        }
        vFrameDigi->AddFrame(fHFrameDCOffset, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5, 5, 5, 5));

	}
	else if (fDeviceType == DeviceType::V1730){
		fLabelSaveRawData = new TGLabel(vFrameDigi, "Save Raw Data");
		vFrameDigi->AddFrame(fLabelSaveRawData, new TGLayoutHints(kLHintsTop | kLHintsCenterX, 5, 5, 5, 5));
		
		TGHorizontalFrame* hFrameSaveRaw = new TGHorizontalFrame(vFrameDigi);
		fTextEntryFileName = new TGTextEntry(hFrameSaveRaw, fOutputFileName.c_str());
        fTextEntryFileName->SetWidth(150); 
		fTextEntryFileName->SetEnabled(kFALSE); // Initially disabled
        fTextEntryFileName->Connect("ReturnPressed()", "WindowMain", this, "DoTextEntry()");
        hFrameSaveRaw->AddFrame(fTextEntryFileName, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 5, 5));
		
		fCheckSaveRawData = new TGCheckButton(hFrameSaveRaw, "", 15);
        fCheckSaveRawData->SetState(kButtonUp); 
        fCheckSaveRawData->Connect("Clicked()", "WindowMain", this, "DoCheckBox()");
        hFrameSaveRaw->AddFrame(fCheckSaveRawData, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 1, 1, 5, 5));
		vFrameDigi->AddFrame(hFrameSaveRaw, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 0, 5, 5));
		
		fLabelV1730AcqMode = new TGLabel(vFrameDigi, "V1730 Acq Mode:");
        vFrameDigi->AddFrame(fLabelV1730AcqMode, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 0, 5, 5));

        fComboV1730AcqMode = new TGComboBox(vFrameDigi, 2);
		fComboV1730AcqMode->AddEntry("Oscilloscope", CAEN_DGTZ_DPP_ACQ_MODE_Oscilloscope);
        fComboV1730AcqMode->AddEntry("List", CAEN_DGTZ_DPP_ACQ_MODE_List);
        fComboV1730AcqMode->AddEntry("Mixed", CAEN_DGTZ_DPP_ACQ_MODE_Mixed);
        fComboV1730AcqMode->Select(Vcfg[0].AcqMode); // Default to Vcfg[0].AcqMode
        fComboV1730AcqMode->Connect("Selected(Int_t)", "WindowMain", this, "DoComboBox()");
        fComboV1730AcqMode->Resize(100, 20);
        vFrameDigi->AddFrame(fComboV1730AcqMode, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));
                
        TGGroupFrame* fVFrameV1730Channels = new TGGroupFrame(vFrameDigi, "V1730 Channels", kHorizontalFrame);
        //TGHorizontalFrame *hFrameChannels = new TGHorizontalFrame(fVFrameV1730Channels);
        TGVerticalFrame *vFrameLeftPart = new TGVerticalFrame(fVFrameV1730Channels);//, 50, 50);
        TGVerticalFrame *vFrameRightPart = new TGVerticalFrame(fVFrameV1730Channels);//, 50, 50);

        
        for (int i = 0; i < Vcfg[0].Nch; i++) {
            fCheckEnableCH[i] = new TGCheckButton(i < (int)(Vcfg[0].Nch/2) ? vFrameLeftPart : vFrameRightPart, Form("CH%i", i), 20+i);
            fCheckEnableCH[i]->SetTextColor(p_color[i]);
            if (i < Vcfg[0].Nch) {
                fCheckEnableCH[i]->SetState(Vcfg[0].ChannelMask & (1 << i) ? kButtonDown : kButtonUp);
				fDrawChannels.push_back(Vcfg[0].ChannelMask & (1 << i) ? true : false);
            } else {
                fCheckEnableCH[i]->SetState(kButtonDisabled);
            }
            fCheckEnableCH[i]->Connect("Clicked()", "WindowMain", this, "DoCheckBox()");
            (i < (int)(Vcfg[0].Nch/2) ? vFrameLeftPart : vFrameRightPart)->AddFrame(fCheckEnableCH[i], new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2));
        }
		
		vFrameLeftPart->Resize();
		vFrameRightPart->Resize();
        fVFrameV1730Channels->AddFrame(vFrameLeftPart, new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 5, 1));
        fVFrameV1730Channels->AddFrame(vFrameRightPart, new TGLayoutHints(kLHintsLeft | kLHintsTop, 2, 2, 5, 1));
		fVFrameV1730Channels->Resize();
        
        vFrameDigi->AddFrame(fVFrameV1730Channels, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));
		
		fButtonTemp = new TGTextButton(vFrameDigi, "&Temp");
    	fButtonTemp->Connect("Clicked()", "WindowMain", this, "TemperatureCheck()");
    	fButtonTemp->SetEnabled(kFALSE);
		fButtonTemp->SetFont(TempFont);						  
		fButtonTemp->Resize(60, 30);							  
    	vFrameDigi->AddFrame(fButtonTemp, new TGLayoutHints(kLHintsTop | kLHintsLeft, 4, 4, 4, 4));
		
		
		TGHorizontalFrame *hf_temp[2];
		for (int b = 0; b<Vcfg[0].NB; b++){
			hf_temp[b] = new TGHorizontalFrame(vFrameDigi, 100, 30);
			for (int i=0; i<MAX_CH/2; i++){
				fTempLabel[b][i] = new TGLabel(hf_temp[b], "--");
				fTempLabel[b][i]->SetTextFont("-adobe-courier-bold-r-*-*-15-*-*-*-*-*-iso8859-1");
				fTempLabel[b][i]->Resize(30,30);
				fTempLabel[b][i]->SetBackgroundColor(p_color[5]);
            	fTempLabel[b][i]->ChangeBackground(p_color[5]); // Force refresh
        		hf_temp[b]->AddFrame(fTempLabel[b][i], new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 1, 0, 1, 1));
			}	
			vFrameDigi->AddFrame(hf_temp[b], new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));
		}
		
	}	
	else {
        fLabelThr = nullptr;
        
        for (int n = 0; n < 2; n++) {
            fNumEntryThr[n] = nullptr;
			fNumEntryDCOffset[n] = nullptr;
        }
    }
		
	vFrameDigi->Layout();
	
	cb->SetEnabled(0); // in future try finaly to handle correct deleting previous vFrameDigi
	cb->Disconnect("Selected(Int_t)");
	
    fMain->MapSubwindows();
    fMain->Layout();
		
	
    // Reset histograms and buttons
    //ClearDrawing();
    EnableControlButtons(kFALSE);
	UpdateHistograms();
	
	gSystem->ProcessEvents(); 
}

void WindowMain::DoComboBox() {
	TGComboBox *cb = (TGComboBox *) gTQSender;
	Int_t id = cb->WidgetId();
	
	if (IsDebugEnabled()) printf("ComboBox: id %i selected: %i\n", id, cb->GetSelected( ));
	
	for (int b = 0; b<Vcfg[0].NB; b++){
		switch (cb->GetSelected( )) {
			case 0: Vcfg[b].AcqMode = CAEN_DGTZ_DPP_ACQ_MODE_Oscilloscope; break; 		
    		case 1: Vcfg[b].AcqMode = CAEN_DGTZ_DPP_ACQ_MODE_List; break; 
    		case 2: Vcfg[b].AcqMode = CAEN_DGTZ_DPP_ACQ_MODE_Mixed; break;  
    	}		
	}
	
	if (!fIsRunning){
		std::lock_guard<std::mutex> lock(fDigitizerMutex);
		for (int b = 0; b<Vcfg[0].NB; b++)
			fDigitizer[b]->SetAcquisitionMode(Vcfg[b].AcqMode);
	
	}
}

void WindowMain::DoCheckBox() {
	TGCheckButton *cb = (TGCheckButton *) gTQSender;
	Int_t id = cb->WidgetId();
	
	if (IsDebugEnabled()) printf("CheckButton: %i\n", id);
	
	if (id <=3)	
		fCheckVP[id] = (cb->GetState() == kButtonDown) ? true : false; 
	
	CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_Success;
		
	if (id == 12)	
		ret = CAEN_DGTZ_SetChannelSelfTrigger(fDigitizer[0]->GetHandle(), cb->GetState() == kButtonDown ? CAEN_DGTZ_TRGMODE_ACQ_ONLY : CAEN_DGTZ_TRGMODE_DISABLED, (1 << 0));
		
	if (id == 13)	
		ret = CAEN_DGTZ_SetChannelSelfTrigger(fDigitizer[0]->GetHandle(), cb->GetState() == kButtonDown ? CAEN_DGTZ_TRGMODE_ACQ_ONLY : CAEN_DGTZ_TRGMODE_DISABLED, (1 << 1));
	
	if (id == 14 ) {
		uint32_t reg_data = 3, shift = 1<<24;
		if (cb->GetState() == kButtonDown){
		   reg_data = reg_data | shift;
		   ret = CAEN_DGTZ_WriteRegister(fDigitizer[0]->GetHandle(), 0x810C, reg_data);
		   ret = CAEN_DGTZ_ReadRegister(fDigitizer[0]->GetHandle(), 0x810C, &reg_data);
		   if (IsDebugEnabled()) printf(" Coincidence ON 0x810C: %i \n", reg_data);
		}    
		else{
			ret = CAEN_DGTZ_WriteRegister(fDigitizer[0]->GetHandle(), 0x810C, 3);
			ret = CAEN_DGTZ_ReadRegister(fDigitizer[0]->GetHandle(), 0x810C, &reg_data);
			if (IsDebugEnabled()) printf(" Coincidence OFF 0x810C: %i \n", reg_data);
		}	
	}
	if (id == 15 ){ // fCheckSaveRawData
        fSaveRawData = cb->GetState() == kButtonDown ? kTRUE : kFALSE;
		fTextEntryFileName->SetEnabled(fSaveRawData);
        if (IsDebugEnabled()) printf("DoCheckBox: Save raw data set to %s\n", fSaveRawData ? "ON" : "OFF");
    }
		
	if (id>=20 && id<36){
		fDrawChannels[id-20] = cb->GetState() == kButtonDown ? true : false;
		fCanvasNeedsUpdate = kTRUE;
	}
	
	 //Timer checkbox
	if (id == 66 ) 
	   fNEMain[3]->SetState( fCTimer->GetState() == kButtonDown ? kTRUE : kFALSE );
			
	if (ret){
		new TGMsgBox(gClient->GetRoot(), fMain, "Error", Form("Can't handle CheckBoxies \n ret = %i", ret), kMBIconStop, kMBOk);
		fDigitizer[0]->Close();
	}
}

void WindowMain::DoSetVal() {
	TGNumberEntry *ne = (TGNumberEntry *) gTQSender;
	Int_t id = ne->WidgetId();
	
	if (IsDebugEnabled()) printf("NumberEntry: %i value (%li)\n", id, ne->GetIntNumber());
	
	switch (id) {
    	case 0: fUpdateInterval = ne->GetIntNumber(); break; 
        case 1: fPSDbin = ne->GetIntNumber(); break; 
		case 2: fBoard = ne->GetIntNumber(); break; 
        case 3: fCh2D = ne->GetIntNumber(); break; 
        case 4: fTimerValue = ne->GetIntNumber(); break;    
    }
	if (id==0){
		fDrawTimer->Disconnect();
		fDrawTimer->Stop();
		fDrawTimer->Connect("Timeout()", "WindowMain", this, "UpdateCanvasPeriodically()");
        fDrawTimer->Start(fUpdateInterval, kFALSE);
	//reconnect timer with new value
	}
	
	if (id != 4){
	
		fCanvasNeedsUpdate = kTRUE;
		std::lock_guard<std::mutex> lock(fHistMutex);	
	
    	UpdateCanvas(fCanvas, fStopwatch, fTimingLabel, fTimingValue, fCheckVP, fCheckStates, fRadioStates, fDrawingStyle,
        	         h_trace, h_ampl, h_integral, h_charge, h_psd, h_psd_ampl, h_psd_int, h_det, h_counts, h_layers, h_xy,
					 fBoard, fCh2D, IsDebugEnabled(), IsShowStats(), fCanvasNeedsUpdate, fDrawChannels);
	
	}
}


void WindowMain::DoTextEntry() {
    if (!fTextEntryFileName) return;
	
	printf("textentry %s \n", fTextEntryFileName->GetText());

    std::string newFileName = fTextEntryFileName->GetText();
    if (newFileName.empty()) {
        new TGMsgBox(gClient->GetRoot(), fMain, "Error", "File name cannot be empty!", kMBIconExclamation, kMBOk);
        fTextEntryFileName->SetText(fOutputFileName.c_str());
        return;
    }

    // Ensure the file name ends with .root
    if (newFileName.find(".root") != newFileName.length() - 5) {
        newFileName += ".root";
        fTextEntryFileName->SetText(newFileName.c_str());
    }

    // Check if the file already exists
    std::ifstream fileCheck(newFileName);
    if (fileCheck.good()) { 
        Int_t ret;
        new TGMsgBox(gClient->GetRoot(), fMain, "File Exists",
                     Form("File '%s' already exists. Do you want to overwrite it?", newFileName.c_str()),
                     kMBIconQuestion, kMBYes | kMBNo, &ret);
        if (ret == kMBNo) {
            fTextEntryFileName->SetText(fOutputFileName.c_str());
            return;
        }
    }
    fileCheck.close();

    if (newFileName != fOutputFileName) {
        fOutputFileName = newFileName;
        if (IsDebugEnabled()) printf("DoTextEntry: Output file name set to %s, marked for reinitialization\n", fOutputFileName.c_str());
    }
}

void WindowMain::InitRootFile() {
std::lock_guard<std::mutex> lock(fRootFileMutex); // Protect fRootFile and fTree	
	
if (fSaveRawData) {
	
		std::ifstream fileCheck(fOutputFileName);
		printf("File name in textentry %s \n", fOutputFileName.c_str());
        if (fileCheck.good()) {
            Int_t ret;
            new TGMsgBox(gClient->GetRoot(), fMain, "File Exists",
                         Form("File '%s' already exists. Do you want to overwrite it?", fOutputFileName.c_str()),
                         kMBIconQuestion, kMBYes | kMBNo, &ret);
            if (ret == kMBNo) {
                if (IsDebugEnabled()) printf("InitRootFile: User declined to overwrite %s, disabling raw data saving\n", fOutputFileName.c_str());
                fSaveRawData = kFALSE;
                fCheckSaveRawData->SetState(kButtonUp);
                fTextEntryFileName->SetEnabled(kFALSE);
                fileCheck.close();
                return;
            }
        }
        fileCheck.close();
	
	// Close existing file if open
        if (fRootFile) {
            if (fTree && fTree->GetEntries() > 0) {
                fRootFile->cd();
                //fRootFile->Delete("RawDataTree;1"); // Delete previous cycle
                fTree->Write("", TObject::kOverwrite);
            }
            fRootFile->Write();
            fRootFile->Close();
            delete fRootFile;
            fRootFile = nullptr;
            fTree = nullptr;
            if (IsDebugEnabled()) printf("InitRootFile: Existing ROOT file closed\n");
        }

        delete fTrace;
        fTrace = nullptr;

        // Open or create the new ROOT file
        fRootFile = new TFile(fOutputFileName.c_str(), "RECREATE"); // Use RECREATE to start fresh
        if (!fRootFile || fRootFile->IsZombie()) {
            if (IsDebugEnabled()) printf("InitRootFile: Failed to create ROOT file %s\n", fOutputFileName.c_str());
            delete fRootFile;
            fRootFile = nullptr;
            return;
        }
        if (IsDebugEnabled()) printf("InitRootFile: %s file created\n", fOutputFileName.c_str());
        // Check if TTree already exists and delete it to avoid duplicates
		fRootFile->cd();
       
        // Create a new TTree
        fTree = new TTree("RawData", "RawData");
		fTree->SetAutoSave(0); // Disable automatic saving to prevent cycles
        fTrace = new std::vector<uint16_t>();

        // Define branches
        fTree->Branch("EventCounter", &fEventCounter, "EventCounter/i");
        fTree->Branch("Channel", &fChannel, "Channel/I");
        fTree->Branch("TimeStamp", &fTimeStamp, "TimeStamp/i");
        fTree->Branch("Trace", "std::vector<uint16_t>", &fTrace);
        
    } else {
       if (fRootFile) {
            if (fTree && fTree->GetEntries() > 0) {
                fRootFile->cd();
                //fRootFile->Delete("RawDataTree;1"); // Delete previous cycle
                fTree->Write("", TObject::kOverwrite);
            }
            fRootFile->Write();
            fRootFile->Close();
            delete fRootFile;
            fRootFile = nullptr;
            fTree = nullptr;
            if (IsDebugEnabled()) printf("InitRootFile: ROOT file closed\n");
        }

        delete fTrace;
        fTrace = nullptr;
    }
}

void WindowMain::WriteToRootFile() {
    while (!fStopWriting) {
        std::unique_lock<std::mutex> lock(fDataQueueMutex);
        fDataQueueCond.wait(lock, [this] { return !fDataQueue.empty() || fStopWriting; });

        while (!fDataQueue.empty()) {
            RawDataEvent data = std::move(fDataQueue.front());
            fDataQueue.pop();
            lock.unlock();
			
			// Synchronize access to fTree and fRootFile
            std::lock_guard<std::mutex> rootLock(fRootFileMutex);
            if (!fTree || !fRootFile) {
                if (IsDebugEnabled()) printf("WriteToRootFile: TTree or TFile is null, skipping write\n");
                lock.lock();
                continue;
            }
			
            // Fill the TTree
            fEventCounter = data.eventCounter;
            fChannel = data.channel;
            fTimeStamp = data.timestamp;
            *fTrace = std::move(data.trace);
            fTree->Fill();

            // Periodically write to disk to prevent data loss
            if (fTree->GetEntries() % 1000 == 0) {
                fTree->AutoSave("SaveSelf"); // Save the TTree without creating new cycles
                if (IsDebugEnabled()) printf("WriteToRootFile: AutoSaved TTree at %lld entries\n", fTree->GetEntries());
            }

            lock.lock();
        }
    }

    // Final write before exiting
	std::lock_guard<std::mutex> rootLock(fRootFileMutex);
    if (fTree && fTree->GetEntries() > 0) {
		fRootFile->cd();
        fTree->Write("", TObject::kOverwrite); // Overwrite the TTree in the file
        fRootFile->Write();
		if (IsDebugEnabled()) printf("WriteToRootFile: Final TTree write with %lld entries\n", fTree->GetEntries());
    }
}

void WindowMain::HandleInit() {
	
    if (fButtonInit->IsEnabled()) {
        if (IsDebugEnabled()) printf("HandleInit: Initializing for mode %s\n", DigiName[(int)fDeviceType]);// change that shite
		fInit = kTRUE; 
        if (fDeviceType == DeviceType::Simulated) {
			if (fDrawChannels.size()==0) {fDrawChannels.push_back(true); fDrawChannels.push_back(true);} // two times
            if (fParams.polarity == 0) {
                if (IsDebugEnabled()) printf("\nError: Polarity not set for Simulated mode\n");
                return;
            }
            if (IsDebugEnabled()) printf("HandleInit: Simulated mode initialized\n");
			for (int i = 0; i<6; i++){
				fCBHistoTypes[i]->SetEnabled(kTRUE);
				fCBHistoTypes[i]->SetState(i==0 ? kButtonDown : kButtonUp);
			}
        }else if (fDigitizer[0]->Init()) { 
        	if (IsDebugEnabled()) printf("HandleInit: %s initialized successfully\n", DigiName[(int)fDeviceType]);
			if (Vcfg[0].NB==2){
				if (fDigitizer[1]->Init())  
        			if (IsDebugEnabled()) printf("HandleInit: Slave %s initialized successfully\n", DigiName[(int)fDeviceType]);
			}
        } else {
        	printf("Error: Failed to initialize board\n");
            SignalParameters params = fParams;
            fDeviceType = DeviceType::Simulated;
            UpdateSignalParameters(params);
            fCBDeviceType->Select(0);
            return;
        }
		
	    fButtonInit->SetEnabled(kFALSE);
		fCBDeviceType->SetEnabled(kFALSE);
		EnableControlButtons(kTRUE);
    }
}

void WindowMain::UpdateCountRateLabel() {
    Double_t elapsedTime = fCountRateStopwatch.RealTime();
    if (elapsedTime > 0) {
		std::lock_guard<std::mutex> lock(fHistMutex);
		        
		Double_t dataRate[2] = {fDataCount[0] / (elapsedTime * 1048576.0f), fDataCount[1] / (elapsedTime * 1048576.0f)}; // 1024*1024
        TString rateStr[2] = {TString::Format("Read @ %.2f MB/s Trg. Rate: ", dataRate[0]), TString::Format("Read @ %.2f MB/s Trg. Rate: ", dataRate[1])};
		
		if (fDeviceType == DeviceType::V1730) {
            for (int b = 0; b < Vcfg[0].NB;  b++) {
            	for (int ch = 0; ch < Vcfg[0].Nch;  ch++) {
					if (IsDebugEnabled()) printf(" Trg. Rate: %i ",fSignalCount[b][ch]);
                	Double_t countRate = fSignalCount[b][ch] / elapsedTime;
                	rateStr[b] += countRate>0 ? TString::Format(" CH[%d]%.2f Hz ", ch, countRate) : TString::Format("CH[%d] No data...", ch);
            	}
			}
        } else {
            // Single trigger rate for Simulated, N957, DT5720
            Double_t countRate = fSignalCount[0][0] / elapsedTime;
            rateStr[0] += TString::Format("%.2f Hz ", countRate);
        }
        if (fDeviceType == DeviceType::V1730){
			for (int b = 0; b<Vcfg[0].NB; b++)
				fTrgRateLabel[b]->SetText(rateStr[b]);
		}
		else if (fDeviceType == DeviceType::DT5770){
			TString crStr = TString::Format("Trg. Rate: %.2f Hz", fSignalCount[0][0] / elapsedTime);
			fTrgRateLabel[0]->SetText(crStr + GetStatistic());
		}
		else 	
			fTrgRateLabel[0]->SetText(rateStr[0]);
							  
        if (IsDebugEnabled()) {
			if (fDeviceType != DeviceType::V1730)
				printf("\n UpdateCountRateLabel: %s\n", rateStr[0].Data());
			else
				printf("\n UpdateCountRateLabel: B[0] %s\n B[1] %s\n", rateStr[0].Data(), rateStr[1].Data());
		}
    }
		
	memset(fSignalCount, 0, sizeof(fSignalCount));
	memset(fDataCount, 0, sizeof(fDataCount));
	
    fCountRateStopwatch.Start(kTRUE); // Reset stopwatch
}


void WindowMain::AcquisitionThread() {
    while (fIsRunning) {
        std::vector<Double_t> data;
        bool hasData = false;
		std::vector<ProcessedEvent> Evts[2]; // Currently up to 2 boards
        
        if (fDeviceType == DeviceType::Simulated) {
            std::lock_guard<std::mutex> lock(fQueueMutex);
			
            if (!fSignalQueue.empty()) {
                data = fSignalQueue.front();
                fSignalQueue.pop();
                {
                    std::lock_guard<std::mutex> lock(fHistMutex);
                    fSignalCount[0][0]++;
                }
                if (IsDebugEnabled()) printf("AcquisitionThread: Simulated mode, fSignalCount=%u\n", fSignalCount[0][0]);
                
                ProcessedEvent evt;
                evt.channel = 0;
                evt.trace = data;
                evt.timestamp = 0;
                evt.eventCounter = fSignalCount[0][0];
                evt.polarity = fParams.polarity;
                //{
                //    std::lock_guard<std::mutex> lock(fAnalysisQueueMutex);
                //    fAnalysisQueue.push(evt);
                //    fAnalysisQueueCond.notify_one();
                //}
				Evts[0].push_back(std::move(evt));
                hasData = true;
            }
        } else{
			int nBoards = (fDeviceType == DeviceType::V1730) ? Vcfg[0].NB : 1;
			for (int b = 0; b<nBoards; b++){
				if (fDigitizer[b] && fDigitizer[b]->ReadData(Evts[b])) { 
            		hasData = true;
            		if (IsDebugEnabled()) printf("AcquisitionThread: %s mode B[%i], Events=%lu\n", DigiName[(int)fDeviceType], b, Evts[b].size());
            		// Save raw data to TTree if enabled
            		if (fSaveRawData && fTree) {
						std::lock_guard<std::mutex> lock(fDataQueueMutex);
						for (const auto& evt : Evts[b]){
                			RawDataEvent eventData;
                			eventData.eventCounter = evt.eventCounter;
                			eventData.channel = evt.channel;
							eventData.board = evt.board;
                			eventData.timestamp = evt.timestamp;
							eventData.charge = evt.charge;
					
                			eventData.trace.resize(evt.trace.size());
                			for (size_t j = 0; j < evt.trace.size(); ++j) {
								eventData.trace[j] = static_cast<uint16_t>(evt.trace[j]);
                			}
					                
                			fDataQueue.push(std::move(eventData));
						}
            	    	fDataQueueCond.notify_one();
           	 		}
				}		
			}	
        }
		//else if (IsDebugEnabled()) printf("AcquisitionThread: No data \n");

        if (hasData) {
			auto start = std::chrono::high_resolution_clock::now();
			{
            			std::lock_guard<std::mutex> lock(fAnalysisQueueMutex);
				if (fDeviceType == DeviceType::V1730 && Vcfg[0].NB==2){
					std::vector<ProcessedEvent> combinedEvts;
                			for (int i = 0; i < ((fDeviceType == DeviceType::V1730) ? Vcfg[0].NB : 1); ++i) {
                	    			if (!Evts[i].empty()) {
                    	    				combinedEvts.insert(combinedEvts.end(), Evts[i].begin(), Evts[i].end());
                    				}
                			}
					fAnalysisQueue.push(combinedEvts);
				}
				else
					fAnalysisQueue.push(Evts[0]);
		
		        	fAnalysisQueueCond.notify_one();
				if (IsDebugEnabled()) printf("AcquisitionThread: Data in queue Evts[0][0].trace[13] : %0.2f\n", Evts[0][0].trace[13]);
			}
			auto end = std::chrono::high_resolution_clock::now();
            fTimingValue[1] = std::chrono::duration<double, std::milli>(end - start).count();
			           
			if (fDeviceType == DeviceType::V1730 && fCheckStates[11] ) {
				auto start = std::chrono::high_resolution_clock::now();
				{
                	std::lock_guard<std::mutex> lock(fCoincidenceQueueMutex);
					
					std::vector<ProcessedEvent> combinedEvts;
                	for (int i = 0; i < ((fDeviceType == DeviceType::V1730) ? Vcfg[0].NB : 1); ++i) {
            	        if (!Evts[i].empty()) {
         	               combinedEvts.insert(combinedEvts.end(), Evts[i].begin(), Evts[i].end());
	                    }
                	}
					fCoincidenceQueue.push(combinedEvts);
                	//fCoincidenceQueue.push(evts);
                	fCoincidenceQueueCond.notify_one();
				}	
				auto end = std::chrono::high_resolution_clock::now();
                fTimingValue[3] = std::chrono::duration<double, std::milli>(end - start).count();
            }
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
			//std::this_thread::yield();
        } 
    }
}

void WindowMain::AnalysisThread() {
	uint32_t totalProcessed = 0;
	while (fIsRunning || !fAnalysisQueue.empty()) {
        std::vector<ProcessedEvent> eventsToProcess;
        {	
			auto start = std::chrono::high_resolution_clock::now();
            std::unique_lock<std::mutex> lock(fAnalysisQueueMutex);
            fAnalysisQueueCond.wait(lock, [this] { return !fAnalysisQueue.empty() || !fIsRunning; });
            if (fAnalysisQueue.empty() && !fIsRunning) break;
            
            while (!fAnalysisQueue.empty()) {
                //eventsToProcess.push_back(fAnalysisQueue.front());
				eventsToProcess = fAnalysisQueue.front();
                fAnalysisQueue.pop();
            }
            auto end = std::chrono::high_resolution_clock::now();
            fTimingValue[2] = std::chrono::duration<double, std::milli>(end - start).count();
        }
				
        for (const auto &evt : eventsToProcess) {
			std::vector<Double_t> traceCopy = evt.trace; // Create a non-const copy
						
			if (fDeviceType == DeviceType::Simulated) {
				
				int ch = fRand.Rndm()>=0.5 ? 1 : 0; 
            	std::lock_guard<std::mutex> lock(fHistMutex);
           		if (!h_trace[0][0][ch]) {
               		if (IsDebugEnabled()) printf("AnalysisThread: Error: h_trace[0] is null, skipping\n");
                	continue;
            	}
            	
            	for (Int_t i = 0; i < fParams.nSamples && i < static_cast<Int_t>(traceCopy.size()); ++i) 
               		h_trace[0][0][ch]->SetBinContent(i + 1, traceCopy[i]);
            	
            	CalcParams(traceCopy, evt.polarity, fPSDbin, h_ampl[0][ch], h_integral[0][ch], h_psd[0][ch], h_psd_ampl[0][ch], h_psd_int[0][ch]);
				totalProcessed++;
			} else if (fDeviceType == DeviceType::N957) {
            	std::lock_guard<std::mutex> lock(fHistMutex);
            	FillHistogram(traceCopy, h_ampl[0][0]);
        	}else if (fDeviceType == DeviceType::DT5770) {
								
            	std::lock_guard<std::mutex> lock(fHistMutex);
            	h_trace[0][0][0]->Reset();
				for (Int_t j = 0; j < static_cast<Int_t>(traceCopy.size()); ++j) 
                	h_trace[0][0][0]->SetBinContent(j + 1, traceCopy[j]);
				
				for (Int_t n = 0; n < 3; n++) {
					h_trace[0][n+1][0]->Reset();
					for (Int_t j = 0; j < static_cast<Int_t>(evt.probe[n].size()); ++j) 
                		h_trace[0][n+1][0]->SetBinContent(j + 1, evt.probe[n][j]);
				}	
				
				h_det->Reset();
				for (Int_t j = 0; j < static_cast<Int_t>(evt.det.size()); ++j) 
					h_det->SetBinContent(j + 1, evt.det[j]);
								
				CalcParams(traceCopy, evt.polarity, fPSDbin, h_ampl[0][evt.channel], h_integral[0][evt.channel], h_psd[0][evt.channel],
                       evt.channel == fCh2D ? h_psd_ampl[0][evt.channel] : nullptr, evt.channel == fCh2D ? h_psd_int[0][evt.channel] : nullptr);
				totalProcessed++;
				
        	} else if (fDeviceType == DeviceType::DT5720) {
				
            	std::lock_guard<std::mutex> lock(fHistMutex);
            	h_trace[0][0][evt.channel]->Reset();
            	for (Int_t j = 0; j < static_cast<Int_t>(traceCopy.size()); ++j) {
                	h_trace[0][0][evt.channel]->SetBinContent(j + 1, traceCopy[j]);
            	}
            	CalcParams(traceCopy, evt.polarity, fPSDbin, h_ampl[0][evt.channel], h_integral[0][evt.channel], h_psd[0][evt.channel],
                       evt.channel == fCh2D ? h_psd_ampl[0][evt.channel] : nullptr, evt.channel == fCh2D ? h_psd_int[0][evt.channel] : nullptr);
				totalProcessed++;
				
			} else if (fDeviceType == DeviceType::V1730) {
				
            	std::lock_guard<std::mutex> lock(fHistMutex);
            	h_trace[evt.board][0][evt.channel]->Reset();
            	for (Int_t j = 0; j < static_cast<Int_t>(traceCopy.size()); ++j) {
                	h_trace[evt.board][0][evt.channel]->SetBinContent(j + 1, traceCopy[j]);
            	}
				h_charge[evt.board][evt.channel]->Fill(evt.charge);
            	CalcParams(traceCopy, evt.polarity, fPSDbin, h_ampl[evt.board][evt.channel], h_integral[evt.board][evt.channel], h_psd[evt.board][evt.channel],
                       evt.channel == fCh2D ? h_psd_ampl[evt.board][evt.channel] : nullptr, evt.channel == fCh2D ? h_psd_int[evt.board][evt.channel] : nullptr);
				totalProcessed++;
				
        	}
			
    	}
	}	
}


void WindowMain::CoincidenceThread() {
    while (fIsRunning || !fCoincidenceQueue.empty()) {
        std::vector<ProcessedEvent> eventsToProcess;
        {	
			auto start = std::chrono::high_resolution_clock::now();
            std::unique_lock<std::mutex> lock(fCoincidenceQueueMutex);
            fCoincidenceQueueCond.wait(lock, [this] { return !fCoincidenceQueue.empty() || !fIsRunning; });
            if (fCoincidenceQueue.empty() && !fIsRunning) break;

            while (!fCoincidenceQueue.empty()) {
                eventsToProcess = fCoincidenceQueue.front();
                fCoincidenceQueue.pop();
            }
			auto end = std::chrono::high_resolution_clock::now();
            fTimingValue[4] = std::chrono::duration<double, std::milli>(end - start).count();
        }
        if (fDeviceType == DeviceType::V1730 && fCheckStates[11]) {
            std::lock_guard<std::mutex> lock(fHistMutex);
            FillHitPattern(eventsToProcess, h_xy, 1000); // 1 us = 1000 ns, charge threshold = 100
        }
    }
}


void WindowMain::UpdateCanvasPeriodically() {
    if (!fIsRunning) return;

    if (IsDebugEnabled()) printf("UpdateCanvasPeriodically: Updating canvas\n");
    
    TString timeStr = TString::Format("Time: %.0f s", fElapsedTimer.RealTime());
    fLabelElapsedTime->SetText(timeStr);
    fElapsedTimer.Continue(); // Continue timing without resetting
	
	//fCanvasNeedsUpdate = kTRUE;
	
	std::lock_guard<std::mutex> lock(fHistMutex);	
		
    UpdateCanvas(fCanvas, fStopwatch, fTimingLabel, fTimingValue, fCheckVP, fCheckStates, fRadioStates, fDrawingStyle, 
                  h_trace, h_ampl, h_integral, h_charge, h_psd, h_psd_ampl, h_psd_int, h_det, h_counts, h_layers, h_xy,
				  fBoard, fCh2D, IsDebugEnabled(), IsShowStats(), fCanvasNeedsUpdate, fDrawChannels);
}

void WindowMain::HandleTab(Int_t id) {
    fCanvasNeedsUpdate = kTRUE;
    if (!fCanvas || !fEmbCanvas) {
        printf("Error: Canvas is null\n");
        return;
    }
	    
    fDrawingStyle = (id == 0); // 0 = Channels tab, 1 = Types tab
	
	fCanvasNeedsUpdate = kTRUE;
	
	std::lock_guard<std::mutex> lock(fHistMutex);	
	
    UpdateCanvas(fCanvas, fStopwatch, fTimingLabel, fTimingValue, fCheckVP, fCheckStates, fRadioStates, fDrawingStyle,
                 h_trace, h_ampl, h_integral, h_charge, h_psd, h_psd_ampl, h_psd_int, h_det, h_counts, h_layers, h_xy,
				 fBoard, fCh2D, IsDebugEnabled(), IsShowStats(), fCanvasNeedsUpdate, fDrawChannels);
}

void WindowMain::HandleCheckButton() {
	TGCheckButton *cb = (TGCheckButton *) gTQSender;
	Int_t id = cb->WidgetId();
	if (IsDebugEnabled()) printf("CheckButton: %i\n", id);
	
    if (!fCanvas || !fEmbCanvas) {
        printf("Error: Canvas is null\n");
        return;
    }
    
    fCheckStates[id] = cb->GetState() == kButtonDown;
	
	fCanvasNeedsUpdate = kTRUE;
	
	std::lock_guard<std::mutex> lock(fHistMutex);
			
    UpdateCanvas(fCanvas, fStopwatch, fTimingLabel, fTimingValue, fCheckVP, fCheckStates, fRadioStates, kTRUE, // Always by channel in Channels tab
                 h_trace, h_ampl, h_integral, h_charge, h_psd, h_psd_ampl, h_psd_int, h_det, h_counts, h_layers, h_xy, 
				 fBoard, fCh2D, IsDebugEnabled(), IsShowStats(), fCanvasNeedsUpdate, fDrawChannels);
}

void WindowMain::HandleRadioButton() {
	TGRadioButton *rb = (TGRadioButton *) gTQSender;
	Int_t id = rb->WidgetId();
	if (IsDebugEnabled()) printf("RadioButton: %i\n", id);
	
    
    if (!fCanvas || !fEmbCanvas) {
        printf("Error: Canvas is null\n");
        return;
    }
		
    for (int i = 0; i < 7; i++){// be carefull with that absolute numbers		
		fRadioStates[i] = (i == id);
		if (IsDebugEnabled()) printf(" RB[%i] %s ", i, fRadioStates[i]?"true":"false");
        fRBHistoTypes[i]->SetState(i==id ? kButtonDown : kButtonUp);
	}	
	if (IsDebugEnabled()) printf(" \n");
	
	fCanvasNeedsUpdate = kTRUE;
	
	std::lock_guard<std::mutex> lock(fHistMutex);
	
    UpdateCanvas(fCanvas, fStopwatch, fTimingLabel, fTimingValue, fCheckVP, fCheckStates, fRadioStates, kFALSE, // Always by type in Types tab
                  h_trace, h_ampl, h_integral, h_charge, h_psd, h_psd_ampl, h_psd_int, h_det, h_counts, h_layers, h_xy, 
				  fBoard, fCh2D, IsDebugEnabled(), IsShowStats(), fCanvasNeedsUpdate, fDrawChannels);
}


void WindowMain::EnableControlButtons(Bool_t enable) {
    if (fButtonStart) fButtonStart->SetEnabled(enable);
    if (fButtonStop) fButtonStop->SetEnabled(enable);
    if (fButtonClear) fButtonClear->SetEnabled(enable);
	if (fButtonTemp && fDeviceType==DeviceType::V1730) fButtonTemp->SetEnabled(enable);
    if (IsDebugEnabled()) printf("EnableControlButtons: %s \n", enable ? "Enabled" : "Disabled");
						   
}

void WindowMain::TemperatureCheck() {
const int N = MAX_CH/2;
	
	for (int b = 0; b<Vcfg[0].NB; b++)	{
		TString temp_str[N];
		uint32_t temp_arr[N];	
		{	
			std::lock_guard<std::mutex> lock(fDigitizerMutex);
			fDigitizer[b]->GetTemperature(temp_arr);
			if (IsDebugEnabled()) printf("GetTemperature B[%i] S:%i T(Ch0-1): %i\n", b, Vcfg[b].SerialNumber, temp_arr[0] );
		}	
		for(int i=0;i<N;i++){
			
			temp_str[i] = TString::Format("%d", temp_arr[i]);
 			fTempLabel[b][i]->SetText(temp_str[i]);
			
			if (temp_arr[i] < 40){
				fTempLabel[b][i]->ChangeBackground(p_color[5]);
			}else if (temp_arr[i] < 58){
				fTempLabel[b][i]->ChangeBackground(p_color[3]);
			}else if (temp_arr[i] < 70){
				fTempLabel[b][i]->ChangeBackground(p_color[6]);
			}else {
				fTempLabel[b][i]->ChangeBackground(p_color[2]);
			}
		}		
	}	
}	

void WindowMain::ClearDrawing() {
    int B_CH = fDeviceType == DeviceType::V1730 ? Vcfg[0].NB : 1;
	int dumm[5] = {1, 1, 1, 2, Vcfg[0].Nch};
	int N_CH = dumm[(int)fDeviceType];
	
	if (IsDebugEnabled()) printf("ClearDrawing: Resetting histograms %i\n", N_CH);
	
	for (int b = 0; b<B_CH; b++)
		for (int i = 0; i<N_CH; i++)
    		h_ampl[b][i]->Reset();
	
	if (fDeviceType == DeviceType::DT5770){ 
		CAENDPP_ClearHistogram(fDigitizer[0]->GetHandle(), 0, 0); // handle channel histoIndex
		h_det->Reset();	
	}	
	
	if (fDeviceType != DeviceType::N957){
		for (int b = 0; b<B_CH; b++)
			for (int i = 0; i<N_CH; i++){
    			h_integral[b][i]->Reset();
    			h_psd[b][i]->Reset();
				h_psd_ampl[b][i]->Reset();
    			h_psd_int[b][i]->Reset();
			}	
    	
	}	
	
	if (fDeviceType == DeviceType::V1730){
		for (int b = 0; b<B_CH; b++)
			for (int i = 0; i<N_CH; i++)
    			h_charge[b][i]->Reset();
				
		h_counts->Reset();
    	h_layers->Reset();
		h_xy->Reset();	
	}	
	
	fElapsedTimer.Stop();
    fElapsedTimer.Reset();
	fCountRateStopwatch.Stop();
	fDroppedSignals = 0;
    fCountRateStopwatch.Reset();
    fLabelElapsedTime->SetText("Time: 0 s");
		
	
    UpdateCanvas(fCanvas, fStopwatch, fTimingLabel, fTimingValue, fCheckVP, fCheckStates, fRadioStates, fDrawingStyle,
                  h_trace, h_ampl, h_integral, h_charge, h_psd, h_psd_ampl, h_psd_int, h_det, h_counts, h_layers, h_xy,
				  fBoard, fCh2D, IsDebugEnabled(), IsShowStats(), fCanvasNeedsUpdate, fDrawChannels);
}

void WindowMain::StartDrawing() {
    if (IsDebugEnabled()) printf("StartDrawing: Checking conditions\n");
    if (!fIsRunning) {
        if (fParams.polarity == 0) {
            if (IsDebugEnabled()) printf("Error: Polarity not set (must be +1 or -1)\n");
            return;
        }
        fIsRunning = kTRUE;
		if (IsDebugEnabled()) printf("StartDrawing: Starting timer with interval=%d ms Running [%s]\n", fUpdateInterval, fIsRunning?"ON":"OFF");
		fButtonStart->SetEnabled(kFALSE);
		fButtonStop->SetEnabled(kTRUE);
        fElapsedTimer.Start(kTRUE);
		
		fCountRateTimer->Start(1000, kFALSE);
		fCountRateStopwatch.Start(kTRUE);
				
		memset(fSignalCount, 0, sizeof(fSignalCount));
		memset(fAbsCount, 0, sizeof(fAbsCount));
		memset(fDataCount, 0, sizeof(fDataCount));
				
		
        fDrawTimer->Connect("Timeout()", "WindowMain", this, "UpdateCanvasPeriodically()");
        fDrawTimer->Start(fUpdateInterval, kFALSE);
		
		if (fCTimer->GetState() == kButtonDown){
			fDAQTimer->Connect("Timeout()", "WindowMain", this, "StopDrawing()");
        	fDAQTimer->Start(fTimerValue * 1000, kFALSE); // timer in ms
		}	
		
		if (fDeviceType == DeviceType::Simulated)
        	StartSignalGeneration();
		if (fDeviceType == DeviceType::N957)
        	N957_StartAcquire(fDigitizer[0]->GetHandle(), Ncfg.ControlMode); //N957ControlModeAuto
		if (fDeviceType == DeviceType::DT5770)
        	CAENDPP_StartAcquisition(fDigitizer[0]->GetHandle(), 0);
		if (fDeviceType == DeviceType::DT5720)
        	CAEN_DGTZ_SWStartAcquisition(fDigitizer[0]->GetHandle());
		if (fDeviceType == DeviceType::V1730){
			if (Vcfg[0].NB==2) {
				for (int b = Vcfg[0].NB-1; b>=0; b--)
					CAEN_DGTZ_ClearData(fDigitizer[b]->GetHandle()); // necessary for multiboard start in future (and now it is)
			}	
			for (int b = Vcfg[0].NB-1; b>=0; b--){
				CAEN_DGTZ_SWStartAcquisition(fDigitizer[b]->GetHandle());
				printf("B[%i] SN: %i Start\n", b, Vcfg[b].SerialNumber);
			}
		
			//
        	CAEN_DGTZ_SWStartAcquisition(fDigitizer[0]->GetHandle());
		}
		// Initialize ROOT file and start writing thread if saving is enabled
        if (fSaveRawData) {
            InitRootFile();
            if (fRootFile) {
                new TGMsgBox(gClient->GetRoot(), fMain, "Info",
                             Form("Raw data will be saved to: %s", fOutputFileName.c_str()),
                             kMBIconAsterisk, kMBOk);
                fStopWriting = false;
                fWriteThread = std::thread(&WindowMain::WriteToRootFile, this);
                if (IsDebugEnabled()) printf("StartDrawing: Writing thread started\n");
            } else {
                if (IsDebugEnabled()) printf("StartDrawing: Failed to initialize ROOT file, disabling save\n");
                fSaveRawData = kFALSE;
                fTextEntryFileName->SetEnabled(kFALSE);
            }
        }
		
		// Start data acquisition thread
        try {
            fAcquisitionThread = std::thread(&WindowMain::AcquisitionThread, this);
    		fAnalysisThread = std::thread(&WindowMain::AnalysisThread, this);
			if(fDeviceType == DeviceType::V1730 && fCheckStates[11])
				fCoincidenceThread = std::thread(&WindowMain::CoincidenceThread, this);
			
        } catch (const std::exception& e) {
            printf("Error starting data thread: %s\n", e.what());
            fIsRunning = kFALSE;
        }
    }
}

void WindowMain::StopDrawing() {
    if (fIsRunning) {
        if (IsDebugEnabled()) printf("StopDrawing: Stopping timer\n");
        fIsRunning = kFALSE;
		fDrawTimer->Disconnect();
		if (fCTimer->GetState() == kButtonDown)
			fDAQTimer->Disconnect();
		
		fButtonStart->SetEnabled(kTRUE);
		fButtonStop->SetEnabled(kFALSE);
		
        fElapsedTimer.Stop();
        if (fDrawTimer) {
            fDrawTimer->Stop();
        }
		if (fDAQTimer) {
            fDAQTimer->Stop();
        }
		
		{
        	std::lock_guard<std::mutex> lock(fAnalysisQueueMutex);
        	fAnalysisQueueCond.notify_all();
    	}
		
		{
            std::lock_guard<std::mutex> lock(fCoincidenceQueueMutex);
            fCoincidenceQueueCond.notify_all();
        }
		
    
    	// Join threads
    	if (fAcquisitionThread.joinable()) {
			if (IsDebugEnabled()) printf("StopAcquisition: Joining fAcquisitionThread\n");
			fAcquisitionThread.join();
		}	
    	if (fAnalysisThread.joinable()) {
			if (IsDebugEnabled()) printf("StopAcquisition: Joining fAnalysisThread\n");
			fAnalysisThread.join();
		}	
		if (fCoincidenceThread.joinable()) {
            if (IsDebugEnabled()) printf("StopAcquisition: Joining fCoincidenceThread\n");
            fCoincidenceThread.join();
        }
		
		fIsRunning = kFALSE; 			
				
		fCountRateStopwatch.Stop();
  		if (fCountRateTimer) {
    	    fCountRateTimer->Stop();
	    }
		if (fDeviceType == DeviceType::Simulated)
        	StopSignalGeneration();
		if (fDeviceType == DeviceType::N957)
			N957_StopAcquire(fDigitizer[0]->GetHandle());
		if (fDeviceType == DeviceType::DT5770)
        	CAENDPP_StopAcquisition(fDigitizer[0]->GetHandle(), 0);
		if (fDeviceType == DeviceType::DT5720)
			CAEN_DGTZ_SWStopAcquisition(fDigitizer[0]->GetHandle());
		if (fDeviceType == DeviceType::V1730){
			for (int b = 0; b<Vcfg[0].NB; b++){
				CAEN_DGTZ_SWStopAcquisition(fDigitizer[b]->GetHandle());
				printf("B[%i] SN: %i Stoped\n", b, Vcfg[b].SerialNumber);
			}
		}
		
		// Stop and clean up the writing thread
        if (fSaveRawData) {
            fStopWriting = true;
            fDataQueueCond.notify_all();
            if (fWriteThread.joinable()) {
                fWriteThread.join();
                if (IsDebugEnabled()) printf("StopDrawing: Writing thread joined\n");
            }
            std::lock_guard<std::mutex> lock(fRootFileMutex);
            if (fRootFile) {
                if (fTree && fTree->GetEntries() > 0) {
                    fRootFile->cd();
                    fTree->Write("", TObject::kOverwrite);
                }
                fRootFile->Write();
				printf("Data saved in: %s\n",fRootFile->GetName());
				fRootFile->Close();
                delete fRootFile;
                fRootFile = nullptr;
                fTree = nullptr;
                if (IsDebugEnabled()) printf("StopDrawing: ROOT file closed\n");
            }
            delete fTrace;
            fTrace = nullptr;
        }
		
    } else {
        if (IsDebugEnabled()) printf("StopDrawing: Already stopped\n");
    }
}

void WindowMain::AddSignalToQueue(const std::vector<Double_t>& signal) {
    std::lock_guard<std::mutex> lock(fQueueMutex);
	if (fSignalQueue.size() < 2000) { // Increased queue size
         fSignalQueue.push(signal);
         if (IsDebugEnabled()) printf("AddSignalToQueue: Added signal, queue size=%lu, dropped=%lu\n",
                                     fSignalQueue.size(), fDroppedSignals.load());
    }
    else {
        fDroppedSignals++;
        if (IsDebugEnabled()) printf("AddSignalToQueue: Queue full, dropped signal #%lu\n", fDroppedSignals.load());
    }
}

void WindowMain::StartSignalGeneration() {
    if (!fSignalRunning) {
        fSignalRunning = true;
        printf("Starting signal generation thread\n");
        try {
            fSignalThread = std::thread([this]() {
                GenerateAndQueueSignal(this, fRand);
            });
            if (!fSignalThread.joinable()) {
                printf("Error: Signal thread failed to start\n");
                fSignalRunning = false;
            }
        } catch (const std::exception& e) {
            printf("Error starting signal thread: %s\n", e.what());
            fSignalRunning = false;
        }
    } else {
        printf("Signal thread already running\n");
    }
}

void WindowMain::StopSignalGeneration() {
    if (fSignalRunning) {
        if (IsDebugEnabled()) printf("StopSignalGeneration: Stopping signal thread\n");
        fSignalRunning = false;
        if (fSignalThread.joinable()) {
            fSignalThread.join();
            if (IsDebugEnabled()) printf("StopSignalGeneration: Signal thread joined\n");
        } 
    } 
}

void WindowMain::SignalDialogClosed() {
    if (IsDebugEnabled()) printf("SignalDialogClosed: Dialog closed\n");
    fSignalDialog = nullptr;
}

void WindowMain::UpdateSignalParameters(const SignalParameters& params) {
    std::lock_guard<std::mutex> lock(fQueueMutex);
    if (IsDebugEnabled()) {
        printf("UpdateSignalParameters: polarity=%d, sampleWidth=%.2f, nSamples=%d, frequency=%d\n",
               params.polarity, params.sampleWidth, params.nSamples, params.frequency);
    }
    fParams = params;
    if (fParams.frequency <= 0) fParams.frequency = 1;
	
	memset(fSignalCount, 0, sizeof(fSignalCount));
	memset(fDataCount, 0, sizeof(fDataCount));
	
    fCountRateStopwatch.Start(kTRUE); // Reset stopwatch

    // Clear the signal queue to prevent using old signals
    while (!fSignalQueue.empty()) {
        fSignalQueue.pop();
    }

 	{
    	std::lock_guard<std::mutex> lock(fHistMutex);
		
		for (int i = 0; i<2; i++){
        	if (h_trace[0][0][i]) {
        		h_trace[0][0][i]->Reset();
            	h_trace[0][0][i]->SetBins(fParams.nSamples, 0.0, fParams.nSamples * fParams.sampleWidth);
            	h_trace[0][0][i]->GetYaxis()->SetRangeUser(fParams.polarity == -1 ? -1.1 : -0.1, fParams.polarity == -1 ? 0.1 : 1.1);
        	} else {
           		h_trace[0][0][i] = new TH1D("h_trace", "PciDAQ Signal Trace;Time (ns);Amplitude (arb. units)",
                			      fParams.nSamples, 0.0, fParams.nSamples * fParams.sampleWidth);
   	       		h_trace[0][0][i]->SetLineColor(kBlue);
   	     	}
			if (fDeviceType == DeviceType::Simulated && i == 0)
				h_trace[0][0][i]->GetYaxis()->SetRangeUser(fParams.polarity == -1 ? -1.1 : -0.1, fParams.polarity == -1 ? 0.1 : 1.1);
			
       	}	
    }
	
    // Restart signal generation with new parameters
    if (fIsRunning) {
        StopSignalGeneration();
        StartSignalGeneration();
    }
}

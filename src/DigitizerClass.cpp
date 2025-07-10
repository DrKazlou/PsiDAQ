
#include "WindowMain.h"
#include "DigitizerClass.h"

#include <TMath.h>
#include <fstream>
#include <iostream>
#include <TGMsgBox.h>
#include <TGClient.h>

// N957Digitizer implementation
N957Digitizer::N957Digitizer(WindowMain* w, N957MCAConfig* cfg)
    : Digitizer(DeviceType::N957, w), config(cfg), buffer(nullptr) {}

N957Digitizer::~N957Digitizer() {
    Close();
}

bool N957Digitizer::Init() {
    if (handle != -1) {
        if (debug) printf("InitN957: N957 already initialized\n");
        return true;
    }

    buffer = (N957_UINT16*)malloc(config->BLDIM * sizeof(N957_UINT16));
    if (!buffer) {
        printf("Error: Failed to allocate N957 buffer\n");
        return false;
    }

    int ret = N957_Init(config->port, &handle);
    if (debug) printf("Init: ret %d handle %d\n", ret, handle);

    if (ret != 0) {
        if (debug) printf("InitN957: Failed to initialize N957, error code %d\n", ret);
        free(buffer);
        buffer = nullptr;
        handle = -1;
        return false;
    }

    char fw_rev[10];
    ret = N957_GetFWRelease(handle, fw_rev, 10);
    if (window) {
        char msg[200];
        sprintf(msg, "Board FW %s\nret=%d handle=%d\n", fw_rev, ret, handle);
        new TGMsgBox(gClient->GetRoot(), window->GetMainFrame(), "Info", msg, kMBIconAsterisk, kMBOk);
    }

    if (Program()) {
        if (debug) printf("InitN957: Successfully initialized\n");
        return true;
    }

    return false;
}

bool N957Digitizer::Program() {
    int ret = N957_SetLLD(handle, config->thr);
    if (debug) printf("SetLLD[%d] ret=%d handle=%d\n", config->thr, ret, handle);

    N957_UINT32 data32;
    ret |= N957_GetScaler(handle, &data32);
    if (debug) printf("Scaler: %d\n", data32);
    ret |= N957_GetTimer(handle, &data32);
    if (debug) printf("Timer: %d\n", data32);
    ret |= N957_GetLiveTime(handle, &data32);
    if (debug) printf("LiveTime: %d\n", data32);

    if (ret != 0) {
        if (debug) printf("ProgramN957: Failed to configure, error code %d\n", ret);
        N957_SwClear(handle);
        return false;
    }

    return true;
}

bool N957Digitizer::ReadData(std::vector<ProcessedEvent> &evts) {
    if (!buffer) {
        if (debug) printf("N957Digitizer::ReadData: Null buffer\n");
        return false;
    }

    N957_UINT16 data_read = config->BLDIM; //32768; // BLDIM
    N957ErrorCodes ret = N957_ReadData(handle, buffer, &data_read);
    if (ret != N957Success) {
        if (debug) printf("N957Digitizer::ReadData: Error reading data, code: %s\n", N957_DecodeError(ret));
        return false;
    }

    if (data_read == 0) {
        return false;
    }

    window->IncrementSignalCount(0, 0, data_read);
    	
    ProcessedEvent evt;
    evt.channel = 0;
    evt.trace.resize(data_read);
    for (uint16_t i = 0; i < data_read; ++i) {
        evt.trace[i] = static_cast<Double_t>(buffer[i]); // not a trace just an PHA values to fill in vector
    }
    evt.timestamp = 0;
    evt.eventCounter = 0;
    evt.polarity = 0; 
	
	evts.push_back(std::move(evt));
	
    if (debug) printf("N957Digitizer::ReadData: Read %u samples\n", data_read);
    return true;
}


void N957Digitizer::Close() {
    if (handle != -1) {
        if (debug) printf("CloseN957: Closing N957\n");
        N957_End(handle);
        handle = -1;
    }

    if (buffer) {
        if (debug) printf("Freeing N957 buffer\n");
        free(buffer);
        buffer = nullptr;
    }
}

void N957Digitizer::FreeBuffer() {
    if (buffer) {
        free(buffer);
        buffer = nullptr;
    }
}

			// DT5770Digitizer implementation
DT5770Digitizer::DT5770Digitizer(WindowMain* w, DT5770DigiConfig* cfg)
    : Digitizer(DeviceType::DT5770, w), config(cfg), AT1(nullptr), AT2(nullptr), DT1(nullptr), DT2(nullptr), h1(nullptr) {}

DT5770Digitizer::~DT5770Digitizer() {
    Close();
	if (AT1) free(AT1);
    if (AT2) free(AT2);
    if (DT1) free(DT1);
    if (DT2) free(DT2);
    if (h1) free(h1);
}

void DT5770Digitizer::InitDgtzParams(CAENDPP_DgtzParams_t *Params) {
    // set listMode default parameters
    Params->ListParams.enabled = FALSE;
    Params->ListParams.saveMode = CAENDPP_ListSaveMode_FileBinary;
    strcpy(Params->ListParams.fileName, "UNNAMED");
    Params->ListParams.maxBuffNumEvents = 0;
    Params->ListParams.saveMask = 0xF;

    // default board parameters
    Params->ChannelMask = 0; // it will be filled later
    Params->EventAggr = 0;
    Params->IOlev = CAENDPP_IOLevel_NIM;
    
    // Generic Writes to Registers
    Params->GWn = 0;                                    // Number of Generic Writes
    memset(Params->GWaddr, 0, MAX_GW*sizeof(uint32_t)); // List of addresses (length = 'GWn')
    memset(Params->GWdata, 0, MAX_GW*sizeof(uint32_t)); // List of datas (length = 'GWn')
    memset(Params->GWmask, 0, MAX_GW*sizeof(uint32_t)); // List of masks (length = 'GWn')

    // Waveform parameters default settings
    Params->WFParams.dualTraceMode = 1;
    Params->WFParams.vp1 = CAENDPP_PHA_VIRTUALPROBE1_Input;
    Params->WFParams.vp2 = CAENDPP_PHA_VIRTUALPROBE2_TrapBLCorr;
    Params->WFParams.dp1 = CAENDPP_PHA_DigitalProbe1_Peaking;
    Params->WFParams.dp2 = CAENDPP_PHA_DigitalProbe2_Trigger;
    Params->WFParams.recordLength = (int32_t)26214; // (8192);
    Params->WFParams.preTrigger = (int32_t)(2000);
    Params->WFParams.probeSelfTriggerVal = 150;
    Params->WFParams.probeTrigger = CAENDPP_PHA_PROBETRIGGER_MainTrig;
    
    // Channel parameters
    for (int32_t ch = 0; ch < 1; ch++) { // MAX_BOARD_CHNUM
        // Channel parameters
        //Params->DCoffset[ch] = 7864;
        Params->DCoffset[ch] = 5000;
        Params->PulsePolarity[ch] = CAENDPP_PulsePolarityPositive;
		//Params->PulsePolarity[ch] = CAENDPP_PulsePolarityNegative;
        
        // Coicidence parameters between channels
        Params->CoincParams[ch].CoincChMask = 0x0;
        Params->CoincParams[ch].CoincLogic = CAENDPP_CoincLogic_None;
        Params->CoincParams[ch].CoincOp = CAENDPP_CoincOp_OR;
        Params->CoincParams[ch].MajLevel = 0;
        Params->CoincParams[ch].TrgWin = 0;

        // DPP Parameters
        Params->DPPParams.M[ch] = 2000;        // Signal Decay Time Constant
        Params->DPPParams.m[ch] = 600;         // Trapezoid Flat Top
        Params->DPPParams.k[ch] = 5000;         // Trapezoid Rise Time
        Params->DPPParams.ftd[ch] = 600;       // Flat Top Delay
        Params->DPPParams.a[ch] = 2;            // Trigger Filter smoothing factor
        Params->DPPParams.b[ch] = 1000;          // Input Signal Rise time
        Params->DPPParams.thr[ch] = 50;       // Trigger Threshold
        Params->DPPParams.nsbl[ch] = 30;         // Number of Samples for Baseline Mean
        Params->DPPParams.nspk[ch] = 1;         // Number of Samples for Peak Mean Calculation
        Params->DPPParams.pkho[ch] = 0;         // Peak Hold Off
        Params->DPPParams.blho[ch] = 1500;      // Base Line Hold Off
        Params->DPPParams.dgain[ch] = 1;        // Digital Probe Gain
        Params->DPPParams.enf[ch] = 1.0;        // Energy Nomralization Factor
        Params->DPPParams.decimation[ch] = 0;   // Decimation of Input Signal   
        Params->DPPParams.trgho[ch] = 1300;      // Trigger Hold Off

        // Reset Detector
        Params->ResetDetector[ch].Enabled = 0;
        Params->ResetDetector[ch].ResetDetectionMode =
            CAENDPP_ResetDetectionMode_Internal;
        Params->ResetDetector[ch].thrhold = 100;
        Params->ResetDetector[ch].reslenmin = 2;
        Params->ResetDetector[ch].reslength = 2000;

        // Parameters for X770
        Params->DPPParams.X770_extraparameters[ch].CRgain = 0;
        Params->DPPParams.X770_extraparameters[ch].InputImpedance = CAENDPP_InputImpedance_50O;
		//Params->DPPParams.X770_extraparameters[ch].InputImpedance = CAENDPP_InputImpedance_1K;
        Params->DPPParams.X770_extraparameters[ch].TRgain = 0;
        Params->DPPParams.X770_extraparameters[ch].SaturationHoldoff = 300; //300
		Params->DPPParams.X770_extraparameters[ch].energyFilterMode = 0;
		Params->DPPParams.X770_extraparameters[ch].trigK = 300;
		Params->DPPParams.X770_extraparameters[ch].trigm = 10;
		Params->DPPParams.X770_extraparameters[ch].trigMODE = 0;

        Params->SpectrumControl[ch].SpectrumMode = CAENDPP_SpectrumMode_Energy;
        Params->SpectrumControl[ch].TimeScale = 1;
    }
}

bool DT5770Digitizer::Init() {
    if (handle != -1) {
        if (debug) printf("InitDT5720: DT5720 already initialized\n");
        return true;
    }
			
	config->acqMode = CAENDPP_AcqMode_Waveform; // Acquisition Mode (Histogram or Waveform)
	config->InputRange = CAENDPP_InputRange_10_0Vpp;
	
	InitDgtzParams(&config->DParams);
	
	config->DParams.ChannelMask = 0x1;

	
	int32_t ret = CAENDPP_InitLibrary(&handle); // The handle will be used to command the library
    if (ret){
		new TGMsgBox(gClient->GetRoot(), window->GetMainFrame(), "Error", Form("Init Library Error  \n ret = %i \n", ret), kMBIconStop, kMBOk);
		return ret;
	}	
		
	config->connParam.LinkType = CAENDPP_ETH;
    config->connParam.LinkNum = 0;
    config->connParam.ConetNode = 0;
    config->connParam.VMEBaseAddress = 0x0;
			
	printf(" Connection: LinkType[%i]  LinkNum[%i] Addr (%s) \n", config->connParam.LinkType, config->connParam.LinkNum, config->connParam.ETHAddress);
	ret = CAENDPP_AddBoard(handle, config->connParam, &config->BID);
	if (ret){
		new TGMsgBox(gClient->GetRoot(), window->GetMainFrame(), "Error", Form("Add Board Error  \n ret = %i \n", ret), kMBIconStop, kMBOk);
		return ret;
	}	
			
	ret = CAENDPP_GetDPPInfo(handle, config->BID, &config->info);
	if (ret){
		new TGMsgBox(gClient->GetRoot(), window->GetMainFrame(), "Error", Form("GET_DPPInfo Error \n ret = %i \n", ret), kMBIconStop, kMBOk);
		return ret;
	}	
		
	new TGMsgBox(gClient->GetRoot(), window->GetMainFrame(), "Info", 
				 Form("Board %s_%u added succesfully (ret = %i) \n", config->info.ModelName, config->info.SerialNumber, ret), kMBIconAsterisk, kMBOk);
		
	ret = CAENDPP_SetBoardConfiguration(handle, config->BID, config->acqMode, config->DParams);
	
	printf("Board is configurated (ret = %i): \n", ret);
	
	ret = CAENDPP_GetBoardConfiguration(handle, config->BID, &config->acqMode, &config->DParams);
	
	printf("Board config (ret = %i): \n \t \t Polarity %i  \n \t \t DCOffset %i \t \t RecordLength %i \n", ret, config->DParams.PulsePolarity[0], config->DParams.DCoffset[0], config->DParams.WFParams.recordLength);
		
	ret = CAENDPP_SetInputRange(handle, 0, config->InputRange);
	
	printf("Input range (ret = %i): %i \n ", ret, config->InputRange);
	
	AT1 = (int16_t*)realloc(AT1, config->DParams.WFParams.recordLength * sizeof(int16_t));
    AT2 = (int16_t*)realloc(AT2, config->DParams.WFParams.recordLength * sizeof(int16_t));
    DT1 = (uint8_t*)realloc(DT1, config->DParams.WFParams.recordLength * sizeof(uint8_t));
    DT2 = (uint8_t*)realloc(DT2, config->DParams.WFParams.recordLength * sizeof(uint8_t));
    h1 = (uint32_t*)calloc(MAX_HISTO_NBINS, sizeof(uint32_t));
	
    if (!AT1 || !AT2 || !DT1 || !DT2 || !h1) 
		return false;
	else 
		printf("Memory for traces and histo allocated successfully \n ");

	return true;
	
}	

bool DT5770Digitizer::ReadData(std::vector<ProcessedEvent> &evts) {
	int32_t ret = 0;
    uint32_t u32, ns = 0;
    double tsamples;
    uint64_t realTime, deadTime;
    int32_t h_index, h_nbins;
    CAENDPP_AcqStatus_t acqStatus;
		
	ret = CAENDPP_GetWaveform(handle, 0, 0, AT1, AT2, DT1, DT2, &ns, &tsamples);
			
    if (ret) {
		TThread::Lock();
        new TGMsgBox(gClient->GetRoot(), window->GetMainFrame(), "Error", Form("Can not read waveform ret = %i", ret), kMBIconStop, kMBOk);
		TThread::UnLock();
        return false;
    }

	if (ns > 0){
		window->IncrementSignalCount(0, 0, 1);
		ret = CAENDPP_GetCurrentHistogram(handle, 0, h1, &u32, &realTime, &deadTime, &acqStatus);
		float real_s = realTime / 1000000000.0;
		float dead_s = deadTime / 1000000000.0;
		
		
		window->SetStatistic(TString::Format(" RealTime %.1f s DeadTime %.1f s Good (%i CNT) Count Rate: %0.2f Hz", real_s, dead_s, u32, u32/real_s));
			
    	if (ret) {
			TThread::Lock();
			new TGMsgBox(gClient->GetRoot(), window->GetMainFrame(), "Error", Form("Can not get current histogram ret = %i", ret), kMBIconStop, kMBOk);
			TThread::UnLock();
        	return false;
		}
		ret = CAENDPP_GetCurrentHistogramIndex(handle, 0, &h_index);
		ret = CAENDPP_GetHistogramSize(handle, 0, h_index, &h_nbins);
		//if (debug) {
		//   printf("Histo (%i bins) RealTime [%.3f s] DeadTime [%.0f ms] Good (%i CNT)\n",
        //   h_nbins, realTime / 1000000000.0, deadTime / 1000000.0, u32);
    	//}
		
		ProcessedEvent evt;
		evt.channel = 0;
		evt.trace.resize(ns);
		for (int n = 0; n < 3; n++) 
			evt.probe[n].resize(ns);
			
		for (uint32_t i = 0; i < ns; i++){ 
			evt.trace[i] = static_cast<Double_t>(AT1[i]); 
			evt.probe[0][i] = static_cast<int16_t>(AT2[i]); 
			evt.probe[1][i] = static_cast<int16_t>(DT1[i]) * 2000; 
			evt.probe[2][i] = static_cast<int16_t>(DT2[i]) * 2000; 
		}	
		evt.det.resize(h_nbins);
		for (int32_t i = 0; i < h_nbins; i++) 
			evt.det[i] = h1[i]; 
		
		evts.push_back(std::move(evt));	
	}
		
	return !evts.empty();
}

void DT5770Digitizer::Close() {
    if (handle != -1) {
        if (debug) printf("CloseDT5770: Closing DT5770\n");
        CAENDPP_EndLibrary(handle);
        handle = -1;
    }
}

////////////////////////////////////////////
/////// DT5720Digitizer implementation//////
////////////////////////////////////////////

DT5720Digitizer::DT5720Digitizer(WindowMain* w, DT5720DigiConfig* cfg)
    : Digitizer(DeviceType::DT5720, w), config(cfg), buffer(nullptr), event(nullptr) {}

DT5720Digitizer::~DT5720Digitizer() {
    Close();
}

bool DT5720Digitizer::Init() {
    if (handle != -1) {
        if (debug) printf("InitDT5720: DT5720 already initialized\n");
        return true;
    }

    CAEN_DGTZ_ConnectionType LinkType = CAEN_DGTZ_USB;
    int LinkNum = config->port;
    int ConetNode = 0;
    uint32_t BaseAddress = 0;

    CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_OpenDigitizer2(LinkType, (void*)&LinkNum, ConetNode, BaseAddress, &handle);
    if (ret) {
        new TGMsgBox(gClient->GetRoot(), window->GetMainFrame(), "Error", Form("Can't open digitizer\nret=%d", ret), kMBIconStop, kMBOk);
        return false;
    }

    CAEN_DGTZ_BoardInfo_t BoardInfo;
    ret = CAEN_DGTZ_GetInfo(handle, &BoardInfo);
    if (window) {
        new TGMsgBox(gClient->GetRoot(), window->GetMainFrame(), "Info",
                     Form("Connected to CAEN Digitizer Model %s\nROC FPGA Release is %s\nAMC FPGA Release is %s\n",
                          BoardInfo.ModelName, BoardInfo.ROC_FirmwareRel, BoardInfo.AMC_FirmwareRel),
                     kMBIconAsterisk, kMBOk);
    }

    // Temporary calibration hack
    for (int ch = 0; ch < 2; ch++) {
        config->DAC_Calib.cal[ch] = 1.0;
        config->DAC_Calib.offset[ch] = 0.0;
    }

    if (!Program()) {
        CAEN_DGTZ_CloseDigitizer(handle);
        handle = -1;
        return false;
    }

    ret = CAEN_DGTZ_AllocateEvent(handle, (void**)&event);
    if (ret) {
        if (window) {
            new TGMsgBox(gClient->GetRoot(), window->GetMainFrame(), "Error", Form("ALLOCATE_EVENT_FAILURE\nret=%d", ret), kMBIconStop, kMBOk);
        }
        CAEN_DGTZ_CloseDigitizer(handle);
        handle = -1;
        return false;
    }

    uint32_t AllocatedSize;
    ret = CAEN_DGTZ_MallocReadoutBuffer(handle, &buffer, &AllocatedSize);
    if (ret) {
        if (window) {
            new TGMsgBox(gClient->GetRoot(), window->GetMainFrame(), "Error", Form("MALLOC_READOUT_BUFFER_FAILURE\nret=%d", ret), kMBIconStop, kMBOk);
        }
        CAEN_DGTZ_FreeEvent(handle, (void**)&event);
        CAEN_DGTZ_CloseDigitizer(handle);
        handle = -1;
        return false;
    }
		
    if (debug) {
        printf("GUI Threshold [0] = %d\n", config->thr[0]);
        printf("GUI Threshold [1] = %d\n", config->thr[1]);
        ret = CAEN_DGTZ_GetChannelTriggerThreshold(handle, 0, &config->cal_thr[0]);
        ret = CAEN_DGTZ_GetChannelTriggerThreshold(handle, 1, &config->cal_thr[1]);
        printf("FPGA Threshold [0] = %d\n", config->cal_thr[0]);
        printf("FPGA Threshold [1] = %d\n", config->cal_thr[1]);
        for (int i = 0; i < config->Nch; i++) {
            printf("Trigger for CH[%d] will be: %d (cal_thr[%d]: %d)\n", i, config->cal_thr[i] - config->thr[i], i, config->cal_thr[i]);
        }
    }
	
    return true;
}

bool DT5720Digitizer::Program() {
    CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_Reset(handle);
    if (ret != CAEN_DGTZ_Success) {
        printf("Error resetting digitizer\n");
        return false;
    }

    uint32_t UserRecordLength = config->RecordLength[0];
    ret = CAEN_DGTZ_SetRecordLength(handle, config->RecordLength[0]);
    ret = CAEN_DGTZ_GetRecordLength(handle, &config->RecordLength[0]);
    if (UserRecordLength != config->RecordLength[0] && debug) {
        printf("Initial UserRecordLength [%d]: was changed by FPGA and now it - [%d]\n", UserRecordLength, config->RecordLength[0]);
    }

    ret = CAEN_DGTZ_SetPostTriggerSize(handle, config->PostTrigger);
    ret = CAEN_DGTZ_SetIOLevel(handle, config->FPIOtype);
    ret = CAEN_DGTZ_SetMaxNumEventsBLT(handle, config->NumEvents);
    ret = CAEN_DGTZ_SetAcquisitionMode(handle, CAEN_DGTZ_SW_CONTROLLED);
    ret = CAEN_DGTZ_SetExtTriggerInputMode(handle, config->ExtTriggerMode);
    ret = CAEN_DGTZ_SetChannelEnableMask(handle, config->EnableMask);

    for (int ch = 0; ch < config->Nch; ch++) {
        if (config->EnableMask & (1 << ch)) {
            ret = CAEN_DGTZ_SetChannelDCOffset(handle, ch, config->DCOffset[ch]);
            ret = CAEN_DGTZ_SetChannelSelfTrigger(handle, config->ChannelTriggerMode[ch], (1 << ch));
            ret = CAEN_DGTZ_SetChannelTriggerThreshold(handle, ch, config->thr[ch]);
            ret = CAEN_DGTZ_SetTriggerPolarity(handle, ch, config->PulsePolarity[ch]);
        }
    }

    ret = Set_relative_Threshold(handle, config); // in FuncMain.cpp
    if (ret != CAEN_DGTZ_Success) {
        printf("Warning: Errors found during digitizer programming\n");
        return false;
    }

    return true;
}

bool DT5720Digitizer::ReadData(std::vector<ProcessedEvent> &evts) {
	CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_Success;
    uint32_t BufferSize = 0, NumEvents = 0;
    CAEN_DGTZ_EventInfo_t EventInfo;
    char* EventPtr = nullptr;
	
	if (debug) printf("DT5720Digitizer::ReadData\n");
	
    ret = CAEN_DGTZ_ReadData(handle, CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, buffer, &BufferSize);
    if (ret != CAEN_DGTZ_Success) {
        if (debug) printf("DT5720Digitizer::ReadData: ERR_READ_DATA, ret=%d\n", ret);
        return false;
    }
	
	NumEvents = 0;
	if (BufferSize != 0) {
		ret = CAEN_DGTZ_GetNumEvents(handle, buffer, BufferSize, &NumEvents);
        if (ret){ 
           	if (debug) printf("DT5720Digitizer::ReadData: ERR_GET_NUM_EVENTS, ret=%d\n", ret);
			return false;
		}	
			
    }
	else {
		uint32_t lstatus;
		ret = CAEN_DGTZ_ReadRegister(handle, CAEN_DGTZ_ACQ_STATUS_ADD, &lstatus);
		if (ret) 
			printf("Warning: Failure reading reg:%x \n (%d)", CAEN_DGTZ_ACQ_STATUS_ADD, ret);
		else {
			if (lstatus & (0x1 << 19)) 
				printf("ERR_OVERTEMP \n");
		}
	}
	
    if (NumEvents > 0) {
    
    	window->IncrementSignalCount(0, 0, NumEvents);
   		window->IncrementDataCount(0, BufferSize);
		
		for (uint32_t n = 0; n < NumEvents; ++n) {
			ret = CAEN_DGTZ_GetEventInfo(handle, buffer, BufferSize, 0, &EventInfo, &EventPtr);
    		if (ret != CAEN_DGTZ_Success) {
      			TThread::Lock();
      			new TGMsgBox(gClient->GetRoot(), window->GetMainFrame(), "Error", Form("DT5720Digitizer::ReadData: ERR_GET_EVENT_INFO, ret=%i", ret), kMBIconStop, kMBOk);
       			TThread::UnLock();
        		return false;
    		}

    		ret = CAEN_DGTZ_DecodeEvent(handle, EventPtr, (void**)&event);
    		if (ret != CAEN_DGTZ_Success) {
       			TThread::Lock();
        		new TGMsgBox(gClient->GetRoot(), window->GetMainFrame(), "Error", Form("DT5720Digitizer::ReadData: ERR_DECODE_EVENT, ret=%i", ret), kMBIconStop, kMBOk);
        		TThread::UnLock();
        		return false;
    		}

    		for (int ch = 0; ch < config->Nch; ch++) {
        		if (config->EnableMask & (1 << ch)) {
					ProcessedEvent evt;
         			evt.channel = ch;
          			evt.trace.resize(event->ChSize[ch]);
            		for (uint32_t j = 0; j < event->ChSize[ch]; ++j) {
                		evt.trace[j] = static_cast<Double_t>(event->DataChannel[ch][j]);
					}
            		evt.timestamp = EventInfo.TriggerTimeTag;
            		evt.eventCounter = EventInfo.EventCounter;
   	         		evt.polarity = config->PulsePolarity[ch] == CAEN_DGTZ_TriggerOnRisingEdge ? 1 : -1;
				
					evts.push_back(std::move(evt));
   	         		if (debug) printf("DT5720Digitizer::ReadData: Event %u, channel %d, trace_size=%u\n", EventInfo.EventCounter, ch, event->ChSize[ch]);
          		}
    		}
		}	
	}	
	FreeEvent();	
	
	return !evts.empty();
}	

void DT5720Digitizer::Close() {
    if (handle != -1) {
        if (debug) printf("CloseDT5720: Closing DT5720\n");
        CAEN_DGTZ_CloseDigitizer(handle);
        handle = -1;
    }
    FreeEvent();
    FreeBuffer();
}

void DT5720Digitizer::FreeBuffer() {
    if (buffer) {
        if (debug) printf("Freeing DT5720 buffer\n");
        CAEN_DGTZ_FreeReadoutBuffer(&buffer);
        buffer = nullptr;
    }
}

void DT5720Digitizer::FreeEvent() {
    if (event) {
        if (debug) printf("Freeing DT5720 event\n");
        CAEN_DGTZ_FreeEvent(handle, (void**)&event);
        event = nullptr;
    }
}

// V1730Digitizer implementation (placeholder)
V1730Digitizer::V1730Digitizer(WindowMain* w, V1730DigiConfig* cfg)
    : Digitizer(DeviceType::V1730, w), config(cfg), buffer(nullptr), waveforms(nullptr)  {
	
	for (uint32_t i = 0; i < MAX_CH; i++) event[i] = nullptr;	
} 

V1730Digitizer::~V1730Digitizer() {
    Close();
}

bool V1730Digitizer::Init() {
	const uint32_t TrgHoldOffAddress[16] = {0x1074, 0x1174, 0x1274, 0x1374, 0x1474, 0x1574, 0x1674, 0x1774,
                                           0x1874, 0x1974, 0x1A74, 0x1B74, 0x1C74, 0x1D74, 0x1E74, 0x1F74};
	const uint32_t ExtraConfigAddress[16] = {0x1084, 0x1184, 0x1284, 0x1384, 0x1484, 0x1584, 0x1684, 0x1784,
                                            0x1884, 0x1984, 0x1A84, 0x1B84, 0x1C84, 0x1D84, 0x1E84, 0x1F84};
	
        
    CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_USB_A4818_V2718, config->PID, config->IsMaster ? 0 : 1, 0, &handle);
    if (ret != CAEN_DGTZ_Success) {
        new TGMsgBox(gClient->GetRoot(), window->GetMainFrame(), "Error", Form("Can't open digitizer\nret=%d", ret), kMBIconStop, kMBOk);
		Close();
        return false;
    }

    // Get board info
    CAEN_DGTZ_BoardInfo_t boardInfo;
    ret = CAEN_DGTZ_GetInfo(handle, &boardInfo);
    if (ret != CAEN_DGTZ_Success) {
        if (debug) printf("V1730Digitizer: Failed to get board info, error=%d\n", ret);
        Close();
        return false;
    }
	new TGMsgBox(gClient->GetRoot(), window->GetMainFrame(), "Info", 
				 Form("Connected to CAEN Digitizer \n Model %s, SN: %u\n ROC FPGA Release: %s\n AMC FPGA Release: %s\n",
    	         boardInfo.ModelName, boardInfo.SerialNumber, boardInfo.ROC_FirmwareRel, boardInfo.AMC_FirmwareRel), kMBIconAsterisk, kMBOk);
    
    config->SerialNumber = boardInfo.SerialNumber;
			
	if (debug) printf("----\nV1730Digitizer: Board initialized (serial=%u, isMaster=%d)\n----\n", config->SerialNumber, config->IsMaster);
	
    // Set channel mask
    config->ChannelMask &= ~0xFFFF;
    for (int i = 0; i < config->Nch; i++) 
        config->ChannelMask |= (1 << i);
    

    // Program digitizer
    if (!Program()) {
        if (debug) printf("V1730Digitizer: Failed to program digitizer\n");
        Close();
        return false;
    }

    // Allocate memory
    uint32_t allocatedSize;
    ret = CAEN_DGTZ_MallocReadoutBuffer(handle, &buffer, &allocatedSize);
    if (ret != CAEN_DGTZ_Success) {
        if (debug) printf("V1730Digitizer: Failed to allocate readout buffer, error=%d\n", ret);
        Close();
        return false;
    }
    
    ret = CAEN_DGTZ_MallocDPPEvents(handle, (void**)event, &allocatedSize);
    if (ret != CAEN_DGTZ_Success) {
        if (debug) printf("V1730Digitizer: Failed to allocate DPP events, error=%d\n", ret);
        Close();
        return false;
    }
        
    ret = CAEN_DGTZ_MallocDPPWaveforms(handle, (void**)&waveforms, &allocatedSize);
    if (ret != CAEN_DGTZ_Success) {
        if (debug) printf("V1730Digitizer: Failed to allocate DPP waveforms, error=%d\n", ret);
        Close();
        return false;
    }

    // Read status register
    uint32_t reg_data;
    ret = CAEN_DGTZ_ReadRegister(handle, 0x8000, &reg_data);
    if (debug) printf("V1730Digitizer: Status register 0x8000 = 0x%08X\n", reg_data);

    // Configure Trigger Hold-Off
    for (int ch = 0; ch < config->Nch; ch++) {
        ReadRegister(TrgHoldOffAddress[ch], &reg_data);
        reg_data = (uint32_t)(config->RecordLength[ch] / 4); // 8 ns steps, RecordLength in 2 ns bins
        WriteRegister(TrgHoldOffAddress[ch], reg_data);
    }

    // Configure Extra Data 
    for (int ch = 0; ch < config->Nch; ch++) {
        ReadRegister(ExtraConfigAddress[ch], &reg_data);
        reg_data = (reg_data & ~0x700) | (config->IsMaster ? (1 << 8) : (1 << 9)); // Master: 001, Slave: 010
        WriteRegister(ExtraConfigAddress[ch], reg_data);
    }

    // Set acquisition and trigger modes
    if (config->IsMaster) {
		// Master board settings
		ret = CAEN_DGTZ_SetRunSynchronizationMode(handle, CAEN_DGTZ_RUN_SYNC_TrgOutTrgInDaisyChain);
        ret = CAEN_DGTZ_SetAcquisitionMode(handle, CAEN_DGTZ_SW_CONTROLLED);
        ret = CAEN_DGTZ_SetSWTriggerMode(handle, CAEN_DGTZ_TRGMODE_ACQ_ONLY); 
		
		if(config->NB==2){
        	ret = CAEN_DGTZ_SetChannelSelfTrigger(handle, CAEN_DGTZ_TRGMODE_ACQ_ONLY, 0x2);//config->ChannelMask);// for multiboard only
        	ret = CAEN_DGTZ_SetExtTriggerInputMode(handle, CAEN_DGTZ_TRGMODE_ACQ_AND_EXTOUT);
		}else	
			ret = CAEN_DGTZ_SetExtTriggerInputMode(handle, CAEN_DGTZ_TRGMODE_ACQ_ONLY);
    } else {
		// Slave board settings
        ret = CAEN_DGTZ_SetAcquisitionMode(handle, CAEN_DGTZ_FIRST_TRG_CONTROLLED);
        ret = CAEN_DGTZ_SetSWTriggerMode(handle, CAEN_DGTZ_TRGMODE_DISABLED);
        ret = CAEN_DGTZ_SetChannelSelfTrigger(handle, CAEN_DGTZ_TRGMODE_DISABLED, 0x2);//config->ChannelMask);
        ret = CAEN_DGTZ_SetExtTriggerInputMode(handle, CAEN_DGTZ_TRGMODE_ACQ_ONLY);
		
		ret = CAEN_DGTZ_SetRunSynchronizationMode(handle, CAEN_DGTZ_RUN_SYNC_TrgOutTrgInDaisyChain);
    }

    if (ret != CAEN_DGTZ_Success) {
        if (debug) printf("V1730Digitizer: Failed to set acquisition/trigger modes, error=%d\n", ret);
       	Close();
        return false;
    }

    if (debug) printf("V1730Digitizer: Initialization complete for SN=%u\n", config->SerialNumber);
    return true;
}


bool V1730Digitizer::Program() {
    
    CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_Success;
	
    //Reset the digitizer
    ret = CAEN_DGTZ_Reset(handle);

    if (ret) {
        printf("ERROR: can't reset the digitizer.\n");
        return ret;
    }
	
    ret = CAEN_DGTZ_SetDPPAcquisitionMode(handle, config->AcqMode, CAEN_DGTZ_DPP_SAVE_PARAM_EnergyAndTime);
    ret = CAEN_DGTZ_SetAcquisitionMode(handle, CAEN_DGTZ_SW_CONTROLLED);
    ret = CAEN_DGTZ_SetIOLevel(handle, config->IOlev);
    ret = CAEN_DGTZ_SetExtTriggerInputMode(handle, CAEN_DGTZ_TRGMODE_ACQ_ONLY);
    ret = CAEN_DGTZ_SetChannelEnableMask(handle, config->ChannelMask);

    // Set how many events to accumulate in the board memory before being available for readout
    // ret = CAEN_DGTZ_SetDPPEventAggregation(handle, config->EventAggr, 0); //MARK 1: SET BOTH ZERO, see line below
	ret = CAEN_DGTZ_SetDPPEventAggregation(handle, 0, 0); 
    ret = CAEN_DGTZ_SetRunSynchronizationMode(handle, CAEN_DGTZ_RUN_SYNC_Disabled);
	    	
    // Set the DPP specific parameters for the channels in the given channelMask
    SetDPPParameters();
    
    for(int i=0; i<config->Nch; i++) {
        if (config->ChannelMask & (1<<i)) {
			//for x725 and x730 Recordlength is common to paired channels (you can set different RL for different channel pairs)
            SetRecordLength(config->RecordLength[i], i);
			SetDCOffset(i, config->DCOffset[i]); 
            SetPreTriggerSize(i, config->PreTrigger[i]); 
            SetPulsePolarity(i, config->PulsePolarity[i]);
			SetInputRange(i, config->InputRange[i]);
        }
    }
   
	ret = CAEN_DGTZ_SetDPP_VirtualProbe(handle, ANALOG_TRACE_1, CAEN_DGTZ_DPP_VIRTUALPROBE_Input);
	ret = CAEN_DGTZ_SetDPP_VirtualProbe(handle, ANALOG_TRACE_2, CAEN_DGTZ_DPP_VIRTUALPROBE_Baseline);
	ret = CAEN_DGTZ_SetDPP_VirtualProbe(handle, DIGITAL_TRACE_1, CAEN_DGTZ_DPP_DIGITALPROBE_Gate);
	ret = CAEN_DGTZ_SetDPP_VirtualProbe(handle, DIGITAL_TRACE_2, CAEN_DGTZ_DPP_DIGITALPROBE_GateShort);
    
	
	if(ret){
		printf("ERROR: can't Program Digitizer. \n");
		return false;
	}
		
    return true;
}

bool V1730Digitizer::ReadData(std::vector<ProcessedEvent> &evts) {
	CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_Success;
    uint32_t BufferSize = 0, NumEvents[M_CH] = {0};
	int b = config->IsMaster ? 0 : 1; // Master - B0, Slave - B1;

    ret = CAEN_DGTZ_ReadData(handle, CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, buffer, &BufferSize);
    if (ret != CAEN_DGTZ_Success) {
       	TThread::Lock();
        new TGMsgBox(gClient->GetRoot(), window->GetMainFrame(), "Error", Form("V1730Digitizer::ReadData: ERR_READ_DATA, ret=%i", ret), kMBIconStop, kMBOk);
        TThread::UnLock();
    	return false;
	}
	if(debug) printf("V1730Digitizer:[%i]:ReadData: BufferSize %i \n", config->SerialNumber, BufferSize);
    if (BufferSize > 0) {
            
   		ret = CAEN_DGTZ_GetDPPEvents(handle, buffer, BufferSize, (void**)event, NumEvents);
   		if (ret != CAEN_DGTZ_Success) {
        	TThread::Lock();
            new TGMsgBox(gClient->GetRoot(), window->GetMainFrame(), "Error", Form("V1730Digitizer::ReadData: ERR_GET_DPP_EVENTS, ret=%i", ret), kMBIconStop, kMBOk);
            TThread::UnLock();
       	    return false;
	    }

    	uint32_t maxNumEvents = 0;
    	for (int ch = 0; ch < config->Nch; ch++) {
     		if (config->ChannelMask & (1 << ch) && NumEvents[ch] > maxNumEvents) {
      	    	maxNumEvents = NumEvents[ch];
       		}
   		}

    	if (maxNumEvents > 0) {
       		window->IncrementDataCount(b, BufferSize);
	   
    		for (int ch = 0; ch < config->Nch; ch++) {
        		if ((config->ChannelMask & (1 << ch))) {
					//printf(" CH[%i]: %i (%i) ", ch, NumEvents[ch], event[ch][0].ChargeLong);
					window->IncrementSignalCount(b, ch, NumEvents[ch]);
					
					for (uint32_t n = 0; n < NumEvents[ch]; ++n) {
						ProcessedEvent evt;
            			evt.channel = ch;
						evt.board = b;
						
						if (config->AcqMode != CAEN_DGTZ_DPP_ACQ_MODE_List){
							ret = CAEN_DGTZ_DecodeDPPWaveforms(handle, &event[ch][0], waveforms);
            				if (ret != CAEN_DGTZ_Success) {
	              		  		TThread::Lock();
               		 			new TGMsgBox(gClient->GetRoot(), window->GetMainFrame(), "Error", Form("V1730Digitizer::ReadData: ERR_DECODE_DPP_WAVEFORMS, ret=%i", ret), kMBIconStop, kMBOk);
               					TThread::UnLock();
       	      	 	 			return false;
	            			}
            			
          		  			evt.trace.resize(waveforms->Ns);
        	    			for (uint32_t j = 0; j < waveforms->Ns; ++j) 
         		   	    		evt.trace[j] = static_cast<Double_t>(waveforms->Trace1[j]);
						}	
						//printf(" CH[%i]: RL %i Trace Size: %li", ch, waveforms->Ns, evt.trace.size());
        	    		evt.timestamp = event[ch][n].TimeTag;
						evt.charge = event[ch][n].ChargeLong;
        	    		evt.eventCounter = window->GetAbsCount(b, ch);
         	 			evt.polarity = config->PulsePolarity[ch] == CAEN_DGTZ_PulsePolarityPositive ? 1 : -1;
					
						evts.push_back(std::move(evt));
        	 			
					}
					if (debug) printf("V1730Digitizer:[%i]:ReadData[%p]: maxNumEvents %d \n", config->SerialNumber, &handle, maxNumEvents);
    		    }
   			}
			
		}else if (debug) printf("UpdateData: V1730 mode, no data read\n");
		//printf("\n");
	}//buffer if
	
	return !evts.empty();
}

bool V1730Digitizer::SetDPPParameters(){
	CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_Success;
	
	CAEN_DGTZ_DPP_PSD_Params_t DPPParams;
	
	 for(int i=0; i<config->Nch; i++) {
		 DPPParams.thr[i] = config->thr[i];
		 DPPParams.nsbl[i] = config->nsbl[i];
		 DPPParams.pgate[i] = config->pgate[i];
		 DPPParams.sgate[i] = config->sgate[i];
		 DPPParams.lgate[i] = config->lgate[i];
		 DPPParams.selft[i] = config->selft[i];
		 DPPParams.trgc[i] = config->trgc[i];
		 DPPParams.discr[i] = config->discr[i];
		 DPPParams.cfdd[i] = config->cfdd[i];
		 DPPParams.cfdf[i] = config->cfdf[i];
		 DPPParams.tvaw[i] = config->tvaw[i];
		 DPPParams.csens[i] = config->csens[i];
	 }	 
		DPPParams.purh = config->purh;
		DPPParams.purgap = config->purgap;
		DPPParams.blthr = config->blthr;
		//DPPParams.bltmo = Dcfg.bltmo;
		DPPParams.trgho = config->trgho;
	
    // Set the DPP specific parameters for the channels in the given channelMask
    ret = CAEN_DGTZ_SetDPPParameters(handle, config->ChannelMask, &DPPParams);
		
	if (ret) {
        printf("ERROR: can't set DPP Parameters.\n");
        return false;
    }
	
	return true;
}	

void V1730Digitizer::GetTemperature(uint32_t temp_arr[]){
	    
    uint32_t temp;
    CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_Success;
    for (int ch = 0; ch < MAX_CH; ch+=2) {
		int ind = ch/2;
        ret = CAEN_DGTZ_ReadTemperature(handle, ch, &temp);
		temp_arr[ind] = temp;
	}
	if (ret != CAEN_DGTZ_Success) {
		TThread::Lock();
        new TGMsgBox(gClient->GetRoot(), window->GetMainFrame(), "Error", Form("V1730Digitizer::GetTemperature: \nERR_READ_TEMPERATURE, ret=%i", ret), kMBIconStop, kMBOk);
        TThread::UnLock();
    }
	
}

bool V1730Digitizer::SetAcquisitionMode(CAEN_DGTZ_DPP_AcqMode_t AcqMode){ // improve that function to handle more cases
	if(window->IsInit()){
		CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_Success;
	
 		ret = CAEN_DGTZ_SetDPPAcquisitionMode(handle, AcqMode, CAEN_DGTZ_DPP_SAVE_PARAM_EnergyAndTime);//CAEN_DGTZ_DPP_SAVE_PARAM_TimeOnly
		
		printf("%s AcqMode changed to %i ret = %i \n", config->IsMaster ? "Master" : "Slave", AcqMode, ret);
	
		uint32_t reg_data = 0;
		CAEN_DGTZ_ReadRegister(handle, 0x8000, &reg_data);
		printf(" After SetDPPAcquisitionMode in  0x%04X: %08X \n", 0x8000, reg_data); 	
		reg_data |= (1<<17); //b17 responsible for Extra settings
		//reg_data &= ~(1<<19); //b19 responsible for charge recording [this line will switch off Charge]
		CAEN_DGTZ_WriteRegister(handle, 0x8000, reg_data); //should be 0xE0910
		CAEN_DGTZ_ReadRegister(handle, 0x8000, &reg_data);
		printf(" Corrected Extra bit in  0x%04X: %08X \n", 0x8000, reg_data); 
	}	
	else
		printf("Initialize V1730Digitizer to SetAcqMode \n");
	
	return true;
}

bool V1730Digitizer::SetInputRange(int channel, CAEN_DGTZ_InputRange_t InputRange){
	if(window->IsInit()){
		CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_Success;
		uint32_t d32;
		uint32_t InputDynamicRangeAddress[16] = { 0x1028, 0x1128, 0x1228, 0x1328, 0x1428, 0x1528, 0x1628, 0x1728,
												  0x1828, 0x1928, 0x1A28, 0x1B28, 0x1C28, 0x1D28, 0x1E28, 0x1F28};
		
		ret = CAEN_DGTZ_ReadRegister(handle, InputDynamicRangeAddress[channel], &d32);
		if (debug) printf("Previously in 0x%4x %i\n",InputDynamicRangeAddress[channel], d32);
    	
		ret = CAEN_DGTZ_WriteRegister(handle, InputDynamicRangeAddress[channel], InputRange);
	
		if (ret != CAEN_DGTZ_Success && debug){
			printf("V1730Digitizer: SetInputRange failed, error=%d\n", ret);
    		return false;
		}
	}	
	else
		printf("Initialize V1730Digitizer to SetInputRange \n");
	
	return true;
}


bool V1730Digitizer::SetPulsePolarity(int channel, CAEN_DGTZ_PulsePolarity_t polarity){
	if(window->IsInit()){
		CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_SetChannelPulsePolarity(handle, channel, polarity);
    	if (ret != CAEN_DGTZ_Success && debug){
			printf("V1730Digitizer: SetPulsePolarity failed, error=%d\n", ret);
    		return false;
		}	
	}	
	else
		printf("Initialize V1730Digitizer to SetPulsePolatiry \n");
	
	return true;	
}

bool V1730Digitizer::SetRecordLength(uint32_t length, int channel){
	if(window->IsInit()){
    	CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_Success;
		//for x725 and x730 Recordlength is common to paired channels (you can set different RL for different channel pairs)
		if (channel % 2 == 0)	
			ret = CAEN_DGTZ_SetRecordLength(handle, length, channel);
    	if (ret != CAEN_DGTZ_Success && debug){
			printf("V1730Digitizer: SetRecordLength failed, error=%d\n", ret);
    		return false;
		}	
	}	
	else
		printf("Initialize V1730Digitizer to SetRecordLength \n");
	
	return true;	
}

bool V1730Digitizer::SetPreTriggerSize(int channel, uint32_t size){
	if(window->IsInit()){
        CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_SetDPPPreTriggerSize(handle, channel, size);
		if (ret != CAEN_DGTZ_Success && debug){
			printf("V1730Digitizer: SetPreTriggerSize failed, error=%d\n", ret);
			return false;
		}	
	}	
	else
		printf("Initialize V1730Digitizer to SetPreTriggerSize \n");
	
	return true;	
}



bool V1730Digitizer::SetDCOffset(int channel, uint32_t offset){
	if(window->IsInit()){
		CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_SetChannelDCOffset(handle, channel, offset);
    	if (ret != CAEN_DGTZ_Success && debug){
			printf("V1730Digitizer: SetDCOffset failed, error=%d\n", ret);
			return false;
		}	
	}	
	else
		printf("Initialize V1730Digitizer to SetDCOffset \n");
	
	return true;	
}

bool V1730Digitizer::ReadRegister(uint32_t address, uint32_t* data){
	if(window->IsInit()){
		CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_ReadRegister(handle, address, data);
    	if (ret != CAEN_DGTZ_Success && debug){ 
			printf("V1730Digitizer: ReadRegister failed, error=%d\n", ret);
			return false;
		}	
	}	
	else
		printf("Initialize V1730Digitizer to ReadRegister \n");
	
	return true;
}

bool V1730Digitizer::WriteRegister(uint32_t address, uint32_t data){
	if(window->IsInit()){
		CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_WriteRegister(handle, address, data);
    	if (ret != CAEN_DGTZ_Success && debug){ 
			printf("V1730Digitizer: WriteRegister failed, error=%d\n", ret);
			return false;
		}	
	}	
	else
		printf("Initialize V1730Digitizer to WriteRegister \n");
	
	return true;
}

void V1730Digitizer::Close() {
    FreeEvent();
    FreeBuffer();
	FreeWaveforms();

	if (handle != -1) {
        if (debug) printf("Close: Closing V1730\n");
        CAEN_DGTZ_CloseDigitizer(handle);
        handle = -1;
    }
	exit(0); // be with that carefull
}

void V1730Digitizer::FreeBuffer() {
    if (buffer) {
        if (debug) printf("Freeing V1730 buffer\n");
        CAEN_DGTZ_FreeReadoutBuffer(&buffer);
        buffer = nullptr;
    }
}

void V1730Digitizer::FreeEvent() {
    if (event[0]) {
        if (debug) printf("Freeing V1730 events\n");
        CAEN_DGTZ_FreeDPPEvents(handle, (void**)event);
        for (uint32_t i = 0; i < MAX_CH; i++) event[i] = nullptr;
    }
}

void V1730Digitizer::FreeWaveforms() {
    if (waveforms) {
        if (debug) printf("Freeing V1730 waveforms\n");
        CAEN_DGTZ_FreeDPPWaveforms(handle, waveforms);
        waveforms = nullptr;
    }
}



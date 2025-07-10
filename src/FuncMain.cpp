#include "FuncMain.h"
#include "FuncGenerator.h"
#include "WindowMain.h"
#include <algorithm>
#include <TMath.h>
#include <TString.h>
#include <chrono>
#include <thread>

// do not move them into FuncMain.h to 
// avoid overlap with Status variable in "CAENDPPLib.h"

#include <X11/Xlib.h> 
#include <X11/Xutil.h> 
#include <TVirtualX.h> 
#include <X11/Xatom.h> 


#include <fstream>
#include <sstream>
#include <string>

static double linear_interp(double x0, double y0, double x1, double y1, double x) {
    if (x1 - x0 == 0) {
        fprintf(stderr, "Cannot interpolate values with same x.\n");
        return HUGE_VAL;
    }
    else {
        const double m = (y1 - y0) / (x1 - x0);
        const double q = y1 - m * x1;
        return m * x + q;
    }
}

void SetIcon(TGMainFrame *fMain, TString filename){
 Display *display = (Display*)gVirtualX->GetDisplay();
    Window window = fMain->GetId();
    Atom net_wm_icon = XInternAtom(display, "_NET_WM_ICON", False);
  
    const int width = 128;
    const int height = 128;
    unsigned long *icon_data = new unsigned long[2 + width * height];
    icon_data[0] = width;  // Width
    icon_data[1] = height; // Height

	// Open and read the text file
	std::ifstream file(filename); //"icon_data.txt"
	if (file.is_open()) {
	    std::string line;
	    int y = 0;
    
		while (std::getline(file, line) && y < height) {
        	std::stringstream ss(line);
        	int x = 0;
        	int value;
        	while (ss >> value && x < width) {
            	// Convert grayscale value (0-255) to ARGB hex
            	unsigned long color = 0xFF000000 | (value << 16) | (value << 8) | value;
            	icon_data[2 + y * width + x] = color;
            	++x;
        	}
        	++y;
    	}
	
    	file.close();
	} else {
    	// Default to black if file cannot be opened
    	for (int y = 0; y < height; ++y) {
       		for (int x = 0; x < width; ++x) {
            	icon_data[2 + y * width + x] = 0xFF000000; // Black background
        	}
    	}
	}
	
    // Set the property
    XChangeProperty(display, window, net_wm_icon, XA_CARDINAL, 32, PropModeReplace, (unsigned char*)icon_data, 2 + width * height);
    delete[] icon_data;				
}


bool ParseConfigFile(const std::string& filename, DeviceType AcqMode, N957MCAConfig *Ncfg, DT5770DigiConfig *Scfg, DT5720DigiConfig *Dcfg, V1730DigiConfig *Vcfg, Bool_t debugEnabled) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        if (debugEnabled) printf("ParseConfigFile: Unable to open %s\n", filename.c_str());
        return false;
    }
	
	char DigiName[5][13] = {"Simulate", "N957", "DT5770", "DT5720", "V1730"}; 
    std::string line;
    bool inN957Section = false;
	bool inDT5770Section = false;
    bool inDT5720Section = false;
	bool inV1730Section = false;
	
    while (std::getline(file, line)) {
        // Remove leading/trailing whitespace and comments
		size_t commentPos = line.find('#');
        if (commentPos != std::string::npos) line = line.substr(0, commentPos);
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') continue;

        // Check section markers
        if (line == "@N957") {
            inN957Section = true;
			inDT5770Section = false;
            inDT5720Section = false;
			inV1730Section = false;
            continue;
		} else if (line == "@DT5770") {
            inN957Section = false;
			inDT5770Section = true;
            inDT5720Section = false;
			inV1730Section = false;
            continue;
        } else if (line == "@DT5720") {
            inN957Section = false;
			inDT5770Section = false;
            inDT5720Section = true;
			inV1730Section = false;
            continue;
        } else if (line == "@V1730") {
            inN957Section = false;
			inDT5770Section = false;
            inDT5720Section = false;
			inV1730Section = true;
            continue;
        }

        // Process only the relevant section based on fAcqMode
        if ((AcqMode == DeviceType::N957 && !inN957Section) ||
			(AcqMode == DeviceType::DT5770 && !inDT5770Section) ||
            (AcqMode == DeviceType::DT5720 && !inDT5720Section) ||
		   	(AcqMode == DeviceType::V1730 && !inV1730Section) ) {
            continue;
        }
		
        std::istringstream iss(line);
        std::string key;
        if (std::getline(iss, key, ' ')) {
            std::vector<std::string> values;
            std::string valueStr;
            while (iss >> valueStr) {
                values.push_back(valueStr);
            }

            // Debug printout for all values
            if (debugEnabled) {
                printf("Key: [%s]", key.c_str());
                for (const auto& val : values) {
                    printf("(%s)", val.c_str());
                }
                printf("\n");
            }
			
            if (AcqMode == DeviceType::N957 && inN957Section) {
                if (!Ncfg) {
                    if (debugEnabled) printf("ParseConfigFile: Ncfg not initialized for N957\n");
                    continue;
                }
				
                if (key == "port") {
                    Ncfg->port = std::stoi(values[0]);
                } else if (key == "Threshold") {
                    Ncfg->thr = static_cast<N957_BYTE>(std::stoi(values[0]));
					printf("value %i thr %i \n",std::stoi(values[0]), Ncfg->thr);
                } else if (key == "BLDIM") {
                    Ncfg->BLDIM = static_cast<N957_UINT16>(std::stoi(values[0]));
                } else if (key == "ControlMode") {
                    if (values[0] == "AUTO") Ncfg->ControlMode = N957ControlModeAuto;
                    else if (values[0] == "EXT_GATE") Ncfg->ControlMode = N957ControlModeExtGate;
                }
			} else if (AcqMode == DeviceType::DT5770 && inDT5770Section) {
                if (!Scfg) {
                    if (debugEnabled) printf("ParseConfigFile: Scfg not initialized for DT5770\n");
                    continue;
                }	
				if (key == "IP") {
					strcpy(Scfg->connParam.ETHAddress, values[0].c_str()); 
                    if (debugEnabled) printf("in IP value : %s\n",values[0].c_str());
                }
            } else if (AcqMode == DeviceType::DT5720 && inDT5720Section) {
                if (!Dcfg) {
                    if (debugEnabled) printf("ParseConfigFile: Dcfg not initialized for DT5720\n");
                    continue;
                }

                if (key == "port") {
                    Dcfg->port = std::stoi(values[0]);
                } else if (key == "RecordLength") {
                    Dcfg->RecordLength[0] = std::stoul(values[0]);
                    Dcfg->RecordLength[1] = Dcfg->RecordLength[0];
				} else if (key == "PostTrigger") {
                    Dcfg->PostTrigger = std::stoi(values[0]);
                } else if (key == "NumEvents") {
                    Dcfg->NumEvents = std::stoi(values[0]);
                } else if (key == "FPIOtype") {
                    if(values[0] == "NIM") Dcfg->FPIOtype = CAEN_DGTZ_IOLevel_NIM;
					else if(values[0] == "TTL") Dcfg->FPIOtype = CAEN_DGTZ_IOLevel_TTL;
                } else if (key == "ExtTriggerMode") {
                    if (values[0] == "DISABLED") Dcfg->ExtTriggerMode = CAEN_DGTZ_TRGMODE_DISABLED;
                    else if (values[0] == "EXTOUT_ONLY") Dcfg->ExtTriggerMode = CAEN_DGTZ_TRGMODE_EXTOUT_ONLY;
                    else if (values[0] == "ACQ_ONLY") Dcfg->ExtTriggerMode = CAEN_DGTZ_TRGMODE_ACQ_ONLY;
                    else if (values[0] == "ACQ_AND_EXTOUT") Dcfg->ExtTriggerMode = CAEN_DGTZ_TRGMODE_ACQ_AND_EXTOUT;
                } else if (key == "ChannelTriggerMode") {
                    if (values.size() >= 1) {
                        if (values[0] == "DISABLED") Dcfg->ChannelTriggerMode[0] = CAEN_DGTZ_TRGMODE_DISABLED;
                        else if (values[0] == "EXTOUT_ONLY") Dcfg->ChannelTriggerMode[0] = CAEN_DGTZ_TRGMODE_EXTOUT_ONLY;
                        else if (values[0] == "ACQ_ONLY") Dcfg->ChannelTriggerMode[0] = CAEN_DGTZ_TRGMODE_ACQ_ONLY;
                        else if (values[0] == "ACQ_AND_EXTOUT") Dcfg->ChannelTriggerMode[0] = CAEN_DGTZ_TRGMODE_ACQ_AND_EXTOUT;
                    }
                    if (values.size() == 2) {
                        if (values[1] == "DISABLED") Dcfg->ChannelTriggerMode[1] = CAEN_DGTZ_TRGMODE_DISABLED;
                        else if (values[1] == "EXTOUT_ONLY") Dcfg->ChannelTriggerMode[1] = CAEN_DGTZ_TRGMODE_EXTOUT_ONLY;
                        else if (values[1] == "ACQ_ONLY") Dcfg->ChannelTriggerMode[1] = CAEN_DGTZ_TRGMODE_ACQ_ONLY;
                        else if (values[1] == "ACQ_AND_EXTOUT") Dcfg->ChannelTriggerMode[1] = CAEN_DGTZ_TRGMODE_ACQ_AND_EXTOUT;
                    }
                } else if (key == "Polarity") {
                    Dcfg->PulsePolarity[0] = (values[0] == "NEGATIVE") ? CAEN_DGTZ_TriggerOnFallingEdge : CAEN_DGTZ_TriggerOnRisingEdge;
                    Dcfg->PulsePolarity[1] = Dcfg->PulsePolarity[0];  
                } else if (key == "DCOffset") {
                    if (values.size() >= 1) Dcfg->DCOffset[0] = std::stoul(values[0]);
                    if (values.size() == 2) Dcfg->DCOffset[1] = std::stoul(values[1]);
                } else if (key == "Threshold") {
                    if (values.size() >= 1) Dcfg->thr[0] = std::stoul(values[0]);
                    if (values.size() == 2) Dcfg->thr[1] = std::stoul(values[1]);
                }
            }
			else if (AcqMode == DeviceType::V1730 && inV1730Section) {
                if (!Vcfg) {
                    if (debugEnabled) printf("ParseConfigFile: Vcfg not initialized for V1730\n");
                    continue;
                }
				
				if (key == "N_BOARDS"){
					printf("We will work with [%i] boards\n",std::stoi(values[0]));
					Vcfg->NB = std::stoi(values[0]);
				}
				else if (key == "PID") {
					Vcfg->PID = std::stoi(values[0]);
				}else if (key == "N_CH") {
				    Vcfg->Nch = std::stoi(values[0]);
                }else if (key == "RecordLength") {
					if (values.size()==1 || Vcfg->Nch!=(int)(2*values.size())){
						for (int i = 0; i<Vcfg->Nch; i++)
                    		Vcfg->RecordLength[i] = std::stoul(values[0]);
					}else{	
						for (int i = 0; i<Vcfg->Nch; i++)
                    		Vcfg->RecordLength[i] = std::stoul(values[i/2]);
						
					}
				} else if (key == "PreTrigger") {
					for (int i = 0; i<Vcfg->Nch; i++)
                    	Vcfg->PreTrigger[i] = std::stoi(values[0]);
                } else if (key == "EventAggr") {
					Vcfg->EventAggr = std::stoi(values[0]);
                }else if (key == "Polarity") {
					for (int i = 0; i<Vcfg->Nch; i++){	
						if(values[0] == "NEGATIVE") Vcfg->PulsePolarity[i] = CAEN_DGTZ_PulsePolarityNegative;
						else if(values[0] == "POSITIVE") Vcfg->PulsePolarity[i] = CAEN_DGTZ_PulsePolarityPositive;
					}
				}else if (key == "InputRange") {
					for (int i = 0; i<Vcfg->Nch; i++){	
						if(values[0] == "0_5") Vcfg->InputRange[i] = InputRange_0_5Vpp;
						else if(values[0] == "2_0") Vcfg->InputRange[i] = InputRange_2Vpp;
					}	
				}else if (key == "AcqMode") {
					if(values[0] == "OSCI") Vcfg->AcqMode = CAEN_DGTZ_DPP_ACQ_MODE_Oscilloscope;
					else if(values[0] == "LIST") Vcfg->AcqMode = CAEN_DGTZ_DPP_ACQ_MODE_List;
					else if(values[0] == "MIXED") Vcfg->AcqMode = CAEN_DGTZ_DPP_ACQ_MODE_Mixed;
				} 
				else if (key == "IOlev") {
                    if(values[0] == "NIM") Vcfg->IOlev = CAEN_DGTZ_IOLevel_NIM;
					else if(values[0] == "TTL") Vcfg->IOlev = CAEN_DGTZ_IOLevel_TTL;
                } else if (key == "DCOffset") {
					for (int i = 0; i<Vcfg->Nch; i++)
                    	Vcfg->DCOffset[i] = std::stoul(values[0]);
                    
                } else if (key == "Threshold") {
					for (int i = 0; i<Vcfg->Nch; i++)
 	                	Vcfg->thr[i] = std::stoul(values[0]);
                }
            }
        }
    }
	
    file.close();
    if (debugEnabled) printf("ParseConfigFile: Successfully parsed %s for %s mode\n",
                                filename.c_str(), DigiName[(int)AcqMode]);
	//printf("port %i thr %i BLDIM %i Mode %i \n", Ncfg->port, Ncfg->thr, Ncfg->BLDIM, Ncfg->ControlMode);
    return true;
}

void CalcParams(std::vector<Double_t>& signal, Int_t polarity, 
                Int_t psdBin, TH1D* h_ampl, TH1D* h_integral, TH1D* h_psd,
                TH2D* h_psd_ampl, TH2D* h_psd_int) {
	if (signal.empty()) return;
	//BL subtraction
	int BL_CUT = 10;
	double BL_mean = 0;
	for (int i = 0; i < BL_CUT; ++i) 
		BL_mean += signal[i];
	BL_mean /= BL_CUT;
	
	for (int i = 0; i < static_cast<int>(signal.size()); ++i) 
		signal[i] -= BL_mean;
	//BL subtraction
	
    // Calculate amplitude based on polarity
    Double_t peak = (polarity == -1) ? *std::min_element(signal.begin(), signal.end())
                                     : *std::max_element(signal.begin(), signal.end());
    int peak_pos = std::distance(signal.begin(), (polarity == -1) ? std::min_element(signal.begin(), signal.end())
                                                                   : std::max_element(signal.begin(), signal.end()));
    h_ampl->Fill(TMath::Abs(peak));
		
    // Calculate integral 
    Double_t integral = 0.0;
    for (int i = 0; i < static_cast<int>(signal.size()); ++i) 
        integral += signal[i];
    
    integral = TMath::Abs(integral);
    h_integral->Fill(integral);

    // Calculate PSD (Pulse Shape Discrimination)
    Double_t Qs = 0.0; // Short integral
    Double_t Ql = 0.0; // Long integral
    for (int i = peak_pos; i < static_cast<int>(signal.size()); ++i) {
        Double_t d = signal[i];
        Ql += d;
        if (i >= peak_pos + psdBin) {
            Qs += d;
        }
    }
    Double_t psd = (Ql != 0.0) ? (Qs / Ql) : 0.0;
    h_psd->Fill(psd);

    // Fill 2D histograms
	
    if (h_psd_ampl)	h_psd_ampl->Fill(TMath::Abs(peak), psd);
    if (h_psd_int)	h_psd_int->Fill(integral, psd);
		
}
///CAEN N957 MCA

void FillHistogram(const std::vector<Double_t>& data, TH1D* h_ampl) {
   
    for (const auto& value : data) 
        h_ampl->Fill(value);
	
}

void SetHistogram(const std::vector<Double_t>& data, TH1D* h_ampl) {
   
    printf("Histo Nbins: %i Data.size : %i\n", h_ampl->GetNbinsX(), static_cast<Int_t>(data.size()) );
	for (Int_t i = 0; i < static_cast<Int_t>(data.size()); ++i) 
    	h_ampl->SetBinContent(i + 1, data[i]);
    
	
}


CAEN_DGTZ_ErrorCode Set_calibrated_DCO(int handle, int ch, DT5720DigiConfig *Dcfg) {
	CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_Success;
	int dc_file[2] = {10, 10}; //dc position from config file in %, try to cut that shite also
	
	if (Dcfg->PulsePolarity[ch] == CAEN_DGTZ_TriggerOnRisingEdge) { //CAEN_DGTZ_PulsePolarityPositive
		Dcfg->DCOffset[ch] = (uint32_t)((float)(fabs((((float)dc_file[ch] - Dcfg->DAC_Calib.offset[ch]) / Dcfg->DAC_Calib.cal[ch]) - 100.))*(655.35));
		if (Dcfg->DCOffset[ch] > 65535) Dcfg->DCOffset[ch] = 65535;
		if (Dcfg->DCOffset[ch] < 0) Dcfg->DCOffset[ch] = 0;
	}
	else if (Dcfg->PulsePolarity[ch] == CAEN_DGTZ_TriggerOnFallingEdge) { //CAEN_DGTZ_PulsePolarityNegative
		Dcfg->DCOffset[ch] = (uint32_t)((float)(fabs(((fabs(dc_file[ch] - 100.) - Dcfg->DAC_Calib.offset[ch]) / Dcfg->DAC_Calib.cal[ch]) - 100.))*(655.35));
		if (Dcfg->DCOffset[ch] < 0) Dcfg->DCOffset[ch] = 0;
		if (Dcfg->DCOffset[ch] > 65535) Dcfg->DCOffset[ch] = 65535;
	}

	ret = CAEN_DGTZ_SetChannelDCOffset(handle, (uint32_t)ch, Dcfg->DCOffset[ch]);
		if (ret)
			printf("Error setting channel %d offset\n", ch);
	
	return ret;
}

CAEN_DGTZ_ErrorCode QuitThreshold(int handle, char* buffer, char* evtbuff){
	
CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_Success;
	
	if (evtbuff != NULL)
        ret = CAEN_DGTZ_FreeEvent(handle, (void**)&evtbuff);
	
    if (buffer != NULL)
        ret = CAEN_DGTZ_FreeReadoutBuffer(&buffer);
	
return ret;
}

CAEN_DGTZ_ErrorCode QuitMain(int handle, char* buffer){
	CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_Success;
	    ret = CAEN_DGTZ_FreeReadoutBuffer(&buffer);
		ret = CAEN_DGTZ_CloseDigitizer(handle);
				
		exit(0);
		return ret;
}

CAEN_DGTZ_ErrorCode  Get_current_baseline(int handle, DT5720DigiConfig *Dcfg, char* buffer, char* EventPtr, double *baselines) {
	CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_Success;
    uint32_t BufferSize = 0;
    int32_t i = 0;
    uint32_t max_sample = 0x1 << Dcfg->Nbit;

    CAEN_DGTZ_UINT16_EVENT_t* Event16 = (CAEN_DGTZ_UINT16_EVENT_t*)EventPtr;
    //CAEN_DGTZ_UINT8_EVENT_t* Event8 = (CAEN_DGTZ_UINT8_EVENT_t*)EventPtr;

    int32_t* histo = (int32_t*)malloc(max_sample * sizeof(*histo));
    if (histo == NULL) {
        fprintf(stderr, "Can't allocate histogram.\n");
       	printf("ERR_MALLOC \n");
    }

    if ((ret = CAEN_DGTZ_ClearData(handle)) != CAEN_DGTZ_Success) {
        fprintf(stderr, "Can't clear data.\n");
        if (histo != NULL)
        free(histo);
    }

    if ((ret = CAEN_DGTZ_SWStartAcquisition(handle)) != CAEN_DGTZ_Success) {
        fprintf(stderr, "Can't start acquisition.\n");
        if (histo != NULL)
        free(histo);
    }

    for (i = 0; i < 100 && BufferSize == 0; i++) {
        if ((ret = CAEN_DGTZ_SendSWtrigger(handle)) != CAEN_DGTZ_Success) {
            fprintf(stderr, "Can't send SW trigger.\n");
            if (histo != NULL)
			free(histo);
        }
        if ((ret = CAEN_DGTZ_ReadData(handle, CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, buffer, &BufferSize)) != CAEN_DGTZ_Success) {
            fprintf(stderr, "Can't read data.\n");
            if (histo != NULL)
			free(histo);
        }
    }

    if ((ret = CAEN_DGTZ_SWStopAcquisition(handle)) != CAEN_DGTZ_Success) {
        fprintf(stderr, "Can't stop acquisition.\n");
        if (histo != NULL)
        free(histo);
    }

    if (BufferSize == 0) {
        fprintf(stderr, "Can't get SW trigger events.\n");
        if (histo != NULL)
        free(histo);
    }
	

    if ((ret = CAEN_DGTZ_DecodeEvent(handle, buffer, (void**)&EventPtr)) != CAEN_DGTZ_Success) {
        fprintf(stderr, "Can't decode events\n");
        if (histo != NULL)
        free(histo);
    }

    memset(baselines, 0, Dcfg->Nch * sizeof(*baselines));
    for (int32_t ch = 0; ch < (int32_t)Dcfg->Nch; ch++) {
        if (Dcfg->EnableMask & (1 << ch)) {
            //int32_t event_ch = (BoardInfo.FamilyCode == CAEN_DGTZ_XX740_FAMILY_CODE) ? (ch * 8) : ch; //for x740 boards shift to channel 0 of next group
            //uint32_t size = (WDcfg->Nbit == 8) ? Event8->ChSize[ch] : Event16->ChSize[ch];
			uint32_t size = Event16->ChSize[ch];
            uint32_t s;
            uint32_t maxs = 0;

            memset(histo, 0, max_sample * sizeof(*histo));
            for (s = 0; s < size; s++) {
                //uint16_t value = (WDcfg->Nbit == 8) ? Event8->DataChannel[ch][s] : Event16->DataChannel[ch][i];
				uint16_t value = Event16->DataChannel[ch][i];
                if (value < max_sample) {
                    histo[value]++;
                    if (histo[value] > histo[maxs])
                        maxs = value;
                }
            }

            if (Dcfg->PulsePolarity[ch] == CAEN_DGTZ_TriggerOnRisingEdge)//CAEN_DGTZ_PulsePolarityPositive)
                baselines[ch] = maxs * 100.0 / max_sample;
            else if (Dcfg->PulsePolarity[ch] == CAEN_DGTZ_TriggerOnFallingEdge)//CAEN_DGTZ_PulsePolarityNegative)
                baselines[ch] = 100.0 * (1.0 - (double)maxs / max_sample);
        }
    }
   if (histo != NULL)
       free(histo);

    return ret;
}	

CAEN_DGTZ_ErrorCode Set_relative_Threshold(int handle, DT5720DigiConfig* Dcfg) {
	int dc_file[2] = {10, 10};
    int ch;
	CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_Success;
    uint32_t  AllocatedSize;
    
    char* buffer = NULL;
    char* evtbuff = NULL;
    uint32_t exdco[2];
    double baselines[2];
    double dcocalib[2][2];

    //preliminary check: if baseline shift is not enabled for any channel quit
    int should_start = 0;
    for (ch = 0; ch < Dcfg->Nch; ch++) {
        if (Dcfg->EnableMask & (1 << ch)) {
            should_start = 1;
            break;
        }
    }
    if (!should_start)
        return CAEN_DGTZ_Success;

    // Memory allocation
    ret = CAEN_DGTZ_MallocReadoutBuffer(handle, &buffer, &AllocatedSize);
    if (ret) {
        printf("ERR_MALLOC \n");
        QuitThreshold(handle, buffer, evtbuff);
    }
    ret = CAEN_DGTZ_AllocateEvent(handle, (void**)&evtbuff);
    if (ret != CAEN_DGTZ_Success) {
        printf("ERR_MALLOC \n");
        QuitThreshold(handle, buffer, evtbuff);
    }
    
    for (ch = 0; ch < Dcfg->Nch; ch++) {
        if (Dcfg->EnableMask & (1 << ch)) {
            // Assume the uncalibrated DCO is not far from the correct one
            if (Dcfg->PulsePolarity[ch] == CAEN_DGTZ_TriggerOnRisingEdge)
                exdco[ch] = (uint32_t)((100.0 - dc_file[ch]) * 655.35);
            else if (Dcfg->PulsePolarity[ch] == CAEN_DGTZ_TriggerOnFallingEdge)
                exdco[ch] = (uint32_t)(dc_file[ch] * 655.35);
			           			
			ret = CAEN_DGTZ_SetChannelDCOffset(handle, ch, exdco[ch]);
            if (ret != CAEN_DGTZ_Success) {
                fprintf(stderr, "Error setting DCOffset for channel %d\n", ch);
                QuitThreshold(handle, buffer, evtbuff);
            }
        }
    }
    // Sleep some time to let the DAC move
    usleep(200*1000);
	
    if ((ret = Get_current_baseline(handle, Dcfg, buffer, evtbuff, baselines)) != CAEN_DGTZ_Success)
        QuitThreshold(handle, buffer, evtbuff);
    for (ch = 0; ch < Dcfg->Nch; ch++) {
        if (Dcfg->EnableMask & (1 << ch)) {
            double newdco = 0.0;
            // save results of this round
            dcocalib[ch][0] = baselines[ch];
            dcocalib[ch][1] = exdco[ch];
            // ... and perform a new round, using measured value and theoretical zero
            if (Dcfg->PulsePolarity[ch] == CAEN_DGTZ_TriggerOnRisingEdge)//CAEN_DGTZ_PulsePolarityPositive)
                newdco = linear_interp(0, 65535, baselines[ch], exdco[ch], dc_file[ch]);
            else if (Dcfg->PulsePolarity[ch] == CAEN_DGTZ_TriggerOnFallingEdge)//CAEN_DGTZ_PulsePolarityNegative)
                newdco = linear_interp(0, 0, baselines[ch], exdco[ch], dc_file[ch]);
            if (newdco < 0)
                exdco[ch] = 0;
            else if (newdco > 65535)
                exdco[ch] = 65535;
            else
                exdco[ch] = (uint32_t)newdco;
         
			ret = CAEN_DGTZ_SetChannelDCOffset(handle, ch, exdco[ch]);
            if (ret != CAEN_DGTZ_Success) {
                fprintf(stderr, "Error setting DCOffset for channel %d\n", ch);
                QuitThreshold(handle, buffer, evtbuff);
            }
        }
    }
    // Sleep some time to let the DAC move
    usleep(200*1000);
	
    if ((ret = Get_current_baseline(handle, Dcfg, buffer, evtbuff, baselines)) != CAEN_DGTZ_Success)
        QuitThreshold(handle, buffer, evtbuff);
    for (ch = 0; ch < Dcfg->Nch; ch++) {
        if (Dcfg->EnableMask & (1 << ch)) {
            // Now we have two real points to use for interpolation
            double newdco = linear_interp(dcocalib[ch][0], dcocalib[ch][1], baselines[ch], exdco[ch], dc_file[ch]);
            if (newdco < 0)
                exdco[ch] = 0;
            else if (newdco > 65535)
                exdco[ch] = 65535;
            else
                exdco[ch] = (uint32_t)newdco;
	   
			ret = CAEN_DGTZ_SetChannelDCOffset(handle, ch, exdco[ch]);
            if (ret != CAEN_DGTZ_Success) {
                fprintf(stderr, "Error setting DCOffset for channel %d\n", ch);
                QuitThreshold(handle, buffer, evtbuff);
            }
        }
    }
	
    usleep(200*1000);
	uint32_t thr = 0;
	
    if ((ret = Get_current_baseline(handle, Dcfg, buffer, evtbuff, baselines)) != CAEN_DGTZ_Success)
        QuitThreshold(handle, buffer, evtbuff);
    for (ch = 0; ch < Dcfg->Nch; ch++) {
        if (Dcfg->EnableMask & (1 << ch)) {
            if (fabs((baselines[ch] - dc_file[ch]) / dc_file[ch]) > 0.05)
                fprintf(stderr, "WARNING: set BASELINE_LEVEL for ch%d differs from settings for more than 5%c.\n", ch, '%');
            //uint32_t thr = 0;
			thr = 0;
            if (Dcfg->PulsePolarity[ch] == CAEN_DGTZ_TriggerOnRisingEdge)//CAEN_DGTZ_PulsePolarityPositive)
                thr = (uint32_t)(baselines[ch] / 100.0 * (0x1 << Dcfg->Nbit)) + Dcfg->thr[ch];
            else
                thr = (uint32_t)((100 - baselines[ch]) / 100.0 * (0x1 << Dcfg->Nbit)) - Dcfg->thr[ch];
            			
			ret = CAEN_DGTZ_SetChannelTriggerThreshold(handle, ch, thr);
            if (ret != CAEN_DGTZ_Success) {
                fprintf(stderr, "Error setting DCOffset for channel %d\n", ch);
                QuitThreshold(handle, buffer, evtbuff);
            }
        }
    }
    printf("Relative threshold [%i] correctly set.\n", thr);

    if (evtbuff != NULL)
        CAEN_DGTZ_FreeEvent(handle, (void**)&evtbuff);
    if (buffer != NULL)
        CAEN_DGTZ_FreeReadoutBuffer(&buffer);
  
  
    return ret;
}

void BaseLineCut(TH1D* hist, int BL_CUT){
	
	Double_t BL_mean = hist->Integral(1, BL_CUT)/BL_CUT;
		
	for (int i = 1; i<=hist->GetNbinsX(); i++)
		hist->SetBinContent(i, hist->GetBinContent(i) - BL_mean);

}

void FillHitPattern(const std::vector<ProcessedEvent>& events, TH2D* h_xy, uint64_t timeWindow) {
	int ch = h_xy->GetXaxis()->GetXmax(); // 8 - one board, 16 - two boards
	
	const char* xlabel[16] = {"16", "17", "18", "19", "20", "21", "22", "23", "24", "25", "26", "27", "28", "29", "30", "31"};
	const char* ylabel[16] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15"};
	
	//char *xlabel[16], *ylabel[16];
	//for (int i = 0; i<ch; i++){
	//	ylabel[i] = Form("%i",i);
	//	xlabel[i] = Form("%i",i+ch);
	//}
    // Sort events by timestamp to process in chronological order
	//printf("FillHitPattern size: %li [0] events CH[%i](%i): %i \n", events.size(), events[0].channel, events[0].eventCounter, events[0].timestamp);
    std::vector<ProcessedEvent> sortedEvents = events;
    std::sort(sortedEvents.begin(), sortedEvents.end(),
              [](const ProcessedEvent& a, const ProcessedEvent& b) {
                  return a.timestamp < b.timestamp;
              });

    // Process events to find coincidences
    for (size_t i = 0; i < sortedEvents.size(); ++i) {
        const auto& evt1 = sortedEvents[i];
      
		bool isHorizontal = (ch==8 && evt1.channel >= 0 && evt1.channel <= 7) || (ch==16 && evt1.board == 0);
        bool isVertical = (ch==8 && evt1.channel >= 8 && evt1.channel <= 15) || (ch==16 && evt1.board == 1);
        // Look for coincident events within timeWindow
         for (size_t j = i + 1; j < sortedEvents.size() && sortedEvents[j].timestamp <= evt1.timestamp + timeWindow; ++j) {
            const auto& evt2 = sortedEvents[j];
          
            bool evt2IsHorizontal = (ch==8 && evt2.channel >= 0 && evt2.channel <= 7) || (ch==16 && evt2.board == 0);
            bool evt2IsVertical = (ch==8 && evt2.channel >= 8 && evt2.channel <= 15) || (ch==16 && evt2.board == 1);
            if (!evt2IsHorizontal && !evt2IsVertical) continue;

            // Check for valid horizontal-vertical pair
            if ((isHorizontal && evt2IsVertical) || (isVertical && evt2IsHorizontal)) {
                int x = isHorizontal ? evt1.channel : evt2.channel;
				//int y = isVertical ? evt1.channel - 8 : evt2.channel - 8;
				int y = isVertical ? evt1.channel  : evt2.channel;
                int64_t timeDiff = static_cast<int64_t>(evt2.timestamp - evt1.timestamp);
                if (std::abs(timeDiff) <= static_cast<int64_t>(timeWindow)) {
                    //h_xy->Fill(x, y);
					h_xy->Fill(xlabel[x], ylabel[y], 1);
                    //printf("FillHitPattern: Coincidence CH[%d](%u): %u ns and CH[%d](%u): %u ns, timeDiff=%ld ns, filled h_xy(%d,%d)\n",
                    //       evt1.channel, evt1.eventCounter, evt1.timestamp,
                    //       evt2.channel, evt2.eventCounter, evt2.timestamp, timeDiff, x, y);
                }
            }
        }
    }
}


void UpdateCanvas(TCanvas* canvas, TStopwatch& stopwatch, TGLabel* TimingLabel[], double TimingValue[], Bool_t checkVP[4],
                  Bool_t checkStates[12], Bool_t radioStates[12], Bool_t byChannel,
                  TH1D* h_trace[2][4][MAX_CH], TH1D* h_ampl[2][MAX_CH], TH1D* h_integral[2][MAX_CH], TH1D* h_charge[2][MAX_CH], TH1D* h_psd[2][MAX_CH],
                  TH2D* h_psd_ampl[2][MAX_CH], TH2D* h_psd_int[2][MAX_CH], TH1D* h_det, TH1D* h_counts, TH1D* h_layers, TH2D* h_xy, int Board, int Ch2D, Bool_t debugEnabled, Bool_t showStats, Bool_t& canvasNeedsUpdate,
                  std::vector<bool>& drawChannels) {
    stopwatch.Start(kTRUE);
	if (debugEnabled) printf("UpdateCanvas: %s Channels: %li\n", canvasNeedsUpdate ? "update is required":"no needs to update", drawChannels.size());
		
gStyle->SetOptStat(showStats ? 1111 : 0);
Bool_t fBL_CUT = checkStates[6];	
int b = Board; 	
Int_t selectedType = -1;	
	
for (int i = 0; i < 7; i++) {
	if (radioStates[i]) {
    	selectedType = i;
        break;
    }
}
	
if (canvasNeedsUpdate) {
        Int_t activePads = 0;

        if (byChannel) {
            for (int i = 0; i < 12; i++) if (checkStates[i] && i!=6) activePads++; //BL_CUT = 6
            if (activePads == 0) {
                if (debugEnabled) printf("UpdateCanvas: No histograms selected, showing trace only\n");
                activePads = 1;
                checkStates[0] = kTRUE;
            }
            Int_t nCols = (activePads <= 2) ? 1 : (activePads <= 4) ? 2 : 3;
            Int_t nRows = (activePads + nCols - 1) / nCols;
            canvas->Clear();
            canvas->Divide(nCols, nRows);
        } else {
            activePads = 0;
            for (size_t i = 0; i < drawChannels.size(); i++) if (drawChannels[i]) activePads++;
            if (activePads == 0) {
                if (debugEnabled) printf("UpdateCanvas: No channels enabled, showing trace only\n");
                activePads = 1;
                if (!drawChannels.empty()) drawChannels[0] = true;
                selectedType = 0; // Default to Trace
                radioStates[0] = kTRUE;
            }
            Int_t nCols = (activePads <= 2) ? 1 : (activePads <= 4) ? 2 : 3;
            Int_t nRows = (activePads + nCols - 1) / nCols;
            canvas->Clear();
            canvas->Divide(nCols, nRows);
        }
		canvas->Modified();
}	

    Int_t pad = 1;
if(byChannel){
    if (checkStates[0]) { // Trace
		canvas->cd(pad++);
		bool first = true;
        for (size_t i = 0; i < drawChannels.size(); i++) {
            if (drawChannels[i]) {
				if (fBL_CUT) BaseLineCut(h_trace[b][0][i], 20);
				
				for (int n = 0; n<4; n++){
					if (checkVP[n] && h_trace[n][i]){
                		h_trace[b][n][i]->Draw(first ? "HIST" : "HIST SAME");
                		first = false;
					}	
				}	
            }
        }
    }
	
    if (checkStates[1]) { // Amplitude
        canvas->cd(pad++);
		bool first = true;
        for (size_t i = 0; i < drawChannels.size(); i++) {
            if (drawChannels[i]) {
                h_ampl[b][i]->Draw(first ? "HIST" : "HIST SAME");
                first = false;
            }
        }
    }
	
    if (checkStates[2]) { // Integral
        canvas->cd(pad++);
        bool first = true;
        for (size_t i = 0; i < drawChannels.size(); i++) {
            if (drawChannels[i]) {
                h_integral[b][i]->Draw(first ? "HIST" : "HIST SAME");
                first = false;
            }
        }
    }
	
    if (checkStates[3]) { // PSD
        canvas->cd(pad++);
        bool first = true;
        for (size_t i = 0; i < drawChannels.size(); i++) {
            if (drawChannels[i]) {
                h_psd[b][i]->Draw(first ? "HIST" : "HIST SAME");
                first = false;
            }
        }
    }
	
    if (checkStates[4]) {
        canvas->cd(pad++);
        h_psd_ampl[b][Ch2D]->Draw("COLZ");
    }
    if (checkStates[5]) {
        canvas->cd(pad++);
        h_psd_int[b][Ch2D]->Draw("COLZ");
    }
	
	if (checkStates[7]) { // Det
        canvas->cd(pad++);
        h_det->Draw("HIST");
    }
	
	if (checkStates[8]) { // Charge
        canvas->cd(pad++);
        bool first = true;
        for (size_t i = 0; i < drawChannels.size(); i++) {
            if (drawChannels[i]) {
                h_charge[b][i]->Draw(first ? "HIST" : "HIST SAME");
                first = false;
            }
        }
    }
	
	if (checkStates[9]) { // Counts
        canvas->cd(pad++);
		for (size_t i = 0; i < drawChannels.size();i++)
			if (drawChannels[i]){
				h_counts->Fill(i, h_ampl[0][i]->GetEntries());
				h_counts->Fill(i+15, h_ampl[1][i]->GetEntries());
			}	
        h_counts->Draw("BAR HIST");
    }
	
	if (checkStates[10]) { // Layers
        canvas->cd(pad++);
		for (size_t i = 0; i < drawChannels.size();i++)
			if (drawChannels[i]){
				h_layers->Fill(i, h_ampl[0][i]->GetMean());
				h_layers->Fill(i+15, h_ampl[1][i]->GetMean());
			}	
        h_layers->Draw("BAR HIST");
    }
	
	if (checkStates[11]) { // XY Hit pattern 8x8 matrix
		canvas->cd(pad++);
		TPad *p_ad = (TPad*)canvas->GetPrimitive(Form("cData_%i",pad-1));
		Double_t padX1 = p_ad->GetXlowNDC();
        Double_t padY1 = p_ad->GetYlowNDC();
        
        // Make pad square by setting equal width and height
        Double_t squareSize = TMath::Min(p_ad->GetWNDC(), p_ad->GetHNDC());
        p_ad->SetPad(padX1, padY1, padX1 + squareSize, padY1 + squareSize);
		p_ad->SetGrid( );
        h_xy->Draw("COLZ");
		h_xy->GetXaxis( )->SetLabelSize(0.08);
		h_xy->GetYaxis( )->SetLabelSize(0.08);
    }
	
}else {
        if (selectedType == 0) { // Trace
            for (size_t i = 0; i < drawChannels.size(); i++) {
                if (drawChannels[i]) {
					if (fBL_CUT) BaseLineCut(h_trace[b][0][i], 20);
                    canvas->cd(pad++);
                    h_trace[b][0][i]->Draw("HIST");
                }
            }
        } else if (selectedType == 1) { // Amplitude
            for (size_t i = 0; i < drawChannels.size(); i++) {
                if (drawChannels[i]) {
                    canvas->cd(pad++);
                    h_ampl[b][i]->Draw("HIST");
                }
            }
        } else if (selectedType == 2) { // Integral
            for (size_t i = 0; i < drawChannels.size(); i++) {
                if (drawChannels[i]) {
                    canvas->cd(pad++);
                    h_integral[b][i]->Draw("HIST");
                }
            }
		} else if (selectedType == 3) { // Charge
            for (size_t i = 0; i < drawChannels.size(); i++) {
                if (drawChannels[i]) {
                    canvas->cd(pad++);
                    h_charge[b][i]->Draw("HIST");
                }
            }
        } else if (selectedType == 4) { // PSD
            for (size_t i = 0; i < drawChannels.size(); i++) {
                if (drawChannels[i]) {
                    canvas->cd(pad++);
                    h_psd[b][i]->Draw("HIST");
                }
            }
        } else if (selectedType == 5) { // PSD vs Amplitude
            for (size_t i = 0; i < drawChannels.size(); i++) {
                if (drawChannels[i]) {
                    canvas->cd(pad++);
                    h_psd_ampl[b][i]->Draw("COLZ");
                }
            }
        } else if (selectedType == 6) { // PSD vs Integral
            for (size_t i = 0; i < drawChannels.size(); i++) {
                if (drawChannels[i]) {
                    canvas->cd(pad++);
                    h_psd_int[b][i]->Draw("COLZ");
                }
            }
        }
    }
	
	//canvas->Modified();
    canvas->Update();
	canvasNeedsUpdate = kFALSE;
	
    stopwatch.Stop();
	TimingValue[0] = stopwatch.RealTime() * 1000.0;
	const char* tlLabels[] = {"UpdateCanvas Time", "AnalysQ Fill", "AnalysQ Unload", "CoincQ Fill", "CoincQ Unload"};
		
	TString timeStr[5];
	for (int i = 0; i<5; i++){
		timeStr[i] = TString::Format("%s: %.2f ms", tlLabels[i], TimingValue[i]);
       	TimingLabel[i]->SetText(timeStr[i]);
	}	
    if (debugEnabled) printf("%s\n", timeStr[0].Data());
	if (debugEnabled) printf("TimingValues : %0.2f %0.2f %0.2f %0.2f %0.2f\n", TimingValue[0], TimingValue[1], TimingValue[2], TimingValue[3], TimingValue[4]);
}


void GenerateAndQueueSignal(WindowMain* gui, TRandom3& rand) {
    if (!gui) {
        if (gui->IsDebugEnabled()) printf("GenerateAndQueueSignal: Invalid gui pointer\n");
        return;
    }

    int frequency = gui->GetSignalParameters().frequency;
    if (frequency <= 0) frequency = 1;
    auto period = std::chrono::microseconds(1000000 / frequency); // Period in µs
    auto lastTime = std::chrono::steady_clock::now();
    int signalCount = 0;
    const size_t maxQueueSize = 1800; // Pause generation if queue exceeds this

    if (gui->IsDebugEnabled()) printf("GenerateAndQueueSignal: Starting with freq=%d Hz, period=%ld µs\n",
                                     frequency, period.count());

    while (gui->IsSignalRunning()) { //IsSignalRunning
        auto startTime = std::chrono::steady_clock::now();
        auto signal = GenerateSignal(gui->GetSignalParameters().nSamples, gui->GetSignalParameters().sampleWidth,
                                     gui->GetSignalParameters().tauRise, gui->GetSignalParameters().tauDecay,
                                     gui->GetSignalParameters().polarity, gui->GetSignalParameters().distMode,
                                     rand, gui);
        gui->AddSignalToQueue(signal);
        signalCount++;

        // Check queue size to prevent overflow
        if (gui->GetQueueSize() > maxQueueSize) {
            if (gui->IsDebugEnabled()) printf("GenerateAndQueueSignal: Queue size=%lu, pausing\n", gui->GetQueueSize());
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        // Precise timing
        auto nextTime = lastTime + period;
        auto now = std::chrono::steady_clock::now();
        if (now < nextTime) {
            std::this_thread::sleep_until(nextTime);
        } else if (gui->IsDebugEnabled() && signalCount % 100 == 0) {
            printf("GenerateAndQueueSignal: Missed timing by %ld µs at signal #%d\n",
                   std::chrono::duration_cast<std::chrono::microseconds>(now - nextTime).count(), signalCount);
        }
        lastTime = nextTime;

        // Debug actual frequency every 100 signals
        if (gui->IsDebugEnabled() && signalCount % 100 == 0) {
            auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime).count();
            double actualFreq = 1000000.0 / elapsed;
            printf("GenerateAndQueueSignal: Target freq=%d Hz, Actual freq=%.2f Hz, Queue size=%lu\n",
                   frequency, actualFreq, gui->GetQueueSize());
        }
     }
     if (gui->IsDebugEnabled()) printf("GenerateAndQueueSignal: Exiting signal generation loop\n");
}

//////////////////////////////////////////////////
///////////////////TIME TO LOGIC//////////////////
//////////////////////////////////////////////////

CAEN_DGTZ_ErrorCode  SwitchOffLogic(int handle, int N_CH) {
    CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_Success;
	
uint32_t reg_data;
	
	ret = CAEN_DGTZ_ReadRegister(handle, 0x8000, &reg_data);
	printf(" Previously in  0x8000: %08X \n", reg_data);
	reg_data = reg_data &~ (1<<2);
	ret = CAEN_DGTZ_WriteRegister(handle, 0x8000, reg_data);	
	ret = CAEN_DGTZ_ReadRegister(handle, 0x8000, &reg_data);
	printf(" In  0x8000: %08X \n", reg_data);
	
	uint32_t CoinceLogicAddress[MAX_CH] = { 0x1080, 0x1180, 0x1280, 0x1380, 0x1480, 0x1580, 0x1680, 0x1780,
																			0x1880, 0x1980, 0x1A80, 0x1B80, 0x1C80, 0x1D80, 0x1E80, 0x1F80};
																			
	//switch on coincidence for every channel and set trigger latency
	for (int i=0; i<N_CH; i++){
		ret = CAEN_DGTZ_ReadRegister(handle, CoinceLogicAddress[i], &reg_data);
		printf(" Previously in  0x%08X: %04X \n", CoinceLogicAddress[i], reg_data);
		reg_data = reg_data &~ (1<<18);
		ret = CAEN_DGTZ_WriteRegister(handle, CoinceLogicAddress[i], reg_data);	
		ret = CAEN_DGTZ_ReadRegister(handle, CoinceLogicAddress[i], &reg_data);
		printf(" In  0x%04X: %08X \n", CoinceLogicAddress[i], reg_data);
	}
	
  return ret;
}

CAEN_DGTZ_ErrorCode  SetLogic(int32_t handle, uint32_t reg_val[2][8], int N_CH) {
    CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_Success;
    
	uint32_t log_address[2][8] = { {0x1084, 0x1284, 0x1484, 0x1684, 0x1884, 0x1A84, 0x1C84, 0x1E84},
													  {0x8180, 0x8184, 0x8188, 0x818C, 0x8190, 0x8194, 0x8198, 0x819C} 
													};  
	
	uint32_t CoinceLogicAddress[MAX_CH] = { 0x1080, 0x1180, 0x1280, 0x1380, 0x1480, 0x1580, 0x1680, 0x1780,
																			0x1880, 0x1980, 0x1A80, 0x1B80, 0x1C80, 0x1D80, 0x1E80, 0x1F80};
																			
 	uint32_t TriggerLatencyAddress[MAX_CH] = { 0x106C, 0x116C, 0x126C, 0x136C, 0x146C, 0x156C, 0x166C, 0x176C,
																			0x186C, 0x196C, 0x1A6C, 0x1B6C, 0x1C6C, 0x1D6C, 0x1E6C, 0x1F6C};					
																			
	uint32_t ShapedTriggerWidthAddress[MAX_CH] = { 0x1070, 0x1170, 0x1270, 0x1370, 0x1470, 0x1570, 0x1670, 0x1770,
																			0x1870, 0x1970, 0x1A70, 0x1B70, 0x1C70, 0x1D70, 0x1E70, 0x1F70};															
	
	int NCouple = (int)(N_CH/2);
	
	uint32_t reg_data;
	
	//switch on coincidence on board
	ret = CAEN_DGTZ_ReadRegister(handle, 0x8000, &reg_data);
	printf(" Previously in  0x8000: %08X \n", reg_data);
	reg_data = reg_data | (1<<2);
	ret = CAEN_DGTZ_WriteRegister(handle, 0x8000, reg_data);	
	ret = CAEN_DGTZ_ReadRegister(handle, 0x8000, &reg_data);
	printf(" In  0x8000: %08X \n", reg_data);
	
	//switch on coincidence for every channel
	for (int i=0; i<N_CH; i++){
		ret = CAEN_DGTZ_ReadRegister(handle, CoinceLogicAddress[i], &reg_data);
		printf(" Previously in  0x%08X: %04X \n", CoinceLogicAddress[i], reg_data);
		reg_data = reg_data | (1<<18);
		ret = CAEN_DGTZ_WriteRegister(handle, CoinceLogicAddress[i], reg_data);	
		ret = CAEN_DGTZ_ReadRegister(handle, CoinceLogicAddress[i], &reg_data);
		printf(" In  0x%04X: %08X \n", CoinceLogicAddress[i], reg_data);
	}
	
	//read shaped trigger width for every channel ~ time window for coincidence
	for (int i=0; i<N_CH; i++){
		ret = CAEN_DGTZ_ReadRegister(handle, ShapedTriggerWidthAddress[i], &reg_data);
		printf(" Previously in  0x%08X: %08X \n", ShapedTriggerWidthAddress[i], reg_data);
		ret = CAEN_DGTZ_WriteRegister(handle, ShapedTriggerWidthAddress[i], 0x40);	 // 0x14 = 20x8ns = 160 ns window | 0x40 = 64x8ns = 512 ns window
		ret = CAEN_DGTZ_ReadRegister(handle, ShapedTriggerWidthAddress[i], &reg_data);
		printf(" In  0x%04X: %08X \n", ShapedTriggerWidthAddress[i], reg_data);
	}
	
	//set trigger latency for every channel
	for (int i=0; i<N_CH; i++){
		ret = CAEN_DGTZ_ReadRegister(handle, TriggerLatencyAddress[i], &reg_data);
		printf(" Previously in  0x%08X: %08X \n", TriggerLatencyAddress[i], reg_data);
		ret = CAEN_DGTZ_WriteRegister(handle, TriggerLatencyAddress[i], 0x9);	 // 9x8ns = 72 ns latency
		ret = CAEN_DGTZ_ReadRegister(handle, TriggerLatencyAddress[i], &reg_data);
		printf(" In  0x%04X: %08X \n", TriggerLatencyAddress[i], reg_data);
	}
		
	for (int i=0; i<2; i++)
		for (int j=0; j<NCouple; j++){
			if (reg_val[i][j] != 0){
				printf("log_address[%i][%i] = 0x%04X  log_val[%i][%i] = 0x%04X\n", i, j, log_address[i][j], i, j, reg_val[i][j]);
				ret = CAEN_DGTZ_ReadRegister(handle, log_address[i][j], &reg_data);
				printf(" Previously in  0x%04X: %08X \n", log_address[i][j], reg_data);
				if (i==0){
				  reg_data = (reg_data & ~0xFF) | reg_val[i][j];
				}
				else
				  reg_data = reg_val[i][j];
			  
				ret = CAEN_DGTZ_WriteRegister(handle, log_address[i][j], reg_data);	 
				ret = CAEN_DGTZ_ReadRegister(handle, log_address[i][j], &reg_data);
				printf(" In  0x%04X: %08X \n", log_address[i][j], reg_data);
			}
		}
	    
    return ret;
}
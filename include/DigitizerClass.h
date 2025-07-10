#ifndef DIGITIZERCLASS_H
#define DIGITIZERCLASS_H

#include "FuncMain.h"

// Abstract base class for CAEN digitizers

class Digitizer {
protected:
    int handle; 
    DeviceType mode; 
    WindowMain* window; // Pointer to WindowMain for GUI interactions
    bool debug; 

public:
    Digitizer(DeviceType m, WindowMain* w) : handle(-1), mode(m), window(w), debug(false) {}
    virtual ~Digitizer() {}

    virtual bool Init() = 0; 
    virtual void Close() = 0; 
    virtual bool Program() = 0; 
    virtual void* GetBuffer() const = 0; // Get device-specific buffer
    virtual void* GetEvent() const = 0; // Get event structure (if applicable)
	virtual void* GetWaveforms() const = 0; // Get waveforms structure (if applicable)
    virtual void FreeBuffer() = 0; // Free buffer memory
    virtual void FreeEvent() = 0; // Free event memory
	virtual void FreeWaveforms() = 0; // Free event memory
	
	// pure virtual methods for V1730 settings
	virtual void GetTemperature(uint32_t temp_arr[]) = 0;
	virtual bool SetAcquisitionMode(CAEN_DGTZ_DPP_AcqMode_t AcqMode) = 0;
    virtual bool SetInputRange(int channel, CAEN_DGTZ_InputRange_t range) = 0; // Set input range
    virtual bool SetPulsePolarity(int channel, CAEN_DGTZ_PulsePolarity_t polarity) = 0; // Set pulse polarity
    virtual bool SetRecordLength(uint32_t length, int channel) = 0; // Set record length
    virtual bool SetPreTriggerSize(int channel, uint32_t size) = 0; // Set pre-trigger size
    virtual bool SetDCOffset(int channel, uint32_t offset) = 0; // Set DC offset
    virtual bool ReadRegister(uint32_t address, uint32_t* data) = 0; // Read register
    virtual bool WriteRegister(uint32_t address, uint32_t data) = 0; // Write register
	virtual bool ReadData(std::vector<ProcessedEvent> &evts) = 0; // New unified read interface
	
	
    int GetHandle() const { return handle; }
    DeviceType GetMode() const { return mode; }
    void SetDebug(bool d) { debug = d; }
};

class N957Digitizer : public Digitizer {
private:
    N957MCAConfig* config; // N957 configuration
    N957_UINT16* buffer; // Data buffer

public:
    N957Digitizer(WindowMain* w, N957MCAConfig* cfg);
    ~N957Digitizer() override;

    bool Init() override;
    void Close() override;
    bool Program() override;
    void* GetBuffer() const override { return buffer; }
    void* GetEvent() const override { return nullptr; } // Not used for N957
	void* GetWaveforms() const override { return nullptr; } // Not used for N957
    void FreeBuffer() override;
    void FreeEvent() override {}
	void FreeWaveforms() override {}
	
	// V1730-specific methods (not supported by N957)
	void GetTemperature(uint32_t temp_arr[]) override {}
	bool SetAcquisitionMode(CAEN_DGTZ_DPP_AcqMode_t AcqMode) override {return false;}
    bool SetInputRange(int channel, CAEN_DGTZ_InputRange_t range) override {return false;}
    bool SetPulsePolarity(int channel, CAEN_DGTZ_PulsePolarity_t polarity) override {return false;}
    bool SetRecordLength(uint32_t length, int channel) override {return false;}
    bool SetPreTriggerSize(int channel, uint32_t size) override {return false;}
    bool SetDCOffset(int channel, uint32_t offset) override {return false;}
    bool ReadRegister(uint32_t address, uint32_t* data) override {return false;}
    bool WriteRegister(uint32_t address, uint32_t data) override {return false;}
	bool ReadData(std::vector<ProcessedEvent> &evts) override; 
	
};

class DT5770Digitizer : public Digitizer {
private:
    DT5770DigiConfig* config; // DT5770 configuration
    int16_t* AT1; // Analog trace 1
    int16_t* AT2; // Analog trace 2
    uint8_t* DT1; // Digital trace 1
    uint8_t* DT2; // Digital trace 2
    uint32_t* h1; // Histogram buffer
	
	void InitDgtzParams(CAENDPP_DgtzParams_t *Params);

public:
    DT5770Digitizer(WindowMain* w, DT5770DigiConfig* cfg);
    ~DT5770Digitizer() override;

    bool Init() override;
    void Close() override;
    bool Program() override {return false;}
    void* GetBuffer() const override { return nullptr; }
    void* GetEvent() const override { return nullptr; }
	void* GetWaveforms() const override { return nullptr; }// Not used for DT5720
    void FreeBuffer() override {}
    void FreeEvent() override {}
	void FreeWaveforms() override {}
	
	// V1730-specific methods (not supported by DT5720 directly, rewrite if you want to use it)
	void GetTemperature(uint32_t temp_arr[]) override {}
	bool SetAcquisitionMode(CAEN_DGTZ_DPP_AcqMode_t AcqMode) override {return false;}	
    bool SetInputRange(int channel, CAEN_DGTZ_InputRange_t range) override {return false;}
    bool SetPulsePolarity(int channel, CAEN_DGTZ_PulsePolarity_t polarity) override {return false;}
    bool SetRecordLength(uint32_t length, int channel) override {return false;}
    bool SetPreTriggerSize(int channel, uint32_t size) override {return false;}
    bool SetDCOffset(int channel, uint32_t offset) override {return false;}
    bool ReadRegister(uint32_t address, uint32_t* data) override {return false;}
    bool WriteRegister(uint32_t address, uint32_t data) override {return false;}
	bool ReadData(std::vector<ProcessedEvent> &evts) override;
	
};

class DT5720Digitizer : public Digitizer {
private:
    DT5720DigiConfig* config; // DT5720 configuration
    char* buffer; // Readout buffer
    CAEN_DGTZ_UINT16_EVENT_t* event; // Event structure

public:
    DT5720Digitizer(WindowMain* w, DT5720DigiConfig* cfg);
    ~DT5720Digitizer() override;

    bool Init() override;
    void Close() override;
    bool Program() override;
    void* GetBuffer() const override { return buffer; }
    void* GetEvent() const override { return event; }
	void* GetWaveforms() const override { return nullptr; }// Not used for DT5720
    void FreeBuffer() override;
    void FreeEvent() override;
	void FreeWaveforms() override {}
	
	// V1730-specific methods (not supported by DT5720 directly, rewrite if you want to use it)
	void GetTemperature(uint32_t temp_arr[]) override {}
	bool SetAcquisitionMode(CAEN_DGTZ_DPP_AcqMode_t AcqMode) override {return false;}
    bool SetInputRange(int channel, CAEN_DGTZ_InputRange_t range) override {return false;}
    bool SetPulsePolarity(int channel, CAEN_DGTZ_PulsePolarity_t polarity) override {return false;}
    bool SetRecordLength(uint32_t length, int channel) override {return false;}
    bool SetPreTriggerSize(int channel, uint32_t size) override {return false;}
    bool SetDCOffset(int channel, uint32_t offset) override {return false;}
    bool ReadRegister(uint32_t address, uint32_t* data) override {return false;}
    bool WriteRegister(uint32_t address, uint32_t data) override {return false;}
	bool ReadData(std::vector<ProcessedEvent> &evts) override; 
	
};

class V1730Digitizer : public Digitizer {
public:
    static const uint32_t M_CH = 16;	
private:
    V1730DigiConfig* config; // Placeholder for V1730S configuration
    char* buffer; // Readout buffer
    CAEN_DGTZ_DPP_PSD_Event_t* event[M_CH]; // Event structure
	CAEN_DGTZ_DPP_PSD_Waveforms_t* waveforms; 

public:
    V1730Digitizer(WindowMain* w, V1730DigiConfig* cfg);
    ~V1730Digitizer() override;

    bool Init() override;
    void Close() override;
    bool Program() override;
    void* GetBuffer() const override { return buffer; }
    void* GetEvent() const override {return const_cast<void*>(static_cast<const void*>(event));}//{ return event; } 
	void* GetWaveforms() const override { return waveforms; }
	
    void FreeBuffer() override;
    void FreeEvent() override;
	void FreeWaveforms() override;
	
	
	// V1730-specific methods
	bool SetDPPParameters();
	
	bool SetAcquisitionMode(CAEN_DGTZ_DPP_AcqMode_t AcqMode) override;
	void GetTemperature(uint32_t temp_arr[]) override;
	
    bool SetInputRange(int channel, CAEN_DGTZ_InputRange_t range) override;
    bool SetPulsePolarity(int channel, CAEN_DGTZ_PulsePolarity_t polarity) override;
    bool SetRecordLength(uint32_t length, int channel) override;
    bool SetPreTriggerSize(int channel, uint32_t size) override;
    bool SetDCOffset(int channel, uint32_t offset) override;
    bool ReadRegister(uint32_t address, uint32_t* data) override;
    bool WriteRegister(uint32_t address, uint32_t data) override;
	bool ReadData(std::vector<ProcessedEvent> &evts) override; 
};

#endif // DIGITIZERCLASS_H
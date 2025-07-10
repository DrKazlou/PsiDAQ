#include "ParamsMenu.h"
#include "WindowMain.h"
#include <TGMsgBox.h>
#include <stdio.h>
#include <string.h>

ParamsMenu::ParamsMenu(const TGWindow *p, WindowMain *parent, Digitizer *digi[2], V1730DigiConfig *vcfg[2])
	: TGMainFrame(p, 800, 600), fParent(parent), fNch(16)	{
    	
    for (int b = 0; b < 2; b++) {
        fDigitizer[b] = digi[b];
        fVcfg[b] = vcfg[b];
    }
	fNch = fVcfg[0]->Nch;	

    const char *paramlabel[] = {"Enable", "InputRange", "Polarity", "RecordLength", "Pretrigger", "DCOffset", "thr", "nsbl", "lgate", "sgate", "pgate", "selft", "trgc", "discr", "cfdd", "cfdf", "tvaw", "csens"};
    char titlelabel[10], str[10];

    //fMain = new TGTransientFrame(p, main, w, h, kHorizontalFrame);
    Connect("CloseWindow()", "ParamsMenu", this, "CloseWindow()");
    DontCallClose();
    SetCleanup(kDeepCleanup);

    fTab = new TGTab(this, 100, 300);
    fTab->Connect("Selected(Int_t)", "ParamsMenu", this, "DoTab()");

    for (int b = 0; b < 2; b++) {
        fCF = fTab->AddTab(fParent->IsInit() ? Form("B[%i] : %i", b, fVcfg[b]->SerialNumber) : Form("B[%i]", b));
        f1[b] = new TGCompositeFrame(fCF, 400, 200, kHorizontalFrame);
        fVF0[b] = new TGVerticalFrame(f1[b], 200, 300);
        f1[b]->AddFrame(fVF0[b], new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));

        fCAll[b] = new TGCheckButton(fVF0[b], "all in once", 666);
        fVF0[b]->AddFrame(fCAll[b], new TGLayoutHints(kLHintsCenterX, 0, 0, 0, 0));

        for (int j = 0; j < 18; j++) {
            fLabel[b][j] = new TGLabel(fVF0[b], paramlabel[j]);
            fLabel[b][j]->SetTextFont(labelFont);
            if (j == 0) fVF0[b]->AddFrame(fLabel[b][j], new TGLayoutHints(kLHintsCenterX, 0, 0, 0, 2));
            else if (j < 4) fVF0[b]->AddFrame(fLabel[b][j], new TGLayoutHints(kLHintsCenterX, 0, 0, 3, 3));
            else if (j < 8) fVF0[b]->AddFrame(fLabel[b][j], new TGLayoutHints(kLHintsCenterX, 0, 0, 3, 4));
            else if (j < 11) fVF0[b]->AddFrame(fLabel[b][j], new TGLayoutHints(kLHintsCenterX, 0, 0, 4, 4));
            else fVF0[b]->AddFrame(fLabel[b][j], new TGLayoutHints(kLHintsCenterX, 0, 0, 3, 4));
        }

        fSetButton[b] = new TGTextButton(fVF0[b], "&Apply", b);
        fSetButton[b]->SetFont(sFont);
        fSetButton[b]->Resize(60, 30);
        fSetButton[b]->Connect("Clicked()", "ParamsMenu", this, "SetButton()");
        fVF0[b]->AddFrame(fSetButton[b], new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 4, 4, 4, 4));

        for (int i = 0; i < fVcfg[b]->Nch; i++) {
            sprintf(titlelabel, "CH%i", i);
            fGF[b][i] = new TGGroupFrame(f1[b], titlelabel, kVerticalFrame);
            fGF[b][i]->SetTitlePos(TGGroupFrame::kCenter);
            f1[b]->AddFrame(fGF[b][i], new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2));

            fVF[b][i] = new TGVerticalFrame(fGF[b][i], 80, 30);
            fGF[b][i]->AddFrame(fVF[b][i], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));

            fC[b][i] = new TGCheckButton(fVF[b][i], "", i + 500 * b);
            (fVcfg[b]->ChannelMask & (1 << i)) ? fC[b][i]->SetState(kButtonDown) : fC[b][i]->SetState(kButtonUp);
            fC[b][i]->Connect("Clicked()", "ParamsMenu", this, "DoCheckBox()");
            fVF[b][i]->AddFrame(fC[b][i], new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 2, 2));

            fCIRange[b][i] = new TGComboBox(fVF[b][i], i + 500 * b);
            fCIRange[b][i]->AddEntry("2 Vpp", 0);
            fCIRange[b][i]->AddEntry("0.5 Vpp", 1);
            fVcfg[b]->InputRange[i] == InputRange_2Vpp ? fCIRange[b][i]->Select(0) : fCIRange[b][i]->Select(1);
            fCIRange[b][i]->Resize(50, 20);
            fCIRange[b][i]->Connect("Selected(Int_t)", "ParamsMenu", this, "DoComboBox()");
            fVF[b][i]->AddFrame(fCIRange[b][i], new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 2, 2));

            fCPol[b][i] = new TGComboBox(fVF[b][i], i + 100 + 500 * b);
            fCPol[b][i]->AddEntry("Negative", 0);
            fCPol[b][i]->AddEntry("Positive", 1);
            fVcfg[b]->PulsePolarity[i] == CAEN_DGTZ_PulsePolarityNegative ? fCPol[b][i]->Select(0) : fCPol[b][i]->Select(1);
            fCPol[b][i]->Resize(50, 20);
            fCPol[b][i]->Connect("Selected(Int_t)", "ParamsMenu", this, "DoComboBox()");
            fVF[b][i]->AddFrame(fCPol[b][i], new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 2, 2));

            for (int j = 0; j < 4; j++) {
                tbuf[b][i][j] = new TGTextBuffer(10);
                switch (j) {
                    case 0: sprintf(str, "%u", fVcfg[b]->RecordLength[i]); break;
                    case 1: sprintf(str, "%u", fVcfg[b]->PreTrigger[i]); break;
                    case 2: sprintf(str, "%u", fVcfg[b]->DCOffset[i]); break;
                    case 3: sprintf(str, "%u", fVcfg[b]->thr[i]); break;
                }
                tbuf[b][i][j]->AddText(0, str);
                fTEntries[b][i][j] = new TGTextEntry(fVF[b][i], tbuf[b][i][j], j * 16 + i + 500 * b);
                if (j == 0 && i%2==1) fTEntries[b][i][j]->SetEnabled(0); // RecordLength for odd channels disabled
                fTEntries[b][i][j]->Connect("ReturnPressed()", "ParamsMenu", this, "DoSetVal()");
                fTEntries[b][i][j]->Resize(50, fTEntries[b][i][j]->GetDefaultHeight());
                fTEntries[b][i][j]->SetFont(paramFont);
                fVF[b][i]->AddFrame(fTEntries[b][i][j], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 3, 3));
            }

            fCnsbl[b][i] = new TGComboBox(fVF[b][i], i + 16 + 500 * b);
            fCnsbl[b][i]->AddEntry("0", 0);
            fCnsbl[b][i]->AddEntry("16", 1);
            fCnsbl[b][i]->AddEntry("64", 2);
            fCnsbl[b][i]->AddEntry("256", 3);
            fCnsbl[b][i]->AddEntry("1024", 4);
            fCnsbl[b][i]->Select(fVcfg[b]->nsbl[i]);
            fCnsbl[b][i]->Resize(50, 20);
            fCnsbl[b][i]->Connect("Selected(Int_t)", "ParamsMenu", this, "DoComboBox()");
            fVF[b][i]->AddFrame(fCnsbl[b][i], new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 2, 2));

            for (int j = 4; j < 7; j++) {
                tbuf[b][i][j] = new TGTextBuffer(10);
                switch (j) {
                    case 4: sprintf(str, "%u", fVcfg[b]->lgate[i]); break;
                    case 5: sprintf(str, "%u", fVcfg[b]->sgate[i]); break;
                    case 6: sprintf(str, "%u", fVcfg[b]->pgate[i]); break;
                }
                tbuf[b][i][j]->AddText(0, str);
                fTEntries[b][i][j] = new TGTextEntry(fVF[b][i], tbuf[b][i][j], j * 16 + i + 500 * b);
                fTEntries[b][i][j]->Connect("ReturnPressed()", "ParamsMenu", this, "DoSetVal()");
                fTEntries[b][i][j]->Resize(50, fTEntries[b][i][j]->GetDefaultHeight());
                fTEntries[b][i][j]->SetFont(paramFont);
                fVF[b][i]->AddFrame(fTEntries[b][i][j], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 3, 3));
            }

            fCselft[b][i] = new TGCheckButton(fVF[b][i], "", i + 16 + 500 * b);
            fVcfg[b]->selft[i] == 0 ? fCselft[b][i]->SetState(kButtonUp) : fCselft[b][i]->SetState(kButtonDown);
            fCselft[b][i]->Connect("Clicked()", "ParamsMenu", this, "DoCheckBox()");
            fVF[b][i]->AddFrame(fCselft[b][i], new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 4, 4));

            fCtrgc[b][i] = new TGComboBox(fVF[b][i], i + 32 + 500 * b);
            fCtrgc[b][i]->AddEntry("Peak", 0);
            fCtrgc[b][i]->AddEntry("Threshold", 1);
            fCtrgc[b][i]->Resize(50, 20);
            fCtrgc[b][i]->Connect("Selected(Int_t)", "ParamsMenu", this, "DoComboBox()");
            fCtrgc[b][i]->SetEnabled(0);
            fVF[b][i]->AddFrame(fCtrgc[b][i], new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 2, 2));

            fCdiscr[b][i] = new TGComboBox(fVF[b][i], i + 48 + 500 * b);
            fCdiscr[b][i]->AddEntry("LED", 0);
            fCdiscr[b][i]->AddEntry("CFD", 1);
            fVcfg[b]->discr[i] == CAEN_DGTZ_DPP_DISCR_MODE_LED ? fCdiscr[b][i]->Select(0) : fCdiscr[b][i]->Select(1);
            fCdiscr[b][i]->Resize(50, 20);
            fCdiscr[b][i]->Connect("Selected(Int_t)", "ParamsMenu", this, "DoComboBox()");
            fVF[b][i]->AddFrame(fCdiscr[b][i], new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 2, 2));

            tbuf[b][i][7] = new TGTextBuffer(10);
            sprintf(str, "%u", fVcfg[b]->cfdd[i]);
            tbuf[b][i][7]->AddText(0, str);
            fTEntries[b][i][7] = new TGTextEntry(fVF[b][i], tbuf[b][i][7], 7 * 16 + i + 500 * b);
            fTEntries[b][i][7]->Connect("ReturnPressed()", "ParamsMenu", this, "DoSetVal()");
            fTEntries[b][i][7]->Resize(50, fTEntries[b][i][7]->GetDefaultHeight());
            fTEntries[b][i][7]->SetFont(paramFont);
            fVF[b][i]->AddFrame(fTEntries[b][i][7], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 3, 3));

            fCcfdf[b][i] = new TGComboBox(fVF[b][i], i + 64 + 500 * b);
            fCcfdf[b][i]->AddEntry("25%", 0);
            fCcfdf[b][i]->AddEntry("50%", 1);
            fCcfdf[b][i]->AddEntry("75%", 2);
            fCcfdf[b][i]->AddEntry("100%", 3);
            fCcfdf[b][i]->Select(fVcfg[b]->cfdf[i]);
            fCcfdf[b][i]->Resize(50, 20);
            fCcfdf[b][i]->Connect("Selected(Int_t)", "ParamsMenu", this, "DoComboBox()");
            fVF[b][i]->AddFrame(fCcfdf[b][i], new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 2, 2));

            tbuf[b][i][8] = new TGTextBuffer(10);
            sprintf(str, "%u", fVcfg[b]->tvaw[i]);
            tbuf[b][i][8]->AddText(0, str);
            fTEntries[b][i][8] = new TGTextEntry(fVF[b][i], tbuf[b][i][8], 8 * 16 + i + 500 * b);
            fTEntries[b][i][8]->Connect("ReturnPressed()", "ParamsMenu", this, "DoSetVal()");
            fTEntries[b][i][8]->Resize(50, fTEntries[b][i][8]->GetDefaultHeight());
            fTEntries[b][i][8]->SetFont(paramFont);
            fTEntries[b][i][8]->SetEnabled(0);
            fVF[b][i]->AddFrame(fTEntries[b][i][8], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 3, 3));

            fCcsens[b][i] = new TGComboBox(fVF[b][i], i + 80 + 500 * b);
            fCcsens[b][i]->AddEntry("5fC/LSB", 0);
            fCcsens[b][i]->AddEntry("20fC/LSB", 1);
            fCcsens[b][i]->AddEntry("80fC/LSB", 2);
            fCcsens[b][i]->AddEntry("320fC/LSB", 3);
            fCcsens[b][i]->AddEntry("1.28pC/LSB", 4);
            fCcsens[b][i]->AddEntry("5.12pC/LSB", 5);
            fCcsens[b][i]->Select(fVcfg[b]->csens[i]);
            fCcsens[b][i]->Resize(50, 20);
            fCcsens[b][i]->Connect("Selected(Int_t)", "ParamsMenu", this, "DoComboBox()");
            fVF[b][i]->AddFrame(fCcsens[b][i], new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 2, 2));

            fVF[b][i]->Resize();
            fGF[b][i]->Resize();
        }

        fCF->AddFrame(f1[b], new TGLayoutHints(kLHintsTop | kLHintsRight, 5, 5, 10, 5));
        fCF->Resize();
    }

    // Addresses tab
    fCF = fTab->AddTab("Addresses");
    TGHorizontalFrame *hf_address = new TGHorizontalFrame(fCF, 200, 40);
    TGLabel *fBLabel = new TGLabel(hf_address, "Board");
    fBLabel->SetTextFont(labelFont);
    hf_address->AddFrame(fBLabel, new TGLayoutHints(kLHintsCenterX, 5, 0, 3, 3));

    fBoardNE = new TGNumberEntry(hf_address, 0, 8, 1000, (TGNumberFormat::EStyle)0);
    fBoardNE->Resize(20, fBoardNE->GetDefaultHeight());
    hf_address->AddFrame(fBoardNE, new TGLayoutHints(kLHintsTop | kLHintsCenterX, 5, 0, 3, 3));

    fAddressTBuf = new TGTextBuffer(10);
    fAddressTBuf->AddText(0, "ADD");
    fAddressTEntry = new TGTextEntry(hf_address, fAddressTBuf, 1001);
    fAddressTEntry->Connect("ReturnPressed()", "ParamsMenu", this, "DoAddresses()");
    fAddressTEntry->Resize(50, fAddressTEntry->GetDefaultHeight());
    fAddressTEntry->SetFont(paramFont);
    hf_address->AddFrame(fAddressTEntry, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 5, 0, 3, 3));

    fAdValTBuf = new TGTextBuffer(10);
    fAdValTBuf->AddText(0, "VAL");
    fAdValTEntry = new TGTextEntry(hf_address, fAdValTBuf, 1002);
    fAdValTEntry->Connect("ReturnPressed()", "ParamsMenu", this, "DoAddresses()");
    fAdValTEntry->Resize(100, fAdValTEntry->GetDefaultHeight());
    fAdValTEntry->SetFont(paramFont);
    hf_address->AddFrame(fAdValTEntry, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 5, 0, 3, 3));
    fCF->AddFrame(hf_address, new TGLayoutHints(kLHintsTop | kLHintsCenterX, 2, 5, 10, 0));

    TGHorizontalFrame *hf_buttons = new TGHorizontalFrame(fCF, 200, 40);
    fReadButton = new TGTextButton(hf_buttons, "&Read", 1);
    fReadButton->SetFont(sFont);
    fReadButton->Resize(60, 30);
    fReadButton->Connect("Clicked()", "ParamsMenu", this, "ReadButton()");
    hf_buttons->AddFrame(fReadButton, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 5, 0, 3, 3));

    fWriteButton = new TGTextButton(hf_buttons, "&Write", 1);
    fWriteButton->SetFont(sFont);
    fWriteButton->Resize(60, 30);
    fWriteButton->Connect("Clicked()", "ParamsMenu", this, "WriteButton()");
    hf_buttons->AddFrame(fWriteButton, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 5, 0, 3, 3));
    fCF->AddFrame(hf_buttons, new TGLayoutHints(kLHintsTop | kLHintsCenterX, 2, 5, 10, 0));

    fCF->Resize();

    AddFrame(fTab, new TGLayoutHints(kLHintsTop | kLHintsRight, 5, 5, 10, 5));
    MapSubwindows();
    Resize();
		
    //CenterOnParent();
    SetWindowName("V1730 Parameters");
    MapWindow();
}

ParamsMenu::~ParamsMenu() {
    
	Cleanup();
}

void ParamsMenu::CloseWindow() {
	fParent->ParamsDialogClosed();
    delete this;
}

void ParamsMenu::SetButton() {
    TGTextButton *tb = (TGTextButton *)gTQSender;
    int b = tb->WidgetId();
    printf("button id %i B[%i]\n", b, b);
		
	std::lock_guard<std::mutex> lock(fParent->GetDigitizerMutex());
	
if(fParent->IsInit()){
	
	uint32_t reg_data;
	//ChannelsMask
	fDigitizer[b]->ReadRegister(EnableMaskAddress, &reg_data);	
	printf("Handle : %i ChannelMask : (before : %i) %i \n", fDigitizer[b]->GetHandle(), reg_data, fVcfg[b]->ChannelMask);
	fDigitizer[b]->WriteRegister(EnableMaskAddress, fVcfg[b]->ChannelMask);	
	
    for (int i = 0; i < fVcfg[b]->Nch; i++) {
        fDigitizer[b]->SetInputRange(i, fVcfg[b]->InputRange[i]);
        fDigitizer[b]->SetPulsePolarity(i, fVcfg[b]->PulsePolarity[i]);
        fDigitizer[b]->SetRecordLength(fVcfg[b]->RecordLength[i], i);
        fDigitizer[b]->SetPreTriggerSize(i, fVcfg[b]->PreTrigger[i]);
        fDigitizer[b]->SetDCOffset(i, fVcfg[b]->DCOffset[i]);
        fDigitizer[b]->WriteRegister(ThresholdAddress[i], fVcfg[b]->thr[i]);
        
        fDigitizer[b]->ReadRegister(DPPAlgControlAddress[i], &reg_data);
        reg_data = (reg_data & ~(0x700000)) | (fVcfg[b]->nsbl[i] << 20);
        fDigitizer[b]->WriteRegister(DPPAlgControlAddress[i], reg_data);

        fDigitizer[b]->WriteRegister(LongGateWidthAddress[i], fVcfg[b]->lgate[i]);
        fDigitizer[b]->WriteRegister(ShortGateWidthAddress[i], fVcfg[b]->sgate[i]);
        fDigitizer[b]->WriteRegister(GateOffsetAddress[i], fVcfg[b]->pgate[i]);

        fDigitizer[b]->ReadRegister(DPPAlgControlAddress[i], &reg_data);
        fVcfg[b]->selft[i] == 0 ? reg_data |= (1 << 24) : reg_data &= ~(1 << 24);
        fDigitizer[b]->WriteRegister(DPPAlgControlAddress[i], reg_data);

        fDigitizer[b]->ReadRegister(DPPAlgControlAddress[i], &reg_data);
        fVcfg[b]->discr[i] == CAEN_DGTZ_DPP_DISCR_MODE_CFD ? reg_data |= (1 << 6) : reg_data &= ~(1 << 6);
        fDigitizer[b]->WriteRegister(DPPAlgControlAddress[i], reg_data);

        fDigitizer[b]->ReadRegister(CFDSettingsAddress[i], &reg_data);
        reg_data = (reg_data & ~0x0F) + fVcfg[b]->cfdd[i];
        fDigitizer[b]->WriteRegister(CFDSettingsAddress[i], reg_data);

        fDigitizer[b]->ReadRegister(CFDSettingsAddress[i], &reg_data);
        reg_data = (reg_data & ~0xF00) | (fVcfg[b]->cfdf[i] << 8);
        fDigitizer[b]->WriteRegister(CFDSettingsAddress[i], reg_data);

        fDigitizer[b]->WriteRegister(ShapedTriggerWidthAddress[i], fVcfg[b]->tvaw[i]);

        fDigitizer[b]->ReadRegister(DPPAlgControlAddress[i], &reg_data);
        reg_data = (reg_data & ~0x00000007) | (fVcfg[b]->csens[i] << 0);
        fDigitizer[b]->WriteRegister(DPPAlgControlAddress[i], reg_data);
    }

	printf("Settings applied\n");
}
	
}

void ParamsMenu::DoSetVal() {
    TGTextEntry *te = (TGTextEntry *)gTQSender;
    Int_t id = te->WidgetId();
    int b = id < 500 ? 0 : 1;
    id = id >= 500 ? id - 500 : id;
    Int_t i = id % 16;
    Int_t j = id / 16;
	
	int value  = atoi(tbuf[b][i][j]->GetString());
	
	fCAll[b]->GetState() == kButtonDown ? fAll = true : fAll = false;
	
	std::lock_guard<std::mutex> lock(fParent->GetDigitizerMutex());

if (atoi(tbuf[b][i][j]->GetString())<0 || atoi(tbuf[b][i][j]->GetString()) >32768){	
	new TGMsgBox(gClient->GetRoot(), this, "Warning", Form("Check what are you typing!"), kMBIconExclamation, kMBOk);	
	fTEntries[b][i][j]->SetText("42");
	return;
}	
	
if(fParent->IsInit()){
		
    if (id < 16) { // RecordLength
        fVcfg[b]->RecordLength[i] = value;
		fVcfg[b]->RecordLength[i+1] = value;
		fTEntries[b][i+1][j]->SetText(tbuf[b][i][j]->GetString());
		
        fDigitizer[b]->SetRecordLength(fVcfg[b]->RecordLength[i], i);
		fParent->ModifyHistograms(b, i);
		fParent->ModifyHistograms(b, i+1);
				
        if (fAll) {
            for (int n = 0; n < fVcfg[b]->Nch; n++) {
                if (n != i) {
                    fVcfg[b]->RecordLength[n] = fVcfg[b]->RecordLength[i];
                    fTEntries[b][n][j]->SetText(tbuf[b][i][j]->GetString());
                    fDigitizer[b]->SetRecordLength(fVcfg[b]->RecordLength[n], n);
                }
            }
        }
    } else if (id < 32) { // PreTrigger
        fVcfg[b]->PreTrigger[i] = atoi(tbuf[b][i][j]->GetString());
        fDigitizer[b]->SetPreTriggerSize(i, fVcfg[b]->PreTrigger[i]);
        if (fAll) {
            for (int n = 0; n < fVcfg[b]->Nch; n++) {
                if (n != i) {
                    fVcfg[b]->PreTrigger[n] = fVcfg[b]->PreTrigger[i];
                    fTEntries[b][n][j]->SetText(tbuf[b][i][j]->GetString());
                    fDigitizer[b]->SetPreTriggerSize(n, fVcfg[b]->PreTrigger[n]);
                }
            }
        }
    } else if (id < 48) { // DCOffset
        fVcfg[b]->DCOffset[i] = atoi(tbuf[b][i][j]->GetString());
        fDigitizer[b]->SetDCOffset(i, fVcfg[b]->DCOffset[i]);
        if (fAll) {
            for (int n = 0; n < fVcfg[b]->Nch; n++) {
                if (n != i) {
                    fVcfg[b]->DCOffset[n] = fVcfg[b]->DCOffset[i];
                    fTEntries[b][n][j]->SetText(tbuf[b][i][j]->GetString());
                    fDigitizer[b]->SetDCOffset(n, fVcfg[b]->DCOffset[n]);
                }
            }
        }
    } else if (id < 64) { // thr
        fVcfg[b]->thr[i] = atoi(tbuf[b][i][j]->GetString());
        fDigitizer[b]->WriteRegister(ThresholdAddress[i], fVcfg[b]->thr[i]);
        if (fAll) {
            for (int n = 0; n < fVcfg[b]->Nch; n++) {
                if (n != i) {
                    fVcfg[b]->thr[n] = fVcfg[b]->thr[i];
                    fTEntries[b][n][j]->SetText(tbuf[b][i][j]->GetString());
                    fDigitizer[b]->WriteRegister(ThresholdAddress[n], fVcfg[b]->thr[n]);
                }
            }
        }
    } else if (id < 80) { // lgate
        fVcfg[b]->lgate[i] = atoi(tbuf[b][i][j]->GetString());
        fDigitizer[b]->WriteRegister(LongGateWidthAddress[i], fVcfg[b]->lgate[i]);
        if (fAll) {
            for (int n = 0; n < fVcfg[b]->Nch; n++) {
                if (n != i) {
                    fVcfg[b]->lgate[n] = fVcfg[b]->lgate[i];
                    fTEntries[b][n][j]->SetText(tbuf[b][i][j]->GetString());
                    fDigitizer[b]->WriteRegister(LongGateWidthAddress[n], fVcfg[b]->lgate[n]);
                }
			}    
        }
    } else if (id < 96) { // sgate
        fVcfg[b]->sgate[i] = atoi(tbuf[b][i][j]->GetString());
        fDigitizer[b]->WriteRegister(ShortGateWidthAddress[i], fVcfg[b]->sgate[i]);
        if (fAll) {
        	for (int n = 0; n < fVcfg[b]->Nch; n++) {
            	if (n != i) {
                	fVcfg[b]->sgate[n] = fVcfg[b]->sgate[i];
                    fTEntries[b][n][j]->SetText(tbuf[b][i][j]->GetString());
                    fDigitizer[b]->WriteRegister(ShortGateWidthAddress[n], fVcfg[b]->sgate[n]);
                }
            }
        }
    } else if (id < 112) { // pgate
        fVcfg[b]->pgate[i] = atoi(tbuf[b][i][j]->GetString());
        fDigitizer[b]->WriteRegister(GateOffsetAddress[i], fVcfg[b]->pgate[i]);
        if (fAll) {
            for (int n = 0; n < fVcfg[b]->Nch; n++) {
                if (n != i) {
                    fVcfg[b]->pgate[n] = fVcfg[b]->pgate[i];
                    fTEntries[b][n][j]->SetText(tbuf[b][i][j]->GetString());
                    fDigitizer[b]->WriteRegister(GateOffsetAddress[n], fVcfg[b]->pgate[n]);
                }
            }
        }
    } else if (id < 128) { // cfdd
        fVcfg[b]->cfdd[i] = atoi(tbuf[b][i][j]->GetString());
        uint32_t reg_data;
        fDigitizer[b]->ReadRegister(CFDSettingsAddress[i], &reg_data);
        reg_data = (reg_data & ~0x0F) + fVcfg[b]->cfdd[i];
        fDigitizer[b]->WriteRegister(CFDSettingsAddress[i], reg_data);
        if (fAll) {
            for (int n = 0; n < fVcfg[b]->Nch; n++) {
                if (n != i) {
                    fVcfg[b]->cfdd[n] = fVcfg[b]->cfdd[i];
                    fTEntries[b][n][j]->SetText(tbuf[b][i][j]->GetString());
                    fDigitizer[b]->ReadRegister(CFDSettingsAddress[n], &reg_data);
                    reg_data = (reg_data & ~0x0F) + fVcfg[b]->cfdd[n];
                    fDigitizer[b]->WriteRegister(CFDSettingsAddress[n], reg_data);
                }
            }
        }
    } else { // tvaw
        fVcfg[b]->tvaw[i] = atoi(tbuf[b][i][j]->GetString());
        fDigitizer[b]->WriteRegister(ShapedTriggerWidthAddress[i], fVcfg[b]->tvaw[i]);
        if (fAll) {
            for (int n = 0; n < fVcfg[b]->Nch; n++) {
            	if (n != i) {
                    fVcfg[b]->tvaw[n] = fVcfg[b]->tvaw[i];
                    fTEntries[b][n][j]->SetText(tbuf[b][i][j]->GetString());
                    fDigitizer[b]->WriteRegister(ShapedTriggerWidthAddress[n], fVcfg[b]->tvaw[n]);
                }
            }
        }
    }

}
    printf("Value set for B[%d], id %d\n", b, id);
    
}

void ParamsMenu::DoCheckBox() {
    TGCheckButton *chb = (TGCheckButton *)gTQSender;
	Int_t id = chb->WidgetId();
    int b = id < 500 ? 0 : 1;
    id = id >= 500 ? id - 500 : id;
	uint32_t reg_data;
	//bool WasRunning = false;
	fCAll[b]->GetState() == kButtonDown ? fAll = true : fAll = false;
	
	std::lock_guard<std::mutex> lock(fParent->GetDigitizerMutex());
if(fParent->IsInit()){
	
    if (id < 16) { // Enable Channel Mask
        fC[b][id]->GetState() == kButtonDown ? fVcfg[b]->ChannelMask |= (1<<id) : fVcfg[b]->ChannelMask &= ~(1<<id);
		fDigitizer[b]->ReadRegister(EnableMaskAddress, &reg_data);
		fDigitizer[b]->WriteRegister(EnableMaskAddress, fVcfg[b]->ChannelMask);
        if (fAll) {
            for (int i = 0; i < fVcfg[b]->Nch; i++) {
                if (i != id) {
                    fC[b][i]->SetState(fC[b][id]->GetState());
                    fC[b][i]->GetState() == kButtonDown ? fVcfg[b]->ChannelMask |= (1<<i) : fVcfg[b]->ChannelMask &= ~(1<<i);
                    fDigitizer[b]->ReadRegister(EnableMaskAddress, &reg_data);
					fDigitizer[b]->WriteRegister(EnableMaskAddress, fVcfg[b]->ChannelMask);
                }
            }
        }
    } else if (id < 32) { // Self-trigger
        fVcfg[b]->selft[id-16] = fCselft[b][id-16]->GetState() == kButtonDown ? 1 : 0;
        fDigitizer[b]->ReadRegister(DPPAlgControlAddress[id-16], &reg_data);
        fVcfg[b]->selft[id-16] == 0 ? reg_data |= (1<<24) : reg_data &= ~(1 << 24);
        fDigitizer[b]->WriteRegister(DPPAlgControlAddress[id-16], reg_data);
        if (fAll) {
            for (int i = 0; i < fVcfg[b]->Nch; i++) {
                if (i != (id - 16)) {
                    fCselft[b][i]->SetState(fCselft[b][id-16]->GetState());
                    fVcfg[b]->selft[i] = fCselft[b][i]->GetState() == kButtonDown ? 1 : 0;
                    fDigitizer[b]->ReadRegister(DPPAlgControlAddress[i], &reg_data);
                    fVcfg[b]->selft[i] == 0 ? reg_data |= (1<<24) : reg_data &= ~(1<<24); // [b24] responsible for self trigger
                    fDigitizer[b]->WriteRegister(DPPAlgControlAddress[i], reg_data);
                }
            }
        }
    }
	
}	
    printf("Checkbox changed for B[%i] id %d\n", b, id);
}

void ParamsMenu::DoComboBox() {
    TGComboBox *cb = (TGComboBox *)gTQSender;
	Int_t id = cb->WidgetId();
    int b = id < 500 ?  0 : 1;
    id = id >= 500 ? id - 500 : id;
    int i = id % 16;
	//bool WasRunning = false;
	fCAll[b]->GetState() == kButtonDown ? fAll = true : fAll = false;
	
	std::lock_guard<std::mutex> lock(fParent->GetDigitizerMutex());
if(fParent->IsInit()){
	
    if (id < 16) { // Input Range
        fVcfg[b]->InputRange[i] = fCIRange[b][i]->GetSelected() == 0 ? InputRange_2Vpp : InputRange_0_5Vpp;
        fDigitizer[b]->SetInputRange(i, fVcfg[b]->InputRange[i]);
        if (fAll) {
            for (int n = 0; n < fVcfg[b]->Nch; n++) {
                if (n != i) {
                    fCIRange[b][n]->Select(fCIRange[b][i]->GetSelected());
                    fVcfg[b]->InputRange[n] = fVcfg[b]->InputRange[i];
                    fDigitizer[b]->SetInputRange(n, fVcfg[b]->InputRange[n]);
                }
            }
        }
    } 
	if (id >= 100 && id < 116) { // Polarity
        fVcfg[b]->PulsePolarity[i] = fCPol[b][i]->GetSelected() == 0 ? CAEN_DGTZ_PulsePolarityNegative : CAEN_DGTZ_PulsePolarityPositive;
        fDigitizer[b]->SetPulsePolarity(i, fVcfg[b]->PulsePolarity[i]);
        if (fAll) {
            for (int n = 0; n < fVcfg[b]->Nch; n++) {
                if (n != i) {
                    fCPol[b][n]->Select(fCPol[b][i]->GetSelected());
                    fVcfg[b]->PulsePolarity[n] = fVcfg[b]->PulsePolarity[i];
                    fDigitizer[b]->SetPulsePolarity(n, fVcfg[b]->PulsePolarity[n]);
                }
            }
        }
    } 
	if (id >= 16 && id < 32) { // nsbl
        fVcfg[b]->nsbl[i] = fCnsbl[b][i]->GetSelected();
        uint32_t reg_data;
        fDigitizer[b]->ReadRegister(DPPAlgControlAddress[i], &reg_data);
        reg_data = (reg_data & ~(0x700000)) | (fVcfg[b]->nsbl[i] << 20);
        fDigitizer[b]->WriteRegister(DPPAlgControlAddress[i], reg_data);
        if (fAll) {
            for (int n = 0; n < fVcfg[b]->Nch; n++) {
                if (n != i) {
                    fCnsbl[b][n]->Select(fCnsbl[b][i]->GetSelected());
                    fVcfg[b]->nsbl[n] = fVcfg[b]->nsbl[i];
                }
            }
        }
	}
	if (id >= 48 && id < 64) { // discr
    	fVcfg[b]->discr[i] = fCdiscr[b][i]->GetSelected() == 0 ? CAEN_DGTZ_DPP_DISCR_MODE_LED : CAEN_DGTZ_DPP_DISCR_MODE_CFD;
        uint32_t reg_data;
        fDigitizer[b]->ReadRegister(DPPAlgControlAddress[i], &reg_data);
        fVcfg[b]->discr[i] == CAEN_DGTZ_DPP_DISCR_MODE_CFD ? reg_data |= (1<<6) : reg_data &= ~(1<<6);
        fDigitizer[b]->WriteRegister(DPPAlgControlAddress[i], reg_data);
        if (fAll) {
            for (int n = 0; n < fVcfg[b]->Nch; n++) {
                if (n != i) {
                    fCdiscr[b][n]->Select(fCdiscr[b][i]->GetSelected());
                    fVcfg[b]->discr[n] = fVcfg[b]->discr[i];
                }
            }
        }
	}
	if (id >= 64 && id < 80) { // cfdf
    	fVcfg[b]->cfdf[i] = fCcfdf[b][i]->GetSelected();
        uint32_t reg_data;
        fDigitizer[b]->ReadRegister(CFDSettingsAddress[i], &reg_data);
        reg_data = (reg_data & ~(0xF00)) | (fVcfg[b]->cfdf[i] << 8);
        fDigitizer[b]->WriteRegister(CFDSettingsAddress[i], reg_data);
        if (fAll) {
        	for (int n = 0; n < fVcfg[b]->Nch; n++) {
                if (n != i) {
                    fCcfdf[b][n]->Select(fCcfdf[b][i]->GetSelected());
                    fVcfg[b]->cfdf[n] = fVcfg[b]->cfdf[i];
                }
            }
        }
    }
	if (id >= 80 && id < 96){ // csens
    	fVcfg[b]->csens[i] = fCcsens[b][i]->GetSelected();
        uint32_t reg_data;
        fDigitizer[b]->ReadRegister(DPPAlgControlAddress[i], &reg_data);
        reg_data = (reg_data & ~(0x0000000007)) | (fVcfg[b]->csens[i] << 0); //clean up [b2:0]
        fDigitizer[b]->WriteRegister(DPPAlgControlAddress[i], reg_data);
        if (fAll) {
        	for (int n = 0; n < fVcfg[b]->Nch; n++) {
            	if (n != i) {
                	fCcsens[b][n]->Select(fCcsens[b][i]->GetSelected());
                    fVcfg[b]->csens[n] = fVcfg[b]->csens[i];
                }
            }
        }
    }
	
}
    printf("Combobox changed for B[%d], id %d\n", b, id);
}

void ParamsMenu::DoTab() {
    TGTab *tab = (TGTab *)gTQSender;
    printf("Active Tab #%i\n", tab->GetCurrent());
}

void ParamsMenu::ReadButton() {
    int b = (int)fBoardNE->GetNumber();
    address = strtol(fAddressTBuf->GetString(), 0, 16);
	
	std::lock_guard<std::mutex> lock(fParent->GetDigitizerMutex());
    fDigitizer[b]->ReadRegister(address, &reg_data);
    printf("B[%d] ADD %s 0x%04X VAL %04X\n", b, fAddressTBuf->GetString(), address, reg_data);
    char str[32];
    snprintf(str, sizeof(str), "%04X", reg_data);
    fAdValTEntry->SetText(str);
}

void ParamsMenu::WriteButton() {
    int b = (int)fBoardNE->GetNumber();
    address = strtol(fAddressTBuf->GetString(), 0, 16);
    reg_data = strtol(fAdValTBuf->GetString(), 0, 16);
	
	std::lock_guard<std::mutex> lock(fParent->GetDigitizerMutex());
    fDigitizer[b]->WriteRegister(address, reg_data);
    printf("B[%d] ADD %s 0x%04X VAL %04X\n", b, fAddressTBuf->GetString(), address, reg_data);
}

void ParamsMenu::DoAddresses() {
    TGTextEntry *te = (TGTextEntry *)gTQSender;
    int id = te->WidgetId();
    printf("Address changed in %i\n", id);
    printf("New val = %s num: %i\n", fAdValTBuf->GetString(), atoi(fAdValTBuf->GetString()));
    printf("New addr = %s num: %i\n", fAddressTBuf->GetString(), atoi(fAddressTBuf->GetString()));
}
#ifndef ParamsMenu_H
#define ParamsMenu_H

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

class WindowMain;


class ParamsMenu : public TGMainFrame{

private:
   
	TGMainFrame *fMain;
	WindowMain* fParent;   
    TGCompositeFrame *f1[2], *fCF;
    TGTab *fTab;
    TGGroupFrame *fGF[2][16];
    TGVerticalFrame *fVF0[2], *fVF[2][16];
    TGCheckButton *fC[2][16], *fCselft[2][16], *fCAll[2];
    TGComboBox *fCIRange[2][16], *fCPol[2][16], *fCnsbl[2][16], *fCtrgc[2][16], *fCdiscr[2][16], *fCcfdf[2][16], *fCcsens[2][16];
    TGLabel *fLabel[2][18];
    TGNumberEntry *fBoardNE;
    TGTextEntry *fTEntries[2][16][9], *fAddressTEntry, *fAdValTEntry;
    TGTextBuffer *tbuf[2][16][9], *fAddressTBuf, *fAdValTBuf;
    TGTextButton *fSetButton[2], *fReadButton, *fWriteButton;

    Digitizer *fDigitizer[2]; // Pointer to Digitizer instances
    V1730DigiConfig *fVcfg[2]; // Configuration per board
    
    int fNch; // Number of channels per board
    bool fAll; // "All in once" flag
    uint32_t address, reg_data;

    // V1730 register addresses (adjust as needed)
    const uint32_t EnableMaskAddress = 0x8120;
    const uint32_t DPPAlgControlAddress[16] = {0x1080, 0x1180, 0x1280, 0x1380, 0x1480, 0x1580, 0x1680, 0x1780, 0x1880, 0x1980, 0x1A80, 0x1B80, 0x1C80, 0x1D80, 0x1E80, 0x1F80};
    const uint32_t ThresholdAddress[16] = {0x1060, 0x1160, 0x1260, 0x1360, 0x1460, 0x1560, 0x1660, 0x1760, 0x1860, 0x1960, 0x1A60, 0x1B60, 0x1C60, 0x1D60, 0x1E60, 0x1F60};
    const uint32_t CFDSettingsAddress[16] = {0x103C, 0x113C, 0x123C, 0x133C, 0x143C, 0x153C, 0x163C, 0x173C, 0x183C, 0x193C, 0x1A3C, 0x1B3C, 0x1C3C, 0x1D3C, 0x1E3C, 0x1F3C};
    const uint32_t ShapedTriggerWidthAddress[16] = {0x1070, 0x1170, 0x1270, 0x1370, 0x1470, 0x1570, 0x1670, 0x1770, 0x1870, 0x1970, 0x1A70, 0x1B70, 0x1C70, 0x1D70, 0x1E70, 0x1F70};
    const uint32_t LongGateWidthAddress[16] = {0x1058, 0x1158, 0x1258, 0x1358, 0x1458, 0x1558, 0x1658, 0x1758, 0x1858, 0x1958, 0x1A58, 0x1B58, 0x1C58, 0x1D58, 0x1E58, 0x1F58};
    const uint32_t ShortGateWidthAddress[16] = {0x1054, 0x1154, 0x1254, 0x1354, 0x1454, 0x1554, 0x1654, 0x1754, 0x1854, 0x1954, 0x1A54, 0x1B54, 0x1C54, 0x1D54, 0x1E54, 0x1F54};
    const uint32_t GateOffsetAddress[16] = {0x105C, 0x115C, 0x125C, 0x135C, 0x145C, 0x155C, 0x165C, 0x175C, 0x185C, 0x195C, 0x1A5C, 0x1B5C, 0x1C5C, 0x1D5C, 0x1E5C, 0x1F5C};

public:
    ParamsMenu(const TGWindow *p, WindowMain *parent, Digitizer *digi[2], V1730DigiConfig *vcfg[2]);
    virtual ~ParamsMenu();

    const char *sFont = "-Ubuntu-bold-r-*-*-16-*-*-*-*-*-iso8859-1";
    const char *labelFont = "-Ubuntu-bold-r-*-*-14-*-*-*-*-*-iso8859-1";
    const char *paramFont = "-adobe-courier-bold-r-*-*-15-*-*-*-*-*-iso8859-1";

    void CloseWindow();
    void DoSetVal();
    void DoCheckBox();
    void DoComboBox();
    void DoTab();
    void DoAddresses();
    void SetButton();
    void ReadButton();
    void WriteButton();
	
	ClassDef(ParamsMenu, 1);
};

#endif
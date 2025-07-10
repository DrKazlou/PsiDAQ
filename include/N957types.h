////////////////////////////////////////////////////////////////////////////////////////////////
/*! \file    N957types.h
*   \brief   N957 public types definitions
*   \author  CAEN S.p.A. ( http://www.caen.it )
*   \version 1.2
*   \date    05/2006
*            
*            Provides properties and defines for N957 library usage
*/
////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __N957TYPES_H
#define __N957TYPES_H

/*!
* \defgroup types N957 types
*/
/*@{*/

////////////////////////////////////////////
// File includes
////////////////////////////////////////////

////////////////////////////////////////////
// Global defines
////////////////////////////////////////////
#if defined (WIN32)
	#if defined( N957LIB_EXPORTS)
		#define N957_DLL_API   __declspec( dllexport )
	#else
		#define N957_DLL_API   __declspec( dllimport )
	#endif
#elif defined (LINUX)
	#define N957_DLL_API
#endif

#define N957_INVALID_HANDLE_VALUE	(-1)	/*!< \brief Value for not valid handles */

#ifdef WIN32
	typedef INT32  int32_t;
	typedef UINT32 uint32_t;
	typedef INT64  int64_t;
	typedef UINT64 uint64_t;
#elif defined (LINUX)
	#include <stdint.h>
#endif

typedef unsigned char		N957_BYTE;		/*!< \brief 8 bits unsigned int type   */
typedef unsigned short		N957_UINT16;	/*!< \brief 16 bits unsigned int type  */
typedef			 uint32_t	N957_UINT32;	/*!< \brief 32 bits unsigned int type  */
typedef			 short		N957_INT16;		/*!< \brief 16 bits signed int type    */
typedef			 int32_t	N957_INT32;		/*!< \brief 32 bits signed int type    */
typedef			 int		N957_BOOL;		/*!< \brief boolean type               */
#define	N957_TRUE			(-1)			/*!< \brief boolean TRUE               */
#define	N957_FALSE			(0)				/*!< \brief boolean FALSE              */


////////////////////////////////////////////////////////////////////////////////////////////////
/*! \enum    _N957ErrorCodes
*   \brief   Error codes
*            
*            Error codes returned by the exported functions.
*/
////////////////////////////////////////////////////////////////////////////////////////////////
typedef enum _N957ErrorCodes
{
	N957Success			=  0,           /*!< \brief Operation completed successfully */
	N957CommError		= -1,           /*!< \brief Communication error              */
	N957GenericError	= -2,           /*!< \brief Unspecified error                */
	N957InvalidParam	= -3            /*!< \brief Invalid parameter                */
} N957ErrorCodes;


////////////////////////////////////////////////////////////////////////////////////////////////
/*! \enum    _N957Registers
*   \brief   Accessible registers.
*            
*            The list of user accessible registers.
*/
////////////////////////////////////////////////////////////////////////////////////////////////
typedef enum _N957Registers
{
	N957StatusReg      = 0x0000,        /*!< \brief Status register                                   */
	N957ControlReg     = 0x0001,        /*!< \brief Control register                                  */
	N957FwRelReg       = 0x0002,        /*!< \brief Firmware Release register                         */
	N957FwDwnldReg     = 0x0003,        /*!< \brief Firmware download register                        */
	N957FlashEnaReg    = 0x0004,        /*!< \brief Flash Enable register                             */
	N957PulserReg      = 0x0005,        /*!< \brief Pulser register                                   */
	N957DACReg         = 0x0006,        /*!< \brief DAC register direct access                        */
	N957BlDimReg       = 0x0007,        /*!< \brief Block Dim register number of word to read         */
	N957PotCtrlReg     = 0x0008,        /*!< \brief Potentiometer control register                    */
	N957CalSetReg      = 0x0009,        /*!< \brief Calibration set register                          */
	N957CalClearReg    = 0x000a,        /*!< \brief Calibration clear register                        */
	N957ScratchReg     = 0x000b,        /*!< \brief Scratch register                                  */
	N957BuffOccReg     = 0x000c,        /*!< \brief Buffer occupancy register                         */
	N957ScalerLSWReg   = 0x000d,        /*!< \brief Scaler LSW register                               */
	N957ScalerMSWReg   = 0x000e,        /*!< \brief Scaler MSW register                               */
	N957TimerLSWReg    = 0x000f,        /*!< \brief Timer  LSW register                               */
	N957TimerMSWReg    = 0x0010,        /*!< \brief Timer  MSW register                               */
	N957LiveTimeLSWReg = 0x0011,        /*!< \brief Live time timer LSW register                      */
	N957LiveTimeMSWReg = 0x0012,        /*!< \brief Live time timer MSW register                      */
	N957SwClearReg     = 0x0013,        /*!< \brief Software clear register                           */
	N957SwResetReg     = 0x0014,        /*!< \brief Software reset register                           */
} N957Registers;

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \enum    _N957StatusRegMsks
*   \brief   Status register bit masks
*            
*            The status register's bits definitions.
*/
////////////////////////////////////////////////////////////////////////////////////////////////
typedef enum _N957StatusRegMsks 
{
	N957StatusRegUSBHiSpeedMsk= 0x0001,      /*!< \brief USB High Speed status flag        */
	N957StatusRegMemEmptyMsk  = 0x0002,      /*!< \brief Data Memory empty flag            */
	N957StatusRegMemFullMsk   = 0x0004,      /*!< \brief Data Memory full (overflow) flag  */
	N957StatusRegFlashBusyMsk = 0x0008,      /*!< \brief Flash Busy flag                   */
} N957StatusRegMsks;


////////////////////////////////////////////////////////////////////////////////////////////////
/*! \enum    _N957ControlRegMsks
*   \brief   Control register bit masks
*            
*            The control register's bits definitions.
*/
////////////////////////////////////////////////////////////////////////////////////////////////
typedef enum _N957ControlRegMsks 
{
	N957ControlRegAcqModeMsk  = 0x0001,		/*!< \brief Enable data acquisition mode                    */
	N957ControlRegADCRateMsk  = 0x000e,		/*!< \brief ADC data rate bits
													<ul>
													<li> 000  ->   600 SPS  
													<li> 001  ->    70 KSPS 
													<li> 010  ->   140 KSPS 
													<li> 011  ->   250 KSPS 
													<li> 100  ->   420 KSPS 
													<li> 101  ->   635 KSPS 
													<li> 110  ->   850 KSPS 
													<li> 111  ->     1 MSPS 
													</ul>
													\sa _N957ControlADCRates
											*/
	N957ControlRegSLSOnMsk    = 0x0010,		/*!< \brief SLS (Sliding scale) activation                  */
	N957ControlRegConvEnMsk   = 0x0040,		/*!< \brief Convertion enable flag                          */
	N957ControlRegSwConvMsk	  = 0x0080,		/*!< \brief Software convertion flag                        */
	N957ControlRegPUREnMsk    = 0x0100,		/*!< \brief PUR (Pile Up rejection) flag                    */
	N957ControlRegModeMsk     = 0x0600,		/*!< \brief function mode bits
													<ul>
													<li> 00  ->  Ext Gate: track on external trigger rising edge
															hold on external trigger falling edge
													<li> 01  ->  Auto    : track on signal level 
															hold on signal peak detect
													</ul>
													\sa _N957ControlModesMsks
											*/
	N957ControlRegDACTestEnMsk= 0x0800,		/*!< \brief DAC Test enable flag                            */
	N957ControlRegSLSTypeMsk  = 0x1000,		/*!< \brief Slading scale Type 0= LFSR 1= RAMP              */
	N957ControlRegOUTPModeMsk = 0x6000,		/*!< \brief Output Lemo configuration                       
													<ul>
														<li> 00 -> OUTP Memory Full Mode    
														<li> 01 -> OUTP Peak Mode           
														<li> 10 -> OUTP Level Mode          
														<li> 11 -> OUTP Pulser Mode         
													</ul>
													\sa _N957ControlOUTPModeMsks
											*/
	N957ControlRegOUTPLevelMsk= 0x8000,		/*!< \brief Output Lemo level                               */
} N957ControlRegMsks;

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \enum    _N957ControlADCRates
*   \brief   Control register ADC rates 
*            
*            The control register's ADC rates enumeration.
*/
////////////////////////////////////////////////////////////////////////////////////////////////
typedef enum _N957ControlADCRates
{
	N957ControlADCRate600SPS= 0,			/*!< \brief  600  SPS           */
	N957ControlADCRate70KSPS,				/*!< \brief   70 KSPS           */
	N957ControlADCRate140KSPS,				/*!< \brief  140 KSPS           */
	N957ControlADCRate250KSPS,				/*!< \brief  250 KSPS           */
	N957ControlADCRate420KSPS,				/*!< \brief  420 KSPS           */
	N957ControlADCRate635KSPS,				/*!< \brief  635 KSPS           */
	N957ControlADCRate850KSPS,				/*!< \brief  850 KSPS           */
	N957ControlADCRate1MSPS,				/*!< \brief    1 MSPS           */
} N957ControlADCRates;

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \enum    _N957ControlADCRatesMsks
*   \brief   Control register ADC rates masks
*            
*            The control register's ADC rates masks.
*/
////////////////////////////////////////////////////////////////////////////////////////////////
typedef enum _N957ControlADCRatesMsks
{
	N957ControlADCRate600SPSMsk = 0x0000,	/*!< \brief 600  SPS            */
	N957ControlADCRate70KSPSMsk = 0x0002,	/*!< \brief  70 KSPS            */
	N957ControlADCRate140KSPSMsk= 0x0004,	/*!< \brief 140 KSPS            */
	N957ControlADCRate250KSPSMsk= 0x0006,	/*!< \brief 250 KSPS            */
	N957ControlADCRate420KSPSMsk= 0x0008,	/*!< \brief 420 KSPS            */
	N957ControlADCRate635KSPSMsk= 0x000a,	/*!< \brief 635 KSPS            */
	N957ControlADCRate850KSPSMsk= 0x000c,	/*!< \brief 850 KSPS            */
	N957ControlADCRate1MSPSMsk  = 0x000e,	/*!< \brief   1 MSPS            */
} N957ControlADCRatesMsks;

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \enum    _N957ControlModes
*   \brief   Control register Modes
*            
*            The control register's modes enumeration.
*/
////////////////////////////////////////////////////////////////////////////////////////////////
typedef enum _N957ControlModes
{
		N957ControlModeExtGate= 0,			/*!< \brief Ext Gate: track on external trigger rising edge
													hold on external trigger falling edge
											*/
		N957ControlModeAuto,				/*!< \brief Auto    : track on signal level 
													hold on signal peak detect
											*/
} N957ControlModes;

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \enum    _N957ControlModesMsks
*   \brief   Control register Modes masks
*            
*            The control register's modes masks.
*/
////////////////////////////////////////////////////////////////////////////////////////////////
typedef enum _N957ControlModesMsks
{
		N957ControlModeExtGateMsk=	0x0000,	/*!< \brief External gate            */
		N957ControlModeAutoMsk=		0x0200,	/*!< \brief Auto                     */
} N957ControlModesMsk;

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \enum    _N957ControlOUTPModeMsks
*   \brief   Control register OUTP Modes masks
*            
*            The control register's OUTP mode masks.
*/
////////////////////////////////////////////////////////////////////////////////////////////////
typedef enum _N957ControlOUTPModeMsks
{
		N957OUTPModeMemFullMsk=		0x00,	/*!< \brief OUTP Memory Full Mode    */
		N957OUTPModePeakMsk=		0x01,	/*!< \brief OUTP Peak Mode           */
		N957OUTPModeLevelMsk=		0x10,	/*!< \brief OUTP Level Mode          */
		N957OUTPModePulseMsk=		0x11,	/*!< \brief OUTP Pulser Mode         */		
} N957ControlOUTPModeMsks;

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \enum    _N957FlashEnaRegMsks
*   \brief   Flash enable bit masks
*            
*/
////////////////////////////////////////////////////////////////////////////////////////////////
typedef enum _N957FlashEnaRegMsks 
{
	N957FlashEnaRegFCSMsk  = 0x0001,		/*!< \brief Flash CS (active low)    */
} N957FlashEnaRegMsks;


#define N957_FLASH_PAGE_SIZE		264		/*!< \brief Flash page byte size     */

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \enum    _N957FlashPageTypes
*   \brief   Flash page types
*            
*            The flash page types for firmware upgrade
*/
////////////////////////////////////////////////////////////////////////////////////////////////
typedef enum _N957FlashPageTypes
{
	N957FlashPageStandard,					/*!< \brief Standard (boot) flash page*/
	N957FlashPageBackup,					/*!< \brief Backup flash page         */
} N957FlashPageTypes;

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \struct  _N957_ConfigROM
*   \brief   N957 board ROM configuration
*            
*            This structure stores the ROM configuration values
*/
////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct _N957_ConfigROM
{
	N957_BYTE m_chksum;						/*!< \brief Configuration ROM checksum */
	N957_BYTE m_version;					/*!< \brief Configuration ROM Purchased version of the Mod.N957 */
	N957_UINT32 m_board_id;					/*!< \brief Configuration ROM Board identifier */
	N957_UINT32 m_revision;					/*!< \brief Configuration ROM Hardware revision identifier */
	N957_UINT16 m_serial;					/*!< \brief Configuration ROM Serial number */
	N957_BYTE m_stretch_rev;                /*!< \brief Stretcher mezzanine firmware revision */
	N957_BYTE m_wiper_sls;                  /*!< \brief Sliding scale wiper calibration position */
	N957_BYTE m_figure_sls;                 /*!< \brief Sliding scale calibration figure */
	N957_BYTE m_wiper_lld;                  /*!< \brief Low Level Detect (LLD) wiper calibration position */
} N957_ConfigROM;

////////////////////////////////////////////
// Global variables declaration
////////////////////////////////////////////

////////////////////////////////////////////
// Global methods declaration
////////////////////////////////////////////

/*@}*/
#endif // __N957TYPES_H

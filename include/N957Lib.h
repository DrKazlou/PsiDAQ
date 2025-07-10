////////////////////////////////////////////////////////////////////////////////////////////////
/*! \file    N957Lib.h
*   \brief   N957 board library public methods definitions
*   \author  CAEN S.p.A. ( http://www.caen.it )
*   \version 1.2
*   \date    05/2006
*            
*/
////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __N957LIB_DEF_H
#define __N957LIB_DEF_H

////////////////////////////////////////////
// File includes
////////////////////////////////////////////
#include "N957oslib.h"
#include "N957types.h"

#ifdef __cplusplus
extern "C"{
#endif

////////////////////////////////////////////
// Global defines
////////////////////////////////////////////

////////////////////////////////////////////
// Global variables declaration
////////////////////////////////////////////

////////////////////////////////////////////
// Global methods declaration
////////////////////////////////////////////

////////////////////////////////////////////
// Public Library methods declaration
////////////////////////////////////////////

/*!
* \defgroup board_handling Boards handling APIs
*/
/*@{*/

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_Init( short BdNum, int32_t *handle)
*   \brief   Initialize a new board
*            
*            Call before any other API to to get a valid board handle: board number canno't be previously initialized
*   \param   BdNum board number (must be a new one)
*   \param   handle board handle (to use for any other API call)
*   \return  N957Success : operation ok
*   \note    Must be called before any other board specific API.
*   \sa      N957ErrorCodes
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_Init( short BdNum, int32_t *handle);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_End( int32_t handle)
*   \brief   Releases the resources allocated for the board
*            
*            Call after any other API for this board: the number of this board can be initialized again
*   \param   handle board's handle (to use for any other API call)
*   \return  N957Success : operation ok
*   \note    Must be called as last API for that board handle.
*   \sa      N957ErrorCodes
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_End( int32_t handle);
/*@}*/

/*!
* \defgroup miscellaneous Miscellaneous APIs
*/
/*@{*/

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_DecodeError( N957ErrorCodes code)
*   \brief   Decodes a N957 error code into string
*            
*   \param   code error to decode
*   \return  Decoded error string
*   \sa      N957ErrorCodes
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API const char* N957_DecodeError( N957ErrorCodes code);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_SWRelease( char *SwRel, int buff_size)
*   \brief   Gets the library release string
*            
*            User must allocate a buffer enough large for library release storage
*            The software release string format is : ##.## ( major_version.minor_version ) 
*   \param   SwRel The caller allocated string buffer to be filled with library release string
*   \param   buff_size The caller allocated buffer size (BYTEs)
*   \return  N957Success : operation ok
*   \sa      N957ErrorCodes
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_SWRelease( char *SwRel, int buff_size);
/*@}*/

/*!
* \defgroup level_0 Level 0 APIs
*/
/*@{*/
////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_WriteReg( int32_t handle, N957_UINT16 reg_addr, N957_UINT16 data)
*   \brief   Writes data to board's register 
*            
*   \param   handle board's handle
*   \param   reg_addr register address
*   \param   data data to write
*   \return  N957Success: write ok : error code otherwise
*   \sa      N957ErrorCodes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_WriteReg( int32_t handle, N957_UINT16 reg_addr, N957_UINT16 data);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_ReadReg( int32_t handle, N957_UINT16 reg_addr, N957_UINT16 *data)
*   \brief   Reads data from board's register 
*            
*   \param   handle board's handle
*   \param   reg_addr register address
*   \param   data data read
*   \return  N957Success: read ok : error code otherwise
*   \sa      N957ErrorCodes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_ReadReg( int32_t handle, N957_UINT16 reg_addr, N957_UINT16 *data);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_SetRegBitMsk( int32_t handle, N957_UINT16 reg_addr, N957_UINT16 bitmsk)
*   \brief   Sets a mask of bits into board's register 
*            
*   \param   handle board's handle
*   \param   reg_addr register address
*   \param   bitmsk: mask of bits to set
*   \return  N957Success: procedure execution ok : error code otherwise
*   \sa      N957ErrorCodes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*            and register must have R/W properties
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_SetRegBitMsk( int32_t handle, N957_UINT16 reg_addr, N957_UINT16 bitmsk);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_ClearRegBitMsk( int32_t handle, N957_UINT16 reg_addr, N957_UINT16 bitmsk)
*   \brief   Clears a mask of bits into board's register 
*            
*   \param   handle board's handle
*   \param   reg_addr register address
*   \param   bitmsk: mask of bits to clear
*   \return  N957Success: procedure execution ok : error code otherwise
*   \sa      N957ErrorCodes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*            and register must have R/W properties
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_ClearRegBitMsk( int32_t handle, N957_UINT16 reg_addr, N957_UINT16 bitmsk);
/*@}*/

/*!
* \defgroup level_1 Level 1 APIs
*/
/*@{*/

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_GetStatus( int32_t handle, N957_UINT16 *data)
*   \brief   Gets Status register 
*            
*   \param   handle board's handle
*   \param   data data read
*   \return  N957Success: read ok : error code otherwise
*   \sa      N957ErrorCodes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_GetStatus( int32_t handle, N957_UINT16 *data);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_GetControl( int32_t handle, N957_UINT16 *data)
*   \brief   Gets Control register 
*            
*   \param   handle board's handle
*   \param   data data read
*   \return  N957Success: read ok : error code otherwise
*   \sa      N957ErrorCodes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_GetControl( int32_t handle, N957_UINT16 *data);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_SetControl( int32_t handle, N957_UINT16 data)
*   \brief   Sets Control register
*            
*   \param   handle board's handle
*   \param   data data to write read
*   \return  N957Success: write ok : error code otherwise
*   \sa      N957ErrorCodes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_SetControl( int32_t handle, N957_UINT16 data);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_GetFWRelease( int32_t handle, char *FwRel, int buff_size)
*   \brief   Gets firmare release string
*            
*            The firmware release string format is : ##.## ( major_version.minor_version ) 
*   \param   handle board's handle
*   \param   FwRel Firmware release buffer (caller allocated) to be filled with firmware release string
*   \param   buff_size Allocated buffer size (BYTEs)
*   \return  N957Success: procedure execution ok : error code otherwise
*   \sa      N957ErrorCodes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_GetFWRelease( int32_t handle, char *FwRel, int buff_size);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_GetDAC( int32_t handle, N957_UINT16 *data)
*   \brief   Gets DAC register 
*            
*   \param   handle board's handle
*   \param   data data read
*   \return  N957Success: read ok : error code otherwise
*   \sa      N957ErrorCodes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_GetDAC( int32_t handle, N957_UINT16 *data);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_SetDAC( int32_t handle, N957_UINT16 data)
*   \brief   Sets DAC register 
*            
*   \param   handle board's handle
*   \param   data data to write
*   \return  N957Success: write ok : error code otherwise
*   \sa      N957ErrorCodes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_SetDAC( int32_t handle, N957_UINT16 data);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_GetBlDim( int32_t handle, N957_UINT16 *data)
*   \brief   Gets Block Dimension register 
*            
*   \param   handle board's handle
*   \param   data data read
*   \return  N957Success: read ok : error code otherwise
*   \sa      N957ErrorCodes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_GetBlDim( int32_t handle, N957_UINT16 *data);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_GetPulser( int32_t handle, N957_UINT16 *data)
*   \brief   Gets Pulser register 
*            
*   \param   handle board's handle
*   \param   data data read
*   \return  N957Success: read ok : error code otherwise
*   \sa      N957ErrorCodes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_GetPulser( int32_t handle, N957_UINT16 *data);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_SetPulser( int32_t handle, N957_UINT16 data)
*   \brief   Sets Pulser register
*            
*   \param   handle board's handle
*   \param   data data to write
*   \return  N957Success: write ok : error code otherwise
*   \sa      N957ErrorCodes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_SetPulser( int32_t handle, N957_UINT16 data);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_GetFwDwnld( int32_t handle, N957_UINT16 *data)
*   \brief   Gets Firmware Download register 
*            
*   \param   handle board's handle
*   \param   data data read
*   \return  N957Success: read ok : error code otherwise
*   \sa      N957ErrorCodes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_GetFwDwnld( int32_t handle, N957_UINT16 *data);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_SetFwDwnld( int32_t handle, N957_UINT16 data)
*   \brief   Sets Firmware Download register
*            
*   \param   handle board's handle
*   \param   data data to write
*   \return  N957Success: write ok : error code otherwise
*   \sa      N957ErrorCodes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_SetFwDwnld( int32_t handle, N957_UINT16 data);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_GetFlashEna( int32_t handle, N957_UINT16 *data)
*   \brief   Gets Flash Enable register 
*            
*   \param   handle board's handle
*   \param   data data read
*   \return  N957Success: read ok : error code otherwise
*   \sa      N957ErrorCodes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_GetFlashEna( int32_t handle, N957_UINT16 *data);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_SetFlashEna( int32_t handle, N957_UINT16 data)
*   \brief   Sets Flash Enable register 
*            
*   \param   handle board's handle
*   \param   data data to write
*   \return  N957Success: write ok : error code otherwise
*   \sa      N957ErrorCodes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_SetFlashEna( int32_t handle, N957_UINT16 data);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_GetUsbFlag( int32_t handle, N957_BOOL *usb_high_speed)
*   \brief   Gets USB flag status
*            
*   \param   handle board's handle
*   \param   usb_high_speed USB port type: FALSE = full speed (USB 1.1) , TRUE = high speed (USB 2.0) 
*   \return  N957Success: read ok : error code otherwise
*   \sa      N957ErrorCodes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_GetUsbFlag( int32_t handle, N957_BOOL *usb_high_speed);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_GetMemEmptyFlag( int32_t handle, N957_BOOL *data)
*   \brief   Gets memory empty flag status
*            
*   \param   handle board's handle
*   \param   data data read
*   \return  N957Success: read ok : error code otherwise
*   \sa      N957ErrorCodes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_GetMemEmptyFlag( int32_t handle, N957_BOOL *data);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_GetMemFullFlag( int32_t handle, N957_BOOL *data)
*   \brief   Gets memory full status flag 
*            
*   \param   handle board's handle
*   \param   data data read
*   \return  N957Success: read ok : error code otherwise
*   \sa      N957ErrorCodes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_GetMemFullFlag( int32_t handle, N957_BOOL *data);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_GetFlashBusyFlag( int32_t handle, N957_BOOL *data)
*   \brief   Gets Flash busy status flag 
*            
*   \param   handle board's handle
*   \param   data data read
*   \return  N957Success: read ok : error code otherwise
*   \sa      N957ErrorCodes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_GetFlashBusyFlag( int32_t handle, N957_BOOL *data);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_GetAcqModeFlag( int32_t handle, N957_BOOL *data)
*   \brief   Gets Acquisition mode control flag
*            
*   \param   handle board's handle
*   \param   data data read
*   \return  N957Success: read ok : error code otherwise
*   \sa      N957ErrorCodes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_GetAcqModeFlag( int32_t handle, N957_BOOL *data);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_SetAcqModeFlag( int32_t handle, N957_BOOL data)
*   \brief   Sets Acquisition mode control flag
*            
*   \param   handle board's handle
*   \param   data data to write
*   \return  N957Success: write ok : error code otherwise
*   \sa      N957ErrorCodes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_SetAcqModeFlag( int32_t handle, N957_BOOL data);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_GetADCRate( int32_t handle, N957ControlADCRates *rate)
*   \brief   Gets ADC rate 
*            
*   \param   handle board's handle
*   \param   rate data read
*   \return  N957Success: read ok : error code otherwise
*   \sa      N957ErrorCodes
*   \sa      N957ControlADCRates
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_GetADCRate( int32_t handle, N957ControlADCRates *rate);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_SetADCRate( int32_t handle, N957ControlADCRates rate)
*   \brief   Sets ADC rate
*            
*   \param   handle board's handle
*   \param   rate data to write
*   \return  N957Success: write ok : error code otherwise
*   \sa      N957ErrorCodes
*   \sa      N957ControlADCRates
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_SetADCRate( int32_t handle, N957ControlADCRates rate);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_GetSlsFlag( int32_t handle, N957_BOOL *data)
*   \brief   Gets Sliding scale control flag
*            
*   \param   handle board's handle
*   \param   data data read
*   \return  N957Success: read ok : error code otherwise
*   \sa      N957ErrorCodes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_GetSlsFlag( int32_t handle, N957_BOOL *data);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_SetSlsFlag( int32_t handle, N957_BOOL data)
*   \brief   Sets Sliding scale control flag
*            
*   \param   handle board's handle
*   \param   data data to write
*   \return  N957Success: write ok : error code otherwise
*   \sa      N957ErrorCodes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_SetSlsFlag( int32_t handle, N957_BOOL data);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_SwReset( int32_t handle)
*   \brief   Performs a software reset
*            
*   \param   handle board's handle
*   \return  N957Success: procedure executed correctly : error code otherwise
*   \sa      N957ErrorCodes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_SwReset( int32_t handle);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_SwClear( int32_t handle)
*   \brief   Performs a software clear
*            
*            Clears data, Scaler counter, Timer and Live time timer
*   \param   handle board's handle
*   \return  N957Success: procedure executed correctly : error code otherwise
*   \sa      N957ErrorCodes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_SwClear( int32_t handle);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_GetConvEnFlag( int32_t handle, N957_BOOL *data)
*   \brief   Gets Convertion enable control flag
*            
*   \param   handle board's handle
*   \param   data data read
*   \return  N957Success: read ok : error code otherwise
*   \sa      N957ErrorCodes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_GetConvEnFlag( int32_t handle, N957_BOOL *data);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_SetConvEnFlag( int32_t handle, N957_BOOL data)
*   \brief   Sets Convertion enable control flag
*            
*   \param   handle board's handle
*   \param   data data to write
*   \return  N957Success: write ok : error code otherwise
*   \sa      N957ErrorCodes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_SetConvEnFlag( int32_t handle, N957_BOOL data);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_GetSwConvFlag( int32_t handle, N957_BOOL *data)
*   \brief   Gets Software convertion control flag
*            
*   \param   handle board's handle
*   \param   data data read
*   \return  N957Success: read ok : error code otherwise
*   \sa      N957ErrorCodes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_GetSwConvFlag( int32_t handle, N957_BOOL *data);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_SetSwConvFlag( int32_t handle, N957_BOOL data)
*   \brief   Sets Software convertion control flag
*            
*   \param   handle board's handle
*   \param   data data to write
*   \return  N957Success: write ok : error code otherwise
*   \sa      N957ErrorCodes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_SetSwConvFlag( int32_t handle, N957_BOOL data);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_GetPurEnFlag( int32_t handle, N957_BOOL *data)
*   \brief   Gets Pile UP rejection control flag
*            
*   \param   handle board's handle
*   \param   data data read
*   \return  N957Success: read ok : error code otherwise
*   \sa      N957ErrorCodes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_GetPurEnFlag( int32_t handle, N957_BOOL *data);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_SetPurEnFlag( int32_t handle, N957_BOOL data)
*   \brief   Sets Pile UP rejection control flag
*            
*   \param   handle board's handle
*   \param   data data to write
*   \return  N957Success: write ok : error code otherwise
*   \sa      N957ErrorCodes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_SetPurEnFlag( int32_t handle, N957_BOOL data);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_GetAcquisitionMode( int32_t handle, N957ControlModes *mode)
*   \brief   Gets acquisition mode
*            
*   \param   handle board's handle
*   \param   mode acquisition mode
*   \return  N957Success: read ok : error code otherwise
*   \sa      N957ErrorCodes
*   \sa      N957ControlModes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_GetAcquisitionMode( int32_t handle, N957ControlModes *mode);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_SetAcquisitionMode( int32_t handle, N957ControlModes mode)
*   \brief   Sets acquisition mode
*            
*   \param   handle board's handle
*   \param   mode acquisition mode
*   \return  N957Success: write ok : error code otherwise
*   \sa      N957ErrorCodes
*   \sa      N957ControlModes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_SetAcquisitionMode( int32_t handle, N957ControlModes mode);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_GetBufferOccupancy( int32_t handle, N957_UINT16 *buffer_size)
*   \brief   Gets the data buffer occupancy
*            
*   \param   handle board's handle
*   \param   buffer_size The buffer occupancy (WORDs) 
*   \return  N957Success: read ok : error code otherwise
*   \sa      N957ErrorCodes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_GetBufferOccupancy( int32_t handle, N957_UINT16 *buffer_size);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_GetRawData( int32_t handle, N957_UINT16 *data_buff, N957_UINT16 *read_data)
*   \brief   Gets raw data from board FIFO
*            
*   \param   handle board's handle
*   \param   data_buff data buffer (caller allocated)
*   \param   read_data The buffer size on input (WORDs) and the number of WORDs read on output
*   \return  N957Success: read ok : error code otherwise
*   \sa      N957ErrorCodes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_GetRawData( int32_t handle, N957_UINT16 *data_buff, N957_UINT16 *read_data);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_GetScaler( int32_t handle, N957_UINT32 *scaler)
*   \brief   Gets the scaler counter
*            
*            This is the number of convertions occurred
*   \param   handle board's handle
*   \param   scaler The scaler value (bits)
*   \return  N957Success: read ok : error code otherwise
*   \sa      N957ErrorCodes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_GetScaler( int32_t handle, N957_UINT32 *scaler);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_GetTimer( int32_t handle, N957_UINT32 *timer)
*   \brief   Gets the timer value
*            
*   \param   handle board's handle
*   \param   timer The timer value (1 msec resolution)
*   \return  N957Success: read ok : error code otherwise
*   \sa      N957ErrorCodes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_GetTimer( int32_t handle, N957_UINT32 *timer);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_GetLiveTime( int32_t handle, N957_UINT32 *live_time)
*   \brief   Gets the live time timer value
*            
*   \param   handle board's handle
*   \param   live_time The live time timer value (1 msec resolution)
*   \return  N957Success: read ok : error code otherwise
*   \sa      N957ErrorCodes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_GetLiveTime( int32_t handle, N957_UINT32 *live_time);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_SetLLD( int32_t handle, N957_BYTE lld)
*   \brief   Sets the low level detect treshold
*            
*   \param   handle board's handle
*   \param   lld The low level detect value in te range [0..99] (0= min value  99= max value)
*   \return  N957Success: read ok : error code otherwise
*   \sa      N957ErrorCodes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
 */
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_SetLLD( int32_t handle, N957_BYTE lld);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_ReadFlashPage( int32_t handle, N957_BYTE data_buff[N957_FLASH_PAGE_SIZE], N957_UINT16 page_number)
*   \brief   Reads a page from flash
*            
*   \param   handle board's handle
*   \param   data_buff data buffer (caller allocated)
*   \param   page_number number of the page to read
*   \return  N957Success: procedure ok : error code otherwise
*   \sa      N957ErrorCodes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_ReadFlashPage( int32_t handle, N957_BYTE data_buff[N957_FLASH_PAGE_SIZE], N957_UINT16 page_number);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_WriteFlashPage( int32_t handle, N957_BYTE data_buff[N957_FLASH_PAGE_SIZE], N957_UINT16 page_number)
*   \brief   Reads a page from flash
*            
*   \param   handle board's handle
*   \param   data_buff data buffer (caller allocated)
*   \param   page_number number of the page to write
*   \return  N957Success: procedure ok : error code otherwise
*   \sa      N957ErrorCodes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_WriteFlashPage( int32_t handle, N957_BYTE data_buff[N957_FLASH_PAGE_SIZE], N957_UINT16 page_number);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_EraseFlashPage( int32_t handle, N957_UINT16 page_number)
*   \brief   Reads a page from flash
*            
*   \param   handle board's handle
*   \param   page_number number of the page to erase
*   \return  N957Success: procedure ok : error code otherwise
*   \sa      N957ErrorCodes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_EraseFlashPage( int32_t handle, N957_UINT16 page_number);

/*@}*/

/*!
* \defgroup level_2 Level 2 APIs
*/
/*@{*/

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_StartAcquire( int32_t handle, N957ControlModes mode)
*   \brief   Starts an acquisition
*            
*   \param   handle board's handle
*   \param   mode  acquisition mode
*   \return  N957Success: procedure executed correctly : error code otherwise
*   \sa      N957ErrorCodes
*   \sa      N957ControlModes 
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_StartAcquire( int32_t handle, N957ControlModes mode);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_PauseAcquire( int32_t handle)
*   \brief   Pauses acquisition: internal buffer will NOT be resetted and scaler and timers  will not be stopped
*            
*   \param   handle board's handle
*   \return  N957Success: procedure executed correctly : error code otherwise
*   \sa      N957ErrorCodes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_PauseAcquire( int32_t handle);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_ResumeAcquire( int32_t handle)
*   \brief   Resumes acquisition from pause
*            
*   \param   handle board's handle
*   \return  N957Success: procedure executed correctly : error code otherwise
*   \sa      N957ErrorCodes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_API N957_ResumeAcquire( int32_t handle);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_StopAcquire( int32_t handle)
*   \brief   Stops acquisition: scaler and timers will be stopped (internal buffer still keep data: use \ref N957_SwClear to clear data/scaler/timers)
*            
*   \param   handle board's handle
*   \return  N957Success: procedure executed correctly : error code otherwise
*   \sa      N957ErrorCodes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init)
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_StopAcquire( int32_t handle);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_ReadData( int32_t handle, N957_UINT16 *data_buff, N957_UINT16 *read_data)
*   \brief   Reads a block of data
*            
*   \param   handle board's handle
*   \param   data_buff data buffer (caller allocated)
*   \param   read_data The buffer size on input (WORDs) and the number of WORDs read on output
*   \return  N957Success: procedure executed correctly : error code otherwise
*   \sa      N957ErrorCodes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init) 
*            and acquisition must be started
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_ReadData( int32_t handle, N957_UINT16 *data_buff, N957_UINT16 *read_data);


////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_GetConfigROM( int32_t handle, N957_ConfigROM* config_rom)
*   \brief   Gets the configuration ROM stored into board flash
*            
*   \param   handle board's handle
*   \param   config_rom The configuration ROM buffer (caller allocated)
*   \return  N957Success: procedure executed correctly : error code otherwise
*   \sa      N957ErrorCodes
*   \sa      N957_ConfigROM
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init) 
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_GetConfigROM( int32_t handle, N957_ConfigROM* config_rom);

////////////////////////////////////////////////////////////////////////////////////////////////
/*! \fn      N957_API N957_FwUpgrade( int32_t handle, const N957_BYTE* data_buff, N957_UINT32 data_size, N957FlashPageTypes page_type)
*   \brief   Updates board firmware
*            
*   \param   handle board's handle
*   \param   data_buff: firmware data buffer
*   \param   data_size buffer size (BYTEs)
*   \param   page_type the type of page to download (Boot or Backup)
*   \return  N957Success: procedure executed correctly : error code otherwise
*   \sa      N957ErrorCodes
*   \sa      N957FlashPageTypes
*   \note    <i>handle</i> must be a valid one (as returned from \ref N957_Init) 
*/
////////////////////////////////////////////////////////////////////////////////////////////////
N957_DLL_API N957_API N957_FwUpgrade( int32_t handle, const N957_BYTE* data_buff, N957_UINT32 data_size, N957FlashPageTypes page_type);

/*@}*/
#ifdef __cplusplus
}
#endif

#endif 


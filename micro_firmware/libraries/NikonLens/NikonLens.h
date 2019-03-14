/*
 * NikonLens.h
 *
 *  Created: 11/14/2013 02:46:07
 *
 *  Lain A.
 *  code@hacktheinter.net
 *  http://hacktheinter.net
 *
 *  Copyright (c) 2013 Lain A.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *  THE SOFTWARE.
 */

#ifndef NIKONLENS_H_
#define NIKONLENS_H_

// known commands
#define CMD_GET_INFO 0x28
#define CMD_FOCUS_INFO 0x26
#define CMD_START_FOCUS 0xE0
#define CMD_SET_APERTURE 0xDA

#include <numeric.h>

//! Control Nikon F-mount lenses from an Arduino
/*! Nikon lenses use a modified version of SPI.
 * See also:
 *  -    Protocol: http://nikonhacker.com/wiki/Lens_Serial_Interface
 *  -      Pinout: http://nikonhacker.com/wiki/F-Mount
 *  - Arduino SPI: http://arduino.cc/en/Reference/SPI
 * 
 * Connections are as follows (with Arduino Uno pin numbers):
 *  - SS   (10) is unused.
 *  - SCK  (13) is connected directly to lens SCLK.
 *  - MISO (12) is connected to the lens Data line, which must be pulled high.
 *  - MOSI (11) drives the lens Data line through a transistor, open-collector.
 * 
 *    So: MOSI -> Base, Data -> Collector, GND -> Emitter
 *  - handshakePin_In  is connected directly to lens H/S - PIN 3 - PD0
 *  - handshakePin_Out drives lens H/S through a transistor, open-collector. - PIN 4 - PD4
 *  - 
 *
 * NOTE: The Nikon D5100 starts communication with the lens at 96kHz, then
 *       increases speed to 156kHz if the lens supports it. Because the
 *       Arduino runs at 16MHz and the maximum divider ratio for SPI is
 *       128, the slowest we can run the bus is 125kHz.
 * 
 *       If using this code with older lenses causes problems, it should
 *       be modified to use the AVR's System Clock Prescaler to drop the CPU
 *       frequency so you can achieve a slower SPI bus speed.
 * 
 *       I haven't tested this, it may interfere with other Arduino features.
 */

namespace lens
{

enum ApertureValue
{
	F1_4 = 0,
	F1_6 = 1,
	F1_8 = 2,
	F2_0 = 3,
	F2_2 = 4,
	F2_5 = 5,
	F2_8 = 6,
	F3_2 = 7,
	F3_5 = 8,
	F4_0 = 9,
	F4_5 = 10,
	F5_0 = 11,
	F5_6 = 12,
	F6_3 = 13,
	F7_1 = 14,
	F8_0 = 15,
	F9_0 = 16,
	F10_0 = 17,
	F11_0 = 18,
	F13_0 = 19,
	F14_0 = 20,
	F16_0 = 21
};

enum FocusValue
{
	FOCUS_MIN = 0,
	FOCUS_1_07 = 1,
	FOCUS_1_12 = 2,
	FOCUS_1_2 = 3,
	FOCUS_1_3 = 4,
	FOCUS_1_4 = 5,
	FOCUS_1_5 = 6,
	FOCUS_1_6 = 7,
	FOCUS_1_8 = 8,
	FOCUS_2 = 9,
	FOCUS_2_5 = 10,
	FOCUS_3 = 11,
	FOCUS_5 = 12,
	FOCUS_6 = 13,
	FOCUS_INF = 14
};

class NikonLens_Class
{
  public:
	
	// Possible result codes for commands
	enum tResultCode
	{
		Success,
		Timeout
	};

	//! Initialize SPI for lens communication.
	/*! See class description for connection information.
	 */
	void begin();

	//! Tear down, relinquish resources.
	void end();

	//todo: functions to send basic commands with more understandable arguments
	// like tResultCode DoAutoFocus(u8 unknown, tDirection dir, u16 steps);

	//! Send lens command and read/write data.
	/*! Sends command \a cmd to the lens.\n
	 If data is to be received, \a byteCountFromLens must be set
	 to the number of bytes expected.\n If data is to be sent,
	 \a byteCountToLens must be set to the number of bytes to send.\n
	 Received data is placed in \a bytesFromLens, so it must be sized
	 accordingly.\n Likewise, data sent to the lens is sourced from
	 \a bytesToLens.
	 
	 \param[in]  cmd				The command byte to send.
	 \param[in]  byteCountFromLens	Number of bytes to receive, or zero if not receiving.
	 \param[out] bytesFromLens		Destination for received bytes, or null if not receiving.
	 \param[in]  byteCountToLens    Number of bytes to send, or zero if not sending.
	 \param[in]  bytesToLens		Source of bytes sent to lens, or null if not sending.
	 \returns A result code indicating success, or the failure cause.
	 */
	tResultCode sendCommand(
		u8 cmd,
		u8 byteCountFromLens = 0,
		u8 *bytesFromLens = nullptr,
		u8 byteCountToLens = 0,
		u8 const *bytesToLens = nullptr);

	/**
	 * @brief Get basic lens information
	 * 
	 * @param code - 26 or 28. 28 by default.
	 * @return tResultCode 
	 */
	tResultCode displayInfo(int code=28);

	/**
	 * @brief Set the Aperture of the lens to the specified value
	 * 
	 * @param aperture - the aperture value desired - possible values are
	 * defined by the ApertureValue enum.
	 * @return tResultCode 
	 */
	tResultCode setAperture(ApertureValue aperture);

	/**
	 * @brief Prints input buffer with length specified by
	 * inputBuffer_length
	 * 
	 */
	void printInputBuffer();

	/**
	 * @brief Get the Current Focus of the lens.
	 * Uses command 0x26 to get a rough estimate of current focus 
	 * 
	 */
	FocusValue getCurrentFocus();


	/**
	 * @brief Sends a series of commands to initialise lens
	 * 
	 */
	void initLens();

	/**
	 * @brief 
	 * 
	 * @param steps 
	 * @return tResultCode 
	 */
	tResultCode driveFocus(int steps);

	private:
	
	// focus lookup table, from 1m to infinity
	// the exclamation marks refer to the real markings on lens, the others are interpolations

	int focus_lookup[15] = {
		0x5D49, /*!	<=1m, 5D49*/
		0x5E48, /* 	1.07m  */
		0x6048, /* 	1.13m  */
		0x6148, /*!	1.2m   */
		0x6348, /* 	1.3m   */
		0x6447, /* 	1.4m   */
		0x6647, /*!	1.5m   */
		0x6847, /* 	1.66m  */
		0x6946, /* 	1.83m  */
		0x6B46, /*!	2m 	   */
		0x6D46, /* 	2.5m   */
		0x6E45, /*!	3m 	   */
		0x7045, /*!	5m 	   */
		0x7245, /*  5_5m   */
		0x7344  /*!	<=inf or 7344 */
	};

	// aperture lookup table
	// 105mm f/1.4 has 22 aperture steps.
	// the lookup table contains the command bytes
	// that correspond to the aperture step value
	// on the lens. 
	// Ex: aperture_lookup[3] is the 4th aperture step (2.0)
	// command byte to set aperture to f/2.0 is 0x0C and 0x1B
	
	u8 aperture_lookup[22][2] = {
		{0x00, 0x15},
		{0x04, 0x1B},
		{0x08, 0x1B},
		{0x0C, 0x1B},
		{0x10, 0x1B},
		{0x14, 0x1B},
		{0x18, 0x1B},
		{0x1C, 0x1B},
		{0x20, 0x1B},
		{0x24, 0x1B},
		{0x28, 0x1B},
		{0x2C, 0x1B},
		{0x30, 0x1B},
		{0x34, 0x1B},
		{0x38, 0x1B},
		{0x3C, 0x1B},
		{0x40, 0x1B},
		{0x44, 0x1B},
		{0x48, 0x1B},
		{0x4C, 0x1B},
		{0x50, 0x1B},
		{0x54, 0x1B}};

	u8 m_handshakePin_In;
	u8 m_handshakePin_Out;

	// input buffer
	u8 inputBuffer[255];
	u8 inputBuffer_length;

	// output buffer
	u8 outputBuffer[8];
	u8 outputBuffer_length;

	// aperture value
	ApertureValue _aperture;

	//! Assert (pull down) the H/S line for \a microseconds.
	/*! \note This will block for the duration. */
	void assertHandshake(u16 microseconds);
	//! Shortcut to read handshake value
	/*! \returns \c true if H/S is asserted (low), \c false otherwise. */

	// bool isHandshakeAsserted() const { return 0 == digitalRead(m_handshakePin_In); } //
	// replaced digitalRead with register access equivalent
	bool isHandshakeAsserted() const { return 0 == ((PIND & (1 << PIND0)) >> PIND0); }

};

extern NikonLens_Class NikonLens;
} /* end namespace lens */

#endif /* NIKONLENS_H_ */

// eof

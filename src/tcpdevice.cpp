//---------------------------------------------------------------------------
// Copyright (c) 2020 Michael G. Brehm
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//---------------------------------------------------------------------------

#include "stdafx.h"
#include "tcpdevice.h"

#include <arpa/inet.h>
#include <cstring>
#include <netinet/tcp.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include "string_exception.h"

#pragma warning(push, 4)

//
// TODO: create socket_exception to replace the string_exceptions() used here
//

// tcpdevice::s_gaintable_e4k
//
std::vector<int> const tcpdevice::s_gaintable_e4k
	{ -10, 15, 40, 65, 90, 115, 140, 165, 190, 215, 240, 290, 340, 420 };

// tcpdevice::s_gaintable_fc0012
//
std::vector<int> const tcpdevice::s_gaintable_fc0012
	{ -99, -40, 71, 179, 192 };

// tcpdevice::s_gaintable_fc0013
//
std::vector<int> const tcpdevice::s_gaintable_fc0013
	{ -99, -73, -65, -63, -60, -58, -54, 58, 61, 63, 65, 67, 68, 70, 71, 179, 181, 182, 184, 186, 188, 191, 197 };

// tcpdevice::s_gaintable_fc2580
//
std::vector<int> const tcpdevice::s_gaintable_fc2580
	{ /* no gain values */ };

// tcpdevice::s_gaintable_r82xx
//
std::vector<int> const tcpdevice::s_gaintable_r82xx
	{ 0, 9, 14, 27, 37, 77, 87, 125, 144, 157, 166, 197, 207, 229, 254, 280, 297, 328, 338, 364, 372, 386, 402, 421, 434, 439, 445, 480, 496 };

// tcpdevice::s_gaintable_unknown
//
std::vector<int> const tcpdevice::s_gaintable_unknown
	{ /* no gain values */ };

//---------------------------------------------------------------------------
// tcpdevice Constructor (private)
//
// Arguments:
//
//	host	- Device host address
//	port	- Device port number

tcpdevice::tcpdevice(char const* host, uint16_t port)
{
	struct addrinfo* addrs = nullptr;			// Host device addresses
	struct addrinfo hints = {};					// getaddrinfo() hint values

	// Convert the port number into a string for getaddrinfo()
	char portstr[12] = { '\0' };
	snprintf(portstr, std::extent<decltype(portstr)>::value, "%d", port);

	hints.ai_family = AF_UNSPEC;				// IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM;			// Preferred socket type
	hints.ai_protocol = IPPROTO_TCP;			// TCP protocol
	hints.ai_flags = AI_PASSIVE;				// Bindable socket

	// Get the address information to connect to the host
	int result = getaddrinfo(host, portstr, &hints, &addrs);
	if(result != 0) throw string_exception("getaddrinfo() failed with result ", result);

	try {

		// Create the TCP/IP socket on which to communicate with the device
		m_socket = static_cast<int>(socket(addrs->ai_family, addrs->ai_socktype, addrs->ai_protocol));
		if(m_socket == -1) throw string_exception("socket() failed");

		try {

			// SO_REUSEADDR
			//
			int reuse = 1;

			result = setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char const*>(&reuse), sizeof(int));
			if(result == -1) throw string_exception("setsockopt(SO_REUSEADDR) failed");

			// SO_LINGER
			//
			struct linger linger = {};
			linger.l_onoff = 1;
			linger.l_linger = 0;

			result = setsockopt(m_socket, SOL_SOCKET, SO_LINGER, reinterpret_cast<char const*>(&linger), sizeof(struct linger));
			if(result == -1) throw string_exception("setsockopt(SO_LINGER) failed");

			// SO_RCVTIMEO
			//
#ifdef _WINDOWS
			DWORD timeout = 1000;
#else
			struct timeval timeout;
			timeout.tv_sec = 1;
			timeout.tv_usec = 0;
#endif
			result = setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char const*>(&timeout), sizeof(timeout));
			if(result == -1) throw string_exception("setsockopt(SO_RCVTIMEO) failed");

			// Establish the TCP/IP socket connection
			result = connect(m_socket, addrs->ai_addr, static_cast<int>(addrs->ai_addrlen));
			if(result != 0) throw string_exception("connect() failed");

			// TCP_NODELAY
			//
			int nodelay = 1;
			result = setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char const*>(&nodelay), sizeof(int));
			if(result == -1) throw string_exception("setsockopt(TCP_NODELAY) failed");

			// Retrieve the device information from the server
			struct device_info deviceinfo = {};
			result = recv(m_socket, reinterpret_cast<char*>(&deviceinfo), sizeof(struct device_info), 0);
			if(result != sizeof(struct device_info)) throw string_exception("recv(struct device_info) failed");

			// Parse the provided device information; only care about the tuner device type
			if(memcmp(deviceinfo.magic, "RTL0", 4) == 0) m_tunertype = static_cast<rtlsdr_tuner>(ntohl(deviceinfo.tuner_type));
			else throw string_exception("invalid device information returned from host");

			// Generate a device name for this instance
			char devicename[256];
			snprintf(devicename, std::extent<decltype(devicename)>::value, "Realtek RTL2832U on %s:%d", host, port);
			m_name.assign(devicename);
		}

		// Shutdown and close the socket on any exception
		catch(...) { close_socket(m_socket); throw; }

		freeaddrinfo(addrs);			// Release the list of address information
	}

	// Release the list of address information on any exception
	catch(...) { freeaddrinfo(addrs); throw; }
}

//---------------------------------------------------------------------------
// tcpdevice Destructor

tcpdevice::~tcpdevice()
{
	// Shutdown and close the socket
	if(m_socket != -1) {

#ifdef _WINDOWS
		shutdown(m_socket, SD_BOTH);
		closesocket(m_socket);
#else
		shutdown(m_socket, SHUT_RDWR);
		close(m_socket);
#endif
	}

	m_socket = -1;
}

//---------------------------------------------------------------------------
// tcpdevice::begin_stream
//
// Starts streaming data from the device
//
// Arguments:
//
//	NONE

void tcpdevice::begin_stream(void) const
{
	// No operation
}

//---------------------------------------------------------------------------
// tcpdevice::cancel_async
//
// Cancels any pending asynchronous read operations from the device
//
// Arguments:
//
//	NONE

void tcpdevice::cancel_async(void) const
{
}

//---------------------------------------------------------------------------
// tcpdevice::close_socket (static)
//
// Closes an open socket, implementation specific
//
// Arguments:
//
//	socket		- Socket instance to be closed

void tcpdevice::close_socket(int socket)
{
	if(socket == -1) return;

#ifdef _WINDOWS
	shutdown(socket, SD_BOTH);
	closesocket(socket);
#else
	shutdown(socket, SHUT_RDWR);
	close(socket);
#endif
}

//---------------------------------------------------------------------------
// tcpdevice::create (static)
//
// Factory method, creates a new tcpdevice instance
//
// Arguments:
//
//	host	- Device host address
//	port	- Device port number

std::unique_ptr<tcpdevice> tcpdevice::create(char const* host, uint16_t port)
{
	return std::unique_ptr<tcpdevice>(new tcpdevice(host, port));
}

//---------------------------------------------------------------------------
// tcpdevice::get_device_name
//
// Gets the name of the device
//
// Arguments:
//
//	NONE

char const* tcpdevice::get_device_name(void) const
{
	return m_name.c_str();
}

//---------------------------------------------------------------------------
// tcpdevice::get_valid_gains
//
// Gets the valid tuner gain values for the device
//
// Arguments:
//
//	dbs			- vector<> to retrieve the valid gain values

void tcpdevice::get_valid_gains(std::vector<int>& dbs) const
{
	dbs.clear();

	// The gain table cannot be interrogated via the TCP interface, use static
	// gain tables derived from librtlsdr ...

	switch(m_tunertype) {

		case rtlsdr_tuner::RTLSDR_TUNER_E4000: dbs = s_gaintable_e4k; break;
		case rtlsdr_tuner::RTLSDR_TUNER_FC0012: dbs = s_gaintable_fc0012; break;
		case rtlsdr_tuner::RTLSDR_TUNER_FC0013: dbs = s_gaintable_fc0013; break;
		case rtlsdr_tuner::RTLSDR_TUNER_FC2580: dbs = s_gaintable_fc2580; break;
		case rtlsdr_tuner::RTLSDR_TUNER_R820T: dbs = s_gaintable_r82xx; break;
		case rtlsdr_tuner::RTLSDR_TUNER_R828D: dbs = s_gaintable_r82xx; break;
		case rtlsdr_tuner::RTLSDR_TUNER_UNKNOWN: dbs = s_gaintable_unknown; break;
	}
}

//---------------------------------------------------------------------------
// tcpdevice::read
//
// Reads data from the device
//
// Arguments:
//
//	buffer		- Buffer to receive the data
//	count		- Size of the destination buffer, specified in bytes

size_t tcpdevice::read(uint8_t* buffer, size_t count) const
{
	assert(m_socket != -1);

	int read = recv(m_socket, reinterpret_cast<char*>(buffer), static_cast<int>(count), 0);
	if(read == -1) throw string_exception("recv() failed");

	return static_cast<size_t>(read);
}

//---------------------------------------------------------------------------
// tcpdevice::read_async
//
// Asynchronously reads data from the device
//
// Arguments:
//
//	callback		- Asynchronous read callback function
//	bufferlength	- Device buffer length, must be multiple of 512

void tcpdevice::read_async(rtldevice::asynccallback const& callback, uint32_t bufferlength) const
{
	return read_async(callback, 0, bufferlength);
}

//---------------------------------------------------------------------------
// tcpdevice::read_async
//
// Asynchronously reads data from the device
//
// Arguments:
//
//	callback		- Asynchronous read callback function
//	numbuffers		- Number of device buffers
//	bufferlength	- Device buffer length, must be multiple of 512

void tcpdevice::read_async(rtldevice::asynccallback const& /*callback*/, uint32_t /*numbuffers*/, uint32_t /*bufferlength*/) const
{
	throw string_exception("not implemented");
}

//---------------------------------------------------------------------------
// tcpdevice::set_automatic_gain_control
//
// Enables/disables the automatic gain control mode of the device
//
// Arguments:
//
//	enable		- Flag to enable/disable test mode

void tcpdevice::set_automatic_gain_control(bool enable) const
{
	assert(m_socket != -1);

	struct device_command command = { 0x03, htonl((enable) ? 1 : 0) };
	int result = send(m_socket, reinterpret_cast<char const*>(&command), sizeof(struct device_command), 0);
	if(result != sizeof(struct device_command)) throw string_exception("send() failed");
}

//---------------------------------------------------------------------------
// tcpdevice::set_center_frequency
//
// Sets the center frequency of the device
//
// Arguments:
//
//	hz		- Frequency to set, specified in hertz

uint32_t tcpdevice::set_center_frequency(uint32_t hz) const
{
	assert(m_socket != -1);

	struct device_command command = { 0x01, htonl(hz) };
	int result = send(m_socket, reinterpret_cast<char const*>(&command), sizeof(struct device_command), 0);
	if(result != sizeof(struct device_command)) throw string_exception("send() failed");

	return hz;
}

//---------------------------------------------------------------------------
// tcpdevice::set_frequency_correction
//
// Sets the frequency correction of the device
//
// Arguments:
//
//	ppm		- Frequency correction to set, specified in parts per million

int tcpdevice::set_frequency_correction(int ppm) const
{
	assert(m_socket != -1);

	struct device_command command = { 0x05, htonl(ppm) };
	int result = send(m_socket, reinterpret_cast<char const*>(&command), sizeof(struct device_command), 0);
	if(result != sizeof(struct device_command)) throw string_exception("send() failed");

	return ppm;
}

//---------------------------------------------------------------------------
// tcpdevice::set_gain
//
// Sets the gain of the device
//
// Arguments:
//
//	db			- Gain to set, specified in tenths of a decibel

int tcpdevice::set_gain(int db) const
{
	std::vector<int>	validgains;			// Gains allowed by the device

	assert(m_socket != -1);

	// Get the list of valid gain values for the device
	get_valid_gains(validgains);
	if(validgains.size() == 0) throw string_exception(__func__, ": failed to retrieve valid device gain values");

	// Select the gain value that's closest to what has been requested
	int nearest = validgains[0];
	for(size_t index = 0; index < validgains.size(); index++) {

		if(std::abs(db - validgains[index]) < std::abs(db - nearest)) nearest = validgains[index];
	}

	// Attempt to set the gain to the detected nearest gain value
	struct device_command command = { 0x04, htonl(nearest) };
	int result = send(m_socket, reinterpret_cast<char const*>(&command), sizeof(struct device_command), 0);
	if(result != sizeof(struct device_command)) throw string_exception("send() failed");

	// Return the gain value that was actually used
	return nearest;
}

//---------------------------------------------------------------------------
// tcpdevice::set_sample_rate
//
// Sets the sample rate of the device
//
// Arguments:
//
//	hz		- Sample rate to set, specified in hertz

uint32_t tcpdevice::set_sample_rate(uint32_t hz) const
{
	assert(m_socket != -1);

	struct device_command command = { 0x02, htonl(hz) };
	int result = send(m_socket, reinterpret_cast<char const*>(&command), sizeof(struct device_command), 0);
	if(result != sizeof(struct device_command)) throw string_exception("send() failed");

	return hz;
}

//---------------------------------------------------------------------------
// tcpdevice::set_test_mode
//
// Enables/disables the test mode of the device
//
// Arguments:
//
//	enable		- Flag to enable/disable test mode

void tcpdevice::set_test_mode(bool enable) const
{
	assert(m_socket != -1);

	struct device_command command = { 0x07, htonl((enable) ? 1 : 0) };
	int result = send(m_socket, reinterpret_cast<char const*>(&command), sizeof(struct device_command), 0);
	if(result != sizeof(struct device_command)) throw string_exception("send() failed");
}

//---------------------------------------------------------------------------

#pragma warning(pop)

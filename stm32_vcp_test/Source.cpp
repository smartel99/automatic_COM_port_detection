#include <iostream>
#include <string>
#include <windows.h>
#include <list>

/* --- Public Variable Definition --- */
HANDLE g_hSerial;
/* --- End of public variable definition --- */

/* --- Public Function Definition --- */
void print_serial_error(const char* msg);
bool initialise_serial_com();
std::string SelectComPort();
/* --- End of Public Function Definition --- */

int main(int argc, char** argv) {
	while (true) {
		if (!initialise_serial_com()) {
			std::cout << "Failed to initialize COM port, exiting program..." << std::endl;
			break;
		}
		else {
			std::cout << "COM initialization successful." << std::endl;
		}
		CloseHandle(g_hSerial);
	}

}


void print_serial_error(const char* msg)
{
	char e[1024];
	std::cout << msg << "\n\t";
	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		e,
		1024,
		NULL);
	std::cout << e << std::endl;
}


bool initialise_serial_com()
{
	std::string portNum = SelectComPort();

	if (portNum == "") {
		std::cout << "No port selected, exiting..." << std::endl;
		return false;
	}

	std::cout << "Opening COM port \"" << portNum << "\"..." << std::endl;
	g_hSerial = CreateFile(portNum.c_str(),
		GENERIC_READ | GENERIC_WRITE,
		0,
		0,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		0);

	if (g_hSerial == INVALID_HANDLE_VALUE) {
		print_serial_error("An error occurred while opening serial port:");
		return false;
	}
	else {
		std::cout << "Serial port opened." << std::endl;
	}

	std::cout << "Getting port settings..." << std::endl;
	DCB dcbSerialParams = { 0 };
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
	if (!GetCommState(g_hSerial, &dcbSerialParams)) {
		print_serial_error("An error occurred while getting port settings:");
		return false;
	}
	else
		std::cout << "Port settings acquired." << std::endl;

	std::cout << "Configuring serial port..." << std::endl;
	dcbSerialParams.BaudRate = CBR_9600;
	dcbSerialParams.ByteSize = 8;
	dcbSerialParams.StopBits = ONESTOPBIT;
	dcbSerialParams.Parity = NOPARITY;
	if (!SetCommState(g_hSerial, &dcbSerialParams)) {
		print_serial_error("An error occurred while configuring serial port:");
		return false;
	}
	else
		std::cout << "Serial port configured." << std::endl;

	std::cout << "Setting serial port timeouts..." << std::endl;
	COMMTIMEOUTS timeouts = { 0 };
	timeouts.ReadIntervalTimeout = 50;
	timeouts.ReadTotalTimeoutConstant = 50;
	timeouts.ReadTotalTimeoutMultiplier = 10;
	timeouts.WriteTotalTimeoutConstant = 50;
	timeouts.WriteTotalTimeoutMultiplier = 10;
	if (!SetCommTimeouts(g_hSerial, &timeouts)) {
		print_serial_error("An error occurred while setting serial port timeouts:");
		return false;
	}
	else
		std::cout << "Serial port timeouts configured." << std::endl;

	std::cout << "Serial port initialization completed." << std::endl;

	return true;
}


std::string SelectComPort()
{
	char lpTargetPath[5000]; // buffer to store the path of the COMPORTS
	bool gotPort = false; // in case the port is not found
	std::string port = "";
	std::list<std::string> portList;
	std::cout << "Please select the COM port:" << std::endl;
	std::string str;

	int num = 0;
	for (int i = 0; i < 255; i++) // checking ports from COM0 to COM255
	{
		str = "COM" + std::to_string(i); // converting to COM0, COM1, COM2
		DWORD test = QueryDosDevice(str.c_str(), lpTargetPath, 5000);

		// Test the return value and error if any
		if (test != 0) //QueryDosDevice returns zero if it didn't find an object
		{
			std::cout << num++ << "-  " << str << ": " << lpTargetPath << std::endl;
			str = "\\\\.\\" + str;
			portList.push_back(str);
		}

		if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		{
		}
	}

	int portNumber = 0;
	while (true) {
		std::cin >> portNumber;
		if (portNumber > portList.size() - 1) {
			std::cout << "Invalid port number. Please enter a number between 0 and " <<
				portList.size() - 1 << "." << std::endl;
			std::cin.clear();
			std::cin.ignore(INT_MAX, '\n');
			continue;
		}
		else if (!std::cin.good()) {
			break;
		}
		else {
			std::list<std::string>::iterator it = portList.begin();
			for (int i = 0; i < portNumber; i++, ++it);
			return *it;
		}
	}
	return port;
}

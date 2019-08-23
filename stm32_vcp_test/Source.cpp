#include <iostream>
#include <string>
#include <windows.h>
#include <list>

/* --- Private define --- */
/* --- End of private defines --- */

/* --- Public Variable Definitions --- */
HANDLE g_hSerial;
/* --- End of public variable definitions --- */

/* --- Public Function Definitions --- */
void print_serial_error(const char* msg);
bool initialise_serial_com();
bool initialise_serial_com(const char* port);
std::string SelectComPort();
std::list<std::string> get_all_available_com_port();
bool find_stm32_com_port();
/* --- End of Public Function Definitions --- */

int main(int argc, char** argv) {
	find_stm32_com_port();
	/*while (true) {
		if (!initialise_serial_com()) {
			std::cout << "Failed to initialize COM port, exiting program..." << std::endl;
			break;
		}
		else {
			std::cout << "COM initialization successful." << std::endl;
		}
		CloseHandle(g_hSerial);
	}*/
	std::cout << "Press any key to continue..." << std::endl;
	char var[10];
	std::cin.getline(var, 10);

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

#ifdef _DEBUG
	std::cout << "Opening COM port \"" << portNum << "\"..." << std::endl;
#endif
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
#ifdef _DEBUG
	else {
		std::cout << "Serial port opened." << std::endl;
	}

	std::cout << "Getting port settings..." << std::endl;
#endif
	DCB dcbSerialParams = { 0 };
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
	if (!GetCommState(g_hSerial, &dcbSerialParams)) {
		print_serial_error("An error occurred while getting port settings:");
		return false;
	}

#ifdef _DEBUG
	else
		std::cout << "Port settings acquired." << std::endl;

	std::cout << "Configuring serial port..." << std::endl;
#endif
	dcbSerialParams.BaudRate = CBR_9600;
	dcbSerialParams.ByteSize = 8;
	dcbSerialParams.StopBits = ONESTOPBIT;
	dcbSerialParams.Parity = NOPARITY;
	if (!SetCommState(g_hSerial, &dcbSerialParams)) {
		print_serial_error("An error occurred while configuring serial port:");
		return false;
	}
#ifdef _DEBUG
	else
		std::cout << "Serial port configured." << std::endl;

	std::cout << "Setting serial port timeouts..." << std::endl;
#endif
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
#ifdef _DEBUG
	else
		std::cout << "Serial port timeouts configured." << std::endl;

	std::cout << "Serial port initialization completed." << std::endl;
#endif

	return true;
}

bool initialise_serial_com(const char* port)
{
	g_hSerial = CreateFile(port,
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

	DCB dcbSerialParams = { 0 };
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
	if (!GetCommState(g_hSerial, &dcbSerialParams)) {
		print_serial_error("An error occurred while getting port settings:");
		return false;
	}

	dcbSerialParams.BaudRate = CBR_9600;
	dcbSerialParams.ByteSize = 8;
	dcbSerialParams.StopBits = ONESTOPBIT;
	dcbSerialParams.Parity = NOPARITY;
	if (!SetCommState(g_hSerial, &dcbSerialParams)) {
		print_serial_error("An error occurred while configuring serial port:");
		return false;
	}

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


std::list<std::string> get_all_available_com_port()
{
	char lpTargetPath[5000]; // buffer to store the path of the COMPORTS
	bool gotPort = false; // in case the port is not found
	std::list<std::string> portList;
	std::string str;

	int num = 0;
	for (int i = 0; i < 255; i++) // checking ports from COM0 to COM255
	{
		str = "COM" + std::to_string(i); // converting to COM0, COM1, COM2
		DWORD test = QueryDosDevice(str.c_str(), lpTargetPath, 5000);

		// Test the return value and error if any
		if (test != 0) //QueryDosDevice returns zero if it didn't find an object
		{
			str = "\\\\.\\" + str;
			portList.push_back(str);
		}

		if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		{
		}
	}

	std::cout << "Found " << portList.size() << " COM ports." << std::endl;
#ifdef _DEBUG
	for (std::list<std::string>::iterator it = portList.begin(); it != portList.end(); ++it) {
		std::cout << "\t- " << *it << std::endl;
	}
#endif
	return portList;
}

bool find_stm32_com_port()
{
	const char* req = "ID";
	DWORD dwBytesRead = 0;
	char rx[6];

	std::list<std::string> portList = get_all_available_com_port();
	for (std::list<std::string>::iterator it = portList.begin(); it != portList.end(); ++it) {
#ifdef _DEBUG
		std::cout << "Trying port " << *it << "..." << std::endl;
#endif
		if (!initialise_serial_com(it->c_str()))
			continue;
		for (int i = 5; i > 0; i--) {
			if (!WriteFile(g_hSerial, req, 3, &dwBytesRead, NULL)) {
				print_serial_error("An error has occurred during data request:");
				continue;
			}

			if (!ReadFile(g_hSerial, rx, 6, &dwBytesRead, NULL)) {
				print_serial_error("An error has occurred during data reading:");
				continue;
			}

			bool is_stm32 = true;
			std::cout << "Received: \"" << rx << "\"" << std::endl;
			for (int j = 0; j < 3; j++) {
				if (rx[j] != req[j]) {
					is_stm32 = false;
					break;
				}

			}
			if (is_stm32) {
				std::cout << "Found STM32's VCP on port " << *it << "!!!" << std::endl;
				return true;
			}
		}
		CloseHandle(g_hSerial);
	}
	std::cout << "Can't find the STM32's VCP anywhere :'(" << std::endl;
	return false;
}

#include <ipc.h>
#include <logger.h>

const char* IPC::SpawnAndWaitForPipe(std::string pipeName) {
	HANDLE hPipe = CreateNamedPipeA(pipeName.c_str(), PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE | PIPE_WAIT, 1, MAX_PATH, MAX_PATH, NMPWAIT_USE_DEFAULT_WAIT, NULL);
	char* readBuffer = (char*)_malloca(MAX_PATH + 1);
	if (!readBuffer) {
		throw std::exception("Failed to allocate buffer");
	}
	while (true) {
		if (ConnectNamedPipe(hPipe, NULL) != false) {
			bool readPipe = ReadFile(hPipe, readBuffer, MAX_PATH, NULL, NULL);
			if (!readPipe) {
				if (GetLastError() == ERROR_BROKEN_PIPE) {
					throw std::exception("Pipe client disconnected");
				}
				else if (GetLastError() == ERROR_PIPE_LISTENING) {
					Sleep(100);
					continue;
				}
				else {
					std::string errMsg = "Couldn't read pipe: GLE: " + std::to_string(GetLastError());
					throw std::exception(errMsg.c_str());
				}
			}
			else {
				DisconnectNamedPipe(hPipe);
				break;
			}
		}
		else {
			Sleep(100);
			Logger::Print("Failed to connect to named pipe, retrying...");
		}
	}
	CloseHandle(hPipe);
	return readBuffer;
}
HANDLE IPC::OpenPipe(std::string pipeName) {
	return CreateFile(pipeName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
}
const char* IPC::ReadBytes(HANDLE hPipe) {
	char msgBuf[MAX_PATH];
	if (ReadFile(hPipe, msgBuf, MAX_PATH, NULL, NULL)) {
		return msgBuf;
	}
	throw std::exception("Cannot read pipe");
}
std::string IPC::ReadMessage(HANDLE hPipe) {
	return ReadBytes(hPipe);
}
void IPC::WriteBytes(HANDLE hPipe, const char bytes[MAX_PATH]) {
	WriteFile(hPipe, bytes, MAX_PATH, NULL, NULL);
}
void IPC::WriteMessage(HANDLE hPipe, std::string message) {
	char msgBuf[MAX_PATH];
	strcpy_s(msgBuf, message.c_str());
	WriteBytes(hPipe, msgBuf);
}
void IPC::ClosePipe(HANDLE hPipe) {
	CloseHandle(hPipe);
}
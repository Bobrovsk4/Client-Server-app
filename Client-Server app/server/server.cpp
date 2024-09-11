#include <iostream>
#include <WinSock2.h>
#include <string>
#include <vector>
#include <thread>

#pragma warning(disable: 4996)

void connecting(SOCKET slisten, sockaddr_in addr) {
	SOCKET newConnection;
	int sizeof_addr = sizeof(addr);

	newConnection = accept(slisten, (SOCKADDR*)&addr, &sizeof_addr);

	if (newConnection == INVALID_SOCKET) {
		std::cout << "Connection error.\n";
		return;
	}

	std::string msgChoose = "Choose game:\n1.Roulette\n2.Craps\n0.Quit.\n";
	send(newConnection, msgChoose.c_str(), msgChoose.length(), NULL);
	char answer[1];
	recv(newConnection, answer, 1, NULL);

	if (atoi(answer) == 1) {
		closesocket(newConnection);

		newConnection = socket(AF_INET, SOCK_STREAM, NULL);
		if (newConnection == INVALID_SOCKET) {
			std::cerr << "Failed to create a new socket.\n";
			return;
		}

		sockaddr_in newServerAddr;
		newServerAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
		newServerAddr.sin_port = htons(11111);
		newServerAddr.sin_family = AF_INET;

		if (connect(newConnection, (SOCKADDR*)&newServerAddr, sizeof(newServerAddr)) == SOCKET_ERROR) {
			std::cerr << "Failed to connect to the Roulette server.\n";
			closesocket(newConnection);
			return;
		}
	}
	else if (atoi(answer) == 2) {
		closesocket(newConnection);

		newConnection = socket(AF_INET, SOCK_STREAM, NULL);
		if (newConnection == INVALID_SOCKET) {
			std::cerr << "Failed to create a new socket.\n";
			return;
		}

		sockaddr_in newServerAddr;
		newServerAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
		newServerAddr.sin_port = htons(22222);
		newServerAddr.sin_family = AF_INET;

		if (connect(newConnection, (SOCKADDR*)&newServerAddr, sizeof(newServerAddr)) == SOCKET_ERROR) {
			std::cerr << "Failed to connect to the Craps server.\n";
			closesocket(newConnection);
			return;
		}
	}
	else if (atoi(answer) == 0) {
		closesocket(newConnection);
		return;
	}
}

int main() {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cerr << "Winsock initialization failed.\n";
		return 1;
	}

	sockaddr_in addr;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(12345);
	addr.sin_family = AF_INET;

	SOCKET slisten = socket(AF_INET, SOCK_STREAM, NULL);
	if (slisten == INVALID_SOCKET) {
		std::cerr << "Failed to create listening socket.\n";
		WSACleanup();
		return 1;
	}

	if (bind(slisten, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) {
		std::cerr << "Bind failed.\n";
		closesocket(slisten);
		WSACleanup();
		return 1;
	}

	if (listen(slisten, SOMAXCONN) == SOCKET_ERROR) {
		std::cerr << "Listen failed.\n";
		closesocket(slisten);
		WSACleanup();
		return 1;
	}

	while (true) {
		std::thread th(connecting, slisten, addr);
		th.detach();
	}

	closesocket(slisten);
	WSACleanup();
	return 0;
}

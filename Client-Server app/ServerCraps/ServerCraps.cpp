#include <iostream>
#include <WinSock2.h>
#include <chrono>
#include <thread>
#include <string>

#pragma warning(disable: 4996)

struct player {
	SOCKET connection;
	std::string name;
	unsigned int balance;

	player() {}
	player(SOCKET con, std::string nm, unsigned int bal) :connection(con), name(nm), balance(bal) {}
};
class Craps {
public:
	static void bet(player& p1, player& p2) {

		char bet1[256], bet2[256];

		std::string offer1 = p1.name + ", make a bet (type -1 to quit): ";
		send(p1.connection, offer1.c_str(), offer1.length(), NULL);
		recv(p1.connection, bet1, 256, NULL);
		int player1_bet = atoi(bet1);

		if (player1_bet == -1) {
			closesocket(p1.connection);

			p1.connection = socket(AF_INET, SOCK_STREAM, NULL);
			if (p1.connection == INVALID_SOCKET) {
				std::cerr << "Failed to create a new socket.\n";
				return;
			}

			sockaddr_in newServerAddr;
			newServerAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
			newServerAddr.sin_port = htons(12345);
			newServerAddr.sin_family = AF_INET;

			if (connect(p1.connection, (SOCKADDR*)&newServerAddr, sizeof(newServerAddr)) == SOCKET_ERROR) {
				std::cerr << "Failed to connect to the new server.\n";
				closesocket(p1.connection);
				return;
			}

			std::string msgLeft = "Opponent left. Waiting for another one. Type -1 to quit: ";
			send(p2.connection, msgLeft.c_str(), msgLeft.length(), NULL);
		}
		else {
			std::string offer2 = p2.name + ", make a bet (type -1 to quit): ";
			send(p2.connection, offer2.c_str(), offer2.length(), NULL);
		}
		recv(p2.connection, bet2, 256, NULL);
		int player2_bet = atoi(bet2);

		if (player2_bet == -1) {
			closesocket(p2.connection);

			p2.connection = socket(AF_INET, SOCK_STREAM, NULL);
			if (p2.connection == INVALID_SOCKET) {
				std::cerr << "Failed to create a new socket.\n";
				return;
			}

			sockaddr_in newServerAddr;
			newServerAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
			newServerAddr.sin_port = htons(12345);
			newServerAddr.sin_family = AF_INET;

			if (connect(p2.connection, (SOCKADDR*)&newServerAddr, sizeof(newServerAddr)) == SOCKET_ERROR) {
				std::cerr << "Failed to connect to the new server.\n";
				closesocket(p2.connection);
				return;
			}
		}
		else {
			p1.balance -= player1_bet;
			p2.balance -= player2_bet;

			throw_craps(p1, p2, player1_bet, player2_bet);
		}
	}

	static void throw_craps(player& p1, player& p2, int bet1, int bet2) {

		std::string msgThrowing = "Throwing 2 craps...\n";
		send(p1.connection, msgThrowing.c_str(), msgThrowing.length(), NULL);
		send(p2.connection, msgThrowing.c_str(), msgThrowing.length(), NULL);
		std::this_thread::sleep_for(std::chrono::milliseconds(500));

		int res1 = 1 + rand() % 5;
		int res2 = 1 + rand() % 5;

		std::string msgRes = "First crap: " + std::to_string(res1) + ".\n" + "Second crap: " + std::to_string(res2) + ".\n";
		send(p1.connection, msgRes.c_str(), msgRes.length(), NULL);
		send(p2.connection, msgRes.c_str(), msgRes.length(), NULL);

		std::string msgWinner;
		if (res1 == res2) {
			msgWinner = "Draw! Each player gets back his bet.\n";
			send(p1.connection, msgWinner.c_str(), msgWinner.length(), NULL);
			send(p2.connection, msgWinner.c_str(), msgWinner.length(), NULL);
			p1.balance += bet1;
			p2.balance += bet2;
		}
		else if (res1 > res2) {
			msgWinner = p1.name + " won!\n";
			send(p1.connection, msgWinner.c_str(), msgWinner.length(), NULL);
			send(p2.connection, msgWinner.c_str(), msgWinner.length(), NULL);
			p1.balance += (bet1 + bet2);
		}
		else if (res1 < res2) {
			msgWinner = p2.name + " won!\n";
			send(p1.connection, msgWinner.c_str(), msgWinner.length(), NULL);
			send(p2.connection, msgWinner.c_str(), msgWinner.length(), NULL);
			p2.balance += (bet1 + bet2);
		}

		std::string balanceInfo1 = "Your balance: " + std::to_string(p1.balance) + "\n";
		std::string balanceInfo2 = "Your balance: " + std::to_string(p2.balance) + "\n";
		send(p1.connection, balanceInfo1.c_str(), balanceInfo1.length(), NULL);
		send(p2.connection, balanceInfo2.c_str(), balanceInfo2.length(), NULL);
	}
};

int main() {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cerr << "Server start error.\n";
		return 1;
	}

	sockaddr_in addr;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(22222);
	addr.sin_family = AF_INET;

	SOCKET slisten = socket(AF_INET, SOCK_STREAM, NULL);
	bind(slisten, (SOCKADDR*)&addr, sizeof(addr));
	listen(slisten, SOMAXCONN);

	SOCKET newConnection;
	SOCKET Connections[2];
	int sizeof_addr = sizeof(addr);
	int counter = 0;
	bool playing = false;

	while (true) {
		if (counter < 2) {
			playing = false;

			for (int i = 0; i < 2; ++i) {
				newConnection = accept(slisten, (SOCKADDR*)&addr, &sizeof_addr);

				if (newConnection == 0) {
					std::cout << "Connection error.\n";
					return 1;
				}

				Connections[i] = newConnection;
				counter++;
				std::string msgWaiting = "Waiting for second player to join...\n";
				send(newConnection, msgWaiting.c_str(), msgWaiting.length(), NULL);
			}
		}
		else if (counter == 2 && !playing) {
			std::string msgStart = "Starting...\n";
			send(Connections[0], msgStart.c_str(), msgStart.length(), NULL);
			send(Connections[1], msgStart.c_str(), msgStart.length(), NULL);
			std::this_thread::sleep_for(std::chrono::milliseconds(200));

			char name1[50], name2[50];
			std::string msgName = "Type your name: ";
			send(Connections[0], msgName.c_str(), msgName.length(), NULL);
			recv(Connections[0], name1, 50, NULL);
			send(Connections[1], msgName.c_str(), msgName.length(), NULL);
			recv(Connections[1], name1, 50, NULL);
			
			std::string nm1 = "", nm2 = "";
			for (int i = 0; i < 50; ++i) {
				if (isalpha(name1[i]) || isalnum(name1[i])) {
					nm1 += name1[i];
				}
				if (isalpha(name2[i]) || isalnum(name2[i])) {
					nm2 += name2[i];
				}
			}
			player p1(Connections[0], nm1, 1000);
			player p2(Connections[1], nm2, 1000);
			Craps::bet(p1, p2);
			playing = true;
		}
	}
	closesocket(slisten);
	WSACleanup();
	return 0;
}
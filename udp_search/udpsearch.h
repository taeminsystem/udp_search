#pragma once

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <string>

class UdpSearch
{
public:
	UdpSearch() {}
	~UdpSearch() {}

	void setPort(int port)
	{
		port_ = port;
	}

	struct IPv4
	{
		unsigned char b1, b2, b3, b4;
	};

	bool init(std::string &ip)
	{
		IPv4 myIP;
		char szBuffer[1024];
		WSADATA wsaData;
		WORD wVersionRequested = MAKEWORD(2, 0);
		if (::WSAStartup(wVersionRequested, &wsaData) != 0)
			return false;

		if (gethostname(szBuffer, sizeof(szBuffer)) == SOCKET_ERROR)
		{
#ifdef WIN32
			WSACleanup();
#endif
			return false;
		}

		struct hostent *host = gethostbyname(szBuffer);
		if (host == NULL)
		{
#ifdef WIN32
			WSACleanup();
#endif
			return false;
		}

		myIP.b1 = ((struct in_addr *)(host->h_addr))->S_un.S_un_b.s_b1;
		myIP.b2 = ((struct in_addr *)(host->h_addr))->S_un.S_un_b.s_b2;
		myIP.b3 = ((struct in_addr *)(host->h_addr))->S_un.S_un_b.s_b3;
		myIP.b4 = ((struct in_addr *)(host->h_addr))->S_un.S_un_b.s_b4;
		

		std::cout << "Ip address " << (int)myIP.b1 << " " << (int)myIP.b2 << " " << (int)myIP.b3 << " " << (int)myIP.b4 << std::endl;
		char buffer[256];
		sprintf(buffer, "%d.%d.%d.%d", myIP.b1, myIP.b2, myIP.b3, myIP.b4);
		//std::cout << buffer << std::endl;
		ip_ = buffer;
		ip = ip_;
		return true;
		return true;
	}

	std::string getSubNetwork(std::string ip)
	{
		std::size_t pos = std::string::npos;

		pos = ip.find_last_of('.');
		if (pos == std::string::npos)
			return "";
		std::string network = ip.substr(0, pos);
		std::cout << "network " << network << std::endl;
		return network;

	}


	bool connect(std::string ip)
	{
		int sock;
		int iret;
		std::string msg = "ping:" + ip;

		struct sockaddr_in si_other;

		int slen = sizeof(si_other);
		sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (sock == SOCKET_ERROR)
			return false;

		memset((char*)&si_other, 0, sizeof(si_other));
		si_other.sin_family = AF_INET;
		si_other.sin_port = htons(port_);
		si_other.sin_addr.S_un.S_addr = inet_addr(ip.c_str());

		DWORD timeout = 1000; // 1 sec timeout
		iret = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,(const char *) &timeout, sizeof(DWORD));
		if (iret != 0)
		{
			std::cout << "Error setsockopt " << iret << std::endl;
			return false;
		}
		iret = sendto(sock, msg.c_str(), msg.length(), 0, (struct sockaddr *)&si_other, slen);
		if (iret == SOCKET_ERROR)
		{
			std::cout << "Sendto failed" << std::endl;
			return false;
		}

		char buf[128];
		iret = recvfrom(sock, buf, sizeof(buf), 0,(struct sockaddr *) &si_other, &slen);
		if (iret == SOCKET_ERROR)
		{
			std::cout << "recvfrom failed" << std::endl;
			return false;
		}

		buf[iret] = 0;


		return true;
	}

	std::string findServer()
	{
		std::string network = getSubNetwork(ip_);
		for (int i = 0; i < 255; i++)
		{
			std::string target = network + "." + std::to_string(i);
			std::cout << target << std::endl;
			bool success = connect(target);
			if (success == true)
				return target;
		}

		return "";
	}
	
	int port_;
	std::string ip_;
};
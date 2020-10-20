#include "Util.h"

#ifdef _WIN32
#include <Winsock2.h>
#pragma comment(lib,"Ws2_32.lib")
#else
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/types.h>
#include <ifaddrs.h>

#include <string.h>

#include <sys/time.h>
#endif

#define MAX_HOSTNAME	128
#define IFA_NAME		"eth0"


std::string& CUtil::StrTrim(std::string &s)
{
	const std::string drop = " ";
	s.erase(s.find_last_not_of(drop)+1);
	s.erase(0, s.find_first_not_of(drop));

	return s;
}

#ifdef _WIN32
int CUtil::InitNet()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	/* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
	wVersionRequested = MAKEWORD(2, 2);

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) 
	{
		/* Tell the user that we could not find a usable */
		/* Winsock DLL.                                  */
		//printf("WSAStartup failed with error: %d\n", err);
		return -1;
	}

	return 0;
}

void CUtil::DelNet()
{
	WSACleanup();
}
#endif

std::string CUtil::GetHostName()
{
	char buf[MAX_HOSTNAME] = {0};
	if(gethostname(buf, sizeof(buf)) != 0)
	{
		//int err = WSAGetLastError();
	}
	
	return std::string(buf);
}

std::string CUtil::GetHotAddr(const std::string &name)
{
	struct hostent *he = NULL;

	he = gethostbyname(name.c_str());
	if(he == NULL)
	{
		return "127.0.0.1";
	}
	
	struct in_addr addr;
	
#ifdef _WIN32
		addr.S_un.S_addr = *(unsigned long *)he->h_addr;
#else
		addr.s_addr = *(unsigned long *)he->h_addr;
		if(std::string(inet_ntoa(addr)) == "127.0.0.1")
		{
			struct ifaddrs *ifadds = NULL;
			void *tmpaddr = NULL;
			getifaddrs(&ifadds);
			while(ifadds != NULL)
			{
				if(strcmp(ifadds->ifa_name, IFA_NAME) == 0)
				{
					tmpaddr = &((struct sockaddr_in*)(ifadds->ifa_addr))->sin_addr;
					if(ifadds->ifa_addr->sa_family == AF_INET)
					{					
						char ip[INET_ADDRSTRLEN];
						inet_ntop(AF_INET, tmpaddr, ip, INET_ADDRSTRLEN);
						printf("%s  IP: %s\n", ifadds->ifa_name, ip);

						return ip;
					}
					else if(ifadds->ifa_addr->sa_family == AF_INET6)
					{
						char ip[INET6_ADDRSTRLEN];
						inet_ntop(AF_INET6, tmpaddr, ip, INET6_ADDRSTRLEN);
						return ip;
					}		 
				}

				ifadds = ifadds->ifa_next;
			}
		}
#endif

	return std::string(inet_ntoa(addr));
}


long long CUtil::GetTimeStamp()
{
#ifdef _WIN32
	LARGE_INTEGER lFreq;
	LARGE_INTEGER lTimeCount;
	QueryPerformanceCounter(&lTimeCount);
	QueryPerformanceFrequency(&lFreq);
	long long time = (lTimeCount.QuadPart * 1000 * 1000/ lFreq.QuadPart);
	return time;
#else
	struct timeval curTime;
	gettimeofday(&curTime, NULL);

	return  (curTime.tv_sec * 1000000L + curTime.tv_usec);
#endif
}
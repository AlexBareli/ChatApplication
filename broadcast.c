#include "project05.h"
#define LAB_ADDR "10.10.13.255"

struct Users currusrs;

struct User lookup_users (char *name) {
	struct User tmp;
	for (int i = 0; i < currusrs.num_users; i++) {
		if ( !strcmp(currusrs.users[i].name, name) ) {
			return currusrs.users[i];
		} 
	}
	return tmp;
}

struct User lookup_users_host (char *host) {
	struct User tmp;
	for (int i = 0; i < currusrs.num_users; i++) {
		if ( !strcmp(currusrs.users[i].host, host) ) {
			return currusrs.users[i];
		} 
	}
	return tmp;
}

void users_update (struct User *tmp) {
	int i;
	for (i = 0; i < currusrs.num_users; i++) {
		if ( !strcmp(currusrs.users[i].name, tmp->name) ) {
			if ( !strcmp(currusrs.users[i].status, tmp->status) ) {
				break;
			} else {
				printf("%s is %s on port %s \n", tmp->name, tmp->status, tmp->port);
				break;
			}
		}
	}
	if (i == currusrs.num_users) {
		currusrs.users[currusrs.num_users++] = *tmp;
		printf("%s is %s on port %s \n", tmp->name, tmp->status, tmp->port);
	}
}

int initialize_broadcast() {
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;
	struct addrinfo *myaddr;
	int addrread = getaddrinfo(NULL, "8221", &hints, &myaddr);
	if (addrread != 0) {
		gai_strerror(addrread);
	}
	int sfd = socket(myaddr->ai_family, myaddr->ai_socktype, myaddr->ai_protocol);
	int enable = 1;
	setsockopt(sfd, SOL_SOCKET, SO_BROADCAST, &enable, sizeof(enable));
	setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
	setsockopt(sfd, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(enable));
	bind(sfd, myaddr->ai_addr, myaddr->ai_addrlen);
	freeaddrinfo(myaddr);
	return sfd;
}

int initialize_tcp (char *port) {
	struct addrinfo thints;
	memset(&thints, 0, sizeof(thints));
	thints.ai_family = AF_INET;
	thints.ai_socktype = SOCK_STREAM;
	thints.ai_flags = AI_PASSIVE;
	thints.ai_protocol = IPPROTO_TCP; 
	struct addrinfo *myaddr;
	int addrread = getaddrinfo(NULL, port, &thints, &myaddr);
	if (addrread != 0) {
		gai_strerror(addrread);
	}
	int sfd = socket(myaddr->ai_family, myaddr->ai_socktype, myaddr->ai_protocol);
	int enable = 1;
	setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
	ioctl(sfd, FIONBIO, enable);
	bind(sfd, myaddr->ai_addr, myaddr->ai_addrlen);
	listen(sfd, NUM_POLLS);
	freeaddrinfo(myaddr);
	return sfd;
}

void read_presence (int sfd) {
	struct sockaddr_storage sockdraw;
	struct User tmp;
	char msg[128];
	char service[NI_MAXSERV];
	socklen_t size = sizeof(sockdraw);
	recvfrom(sfd, msg, sizeof(msg), 0, (struct sockaddr *) &sockdraw, &size);
	sscanf(msg, "%s %s %s", tmp.name, tmp.status, tmp.port);
	int host = getnameinfo((struct sockaddr *) &sockdraw, size, tmp.host, NI_MAXHOST,
	 			service, NI_MAXSERV, NI_NUMERICSERV);
	users_update(&tmp);
	memset(msg, 0, sizeof(msg));
}

void read_tcp (int tcp) {
	struct sockaddr_in peer;
	socklen_t size = sizeof(peer);
	char msg[128];
	char service[NI_MAXSERV];
	char tcphost[NI_MAXHOST];
	if ( recv(tcp, msg, sizeof(msg), 0) != 0 ) {
		getpeername(tcp, (struct sockaddr *) &peer, &size);
		getnameinfo((struct sockaddr *) &peer, size, tcphost, NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICSERV);
		struct User tmp = lookup_users_host(tcphost);
		printf("%s says: %s \n", tmp.name, msg);
	}
}

void write_presence (int sfd, char *name, char *status, char *port) {
	char str[200];
	snprintf(str, sizeof(str), "%s %s %s", name, status, port);
	struct sockaddr_in mysockin;
	mysockin.sin_family = AF_INET;
	inet_pton(AF_INET, LAB_ADDR, &mysockin.sin_addr);
	mysockin.sin_port = htons(8221);
	sendto(sfd, &str, strlen(str) + 1, 0, (struct sockaddr *) &mysockin, 128);
	memset(str, 0, sizeof(str));
}

void write_tcp (char *usr, char *msg) {
	struct User tmp = lookup_users(usr);
	struct addrinfo hints;
	struct addrinfo *host;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	int addrread = getaddrinfo(tmp.host, tmp.port, &hints,  &host);
	int tcpfd = socket(host->ai_family, host->ai_socktype, host->ai_protocol);
	connect(tcpfd, host->ai_addr, host->ai_addrlen);
	send(tcpfd, msg, sizeof(msg), 0);
	freeaddrinfo(host);
}



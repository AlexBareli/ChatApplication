#define _project05_H

#include <poll.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/ioctl.h>
#define TIMEOUT 100
#define NUM_POLLS 50

struct User {
	char name[64];
	char status[64];
	char port[64];
	char host[128];
};

struct Users {
	struct User users[NUM_POLLS];
	int num_users;
};

struct polls {
	struct pollfd polls[NUM_POLLS];
	int polls_index;
};
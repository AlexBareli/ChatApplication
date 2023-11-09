
#include "broadcast.c"

void init_poll (struct pollfd *poll, int pfd, int polltype) {
	if (pfd >= NUM_POLLS) {
		printf("Exceeds number of maximum polls");
		exit(-1);
	}
	poll[pfd].fd = polltype;
	poll[pfd].events = POLLIN;
	poll[pfd].revents = 0;
}

void setup_polls (struct polls *poll) {
	poll->polls_index = 0;
	init_poll(poll->polls, poll->polls_index++, STDIN_FILENO);
}

void process_str (char *str) {
	char tmp[sizeof(str)];
	const char s[1] = ":";
	char *token;
	char usr[64];
	char msg[128];
	if (str[0] != '@') {
		printf("Invalid User, please use following format: '@User: Msg' \n");
	} else {
		token = strtok(str, s);
		sscanf(token, "@%s:", usr);
		while (token != NULL) {
			strcpy(msg, token);
			token = strtok(NULL, s);
		}  
		write_tcp(usr, msg);
	}
}

int accum_chat (char *chatstr, int len) {
	bool eof = false;
	char ch;
	int readpoll;
	readpoll = read(STDIN_FILENO, &ch, 1);
	if (readpoll == 0) {
		eof = true;
	} else if (readpoll == 1) {
		chatstr[len] = ch;
		chatstr[len+1] = '\0';
	} else if (readpoll == -1) {
		printf("Error");
	}
	return eof;
}

bool process_polls (struct polls *mypolls, int num_readable) {
	static char buff[1024];
	static int len = 0;
	bool eof = false;
	for (int i = 0; i < mypolls->polls_index; i++) {
		if ( !(mypolls->polls[i].revents & POLLIN) ) {
			continue;
		}
		num_readable--;
		if (i == 0) {
			eof = accum_chat(buff, len);
			if (buff[len] == '\n') {
				process_str(buff);
				len = 0;
			} else {
				len++;
			}
		} else if (i == 1) {
			read_presence(mypolls->polls[i].fd);
		} else if (i == 2) {
			int chatfd = accept(mypolls->polls[i].fd, NULL, NULL);
			init_poll(mypolls->polls, mypolls->polls_index++, chatfd);
		} else if (mypolls->polls[i].fd > 2) {
			read_tcp(mypolls->polls[i].fd);
		} 
		if (num_readable == 0) {
			break;
		}
	}
	return eof;
}

int main (int argc, char *argv[]) {
	if (argc < 3) {
		printf("invalid argc \n");
		exit(-1);
	}
	int counter = 0;
	struct polls mypolls;
	setup_polls(&mypolls);
	int sfd = initialize_broadcast();
	int tcp = initialize_tcp(argv[2]);
	init_poll(mypolls.polls, mypolls.polls_index++, sfd);
	init_poll(mypolls.polls, mypolls.polls_index++, tcp);
	bool eof = false;
	while (!eof) {
		int num_readable = poll(mypolls.polls, mypolls.polls_index, TIMEOUT);
		if (num_readable == -1) {
			perror("Poil Failed");
			exit(-1);
		} else {
			eof = process_polls(&mypolls, num_readable);
		}
		if (counter == 30) {
			write_presence(sfd, argv[1], "online", argv[2]);
			counter = 0;
		}
		counter++; 
	}
	write_presence(sfd, argv[1], "offline", argv[2]);

	return(0);
}
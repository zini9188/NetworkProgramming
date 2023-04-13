/*
���ϸ� : wecho_server.c
��  �� : echo ���񽺸� �����ϴ� ����
������ : cc -o wecho_server wecho_server.c
���� : wecho_server [port]
*/
#define _CRT_SECURE_NO_WARNINGS
#include <winsock.h>
#include <signal.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <ctype.h>

WSADATA wsadata;
int	main_socket;

void exit_callback(int sig)
{
	closesocket(main_socket);
	WSACleanup();
	exit(0);
}

void init_winsock()
{
	WORD sversion;
	u_long iMode = 1;

	// winsock ����� ���� �ʼ�����
	signal(SIGINT, exit_callback);
	sversion = MAKEWORD(1, 1);
	WSAStartup(sversion, &wsadata);
}

#define BUF_LEN 128
#define ECHO_SERVER "0.0.0.0"
#define ECHO_PORT "30000"

int main(int argc, char* argv[]) {
	struct sockaddr_in server_addr, client_addr;
	int server_fd, client_fd;			/* ���Ϲ�ȣ */
	int len, msg_size;
	char buf[BUF_LEN + 1];
	char wc[BUF_LEN + 1];
	char id[BUF_LEN + 1] = { 0 };
	char pw[BUF_LEN + 1] = { 0 };
	char welcome[] = "Welcome to Server!!";
	unsigned int set = 1;
	char* ip_addr = ECHO_SERVER, * port_no = ECHO_PORT;

	if (argc == 2) {
		port_no = argv[1];
	}

	init_winsock();

	/* ���� ���� */
	if ((server_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Server: Can't open stream socket.");
		exit(0);
	}
	main_socket = server_fd;

	printf("echo_server waiting connection..\n");
	printf("server_fd = %d\n", server_fd);
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&set, sizeof(set));

	/* server_addr�� '\0'���� �ʱ�ȭ */
	memset((char*)&server_addr, 0, sizeof(server_addr));
	/* server_addr ���� */
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(atoi(port_no));

	/* bind() ȣ�� */
	if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		printf("Server: Can't bind local address.\n");
		exit(0);
	}

	/* ������ ���� ������ ���� */
	listen(server_fd, 5);

	/* iterative  echo ���� ���� */
	printf("Server : waiting connection request.\n");
	len = sizeof(client_addr);

	while (1) {
		/* �����û�� ��ٸ� */
		client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &len);
		if (client_fd < 0) {
			printf("Server: accept failed.\n");
			exit(0);
		}
		printf("Client connected from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
		printf("client_fd = %d\n", client_fd);


		// wc�迭�� welcome ���ڿ��� ������.
		sprintf(wc, "%s\n", welcome);
		// wc �迭�� �۽�
		msg_size = send(client_fd, wc, BUF_LEN, 0);
		if (msg_size <= 0) {
			printf("send error\n");
			break;
		}
		printf("Sending len = %d : %s\n", msg_size, wc);

		//id�� ����� �޾ƿ��� �������� ���� �ƴϸ� �ݺ�
		while (1) {
			int cnt = 0;
			int cnt2 = 0;
			int length = 0;
			memset(buf, 0,128);
			//�޼����� �޾ƿ�.
			msg_size = recv(client_fd, buf, BUF_LEN, 0);
			if (msg_size <= 0) {
				printf("recv error\n");
				break;
			}
			// �޾ƿ� ���ۿ� �����̽��� ����
			for (int i = 0; i < msg_size; i++) {
				if (isspace(buf[i])) {
					cnt = i;
					break;
				}
			}
			for (int i = 0; i < msg_size; i++) {
				if (isspace(buf[i])) {
					cnt2 = i;
				}
			}

			//id�� ������ ù �����̽����� ����
			for (int i = 0; i < cnt; i++) {
				id[i] = buf[i];
			}
			id[cnt] = '\0';

			while (1) {
				if ((length + cnt + 2) == cnt2)
					break;
				pw[length] = buf[length + cnt + 1];
				length++;
			}
			pw[cnt2] = '\0';
			length = 0;
			printf("Received id = %s pass = %s\n", id, pw);

			if (strcmp(id, "Hansung") != 0) {
				char id_error[] = "401 Invalid ID";
				msg_size = send(client_fd, id_error, BUF_LEN, 0);
				printf("Sending len = %d : %s\n", msg_size, id_error);
				if (msg_size <= 0) {
					printf("send error\n");
					break;
				}
			}
			else if (strcmp(pw, "computer\0") != 0) {
				char id_error[] = "402 Invalid Password";
				msg_size = send(client_fd, id_error, BUF_LEN, 0);
				memset(client_fd, 128, buf);
			
				printf("Sending len = %d : %s\n", msg_size, id_error);
				if (msg_size <= 0) {
					printf("send error\n");
					break;
				}
			}
			if (strcmp(pw, "computer\0") == 0 && strcmp(id, "Hansung") == 0) {
				char wc[] = "Welcome hansung!!";
				msg_size = send(client_fd, wc, BUF_LEN, 0);
				printf("Sending len = %d : 200 %s\n", msg_size, wc);
				if (msg_size <= 0) {
					printf("send error\n");
					break;
				}
				break;
			}
		}


		while (1) {
			char* s = buf;
			msg_size = recv(client_fd, buf, BUF_LEN, 0);
			if (msg_size <= 0) {
				printf("recv error\n");
				break;
			}

			//�޾ƿ� menu�� ������
			char num[2];
			num[0] = buf[0];
			num[1] = '\0';
			buf[msg_size] = '\0'; // ���ڿ� ���� NULL�� �߰��ϱ� ����

			if (num[0] == '4') {
				printf("Session finished for %s", id);
				break;
			}

			char temp[128];
			int length = 0;

			while (1) {
				if (length == strlen(buf) - 4)
					break;
				temp[length] = buf[length + 2];
				length++;
			}
			temp[length] = '\0';
			sprintf(buf, " % s\n", temp);

			printf("Received len=%d : %s%s", msg_size, num, buf);

			if (num[0] == '1') {
				//�빮�� ��ȯ
				while (*s) {
					*s = toupper(*s);
					s++;
				}
			}

			if (num[0] == '2') {
				//�ҹ��� ��ȯ
				while (*s) {
					*s = tolower(*s);
					s++;
				}
			}

			if (num[0] == '3') {
				//��->�� ��->�� �� �ٲ�
				while (*s) {
					if (islower(*s))
						*s = toupper(*s);
					else
						*s = tolower(*s);
					s++;
				}
			}

			msg_size = send(client_fd, buf, msg_size, 0);
			if (msg_size <= 0) {
				printf("send error\n");
				break;
			}
			printf("Sending len=%d : %s", msg_size, buf);
		}
		closesocket(client_fd); // close(client_fd);

	}
	closesocket(server_fd); // close(client_fd);
	return(0);

}
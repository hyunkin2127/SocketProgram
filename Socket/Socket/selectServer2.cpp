#include "main.h"


#define MAXBUF 1024 
#define MAX 100
#define ALL 1
#define USER 20


char greet[] = "���� �Ǿ����ϴ�";
char no_greet[] = "���� ���� �ʾҽ��ϴ�";


//�޼��� ���� �Լ�
void writeMessage(void * client_message, void * num, int basefd, int maxfd);


//Ŭ���̾�Ʈ�� ���� �޼����� ���۹��� Ŭ���̾�Ʈ�� �̸��� ����
struct message {
	char user[USER];
	char sbuf[MAXBUF];
};



//������ ������ Ŭ���̾�Ʈ�� ��ũ���Ϳ� �ű⿡ ��ġ�Ǵ� �ش� Ŭ���̾�Ʈ�� �̸��� ����

struct add_num {
	int anum;
	char name[MAXBUF];
};


int main() {

	int optval = 1;                        //���Ͽɼ��� ������               
	int ssock, csock;                       //����
	struct sockaddr_in server_addr, client_addr;         //IP�� Port��(�� �ּҰ�)
	int clen, data_len;
	fd_set read_fds, tmp_fds;               //��ũ���� ��Ʈ(���� ��Ʈ ���̺�)
	int fd;
	struct add_num add_num[USER];          //�����������ϴ� Ŭ���̾�Ʈ�� ������ �����ϴ� ��ü
	int index, maxfd;
	struct message read_message;           //Ŭ���̾�Ʈ�� ���� ���� �޼��� ����ü
	

	 /*
	 * Ŭ���̾�Ʈ�� ������ ������ ����
	 * ������, Ÿ��, ���������� ���ڷ� ����.
	 */
	if ((ssock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))<0) {
		perror("socket error : ");
		exit(1);
	}



	//���� �ɼ��� ����(�ɼ� �ؼ��� ���� Ŀ�� �� �ý��� �ڵ��� ����, �ɼ��̸�, �ɼ��� �� ��--SO_SNDBUF,SO_BROADCAST,SO_KEEPALIVE) 
	setsockopt(ssock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	//�ش� ������ �ʱ�ȭ, �ּҸ� ����
	memset(add_num, 0, sizeof(add_num));
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(3333);


	//������ �ش� �ּҷ� ����
	if (bind(ssock, (struct sockaddr *)&server_addr, sizeof(server_addr))<0)
	{
		perror("bind error: ");
		exit(1);
	}
	

	//Ŭ���̾�Ʈ�� ������ ��ٸ�~
	if (listen(ssock, 5)<0) {
		perror("listen error : ");
		exit(1);
	}

	clen = sizeof(client_addr);

	//FD_SET ��ũ������ ����
	FD_ZERO(&read_fds);
	FD_SET(ssock, &read_fds);

	maxfd = ssock;


	while (1) {
		//fd_set��ũ���� ���̺��� ��ȸ��. �׷��� ������ �ش簪�� �̸� �Ű� ���� �����ؾ� �Ѵ�. �׷��� ������ ���縦 ���� �ϰ� �����ؾ� �Ѵ�. 
		tmp_fds = read_fds;


		//�������̽� �󿡼� ����̽��� ���� �Է¿� ���� �ﰢ���� ������ �ʿ�.

		if (select(maxfd + 1, &tmp_fds, 0, 0, (struct timeval *)0)<1) {
			perror("select error : ");
			exit(1);
		}


		for (fd = 0; fd<maxfd + 1; fd++) {
			if (FD_ISSET(fd, &tmp_fds)) {
				if (fd == ssock) {
					if ((csock = accept(ssock, (struct sockaddr *)&client_addr, &clen))<0) {
						perror("accept error : ");
						exit(0);
					}

					FD_SET(csock, &read_fds);
					printf("���ο� Ŭ���̾�Ʈ %d�� ���� ��ũ���� ����\n", csock);

					for (index = 0; index<MAX; index++) {
						if (add_num[index].anum == 0) {
							add_num[index].anum = csock;
							maxfd++;
							break;
						}
					}
										
					if (csock>maxfd) {
						maxfd = csock;
					}

									}
				else {
					memset(&read_message, 0, sizeof(read_message));

					//Ŭ���̾�Ʈ�� ���� �޼����� ���Ź޴´�.
					data_len = read(fd, (struct message*)&read_message, sizeof(read_message));

					//Ŭ���̾�Ʈ�κ��� �޽����� ���Դٸ� �޽��� ����
					if (data_len>0) {
						writeMessage((void*)&read_message, (void*)add_num, fd, maxfd);
					}
					else if (data_len == 0) {

						for (index = 0; index<USER; index++) {
							if (add_num[index].anum == fd) {
								add_num[index].anum = 0;
								strcpy(add_num[index].name, "");
								break;
							}
						}

						close(fd);
						FD_CLR(fd, &read_fds);
						if (maxfd == fd)
							maxfd--;

						printf("Ŭ���̾�Ʈ %d�� ���� ��ũ���� ����\n", fd);
					}
					else if (data_len<0) {
						perror("read error : ");
						exit(1);
					}

				}

			}

		}

	}

	return 0;

}



//����ڷκ��� ���� �޽����� �ش� Ŭ���̾�Ʈ�� ������ �ִ� �Լ�

void writeMessage(void *client_message, void *num, int basefd, int maxfd) {

	int index;

	struct message *cl_message;

	struct add_num *index_num;

	char all[] = "ALL";



	cl_message = (struct message*)client_message;

	index_num = (struct add_num*)num;



	//Ŭ���̾�Ʈ�� �����ߴٰ� �޽����� ��������	

	if (strcmp(cl_message->sbuf, "") == 0) {

		printf("��� �ϰڽ��ϴ�.\n");

		for (index = 0; index<USER; index++) {

			if (((index_num + index)->anum) == basefd) {

				strcpy((index_num + index)->name, (cl_message->user));

			}

		}

		write(basefd, greet, sizeof(greet));

		//Ŭ���̾�Ʈ�� �ٸ� Ŭ���̾�Ʈ�� �޽����� ���� �� ��

	}
	else {

		//��� ����ڿ��� �޽����� �����Ѵ�.

		all[strlen(all)] = '\0';

		if (strcmp(cl_message->user, all) == 0) {

			for (index = 0; index<maxfd; index++) {

				if ((index_num + index)->anum != 0)

					write(((index_num + index)->anum), cl_message->sbuf, MAXBUF);

			}

			//������ ����ڿ��� �޽����� �����Ѵ�.

		}
		else {



			for (index = 0; index<USER; index++) {

				if (strcmp(((index_num + index)->name), cl_message->user) == 0) {

					if (write(((index_num + index)->anum), cl_message->sbuf, MAXBUF)<0) {

						perror("write error : ");

						exit(1);

					}

					break;

				}

				//������ �������� �ʴ´ٸ�

				if (index + 1 == USER)

					write(basefd, no_greet, sizeof(no_greet));

			}

		}

	}

}
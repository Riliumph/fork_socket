#include <arpa/inet.h>
#include <errno.h>
#include <iostream>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int
main(int argc, char** argv)
{
  std::cout << "[sub] start process" << std::endl;
  auto port_no = std::stoi(std::string(argv[1]));
  std::cout << "[sub] port_no" << port_no << std::endl;
  // IPv4 TCP のソケットを作成する
  int sd = socket(AF_INET, SOCK_STREAM, 0);
  if (sd < 0) {
    perror("socket");
    return -1;
  }

  // 送信先アドレスとポート番号を設定する
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port_no);
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  // サーバ接続（TCP の場合は、接続を確立する必要がある）
  auto r = connect(sd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in));
  if (r == -1) {
    perror("[sub] connect:");
    close(sd);

    return -1;
  }

  // パケットを TCP で送信
  if (send(sd, "I am send process", 17, 0) < 0) {
    perror("[sub] send");
    close(sd);
    return -1;
  }

  close(sd);

  return 0;
}

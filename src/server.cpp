#include <cstring>
#include <errno.h>
#include <iostream>
#include <netdb.h>      /* getnameinfo() */
#include <netinet/in.h> /* struct sockaddr_in */
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/socket.h> /* socket() */
#include <sys/time.h>   /* select() */
#include <sys/types.h>  /* socket(), time(), select() */
#include <unistd.h>     /* select() */

void
echo_reply_select(int);

int
tcp_acc_port(int);

ssize_t
readline(int fd, std::string&);

int
create(int port_no);

void
echo_reply_select(int portno)
{
  fd_set readfds_save;

  auto acc = tcp_acc_port(portno);
  if (acc < 0) {
    exit(-1);
  }
  std::string t = "a\r\n";
  std::cout << "t(" << t.size() << "): " << t << std::endl;
  FD_ZERO(&readfds_save);
  FD_SET(acc, &readfds_save);
  while (1) {
    std::cout << "[main] waiting..." << std::endl;
    fd_set readfds = readfds_save;
    int n = select(FD_SETSIZE, &readfds, 0, 0, 0);
    if (n <= 0) {
      perror("select");
      exit(1);
    }
    if (FD_ISSET(acc, &readfds)) {
      std::cout << "[main] new comer!!" << std::endl;
      FD_CLR(acc, &readfds);
      auto com = accept(acc, 0, 0);
      if (com < 0) {
        perror("[main] accept");
        exit(-1);
      }
      FD_SET(com, &readfds_save);
    }
    for (int i = 0; i < FD_SETSIZE; i++) {
      if (FD_ISSET(i, &readfds)) {
        bool is_fin = false;
        std::string buf;
        buf.resize(20);
        auto recv_size = read(i, buf.data(), buf.length());
        buf.shrink_to_fit();
        if (recv_size == 0) {
          close(i);
          FD_CLR(i, &readfds_save);
          break;
        }
        // std::cout << "size(" << recv_size << "): " << buf << std::endl;
        // std::cout << "compare: " << buf.compare("create\r\n") << std::endl;
        // std::cout << "strcmp:  " << strcmp("create\r\n", buf.data())
        //           << std::endl;
        if (strcmp("create\r\n", buf.data()) == 0) {
          create(portno);
        } else {
          std::cout << "[main] new msg: size(" << recv_size << "): " << buf
                    << std::endl;
        }
      }
    }
  }
}

int
tcp_acc_port(int portno)
{
  struct sockaddr_in addr;
  auto s = socket(PF_INET, SOCK_STREAM, 0);
  if (s < 0) {
    perror("[main] socket");
    return (-1);
  }

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(portno);

  int yes = 1;
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(yes));

  if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    perror("[main] bind");
    fprintf(
      stderr,
      "[main] port number %d is already used. wait a moment or kill another "
      "program.\n",
      portno);
    return (-1);
  }
  if (listen(s, 5) < 0) {
    perror("[main] listen");
    close(s);
    return (-1);
  }
  return (s);
}

ssize_t
readline(int fd, std::string& buffer)
{
  std::size_t whole_byte = 0;
  auto byte = read(fd, buffer.data(), buffer.length());
  buffer.shrink_to_fit();
  whole_byte += byte;
  return whole_byte;
}

int
create(int port_no)
{
  std::cout << "[main] forking..." << std::endl;
  auto pid = fork();
  if (pid == -1) {
    perror("[main] fork");
    return -1;
  } else if (pid == 0) {
    // 子プロセス
    // 子プロセスはfdの状態も引き継いでしまう。
    // 子プロセス側の処理でfdをすべて閉じる
    static const int start_fd = 3;
    for (auto i = start_fd; i < FD_SETSIZE; ++i) {
      close(i);
    }
    char port[10];
    snprintf(port, sizeof(port), "%d", port_no);
    auto argc = 0;
    char* argv[1024];
    argv[argc++] = "./bin/sub";
    argv[argc++] = port;
    argv[argc++] = NULL; // NULL終端
    std::cout << "[main] execve..." << std::endl;
    execve(argv[0], argv, NULL);
    // 以降、到達の場合はエラー
    printf("error: %s", strerror(errno));
    _exit(-1);
  }
  // End Block
  std::cout << "[main] return" << std::endl;
  return 0;
}

int
main()
{
  int portno = 50000;
  echo_reply_select(portno);
}

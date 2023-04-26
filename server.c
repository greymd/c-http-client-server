#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main() {
  int server_fd, client_fd;
  struct sockaddr_in server_addr, client_addr;
  socklen_t addr_len;
  char buffer[1024];

  // HTTP response header and body
  const char *response_header =
      "HTTP/1.1 200 OK\r\n"
      "Content-Type: text/plain\r\n"
      "Content-Length: 5\r\n"
      "Connection: close\r\n\r\n";
  const char *response_body = "hello";

  // サーバのソケットを作成
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  // 8000 番ポートで待ち受けるように設定
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(8000);
  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
    perror("bind");
    exit(EXIT_FAILURE);
  }

  // Listen
  if (listen(server_fd, 10) == -1) {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  printf("Server listening on port 8000...\n");

  while (1) {
    // 受信待ち
    printf("accepting...\n");
    addr_len = sizeof(client_addr);
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
    if (client_fd == -1) {
      perror("accept");
      continue;
    }

    // 受信した内容を buffer に格納
    printf("memset\n");
    memset(buffer, 0, sizeof(buffer));
    read(client_fd, buffer, sizeof(buffer) - 1);
    printf("Received: %s", buffer);

    // HTTP レスポンスを送信
    printf("Reply response\n");
    write(client_fd, response_header, strlen(response_header));
    write(client_fd, response_body, strlen(response_body));

    // クライアントのソケットを閉じる
    printf("Close client socket\n");
    close(client_fd);
  }

  // サーバのソケットを閉じる
  printf("Close server socket\n");
  close(server_fd);

  return 0;
}

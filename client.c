/*
 * SO_REUSEPORT テスト
 *
 * 動作方法
 * 端末1:
 *   gcc -o hc hc.c
 *   ./hc
 * 端末2:
 *   python3 -m http.server 8000
 *
 */
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <unistd.h>

int main() {
    const char *server_ip = "172.31.8.71";
    int server_port = 8000;
    const char *http_request = "GET / HTTP/1.1\r\nHost: 172.31.8.71:8000\r\nConnection: close\r\n\r\n";
    char buffer[4096];

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    // SO_REUSEPORT を設定する
    // int reuse = 1;
    // if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0) {
    //     perror("setsockopt");
    //     close(sockfd);
    //     return 1;
    // }

    // 明示的にエフェメラルポートを指定する
    struct sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(12345); // ポート番号を指定
    local_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sockfd, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
        perror("bind");
        close(sockfd);
        return 1;
    }

    // サーバーのアドレスを設定する
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sockfd);
        return 1;
    }

    // サーバーに接続する
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sockfd);
        return 1;
    }

    // エフェメラルポートを取得して表示する
    socklen_t local_addr_len = sizeof(local_addr);
    if (getsockname(sockfd, (struct sockaddr *)&local_addr, &local_addr_len) < 0) {
        perror("getsockname");
        close(sockfd);
        return 1;
    }
    int ephemeral_port = ntohs(local_addr.sin_port);
    fprintf(stderr, "Ephemeral port: %d\n", ephemeral_port);

    // リクエストヘッダを送信
    if (send(sockfd, http_request, strlen(http_request), 0) < 0) {
        perror("send");
        close(sockfd);
        return 1;
    }

    // recv でソケットからデータを受信し、標準出力に出力する
    ssize_t n;
    while ((n = recv(sockfd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[n] = '\0';
        printf("%s", buffer);
    }

    if (n < 0) {
        perror("recv");
    }

    // sleep することで FIN を送信する前にソケットを閉じる（この間に端末2を終了させる）
    sleep(30);

    // ソケットを閉じる (ここで FIN が送信されるはず)
    close(sockfd);
    return 0;
}

// MT25064_client.c
// MT25064
#include <stdio.h> // printf , fprintf
#include <stdlib.h> // malloc, 
#include <unistd.h> // close
#include <arpa/inet.h> // inet_pton
#include <sys/socket.h> // socket, connect
#include <time.h> // time
#include <string.h> // memset

int main(int argc, char *argv[]) {
    if (argc < 5) {
        fprintf(stderr, "Usage: %s <ip> <port> <msg_size> <duration_sec>\n", argv[0]);
        return 1;
    }
    
    char *ip = argv[1];
    int port = atoi(argv[2]);
    size_t msg_size = atoi(argv[3]);
    int duration = atoi(argv[4]);
    

    // create socket 
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        perror("socket");
        return 1;
    }
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    
    // convert IP address to binary 
    if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0) {
        perror("inet_pton");
        return 1;
    }
    
    // send connect request ---> initiates 3-way TCP handshake 
    if (connect(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        return 1;
    }
    
    // receiver buffer 
    char *buf = malloc(msg_size);
    if (!buf) {
        perror("malloc");
        return 1;
    }
    
    size_t total_bytes = 0;
    time_t start = time(NULL);
    
    // keep looping until elapsed time < duration
    while (time(NULL) - start < duration) {
        // receive data from connected socket 
        ssize_t n = recv(s, buf, msg_size, 0);
        if (n <= 0) break; // connection closed / timeout 
        total_bytes += n;
    }
    
    // Print ONLY the number to stdout
    printf("%zu\n", total_bytes);
    
    // cleanup
    free(buf);
    close(s);
    return 0;
}
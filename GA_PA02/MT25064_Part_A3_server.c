// MT25064_Part_A3_server.c
// MT25064
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include <linux/errqueue.h>
#include <poll.h>


// common message structure with 8 fields
typedef struct {
    char *field1;
    char *field2;
    char *field3;
    char *field4;
    char *field5;
    char *field6;
    char *field7;
    char *field8;
} message_t;

// common client information for each thread
typedef struct {
    int sock;
    size_t msg_size;
} client_arg_t;



message_t* create_message_aligned(size_t field_size) {
    message_t *msg = malloc(sizeof(message_t));
    if (!msg) return NULL;
    
    // Page-aligned allocations for zero-copy
    if (posix_memalign((void**)&msg->field1, 4096, field_size) != 0) goto err;
    if (posix_memalign((void**)&msg->field2, 4096, field_size) != 0) goto err;
    if (posix_memalign((void**)&msg->field3, 4096, field_size) != 0) goto err;
    if (posix_memalign((void**)&msg->field4, 4096, field_size) != 0) goto err;
    if (posix_memalign((void**)&msg->field5, 4096, field_size) != 0) goto err;
    if (posix_memalign((void**)&msg->field6, 4096, field_size) != 0) goto err;
    if (posix_memalign((void**)&msg->field7, 4096, field_size) != 0) goto err;
    if (posix_memalign((void**)&msg->field8, 4096, field_size) != 0) goto err;
    
    memset(msg->field1, 'A', field_size);
    memset(msg->field2, 'B', field_size);
    memset(msg->field3, 'C', field_size);
    memset(msg->field4, 'D', field_size);
    memset(msg->field5, 'E', field_size);
    memset(msg->field6, 'F', field_size);
    memset(msg->field7, 'G', field_size);
    memset(msg->field8, 'H', field_size);
    
    return msg;
    
err:
    free(msg->field1); free(msg->field2); free(msg->field3); free(msg->field4);
    free(msg->field5); free(msg->field6); free(msg->field7); free(msg->field8);
    free(msg);
    return NULL;
}


void free_message(message_t *msg) {
    if (!msg) return;
    free(msg->field1);
    free(msg->field2);
    free(msg->field3);
    free(msg->field4);
    free(msg->field5);
    free(msg->field6);
    free(msg->field7);
    free(msg->field8);
    free(msg);
}

void *client_thread(void *arg) {
    client_arg_t *c = (client_arg_t*)arg;
    size_t field_size = c->msg_size / 8;
    
    // Enable zero-copy on this socket
    int val = 1;
    if (setsockopt(c->sock, SOL_SOCKET, SO_ZEROCOPY, &val, sizeof(val)) < 0) {
        perror("setsockopt SO_ZEROCOPY");
    }
    
    message_t *msg = create_message_aligned(field_size);
    if (!msg) {
        close(c->sock);
        free(c);
        return NULL;
    }
    
    // Zero-copy implementation using sendmsg() with MSG_ZEROCOPY
    struct iovec iov[8];
    iov[0].iov_base = msg->field1;
    iov[0].iov_len = field_size;
    iov[1].iov_base = msg->field2;
    iov[1].iov_len = field_size;
    iov[2].iov_base = msg->field3;
    iov[2].iov_len = field_size;
    iov[3].iov_base = msg->field4;
    iov[3].iov_len = field_size;
    iov[4].iov_base = msg->field5;
    iov[4].iov_len = field_size;
    iov[5].iov_base = msg->field6;
    iov[5].iov_len = field_size;
    iov[6].iov_base = msg->field7;
    iov[6].iov_len = field_size;
    iov[7].iov_base = msg->field8;
    iov[7].iov_len = field_size;
    
    struct msghdr msghdr = {0};
    msghdr.msg_iov = iov;
    msghdr.msg_iovlen = 8;
    
    while (1) {
        if (sendmsg(c->sock, &msghdr, MSG_ZEROCOPY) <= 0) break;
    }
    
    close(c->sock);
    free_message(msg);
    free(c);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <port> <msg_size> <threads>\n", argv[0]);
        return 1;
    }
    
    int port = atoi(argv[1]);
    size_t msg_size = atoi(argv[2]);
    int max_threads = atoi(argv[3]);

    // create socket
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        perror("socket");
        return 1;
    }
    
    int opt = 1;

    // set SO_REUSEADDR
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    
    // bind to port 
    if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }
    
    // listen 
    if (listen(s, max_threads) < 0) {
        perror("listen");
        return 1;
    }
    
    fprintf(stderr, "A3 Server listening on port %d (zero-copy with MSG_ZEROCOPY)\n", port);
    
    int client_count = 0;
    // accept max_threads clients
    while (client_count < max_threads) {

        
        // accept incoming client 
        int client_sock = accept(s, NULL, NULL);
        if (client_sock < 0) {
            perror("accept");
            continue;
        }
        
        client_arg_t *arg = malloc(sizeof(client_arg_t));
        arg->sock = client_sock;
        arg->msg_size = msg_size;
        
        // create thread for this client 
        pthread_t tid;
        pthread_create(&tid, NULL, client_thread, arg);
        pthread_detach(tid);
        
        client_count++;
    }
    
    // Keep server running
    while (1) {
        sleep(1);
    }
    
    close(s);
    return 0;
}
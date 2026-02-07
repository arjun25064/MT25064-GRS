// MT25064_Part_A2_server.c
// MT25064
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

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


// dynamically create message 
message_t* create_message(size_t field_size) {
    message_t *msg = malloc(sizeof(message_t));
    if (!msg) return NULL;

    // dynamically allocate 'field_size' bytes of memory for all the 8 fields 
    msg->field1 = malloc(field_size);
    msg->field2 = malloc(field_size);
    msg->field3 = malloc(field_size);
    msg->field4 = malloc(field_size);
    msg->field5 = malloc(field_size);
    msg->field6 = malloc(field_size);
    msg->field7 = malloc(field_size);
    msg->field8 = malloc(field_size);
    
    // check if all fields allocated successfully 
    if (!msg->field1 || !msg->field2 || !msg->field3 || !msg->field4 ||
        !msg->field5 || !msg->field6 || !msg->field7 || !msg->field8) {
        free(msg->field1); free(msg->field2); free(msg->field3); free(msg->field4);
        free(msg->field5); free(msg->field6); free(msg->field7); free(msg->field8);
        free(msg);
        return NULL;
    }
    
    // each field with 'field_size' bytes of character
    memset(msg->field1, 'A', field_size); 
    memset(msg->field2, 'B', field_size);
    memset(msg->field3, 'C', field_size);
    memset(msg->field4, 'D', field_size);
    memset(msg->field5, 'E', field_size);
    memset(msg->field6, 'F', field_size);
    memset(msg->field7, 'G', field_size);
    memset(msg->field8, 'H', field_size);
    
    return msg;
}

// function to free up dynamically allocated memory for message buffer 
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

// worker thread function for each connected client 
void *client_thread(void *arg) {
    client_arg_t *c = (client_arg_t*)arg;
    size_t field_size = c->msg_size / 8;
    
    // creates message packet 
    message_t *msg = create_message(field_size);
    if (!msg) {
        close(c->sock);
        free(c);
        return NULL;
    }
    
    // *** One-copy implementation using sendmsg() with iovec ***
    struct iovec iov[8]; // vectorized i/o 
    
    iov[0].iov_base = msg->field1; // this tells starting address of field1 , say 0x1000
    iov[0].iov_len = field_size; // this tells the length/bytes to be read from this address, say 8
    
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
    
    // this stores the above list 
    struct msghdr msghdr = {0};
    msghdr.msg_iov = iov; 
    msghdr.msg_iovlen = 8; // 8 things in my list
    
    while (1) {
        // single systemcall sends all data to NIC 
        if (sendmsg(c->sock, &msghdr, 0) <= 0) break;
    }
    
    // cleanup 
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
    
    // set SO_REUSEADDR
    int opt = 1;
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
    
    fprintf(stderr, "A2 Server listening on port %d (one-copy with sendmsg)\n", port);
    
    int client_count = 0;
    // accept max_threads clients 
    while (client_count < max_threads) {
        int client_sock = accept(s, NULL, NULL);
        if (client_sock < 0) {
            perror("accept");
            continue;
        }
        
        // each client gets it own thread 
        client_arg_t *arg = malloc(sizeof(client_arg_t));
        arg->sock = client_sock;
        arg->msg_size = msg_size;
        
        // create thread for this client 
        pthread_t tid;
        pthread_create(&tid, NULL, client_thread, arg);
        pthread_detach(tid); // main thread does not wait to finish ==> moves to service next client 
        
        client_count++; 
    }
    
    // Keep server running
    while (1) {
        sleep(1);
    }
    
    close(s);
    return 0;
}
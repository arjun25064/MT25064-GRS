// MT25064_Part_A1_server.c
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


// dynamically creates message 
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
    memset(msg->field1, 'A', field_size); // A A A .... 
    memset(msg->field2, 'B', field_size); // B B B .... 
    memset(msg->field3, 'C', field_size); // and so on ...
    memset(msg->field4, 'D', field_size);
    memset(msg->field5, 'E', field_size);
    memset(msg->field6, 'F', field_size);
    memset(msg->field7, 'G', field_size);
    memset(msg->field8, 'H', field_size); // all field_size bytes/characters 
    
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
    
    // **** Two-copy implementation using send() ****

    while (1) {
        // send each field separately (demonstrating multiple send calls)
        // copies data from user space buffer ---> kernel socket buffer ---> NIC
        // reads and copies each field separately (located at 8 different locations)
        // cpu wastage
        if (send(c->sock, msg->field1, field_size, 0) <= 0) break;
        if (send(c->sock, msg->field2, field_size, 0) <= 0) break;
        if (send(c->sock, msg->field3, field_size, 0) <= 0) break;
        if (send(c->sock, msg->field4, field_size, 0) <= 0) break;
        if (send(c->sock, msg->field5, field_size, 0) <= 0) break;
        if (send(c->sock, msg->field6, field_size, 0) <= 0) break;
        if (send(c->sock, msg->field7, field_size, 0) <= 0) break;
        if (send(c->sock, msg->field8, field_size, 0) <= 0) break;
    }
    // cleanup thread
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
    
    fprintf(stderr, "A1 Server listening on port %d (two-copy)\n", port);
    
    // accept max_threads clients
    for (int i = 0; i < max_threads; i++) {

        // accept connection request
        int client_sock = accept(s, NULL, NULL);


        if (client_sock < 0) {
            perror("accept");
            i--;
            continue;
        }

        // each thread gets its own block of information 
        client_arg_t *arg = malloc(sizeof(client_arg_t));
        arg->sock = client_sock;
        arg->msg_size = msg_size;
        

        // create thread for this client and start executing thread worker 
        pthread_t tid;
        pthread_create(&tid, NULL, client_thread, arg);
        pthread_detach(tid);  // main thread does not wait to finish ==> moves to service next client 
        
        fprintf(stderr, "Client %d/%d connected\n", i+1, max_threads);
    }
    
    fprintf(stderr, "All %d clients connected, running...\n", max_threads);
    while (1) {
        sleep(10);
    }
    
    close(s);
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include "socket.h"

#ifndef PORT
    #define PORT 52359
#endif

#define LISTEN_SIZE 5
#define WELCOME_MSG "Welcome to CSC209 Twitter! Enter your username: \r\n"
#define SEND_MSG "send"
#define SHOW_MSG "show"
#define FOLLOW_MSG "follow"
#define UNFOLLOW_MSG "unfollow"
#define QUIT_MSG "quit"
#define USERNAME_MSG "Please enter your username: \r\n"
#define INVAL_USERNAME_MSG "You have entered an invalid username, please enter a new one: \r\n"
#define FOLLOWING_LIMIT_MSG "You have already reached your following limit, so you can not follow this user.\r\n"
#define NO_CLIENT_MSG "No active client with this username exists.\r\n"
#define INVAL_COMMAND_MSG "You have entered an invalid command.\r\n"
#define USERNAME_TAKEN_MSG "This username is taken. Please enter a new one: \r\n"
#define NO_MORE_MSGS "You have reached your message limit.\r\n"
#define NO_MORE_FOLLOWERS "This user cannot have any more followers.\r\n"
#define CANNOT_FOLLOW "You cannot follow this user.\r\n"
#define BUF_SIZE 256
#define MSG_LIMIT 8
#define FOLLOW_LIMIT 5

struct client {
    int fd;
    struct in_addr ipaddr;
    char username[BUF_SIZE];
    char message[MSG_LIMIT][BUF_SIZE];
    struct client *following[FOLLOW_LIMIT]; // Clients this user is following
    struct client *followers[FOLLOW_LIMIT]; // Clients who follow this user
    char inbuf[BUF_SIZE]; // Used to hold input from the client
    char *in_ptr; // A pointer into inbuf to help with partial reads
    struct client *next;
};

// The set of socket descriptors for select to monitor.
// This is a global variable because we need to remove socket descriptors
// from allset when a write to a socket fails. 
fd_set allset;

// Provided functions. 
//void add_client(struct client **clients, int fd, struct in_addr addr);
//void remove_client(struct client **clients, int fd);

// These are some of the function prototypes that we used in our solution 
// You are not required to write functions that match these prototypes, but
// you may find them helpful when thinking about operations in your program.

/* 
 * Create a new client, initialize it, and add it to the head of the linked
 * list.
 */
void add_client(struct client **clients, int fd, struct in_addr addr) {

    struct client *p = malloc(sizeof(struct client));
    if (!p) {
        perror("malloc");
        exit(1);
    }

    printf("Adding client %s\n", inet_ntoa(addr));
    p->fd = fd;
    p->ipaddr = addr;
    p->username[100] = '\0';
    p->in_ptr = p->inbuf;
    p->inbuf[0] = '\0';
    p->next = *clients;

    // initialize messages to empty strings
    for (int i = 0; i < MSG_LIMIT; i++) {
        p->message[i][0] = '\0';
    }

    // initialize followees and followers to NULL
    for (int i = 0; i < FOLLOW_LIMIT; i++) {
        p->following[i] = NULL;
        p->followers[i] = NULL;
    }

    *clients = p; // this adds it to the head of the linked list!
}

/* 
 * Remove client from the linked list and close its socket.
 * Also, remove socket descriptor from allset.
 */
void remove_client(struct client **clients, int fd) {

    struct client **p;
    for (p = clients; *p && (*p)->fd != fd; p = &(*p)->next)
        ;

    // Now, p points to (1) top, or (2) a pointer to another client
    // This avoids a special case for removing the head of the list
    if (*p) {

        // TODO: Remove the client from other clients' following/followers lists
        // Loop through all active clients:
        for (struct client *ac = *clients; ac != NULL; ac = ac->next) {

            // SECTION 1: remove client from ac's *FOLLOWERS* list, if client exists in it
            for (int i = 0; i < FOLLOW_LIMIT; i++) { // iterate through FOLLOWERS list
                if (((ac->followers)[i]) != NULL && ((ac->followers)[i]->fd == fd)) {
                    (ac->followers)[i] = NULL; // remove client from list
                    break;
                }
            }

            // SECTION 2: remove client from ac's *FOLLOWING* list, if client exists in it
            for (int i = 0; i < FOLLOW_LIMIT; i++) {  // iterate through FOLLOWING list
                if (((ac->following)[i]) != NULL && ((ac->following)[i]->fd == fd)) {
                    (ac->following)[i] = NULL; // remove client from list
                    break;
                }
            }
        }

        // Remove the client
        struct client *t = (*p)->next;
        printf("Removing client %d %s\n", fd, inet_ntoa((*p)->ipaddr));
        FD_CLR((*p)->fd, &allset);
        close((*p)->fd);
        free(*p);
        *p = t;

    } else {
        fprintf(stderr, 
            "Trying to remove fd %d, but I don't know about it\n", fd);
    }
}


int find_network_newline(const char *buf, int num_bytes) {
    int index = 0;
    while (index < num_bytes - 1) {
        if (buf[index] == '\r' && buf[index + 1] == '\n') {
            return index + 2;
        }
        index++;
    }
    return -1;
}

int read_from(struct client **q, int *inbuf) {
    struct client *c = *q;
    c->inbuf[0] = '\0';
    int room = sizeof(c->inbuf);
    c->in_ptr = c->inbuf;

    int nbytes;
    while ((nbytes = read(c->fd, c->in_ptr, room)) > 0) {
        *inbuf += nbytes;

        int where;
        if ((where = find_network_newline(c->inbuf, *inbuf)) > 0) {
            c->inbuf[where - 2] = '\0';
            break;
        }

        room = BUF_SIZE - (*inbuf);
        c->in_ptr = c->inbuf + (*inbuf);
    } 

    if (nbytes == 0) { // client has disconnected
        return -1;
    }
    
    return 1; // read happened properly
}


// if handle_username returns 1, means username is invalid, if returns 0, means username is valid
int handle_username(struct client *client, char *username, struct client **active_clients) {
    
    // STEP 1: Check if username is the empty string               
    if (strcmp(username, "") == 0) { 
        int wbytes = write(client->fd, INVAL_USERNAME_MSG, strlen(INVAL_USERNAME_MSG));
        if (wbytes != strlen(INVAL_USERNAME_MSG)) {
            remove_client(active_clients, client->fd);
        }
        return 1;
    } 
    // STEP 2: Check if username is a copy of another username
    else { 
        
        for (struct client *ac = *active_clients; ac != NULL; ac = ac->next) { // iterate through active_clients
            if (strcmp(username, ac->username) == 0) { // username is already taken
                int wbytes = write(client->fd, USERNAME_TAKEN_MSG, strlen(USERNAME_TAKEN_MSG));
                if (wbytes != strlen(USERNAME_TAKEN_MSG)) {
                    remove_client(active_clients, client->fd);
                }
                return 1;
            } 
        }
    }
    // gets here only if username is valid
    strcpy(client->username, username);
    return 0; 
}

void handle_follow(struct client *client, char *username, struct client **active_clients) {

    // STEP 1: check if this client has followed the max amount of people already
    int num_following = 0; 
    int can_be_followed = 0;
    int found_match = 0;

    for (int i = 0; i < FOLLOW_LIMIT; i++) { // iterate through client's following list
        if ((client->following)[i] != NULL) { num_following++; }
    }

    if (num_following == FOLLOW_LIMIT) { // client has reached his follow limit
        int wbytes = write(client->fd, FOLLOWING_LIMIT_MSG, strlen(FOLLOWING_LIMIT_MSG));
        if (wbytes != strlen(FOLLOWING_LIMIT_MSG)) {
            remove_client(active_clients, client->fd);
        } return;
    }

    // STEP 2: check if username the client entered is the username of an active client
    // (only reaches this part of the code if the client has room to follow more users)

    // iterate through all active clients
    for (struct client *ac = *active_clients; ac != NULL; ac = ac->next) {
        if (strcmp(ac->username, username) == 0) { // found username match
            // add this active user to the client's following list by adding it to 
            // a space in the list where there is a NULL
            for (int i = 0; i < FOLLOW_LIMIT; i++) {
                if ((client->following)[i] == NULL) { 
                    (client->following)[i] = ac; 
                    found_match = 1;
                    break;
                }
            }

            // add the client to the active clients followers list
            for (int a = 0; a < FOLLOW_LIMIT; a++) {
                if ((ac->followers)[a] == NULL) { 
                    (ac->followers)[a] = client; 
                    can_be_followed = 1;
                    break;
                }
            }
        }
    }

    // user client is trying to follow cannot be followed anymore bc active cli reached limit
    if (!can_be_followed || !found_match) {
        int wbytes = write(client->fd, CANNOT_FOLLOW, strlen(CANNOT_FOLLOW));
        if (wbytes != strlen(CANNOT_FOLLOW)) {
            remove_client(active_clients, client->fd);
        } return;

    } 

    fprintf(stderr, "%s has followed a user\n", client->username);
    return;
}

void handle_unfollow(struct client *client, char *username, struct client **active_clients) {

    // STEP 1: check if username the client entered is the username of an active client
    // Iterate through all active clients:
    for (struct client *ac = *active_clients; ac != NULL; ac = ac->next) {
        if (strcmp(ac->username, username) == 0) { // found username match
            // remove the active client with this username from the client's following list
            for (int i = 0; i < FOLLOW_LIMIT; i++) { 
                if ((client->following)[i] != NULL && (strcmp((client->following)[i]->username, username) == 0)) {
                    (client->following)[i] = NULL;
                    fprintf(stderr, "%s has unfollowed %s\n", client->username, ac->username);
                }

                // now remove the client from the unfollowed user's follower list 
                for (int a = 0; a < FOLLOW_LIMIT; a++) { // traverse follower list of unfollowed user
                    if (((ac->followers[a]) != NULL) && ((ac->followers[a])->fd == client->fd)) {
                        (ac->followers[a]) = NULL;
                        return;
                    }
                }
            }  
        }
    }

    // if it reaches this part of the code, means that no active client with that username has been
    // found
    int wbytes = write(client->fd, NO_CLIENT_MSG, strlen(NO_CLIENT_MSG));
    if (wbytes != strlen(NO_CLIENT_MSG)) {
        remove_client(active_clients, client->fd);
    } return;
}

void handle_show(struct client *client, struct client **active_clients) {
    for (int i = 0; i < FOLLOW_LIMIT; i++) { // iterate through all users this user is following
        if ((client->following)[i] != NULL) {
            fprintf(stderr, "NULL THIS MANY TIMES\n");
            fprintf(stderr, "username %s\n", (client->following)[i]->username);
            for (int a = 0; a < MSG_LIMIT; a++) { // iterate through the followee's messages
                if ((client->following)[i]->message[a][0] != '\0') {
                    // write the message to this client
                    char new_message[100];
                    strcpy(new_message, (client->following)[i]->username);
                    strcat(new_message, ": ");
                    strcat(new_message, (client->following)[i]->message[a]);
                    strcat(new_message, "\r\n");
                    new_message[strlen((client->following)[i]->username) + 2 + strlen((client->following)[i]->message[a]) + 2] = '\0';
                     
                    int wbytes = write(client->fd, new_message, strlen(new_message));
                    if (wbytes != strlen(new_message)) {
                        remove_client(active_clients, (client->following)[i]->fd);
                        return;
                    }
                }
            } 
        }
    }

   
}

void handle_send(struct client *client, char *message, struct client **active_clients) {

    // count number of messages this user has sent
    int num_messages = 0;
    for (int i = 0; i < MSG_LIMIT; i++) {
        if ((client->message[i][0] != '\0')) {
            num_messages++;
        }
    }

    if (num_messages == MSG_LIMIT) {
        int wbytes = write(client->fd, NO_MORE_MSGS, strlen(NO_MORE_MSGS));
        if (wbytes != strlen(NO_MORE_MSGS)) {
            remove_client(active_clients, client->fd);
        }
    }

    // add message to clients message list
    for (int i = 0; i < MSG_LIMIT; i++) {
        if ((client->message[i][0] == '\0')) {
            strcpy(client->message[i], message);
            (client->message[i])[strlen(message)] = '\0';
            break;
        }
    }

    // create a message that will pop up for the client's followers (format is "username: message")
    char new_message[100];
    strcpy(new_message, client->username);
    strcat(new_message, ": ");
    strcat(new_message, message);
    strcat(new_message, "\r\n");
    new_message[strlen(client->username) + 2 + strlen(message) + 2] = '\0';

    // write the message to all of this client's followers
    for (int i = 0; i < FOLLOW_LIMIT; i++) {
        if ((client->followers)[i] != NULL) {
            int wbytes = write((client->followers)[i]->fd, new_message, strlen(new_message));
            if (wbytes != strlen(new_message)) {
                fprintf(stderr, "ya no work\n");
                remove_client(active_clients, (client->followers)[i]->fd);
            }
        }
    }
}

void handle_inval_command(struct client *client, struct client **active_clients) {
    int wbytes = write(client->fd, INVAL_COMMAND_MSG, strlen(INVAL_COMMAND_MSG));
    if (wbytes != strlen(INVAL_COMMAND_MSG)) {
        remove_client(active_clients, client->fd);
    }
}


// Send the message in s to all clients in active_clients. 
void announce(struct client **active_clients, char *s) {
    for (struct client *ac = *active_clients; ac != NULL; ac = ac->next) {
        int wbytes = write(ac->fd, s, strlen(s));
        if (wbytes != strlen(s)) {
            remove_client(active_clients, ac->fd);
        } return;
    }
}

int main (int argc, char **argv) {

    int clientfd, maxfd, nready;
    struct client *p;
    struct sockaddr_in q;
    fd_set rset;

    // If the server writes to a socket that has been closed, the SIGPIPE
    // signal is sent and the process is terminated. To prevent the server
    // from terminating, ignore the SIGPIPE signal. 
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGPIPE, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    // A list of active clients (who have already entered their names). 
    struct client *active_clients = NULL;

    // A list of clients who have not yet entered their names. This list is
    // kept separate from the list of active clients, because until a client
    // has entered their name, they should not issue commands or 
    // or receive announcements. 
    struct client *new_clients = NULL;

    struct sockaddr_in *server = init_server_addr(PORT);
    int listenfd = set_up_server_socket(server, LISTEN_SIZE);

    // Initialize allset and add listenfd to the set of file descriptors
    // passed into select 
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    // maxfd identifies how far into the set to search
    maxfd = listenfd;


    while (1) {
        // make a copy of the set before we pass it into select
        rset = allset;

        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if (nready == -1) {
            perror("select");
            exit(1);
        } else if (nready == 0) {
            continue;
        }

        // check if a new client is connecting
        if (FD_ISSET(listenfd, &rset)) {
            printf("A new client is connecting\n");
            clientfd = accept_connection(listenfd, &q);

            FD_SET(clientfd, &allset);
            if (clientfd > maxfd) {
                maxfd = clientfd;
            }
            printf("Connection from %s\n", inet_ntoa(q.sin_addr));
            add_client(&new_clients, clientfd, q.sin_addr);
            char *greeting = WELCOME_MSG;
            if (write(clientfd, greeting, strlen(greeting)) == -1) {
                fprintf(stderr, 
                    "Write to client %s failed\n", inet_ntoa(q.sin_addr));
                remove_client(&new_clients, clientfd);
            }
        }

        // Check which other socket descriptors have something ready to read.
        // The reason we iterate over the rset descriptors at the top level and
        // search through the two lists of clients each time is that it is
        // possible that a client will be removed in the middle of one of the
        // operations. This is also why we call break after handling the input.
        // If a client has been removed, the loop variables may no longer be 
        // valid.
        int cur_fd, handled;
        for (cur_fd = 0; cur_fd <= maxfd; cur_fd++) {
            if (FD_ISSET(cur_fd, &rset)) {
                handled = 0;

                // Check if any new clients are entering their names
                for (p = new_clients; p != NULL; p = p->next) {
                    if (cur_fd == p->fd) {
                        handled = 1;
                        // TODO: handle input from a new client who has not yet
                        // entered an acceptable name

                        int inbuf = 0;
                        int result = read_from(&p, &inbuf);
                        if (result == -1) {
                            remove_client(&new_clients, p->fd);
                        }
                        
                        // if 0, username valid and loop ends, if 1, username invalid
                        int invalid_username = handle_username(p, p->inbuf, &active_clients);
                        
                        if (invalid_username == 0) {

                            // Remove them from the new_clients list and add them to the active_clients list
                            struct client *copy = new_clients->next;
                            new_clients->next = active_clients;
                            active_clients = new_clients;
                            new_clients = copy;

                            // Notify all active_clients
                            char welcome_msg[1024];
                            welcome_msg[100] = '\0';
                            strcpy(welcome_msg, p->username);
                            strcat(welcome_msg, " has just joined!\r\n");
                            fprintf(stderr, "%s", welcome_msg); // write message to stdout
                            
                            for (struct client *a_cli = active_clients; a_cli != NULL; a_cli = a_cli->next) {
                                if (write(a_cli->fd, welcome_msg, strlen(welcome_msg)) == -1) {
                                    remove_client(&active_clients, a_cli->fd);
                                }
                            }
                        }
                    }
                } 
                if (!handled) {
                    // Check if this socket descriptor is an active client
                    for (p = active_clients; p != NULL; p = p->next) { 
                        if (cur_fd == p->fd) {

                                // Start process to begin reading from client
                                int inbuf = 0;
                                int result = read_from(&p, &inbuf);

                                if (result == -1) {
                                    remove_client(&active_clients, p->fd);
                                }

                                char copy[100];
                                int i;
                                for (i = 0; i < strlen(p->inbuf); i++) {
                                    copy[i] = p->inbuf[i];
                                }
                                copy[i] = '\0';

                                // split command with spaces
                                const char split[2] = " ";
                                char *first_word; // word before first space (if exists)
                                char *second; // word(s) after first space (if exists)
                                first_word = strtok(copy, split);
                                second = strtok(NULL, split);
                                char second_word[100];

                                if (second != NULL) {
                                    int index = strlen(first_word) + 1;
                                    
                                    int i;
                                    for (i = 0; i < strlen(p->inbuf) - strlen(first_word) + 1; i++) {
                                        second_word[i] = p->inbuf[index];
                                        index++;
                                    }

                                    second_word[i] = '\0';

                                } else {
                                    second_word[0] = '\0';
                                }
                                        
                                if ((strcmp(first_word, FOLLOW_MSG) == 0) && (second_word[0] != '\0')) {
                                    handle_follow(p, second_word, &active_clients);
                                }

                                else if ((strcmp(first_word, UNFOLLOW_MSG) == 0) && (second_word[0] != '\0')) {
                                    handle_unfollow(p, second_word, &active_clients);
                                }

                                else if ((strcmp(first_word, SHOW_MSG) == 0) && (second_word[0] == '\0')) {
                                    handle_show(p, &active_clients);
                                }

                                else if ((strcmp(first_word, SEND_MSG) == 0) && (second_word[0] != '\0')) {
                                    handle_send(p, second_word, &active_clients);
                                }

                                else if ((strcmp(first_word, QUIT_MSG) == 0) && (second_word[0] == '\0')) {
                                    remove_client(&active_clients, p->fd); 
                                }

                                else { handle_inval_command(p, &active_clients); }
                        }
                    }
                } 
            }
        }
    }

return 0;
}
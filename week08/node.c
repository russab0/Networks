#include "node.h"

// Compares two peers and see if their ip addresses and ports are the same
int peer_cmp(Peer p1, Peer p2){
    int acc;
    acc = strcmp(p1.ip_address, p2.ip_address);
    if (acc == 0 && p1.port == p2.port)
        return TRUE;
    else
        return FALSE;
}

// Adds peer to the linked list of peers
void add_peer(struct LinkedPeerList *list, struct Peer item){
    if (list->length == 0){
        list->self = (struct LinkedPeerNode *) malloc(sizeof(struct LinkedPeerNode));
        list->self->value = item;
        list->self->previous = NULL;
        list->self->next = NULL;
    }
    else{
        struct LinkedPeerNode *prev = list->self;
        struct LinkedPeerNode *cur = list->self->next;
        while (cur != NULL){
            prev = cur;
            cur = cur->next;
        }
        prev->next = (struct LinkedPeerNode *) malloc(sizeof(struct LinkedPeerNode));
        prev->next->previous = prev;
        prev->next->next = NULL;
        prev->next->value = item;
    }
    list->length++;
}

// Find peer in the linked list of peers
int find_peer(struct LinkedPeerList *list, struct Peer item){
    if (list->length == 0)
        return FALSE;
    else if (peer_cmp(item, this_node.self) == TRUE){
        return TRUE;
    } 
    else{
        struct LinkedPeerNode *cur = list->self;
        while (cur != NULL && peer_cmp(item, cur->value) == FALSE){
            cur = cur->next;
        }
        if (cur == NULL){
            return FALSE;
        }
        return TRUE;
    }
}

// Remove peer from the linked list of peers
void remove_peer(struct LinkedPeerList *list, struct Peer item){
    if (list->length == 0){
        return;
    } 
    else{
        struct LinkedPeerNode *prev = NULL;
        struct LinkedPeerNode *cur = list->self;
        while (cur != NULL && peer_cmp(item, cur->value) == FALSE){
            prev = cur;
            cur = cur->next;
        }
        if (cur == NULL)
            return;
        else{
            if (prev == NULL)
                list->self = cur->next;
	    else
                prev->next = cur->next;
            if (cur->next != NULL)
                cur->next->previous = cur->previous;
            free(cur);
        }
    }
    list->length--;
}

// Get list of all peers and store it in the items
void get_peers(struct LinkedPeerList list, struct Peer *items){
    struct LinkedPeerNode *cur = list.self;
    for (int i = 0; i < list.length; ++i){
        items[i] = cur->value;
        cur = cur->next;
    }
}

// Check if two files are the same
int file_cmp(struct PeerFile p1, struct PeerFile p2){
    int acc;
    acc = strcmp(p1.name, p2.name);
    if (acc == 0)
        return TRUE;
    else
        return FALSE;
}

// Add file to the linked list of files
void add_file(struct LinkedFileList *list, struct PeerFile file){
    if (list->length == 0){
        list->self = (struct LinkedFileNode *) malloc(sizeof(struct LinkedFileNode));
        list->self->value = file;
        list->self->previous = NULL;
        list->self->next = NULL;
    } 
    else{
        struct LinkedFileNode *prev = list->self;
        struct LinkedFileNode *cur = list->self->next;
        while (cur != NULL){
            prev = cur;
            cur = cur->next;
        }
        prev->next = (struct LinkedFileNode *) malloc(sizeof(struct LinkedFileNode));
        prev->next->previous = prev;
        prev->next->next = NULL;
        prev->next->value = file;
    }
    list->length++;
}

// Find file in the linked list of files
int find_file(struct LinkedFileList *list, struct PeerFile file){
    if (list->length == 0)
        return FALSE;
    else{
        struct LinkedFileNode *cur = list->self;
        while (cur != NULL && file_cmp(file, cur->value) == FALSE)
            cur = cur->next;
        if (cur == NULL)
            return FALSE;
        return TRUE;
    }
}

// Remove file from the linked list of files
void remove_file(struct LinkedFileList *list, struct PeerFile file){
    if (list->length == 0){
        return;
    } 
    else{
        struct LinkedFileNode *prev = NULL;
        struct LinkedFileNode *cur = list->self;
        while (cur != NULL && file_cmp(file, cur->value) == TRUE){
            prev = cur;
            cur = cur->next;
        }
        if (cur == NULL)
            return;
        else{
            if (prev == NULL){
                list->self = cur->next;
            } 
	    else
                prev->next = cur->next;
            if (cur->next != NULL){
                cur->next->previous = cur->previous;
            }
            free(cur);
        }
    }
    list->length--;
}

// Get list of all files and store in the files
void get_file(struct LinkedFileList list, struct PeerFile *files){
    struct LinkedFileNode *cur = list.self;
    for (int i = 0; i < list.length; ++i){
        files[i] = cur->value;
        cur = cur->next;
    }
}

// Download new file
void download_file(struct Peer peer, struct PeerFile file){
    printf("Got new file %s \n", file.name);
    add_file(&this_node.files, file);
    ssize_t bytes_sent, bytes_received;
    int client_socket, file_size;
    struct Protocol p;
    struct sockaddr_in destination_addr;

    // Create client's socket from which he will connect
    socklen_t addr_len = sizeof(struct sockaddr);
    if ((client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
        fprintf(stderr, "failed to create a socket to download a file: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    destination_addr.sin_family = AF_INET;
    destination_addr.sin_addr.s_addr = inet_addr(peer.ip_address);
    destination_addr.sin_port = htons(peer.port);

    // Connect to the server
    if (connect(client_socket, (struct sockaddr *) &destination_addr, addr_len) == -1){
        fprintf(stderr, "failed to connect to server to download a file:%d\n", errno);
        exit(EXIT_FAILURE);
    }
    p.type = PROT_GET_FILE;

    // Send protocol type
    bytes_sent = sendto(client_socket, (void *) &p, sizeof(p), 0,
                        (struct sockaddr *) &destination_addr,
                        sizeof(struct sockaddr));
    if (bytes_sent == -1){
        fprintf(stderr, "error on send protocol to load a file: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    // Send file name
    bytes_sent = sendto(client_socket, (void *) &file, sizeof(struct PeerFile), 0,
                        (struct sockaddr *) &destination_addr,
                        sizeof(struct sockaddr));
    if (bytes_sent == -1){
        fprintf(stderr, "error on send to send file name: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    // Receive answer from the sever
    bytes_received = recvfrom(client_socket, (void *) &p, sizeof(p), 0,
                              (struct sockaddr *) &destination_addr,
                              &addr_len);
    if (bytes_received == -1){
        fprintf(stderr, "error on receive protocol answer to download a file: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    // If server doesn't have such file
    if (p.type == PROT_NO){
        close(client_socket);
        return;
    } 
    // If server has such file
    else if (p.type == PROT_OK){
        printf("Beginning the download of %s\n", file.name);

        // Get the size of the file
        bytes_received = recvfrom(client_socket, (void *) &file_size, sizeof(file_size), 0,
                                  (struct sockaddr *) &destination_addr,
                                  &addr_len);
        if (bytes_received == -1){
            fprintf(stderr, "error on receive file size errno: %d\n", errno);
            exit(EXIT_FAILURE);
        }

        FILE *load_file;
        // Open/create the file
        load_file = fopen(file.name, "w+");
        while (file_size > 0){
            char buf[20];
            memset(buf, 0, sizeof(buf));
            // Get next word
            bytes_received = recvfrom(client_socket, (void *) &buf, sizeof(buf), 0,
                                      (struct sockaddr *) &destination_addr,
                                      &addr_len);
            if (bytes_received == -1){
                fprintf(stderr, "error on receive next word number %d errno: %d\n", file_size, errno);
                exit(EXIT_FAILURE);
            }

            //Write it to the file
            fwrite(buf, sizeof(char), strlen(buf), load_file);
            file_size--;
            if (file_size != 0){
                //Write space in the end
                fwrite(" ", sizeof(char), strlen(" "), load_file);
            }
        }
        fclose(load_file);
        printf("Loaded file %s\n", file.name);
        close(client_socket);
    }
}

// Count the words in the file. Sets current position to the begnning
int words_count(FILE *file){
    int num_words = 0;
    int c;
    rewind(file);
    if (feof(file))
        return 0;

    while ((c = getc(file)) != EOF){
        if (isalpha(c)){
            continue;
        } 
	else if (c == ' '){
            num_words++;
        }
    }
    rewind(file);
    return num_words + 1;
}

// Connect to a peer
void connect_to_peer(struct Peer peer){
    ssize_t bytes_sent, bytes_received;
    int client_socket;
    struct Protocol p;
    struct sockaddr_in destination_addr;
    if (peer_cmp(peer, this_node.self) == FALSE && find_peer(&this_node.peers, peer) == FALSE){
        // Create client's socket from which he will connect
        socklen_t addr_len = sizeof(struct sockaddr);
        if ((client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
            fprintf(stderr, "failed to create a client socket to add a peer errno: %d\n", errno);
            exit(EXIT_FAILURE);
        }

        // Set up ping address
        destination_addr.sin_family = AF_INET;
        destination_addr.sin_addr.s_addr = inet_addr(peer.ip_address);
        destination_addr.sin_port = htons(peer.port);

        // Connect to the server
        if (connect(client_socket, (struct sockaddr *) &destination_addr, addr_len) == -1){
            fprintf(stderr, "failed to connect to server by client to connect a peer errno:%d\n", errno);
            exit(EXIT_FAILURE);
        }

        p.type = PROT_ADD_PEER;
        // Send protocol type
        bytes_sent = sendto(client_socket, (void *) &p, sizeof(p), 0,
                            (struct sockaddr *) &destination_addr,
                            sizeof(struct sockaddr));
        if (bytes_sent == -1){
            fprintf(stderr, "error on send protocol on client to connect a peer errno: %d\n", errno);
            exit(EXIT_FAILURE);
        }
        addr_len = sizeof(struct sockaddr_in);

        // Get name of the peer
        bytes_received = recvfrom(client_socket, (void *) &peer.name, sizeof(peer.name), 0,
                                  (struct sockaddr *) &destination_addr,
                                  &addr_len);
        if (bytes_received == -1){
            fprintf(stderr, "Error on recv name of the node errno: %d\n", errno);
            exit(EXIT_FAILURE);
        }

        // Add address to the list (address of the server)
        add_peer(&this_node.peers, peer);
        printf("Got new node! Name: %s:%s:%u\n", peer.name, peer.ip_address,
               peer.port);
        // Send data about self
        bytes_sent = sendto(client_socket, (void *) &this_node.self, sizeof(this_node), 0,
                            (struct sockaddr *) &destination_addr,
                            sizeof(struct sockaddr));
        if (bytes_sent == -1){
            fprintf(stderr, "error on send self info in connect to a peer errno: %d\n", errno);
            exit(EXIT_FAILURE);
        }
        close(client_socket);
    }
}

// Initialise server
void *initialise_server(void *data){
    // Create socket and server addresses for binding
    int server_socket, clients_fd, current_connect = 0;
    pthread_t clients[CONNECT_N], pinger;
    socklen_t addrlen;
    struct sockaddr_in server_addr;

    addrlen = sizeof(struct sockaddr_in);
    //Create server socket that is datagram for tcp transmissions
    if ((server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
        fprintf(stderr, "failed to create a server socket errno: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr)); //Clean up server address
    // Set server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(MY_IP_ADDRESS);

    //Bind server socket to server
    if ((bind(server_socket, (struct sockaddr *) &server_addr, sizeof(struct sockaddr)) == -1)){
        fprintf(stderr, "failed to bind server socket errno: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    // Copy node information about us to the self part
    memset(this_node.self.ip_address, 0, sizeof(this_node.self.ip_address));
    strcpy(this_node.self.ip_address, MY_IP_ADDRESS);
    this_node.self.port = SERVER_PORT;

    //Begin listening
    if (listen(server_socket, CONNECT_N) < 0){
        fprintf(stderr, "failed to listen server errno: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    // Create thread of the pinger
    pthread_create(&pinger, NULL, ping_clients, NULL);
    while (TRUE){
        struct greet_client_data c_data;
        struct sockaddr_in client_addr;
        clients_fd = accept(server_socket,
                            (struct sockaddr *) &client_addr,
                            &addrlen);
        //Relocate every new client to the new thread
        c_data.client_socket = clients_fd;
        c_data.client_addr = client_addr;
        pthread_create(&clients[current_connect], NULL, handle_client, (void *) &c_data);
        current_connect = (current_connect + 1) % CONNECT_N;
    }
}

//Pinger
void *ping_clients(void *data){
    struct Peer *peers = malloc(sizeof(char) * 0);
    struct Protocol p;
    struct Peer peer_buf[PEER_BUF];
    struct PeerFile file_buf[PEER_BUF];
    ssize_t bytes_received, bytes_sent;
    p.type = PROT_PING;
    int connect_fd, peer_num, peer_sync_num;
    struct sockaddr_in server_addr;
    socklen_t addr_len;

    while (TRUE){
        sleep(PING_INTERVAL);

        peer_num = this_node.peers.length;
        peers = (Peer *) realloc(peers, sizeof(Peer) * peer_num);
        get_peers(this_node.peers, peers);
        for (int i = 0; i < peer_num; ++i){
            addr_len = sizeof(server_addr);
            p.type = PROT_PING;
            //Create new socket for the ping duration
            if ((connect_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
                fprintf(stderr, "failed to create a socket to ping clients errno: %d\n", errno);
                exit(EXIT_FAILURE);
            }

            //Setup server address
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(peers[i].port);
            server_addr.sin_addr.s_addr = inet_addr(peers[i].ip_address);

            //Try to connect to the node
            if (connect(connect_fd, (struct sockaddr *) &server_addr, addr_len) == -1){
                if (errno == ECONNREFUSED){
                    printf("Node Name:%s:%s:%u left\n", peers[i].name,
                           peers[i].ip_address, peers[i].port);
                    //Remove item from the list
                    remove_peer(&this_node.peers, peers[i]);
                    close(connect_fd);
                    continue;
                } 
		else{
                    fprintf(stderr, "failed to connect to ping errno:%d\n", errno);
                    exit(EXIT_FAILURE);
                }
            }

            //Send protocol type
            bytes_sent = sendto(connect_fd, (void *) &p, sizeof(p), 0,
                                (struct sockaddr *) &server_addr,
                                sizeof(struct sockaddr));
            if (bytes_sent == -1){
                fprintf(stderr, "error on send ping errno: %d\n", errno);
                exit(EXIT_FAILURE);
            }

            //Receive answer
            bytes_received = recvfrom(connect_fd, (void *) &p, sizeof(p), 0,
                                      (struct sockaddr *) &server_addr,
                                      &addr_len);
            if (bytes_received == -1){
                if (errno == ETIMEDOUT){
                    printf("Node Name:%s:%s:%u left\n", peers[i].name,
                           peers[i].ip_address, peers[i].port);
                    //Remove item from the linked list
                    remove_peer(&this_node.peers, peers[i]);
                    close(connect_fd);
                    continue;
                } 
	        else{
                    fprintf(stderr, "error on receive ack errno: %d\n", errno);
                    exit(EXIT_FAILURE);
                }
            }
            //If successfully pinged - begin SYNC

            //SYNC PEERS
            p.type = PROT_SYNC_PEERS;
            //Send protocol for syncing the peers
            bytes_sent = sendto(connect_fd, (void *) &p, sizeof(p), 0,
                                (struct sockaddr *) &server_addr,
                                sizeof(struct sockaddr));
            if (bytes_sent == -1){
                fprintf(stderr, "error on send sync peers protocol errno: %d\n", errno);
                exit(EXIT_FAILURE);
            }

            // Get num of peers
            bytes_received = recvfrom(connect_fd, (void *) &peer_sync_num, sizeof(peer_sync_num), 0,
                                      (struct sockaddr *) &server_addr,
                                      &addr_len);
            if (bytes_received == -1){
                fprintf(stderr, "error on receive number of peers errno: %d\n", errno);
                exit(EXIT_FAILURE);
            }
            memset(peer_buf, 0, sizeof(peer_buf));
            while (peer_sync_num > 0){
                //Receive buffer of peers
                bytes_received = recvfrom(connect_fd, (void *) &peer_buf, sizeof(peer_buf), 0,
                                          (struct sockaddr *) &server_addr,
                                          &addr_len);
                if (bytes_received == -1){
                    fprintf(stderr, "error on receive peer buf errno: %d\n", errno);
                    exit(EXIT_FAILURE);
                }
                for (int j = 0; j < min(PEER_BUF, peer_sync_num); ++j){
                    if (find_peer(&this_node.peers, peer_buf[j]) == FALSE){
                        //Try to connect to new peers
                        connect_to_peer(peer_buf[i]);
                    }
                }
                peer_sync_num -= PEER_BUF;
            }


            //SYNC FILES
            p.type = PROT_SYNC_FILES;

            //Send protocol to sync files
            bytes_sent = sendto(connect_fd, (void *) &p, sizeof(p), 0,
                                (struct sockaddr *) &server_addr,
                                sizeof(struct sockaddr));
            if (bytes_sent == -1){
                fprintf(stderr, "error on send files protocol errno: %d\n", errno);
                exit(EXIT_FAILURE);
            }

            int file_sync_num;

            // Get num of files
            bytes_received = recvfrom(connect_fd, (void *) &file_sync_num, sizeof(file_sync_num), 0,
                                      (struct sockaddr *) &server_addr,
                                      &addr_len);
            if (bytes_received == -1){
                fprintf(stderr, "error on receive files num errno: %d\n", errno);
                exit(EXIT_FAILURE);
            }
            memset(file_buf, 0, sizeof(file_buf));
            while (file_sync_num > 0){
                //Recv files buffer
                bytes_received = recvfrom(connect_fd, (void *) &file_buf, sizeof(file_buf), 0,
                                          (struct sockaddr *) &server_addr,
                                          &addr_len);
                if (bytes_received == -1){
                    fprintf(stderr, "error on receive file buf errno: %d\n", errno);
                    exit(EXIT_FAILURE);
                }
                for (int j = 0; j < min(PEER_BUF, file_sync_num); ++j){
                    if (find_file(&this_node.files, file_buf[i]) == FALSE){
                        //Try to download files no in the possession
                        download_file(peers[i], file_buf[i]);
                    }
                }
                file_sync_num -= PEER_BUF;
            }
            close(connect_fd);
        }
    }
}

//Server client handler
void *handle_client(void *data){
    struct greet_client_data *client_data = (struct greet_client_data *) data;
    socklen_t addr_len;
    ssize_t received_bytes, bytes_sent;
    struct Peer new_node;
    struct Protocol p;

    //Receive protocol data from client
    received_bytes = recvfrom(client_data->client_socket, (void *) &p, sizeof(p), 0,
                              (struct sockaddr *) &client_data->client_addr, &addr_len);
    if (received_bytes == -1){
        fprintf(stderr, "Error on recv protocol errno: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    //Check protocol type, do appropriate things according to it
    if (p.type == PROT_PING){
        p.type = PROT_ACK;
        //Send ping acknowledge
        bytes_sent = sendto(client_data->client_socket, (void *) &p, sizeof(p), 0,
                            (struct sockaddr *) &client_data->client_addr,
                            sizeof(struct sockaddr));
        if (bytes_sent == -1){
            fprintf(stderr, "Error on sending ack errno : %d \n", errno);
            exit(EXIT_FAILURE);
        }
        printf("Got pinged, answering\n");

        received_bytes = recvfrom(client_data->client_socket, (void *) &p, sizeof(p), 0,
                                  (struct sockaddr *) &client_data->client_addr, &addr_len);
        if (received_bytes == -1){
            fprintf(stderr, "Error on recv protocol after ping peers errno: %d\n", errno);
            exit(EXIT_FAILURE);
        }
        //Begin sync process
        if (p.type != PROT_SYNC_PEERS){
            return NULL;
        }
        int peer_size = this_node.peers.length;
        struct Peer *sync_peers = malloc(peer_size * sizeof(struct Peer));
        struct Peer peer_buf[PEER_BUF];

        //send number of peers
        bytes_sent = sendto(client_data->client_socket, (void *) &peer_size, sizeof(int), 0,
                            (struct sockaddr *) &client_data->client_addr,
                            sizeof(struct sockaddr));
        if (bytes_sent == -1){
            fprintf(stderr, "Error on sending peer size errno : %d\n", errno);
            exit(EXIT_FAILURE);
        }
        get_peers(this_node.peers, sync_peers);

        memset(peer_buf, 0, sizeof(peer_buf));
        while (peer_size > 0){
            memcpy(peer_buf, sync_peers, sizeof(peer_buf));
            sync_peers += sizeof(peer_buf);
            peer_size -= PEER_BUF;
            //Send peers buffer
            bytes_sent = sendto(client_data->client_socket, (void *) &peer_buf, sizeof(peer_buf), 0,
                                (struct sockaddr *) &client_data->client_addr,
                                sizeof(struct sockaddr));
            if (bytes_sent == -1){
                fprintf(stderr, "Error on sending peer buf errno : %d\n", errno);
                exit(EXIT_FAILURE);
            }
        }
        // Get next protocol
        received_bytes = recvfrom(client_data->client_socket, (void *) &p, sizeof(p), 0,
                                  (struct sockaddr *) &client_data->client_addr, &addr_len);
        if (received_bytes == -1){
            fprintf(stderr, "Error on recv protocol after ping about files errno: %d\n", errno);
            exit(EXIT_FAILURE);
        }

        if (p.type != PROT_SYNC_FILES){
            return NULL;
        }
        int file_size = this_node.files.length;
        struct PeerFile *sync_files = malloc(file_size * sizeof(struct PeerFile));
        struct PeerFile files_buf[PEER_BUF];

        //Send number of files
        bytes_sent = sendto(client_data->client_socket, (void *) &file_size, sizeof(int), 0,
                            (struct sockaddr *) &client_data->client_addr,
                            sizeof(struct sockaddr));
        if (bytes_sent == -1){
            fprintf(stderr, "Error on sending files size errno : %d\n", errno);
            exit(EXIT_FAILURE);
        }
        //Get the list of the files
        get_file(this_node.files, sync_files);
        memset(files_buf, 0, sizeof(files_buf));

        while (file_size > 0){
            memcpy(files_buf, sync_files, sizeof(files_buf));
            sync_files += sizeof(files_buf);
            file_size -= PEER_BUF;
            //Send files buf
            bytes_sent = sendto(client_data->client_socket, (void *) &files_buf, sizeof(files_buf), 0,
                                (struct sockaddr *) &client_data->client_addr,
                                sizeof(struct sockaddr));
            if (bytes_sent == -1){
                fprintf(stderr, "Error on sending files buf errno : %d\n", errno);
                exit(EXIT_FAILURE);
            }
        }
    } 
    else if (p.type == PROT_ADD_PEER){
        //Sent own name
        bytes_sent = sendto(client_data->client_socket, (void *) &this_node.self.name, sizeof(this_node.self.name), 0,
                            (struct sockaddr *) &client_data->client_addr,
                            sizeof(struct sockaddr));
        if (bytes_sent == -1){
            fprintf(stderr, "Error on sending own name errno : %d\n", errno);
            exit(EXIT_FAILURE);
        }

        //Receive data about self from new client
        received_bytes = recvfrom(client_data->client_socket, (void *) &new_node, sizeof(new_node), 0,
                                  (struct sockaddr *) &client_data->client_addr, &addr_len);

        if (received_bytes == -1){
            fprintf(stderr, "Error on recv self info about client errno: %d\n", errno);
            exit(EXIT_FAILURE);
        }

        if (find_peer(&this_node.peers, new_node) == FALSE){
            printf("Got new node! Name: %s:%s:%u\n", new_node.name, new_node.ip_address,
                   new_node.port);
            //Add item
            add_peer(&this_node.peers, new_node);
        }
    } 
    else if (p.type == PROT_GET_FILE){
        //Steps if file was requested
        int num_words;
        char words_buf[20];
        struct PeerFile file;
        FILE *send_file;

        //Get file name
        received_bytes = recvfrom(client_data->client_socket, (void *) &file, sizeof(file), 0,
                                  (struct sockaddr *) &client_data->client_addr, &addr_len);
        if (received_bytes == -1){
            fprintf(stderr, "Error on recv self info about client errno: %d\n", errno);
            exit(EXIT_FAILURE);
        }
        //Check if file is present
        send_file = fopen(file.name, "r+");
        //If not return PROT_NO
        if (send_file == NULL)
            p.type = PROT_NO;
        else
            p.type = PROT_OK;
        //Send asnwer
        bytes_sent = sendto(client_data->client_socket, (void *) &p, sizeof(p), 0,
                            (struct sockaddr *) &client_data->client_addr,
                            sizeof(struct sockaddr));
        if (bytes_sent == -1){
            fprintf(stderr, "Error on sending ack errno : %d\n", errno);
            exit(EXIT_FAILURE);
        }

        //Close if no file
        if (p.type == PROT_NO){
            close(client_data->client_socket);
            return NULL;
        }
        printf("Beginning to send file %s \n", file.name);
        num_words = words_count(send_file);
        //Send number of words
        bytes_sent = sendto(client_data->client_socket, (void *) &num_words, sizeof(num_words), 0,
                            (struct sockaddr *) &client_data->client_addr,
                            sizeof(struct sockaddr));
        if (bytes_sent == -1){
            fprintf(stderr, "Error on sending num words errno : %d\n", errno);
            exit(EXIT_FAILURE);
        }

        //Send number of words
        while (num_words > 0){
            //Send words by one words at the time
            memset(words_buf, 0, sizeof(words_buf));
            fscanf(send_file, "%[^ ] ", words_buf);
            bytes_sent = sendto(client_data->client_socket, (void *) &words_buf, sizeof(words_buf), 0,
                                (struct sockaddr *) &client_data->client_addr,
                                sizeof(struct sockaddr));
            if (bytes_sent == -1){
                fprintf(stderr, "Error on sending words buf errno : %d\n", errno);
                exit(EXIT_FAILURE);
            }
            num_words--;
        }
        printf("Ended transmitting\n");

    }
    close(client_data->client_socket);
    return 0;
}

void *initialise_client(void *data){
    struct Peer new_peer;
    new_peer = *((struct Peer *) data);
    connect_to_peer(new_peer);
    return 0;
}

int main(void){
    pthread_t client, server;
    ssize_t bytes_read;
    memset(&this_node, 0, sizeof(this_node));
    printf("How should I call you?\n");
    bytes_read = read(0, this_node.self.name, sizeof(this_node.self.name) - 1);
    this_node.self.name[bytes_read - 1] = '\0';
    printf("Ok! Your name is: %s\n", this_node.self.name);
    pthread_create(&server, NULL, initialise_server, NULL);
    while (TRUE){
        struct Peer new_peer;
        char ip[20];
        uint16_t port;
        char buf[2];
        printf("What do you want to do?\n");
        printf("To connect - 1. To add file - 2. (Server works on background)\n");
        read(0, buf, sizeof(buf));
        buf[1] = '\0';
        if (strcmp(buf, "1") == 0){
            //Add connection
            printf("Enter IP:Port of the server\n");
            scanf("%[^:]:%hu", ip, &port);
            strcpy(new_peer.ip_address, ip);
            new_peer.port = port;
            pthread_create(&client, NULL, initialise_client, (void *) &new_peer);
        } 
	else if (strcmp(buf, "2") == 0){
            // Add file
            char file_buf[26];
            FILE *file;
            printf("Enter filename\n");
            bytes_read = read(0, file_buf, sizeof(file_buf) - 1);
            file_buf[bytes_read - 1] = '\0';
            file = fopen(file_buf, "r+");
            fflush(stdin);
            if (file == NULL){
                printf("No such file exists\n");
            } 
	    else{
                printf("Added file %s\n", file_buf);
                struct PeerFile file1;
                memset(&file1, 0, sizeof(struct PeerFile));
                strcpy(file1.name, file_buf);
                fclose(file);
                add_file(&this_node.files, file1);
            }
        }
    }
}

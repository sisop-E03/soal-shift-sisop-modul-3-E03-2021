#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <fcntl.h>

#define PORT 8080
#define HANDSHAKE "800"
#define SUCCESS "200"
#define FAILED "100"
#define LOGIN_CODE "11"
#define REGISTER_CODE "12"
#define ADD_CODE "51"
#define DOWNLOAD_CODE "52"
#define DELETE_CODE "53"
#define SEE_CODE "54"
#define FIND_CODE "55"
#define LOGOUT_CODE "56"
#define END_CONN "57"

int create_socket();
void connect_to_server(int socketfd);
void app_menu(int socketfd);
int handle_login(int socketfd);
void handle_register(int socketfd);
void authenticate_user_interface(int socketfd);
void handle_add_command(int socketfd);
void handle_download_command(int socketfd);
void handle_see_command(int socketfd);
void handle_delete_command(int socketfd);
void handle_find_command(int socketfd);
void handle_logout_command(int socketfd);
void create_upload_request(int socketfd, char file_path[]);
void clear_buffer(char* b);

int main(int argc, char const *argv[]) {
    int socketfd = create_socket();

    if (socketfd == -1)
        exit(0);

    connect_to_server(socketfd);

    return 0;
}

int create_socket() {
    struct sockaddr_in address;
    int socketfd, valread;
    struct sockaddr_in serv_addr;
    char buffer[BUFSIZ] = {0};
    if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stdout, "\n Socket creation error \n");
        return -1;
    }
  
    memset(&serv_addr, '0', sizeof(serv_addr));
  
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
      
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) {
        fprintf(stdout, "\nInvalid address/ Address not supported \n");
        return -1;
    }
  
    if (connect(socketfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        fprintf(stdout, "\nConnection Failed \n");
        return -1;
    }

    return socketfd;
}

void connect_to_server(int socketfd) {
    char buffer[BUFSIZ];
    int valread;
    
    send(socketfd, HANDSHAKE, strlen(HANDSHAKE), 0);

    fprintf(stdout, "Menunggu koneksi dari server...\n");
    
    clear_buffer(buffer);
    valread = read(socketfd , buffer, BUFSIZ);
    fprintf(stdout, "Terhubung dengan server\n");

    app_menu(socketfd);
}

void app_menu(int socketfd) {
    char buffer[BUFSIZ];
    int valread;

    while(1) {
        fprintf(stdout, "Menu:\n1. Login\n2.Register\n3.End connection\n");
        fprintf(stdout, "Type command: ");

        char choice[10];
        fscanf(stdin, "%s", choice);
        
        if (strlen(choice) == 1 && choice[0] == '1') {
            send(socketfd, LOGIN_CODE, strlen(LOGIN_CODE), 0);

            clear_buffer(buffer);
            valread = read(socketfd , buffer, BUFSIZ);

            if (handle_login(socketfd)) {
                authenticate_user_interface(socketfd);
            }

        } else if (strlen(choice) == 1 && choice[0] == '2') {
            send(socketfd, REGISTER_CODE, strlen(REGISTER_CODE), 0);

            clear_buffer(buffer);
            valread = read(socketfd , buffer, BUFSIZ);
            
            handle_register(socketfd);
        } else if (strlen(choice) == 1 && choice[0] == '3') {
            send(socketfd, END_CONN, strlen(END_CONN), 0);
            break;
        } else {
            fprintf(stdout, "Command not valid\n");
        }
    }
}

int handle_login(int socketfd) {

    char buffer[BUFSIZ];
    int valread;

    char username[100];
    char password[100];
    char auth_data[200];

    fprintf(stdout, "Login\n");
    fprintf(stdout, "username: ");
    fscanf(stdin, "%s", username);
    fprintf(stdout, "password: ");
    fscanf(stdin, "%s", password);
    sprintf(auth_data, "%s:%s", username, password);

    send(socketfd, auth_data, strlen(auth_data), 0);
    
    clear_buffer(buffer);
    valread = read(socketfd , buffer, BUFSIZ);

    if (strcmp(buffer, SUCCESS) == 0) {
        fprintf(stdout, "Login success\n");
        return 1;
    } else {
        fprintf(stdout, "Login failed\n");
        return 0;
    }
}

void handle_register(int socketfd) {
    char buffer[BUFSIZ];
    int valread;

    char username[100];
    char password[100];
    char reg_data[200];
    fprintf(stdout, "Register\n");
    fprintf(stdout, "username: ");
    fscanf(stdin, " %s", username);
    fprintf(stdout, "password: ");
    fscanf(stdin, " %s", password);
    sprintf(reg_data, "%s:%s", username, password);

    send(socketfd, reg_data, strlen(reg_data), 0);

    clear_buffer(buffer);
    valread = read(socketfd, buffer, BUFSIZ);

    if (strcmp(buffer, SUCCESS) == 0) {
        fprintf(stdout, "Register success\n");
    } else {
        fprintf(stdout, "Register failed\n");
    }
}

void authenticate_user_interface(int socketfd) {    
    while (1) {
        fprintf(stdout, "Make a command: {add, download, delete, see, find, logout}\n");
        char cmd[10];
        fscanf(stdin, "%s", cmd);
        if (strcmp(cmd, "add") == 0) {
            handle_add_command(socketfd);
        } else if (strcmp(cmd, "download") == 0) {
            handle_download_command(socketfd);
        } else if (strcmp(cmd, "delete") == 0) {
            handle_delete_command(socketfd);
        } else if (strcmp(cmd, "see") == 0) {
            handle_see_command(socketfd);
        } else if (strcmp(cmd, "find") == 0) {
            handle_find_command(socketfd);
        } else if (strcmp(cmd, "logout") == 0) {
            handle_logout_command(socketfd);
            break;
        } else {
            fprintf(stdout, "Command not valid\n");
        }
    }
}

void handle_add_command(int socketfd) {
    char buffer[BUFSIZ];
    int valread;

    char publisher[50];
    char tahun_pub[10];
    char file_path[50];
    char add_data[110];

    send(socketfd, ADD_CODE, strlen(ADD_CODE), 0);

    clear_buffer(buffer);
    valread = read(socketfd, buffer, BUFSIZ);

    fprintf(stdout, "Publisher: ");
    fscanf(stdin, "%s", publisher);
    fprintf(stdout, "Tahun Publikasi: ");
    fscanf(stdin, "%s", tahun_pub);
    fprintf(stdout, "Filepath: ");
    fscanf(stdin, "%s", file_path);

    sprintf(add_data, "%s:%s:%s", file_path, publisher, tahun_pub);

    send(socketfd, add_data, strlen(add_data), 0);

    create_upload_request(socketfd, file_path);

    clear_buffer(buffer);
    valread = read(socketfd , buffer, BUFSIZ);

    if (strcmp(buffer, SUCCESS) == 0) {
        fprintf(stdout, "Add data success\n");
    } else {
        fprintf(stdout, "Add data failed\n");
    }
}

void handle_download_command(int socketfd) {
    char buffer[BUFSIZ];
    int valread;
    
    int file_size;
    int remain_data;
    ssize_t len;

    send(socketfd, DOWNLOAD_CODE, strlen(DOWNLOAD_CODE), 0);

    clear_buffer(buffer);
    valread = read(socketfd, buffer, BUFSIZ);

    char filename_to_download[50];
    fscanf(stdin, "%s", filename_to_download);

    send(socketfd, filename_to_download, strlen(filename_to_download), 0);

    clear_buffer(buffer);
    valread = read(socketfd, buffer, BUFSIZ);

    if (strcmp(buffer, SUCCESS) == 0) {
        send(socketfd, SUCCESS, strlen(SUCCESS), 0);
            
        clear_buffer(buffer);
        recv(socketfd, buffer, BUFSIZ, 0);
        file_size = atoi(buffer);

        FILE *received_file;
        
        received_file = fopen(filename_to_download, "w");
        if (received_file == NULL) {
                perror("Failed to open file");
                exit(EXIT_FAILURE);
        }

        remain_data = file_size;

        while ((remain_data > 0) && ((len = recv(socketfd, buffer, BUFSIZ, 0)) > 0)) {
                fwrite(buffer, sizeof(char), len, received_file);
                remain_data -= len;
        }

        fclose(received_file);
    } else {
        fprintf(stdout, "File %s does not exist\n", filename_to_download);
    }
}

void handle_see_command(int socketfd) {
    char buffer[BUFSIZ];
    int valread;
    
    send(socketfd, SEE_CODE, strlen(SEE_CODE), 0);

    clear_buffer(buffer);
    valread = read(socketfd, buffer, BUFSIZ);
    fprintf(stdout, "%s", buffer);
}

void handle_delete_command(int socketfd) {
    char buffer[BUFSIZ];
    int valread;
    
    send(socketfd, DELETE_CODE, strlen(DELETE_CODE), 0);

    clear_buffer(buffer);
    valread = read(socketfd, buffer, BUFSIZ);

    char filename[50];
    fscanf(stdin, "%s", filename);

    send(socketfd, filename, strlen(filename), 0);

    clear_buffer(buffer);
    valread = read(socketfd, buffer, BUFSIZ);
    if (strcmp(buffer, SUCCESS) == 0) {
        fprintf(stdout, "%s deleted\n", filename);
    } else {
        fprintf(stdout, "File %s do not exist\n", filename);
    }
}

void handle_find_command(int socketfd) {
    char buffer[BUFSIZ];
    int valread;
    
    send(socketfd, FIND_CODE, strlen(FIND_CODE), 0);

    clear_buffer(buffer);
    valread = read(socketfd, buffer, BUFSIZ);

    char filename_to_find[50];
    fscanf(stdin, "%s", filename_to_find);

    send(socketfd, filename_to_find, strlen(filename_to_find), 0);

    clear_buffer(buffer);
    valread = read(socketfd, buffer, BUFSIZ);
    if (strcmp(buffer, FAILED) == 0)
        fprintf(stdout, "No file exist with substring %s\n", filename_to_find);
    else 
        fprintf(stdout, "%s", buffer);
}

void handle_logout_command(int socketfd) {
    char buffer[BUFSIZ];
    int valread;

    send(socketfd, LOGOUT_CODE, strlen(LOGOUT_CODE), 0);

    clear_buffer(buffer);
    valread = read(socketfd , buffer, BUFSIZ);
}

void create_upload_request(int socketfd, char file_path[]) {
    char buffer[BUFSIZ];
    int valread;

    int fd;
    int sent_bytes = 0;
    char file_size[256];
    struct stat file_stat;
    off_t offset;
    int remain_data;

    clear_buffer(buffer);
    valread = read(socketfd, buffer, BUFSIZ);

    fd = open(file_path, O_RDONLY);

    if (fstat(fd, &file_stat) < 0) {
            perror("filestat error");
            exit(EXIT_FAILURE);
    }

    sprintf(file_size, "%ld", file_stat.st_size);

    send(socketfd, file_size, sizeof(file_size), 0);

    offset = 0;
    remain_data = file_stat.st_size;

    while (((sent_bytes = sendfile(socketfd, fd, &offset, BUFSIZ)) > 0) && (remain_data > 0))
    {
            remain_data -= sent_bytes;
    }
}

void clear_buffer(char* b) {
    int i;
    for (i = 0; i < BUFSIZ; i++)
        b[i] = '\0';
}
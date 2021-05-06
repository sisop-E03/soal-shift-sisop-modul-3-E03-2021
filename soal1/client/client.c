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
#define OK "200"
#define ACCEPTED "202"
#define BAD_REQUEST "400"
#define SUCCESS_CODE "1"
#define FAILED_CODE "0"
#define LOGIN_CODE "11"
#define REGISTER_CODE "12"
#define ADD_CODE "51"
#define DONWLOAD_CODE "52"
#define DELETE_CODE "53"
#define SEE_CODE "54"
#define FIND_CODE "55"
#define LOGOUT_CODE "56"

void clear_buffer(char* b)
{
    int i;
    for (i = 0; i < BUFSIZ; i++)
        b[i] = '\0';
}

int handle_login(char buffer[], int sock, int valread) {
    char username[100];
    char password[100];
    char auth_data[200];

    fprintf(stdout, "Login\n");
    fprintf(stdout, "username: ");
    fscanf(stdin, "%s", username);
    fprintf(stdout, "password: ");
    fscanf(stdin, "%s", password);
    sprintf(auth_data, "%s:%s", username, password);

    send(sock, auth_data, strlen(auth_data), 0);
    fprintf(stdout, "Sent authentication data: %s\n", auth_data);
    
    clear_buffer(buffer);
    valread = read(sock , buffer, BUFSIZ);

    if (strcmp(buffer, SUCCESS_CODE) == 0) {
        fprintf(stdout, "Login berhasil\n");
        return 1;
    } else {
        fprintf(stdout, "Login gagal\n");
        return 0;
    }
}

void handle_register(char buffer[], int sock, int valread) {
    char username[100];
    char password[100];
    char reg_data[200];
    fprintf(stdout, "Register\n");
    fprintf(stdout, "username: ");
    fscanf(stdin, " %s", username);
    fprintf(stdout, "password: ");
    fscanf(stdin, " %s", password);
    sprintf(reg_data, "%s:%s", username, password);

    send(sock, reg_data, strlen(reg_data), 0);
    fprintf(stdout, "Sent registration data: %s\n", reg_data);

    clear_buffer(buffer);
    valread = read(sock, buffer, BUFSIZ);

    if (strcmp(buffer, SUCCESS_CODE) == 0) {
        fprintf(stdout, "Register berhasil\n");
    } else {
        fprintf(stdout, "Register gagal\n");
    }
}

void create_upload_request(char buffer[], int sock, int valread, char file_path[]) {
    int fd;
    int sent_bytes = 0;
    char file_size[256];
    struct stat file_stat;
    off_t offset;
    int remain_data;

    clear_buffer(buffer);
    valread = read(sock, buffer, BUFSIZ);
    fprintf(stdout, "Receive status %s\n", buffer);

    fd = open(file_path, O_RDONLY);

    if (fstat(fd, &file_stat) < 0)
    {
            perror("filestat error");

            exit(EXIT_FAILURE);
    }

    sprintf(file_size, "%ld", file_stat.st_size);

    send(sock, file_size, sizeof(file_size), 0);
    fprintf(stdout, "Sent file size %s bytes\n", file_size);

    offset = 0;
    remain_data = file_stat.st_size;

    while (((sent_bytes = sendfile(sock, fd, &offset, BUFSIZ)) > 0) && (remain_data > 0))
    {
            fprintf(stdout, "1. Client upload %d bytes from file's data, offset is now : %ld and remaining data = %d\n", sent_bytes, offset, remain_data);
            remain_data -= sent_bytes;
            fprintf(stdout, "2. Client upload %d bytes from file's data, offset is now : %ld and remaining data = %d\n", sent_bytes, offset, remain_data);
    }
}

void handle_add_command(char buffer[], int sock, int valread) {
    char publisher[50];
    char tahun_pub[10];
    char file_path[50];
    char add_data[110];

    send(sock, ADD_CODE, strlen(ADD_CODE), 0);
    fprintf(stdout, "Sent add code %s\n", ADD_CODE);

    clear_buffer(buffer);
    valread = read(sock, buffer, BUFSIZ);
    fprintf(stdout, "Receive status %s\n", buffer);

    fprintf(stdout, "Publisher: ");
    fscanf(stdin, "%s", publisher);
    fprintf(stdout, "Tahun Publikasi: ");
    fscanf(stdin, "%s", tahun_pub);
    fprintf(stdout, "Filepath: ");
    fscanf(stdin, "%s", file_path);

    sprintf(add_data, "%s:%s:%s", publisher, tahun_pub, file_path);

    send(sock, add_data, strlen(add_data), 0);
    fprintf(stdout, "Sent new book data: %s\n", add_data);

    create_upload_request(buffer, sock, valread, file_path);

    clear_buffer(buffer);
    valread = read(sock , buffer, BUFSIZ);

    if (strcmp(buffer, SUCCESS_CODE) == 0) {
        fprintf(stdout, "Add data success\n");
    } else {
        fprintf(stdout, "Add data failed\n");
    }
}

void handle_download_command(char buffer[], int sock, int valread) {
    int file_size;
    int remain_data;
    ssize_t len;
    FILE *received_file;

    send(sock, DONWLOAD_CODE, strlen(DONWLOAD_CODE), 0);
    fprintf(stdout, "Sent registration data: %s\n", DONWLOAD_CODE);

    clear_buffer(buffer);
    valread = read(sock, buffer, BUFSIZ);
    fprintf(stdout, "Receive status %s\n", buffer);

    char filename_to_download[50];
    fscanf(stdin, "%s", filename_to_download);

    send(sock, filename_to_download, strlen(filename_to_download), 0);
    fprintf(stdout, "Sent filename to download %s\n", filename_to_download);

    clear_buffer(buffer);
    recv(sock, buffer, BUFSIZ, 0);
    file_size = atoi(buffer);

    fprintf(stdout, "Received file size : %d\n", file_size);
    
    received_file = fopen(filename_to_download, "w");
    if (received_file == NULL)
    {
            perror("Failed to open file");

            exit(EXIT_FAILURE);
    }

    remain_data = file_size;

    while ((remain_data > 0) && ((len = recv(sock, buffer, BUFSIZ, 0)) > 0))
    {
            fwrite(buffer, sizeof(char), len, received_file);
            fprintf(stdout, "%s\n", buffer);
            remain_data -= len;
            fprintf(stdout, "Receive %ld bytes and %d bytes remaining\n", len, remain_data);
    }
    fclose(received_file);
}

void handle_see_command(char buffer[], int sock, int valread) {
    send(sock, SEE_CODE, strlen(SEE_CODE), 0);
    fprintf(stdout, "Sent see code %s\n", SEE_CODE);

    clear_buffer(buffer);
    valread = read(sock, buffer, BUFSIZ);
    fprintf(stdout, "%s", buffer);
}

void handle_delete_command(char buffer[], int sock, int valread) {
    send(sock, DELETE_CODE, strlen(DELETE_CODE), 0);
    fprintf(stdout, "Sent delete code %s\n", DELETE_CODE);

    clear_buffer(buffer);
    valread = read(sock, buffer, BUFSIZ);
    fprintf(stdout, "Receive status %s\n", buffer);

    char filename_to_find[50];
    fscanf(stdin, "%s", filename_to_find);

    send(sock, filename_to_find, strlen(filename_to_find), 0);
    fprintf(stdout, "Sent filename to delete %s\n", filename_to_find);

    clear_buffer(buffer);
    valread = read(sock, buffer, BUFSIZ);
    fprintf(stdout, "Receive status %s\n", buffer);
}

void handle_find_command(char buffer[], int sock, int valread) {
    send(sock, FIND_CODE, strlen(FIND_CODE), 0);
    fprintf(stdout, "Sent see code %s\n", FIND_CODE);

    clear_buffer(buffer);
    valread = read(sock, buffer, BUFSIZ);
    fprintf(stdout, "Receive status %s\n", buffer);

    char filename_to_find[50];
    fscanf(stdin, "%s", filename_to_find);

    send(sock, filename_to_find, strlen(filename_to_find), 0);
    fprintf(stdout, "Sent filename to find %s\n", filename_to_find);

    clear_buffer(buffer);
    valread = read(sock, buffer, BUFSIZ);
    fprintf(stdout, "%s", buffer);
}

void interface(char buffer[], int sock, int valread) {
    while (1) {
        fprintf(stdout, "Make a command: {add, download, delete, see, find, logout}\n");
        char cmd[10];
        fscanf(stdin, "%s", cmd);
        if (strcmp(cmd, "add") == 0) {
            handle_add_command(buffer, sock, valread);
        } else if (strcmp(cmd, "download") == 0) {
            handle_download_command(buffer, sock, valread);
        } else if (strcmp(cmd, "delete") == 0) {
            handle_delete_command(buffer, sock, valread);
        } else if (strcmp(cmd, "see") == 0) {
            handle_see_command(buffer, sock, valread);
        } else if (strcmp(cmd, "find") == 0) {
            handle_find_command(buffer, sock, valread);
        } else if (strcmp(cmd, "logout") == 0) {
            send(sock, LOGOUT_CODE, strlen(LOGOUT_CODE), 0);
            fprintf(stdout, "Sent logout code %s\n", LOGOUT_CODE);

            clear_buffer(buffer);
            valread = read(sock , buffer, BUFSIZ);
            fprintf(stdout, "Receive code %s\n", buffer);

            break;
        } else {
            fprintf(stdout, "Command not valid\n");
        }
    }
}

int main(int argc, char const *argv[]) {
    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[BUFSIZ] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
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
  
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        fprintf(stdout, "\nConnection Failed \n");
        return -1;
    }

    while(1) {
        fprintf(stdout, "Menu:\n1. Login\n2.Register\n");
        fprintf(stdout, "Your choice: ");

        char choice[10];
        fscanf(stdin, "%s", choice);
        
        if (strlen(choice) == 1 && choice[0] == '1') {
            send(sock, LOGIN_CODE, strlen(LOGIN_CODE), 0);
            fprintf(stdout, "Sent menu code %s\n", LOGIN_CODE);

            clear_buffer(buffer);
            valread = read(sock , buffer, BUFSIZ);

            fprintf(stdout, "Receive status code %s\n", buffer);

            if (handle_login(buffer, sock, valread)) {
                interface(buffer, sock, valread);
            }

        } else if (strlen(choice) == 1 && choice[0] == '2') {
            send(sock, REGISTER_CODE, strlen(REGISTER_CODE), 0);
            fprintf(stdout, "Sent menu code %s\n", REGISTER_CODE);

            clear_buffer(buffer);
            valread = read(sock , buffer, BUFSIZ);

            fprintf(stdout, "Receive status code %s\n", buffer);
            
            handle_register(buffer, sock, valread);
        } else {
            fprintf(stdout, "Choice code not valid\n");
        }
    }

    return 0;
}
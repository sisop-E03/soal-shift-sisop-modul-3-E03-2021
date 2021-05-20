#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <wait.h>
#include <time.h>

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

void launch_server();
void* handle_new_connection(void *arg);
void handle_menu_request(int socketfd);
void handle_client_request(int socketfd, char user_auth[]);
char* handle_login(int socketfd);
void handle_register(int socketfd);
void handle_add_request(int socketfd, char user_auth[]);
void handle_upload_request(int socketfd, char filename[]);
void handle_download_request(int socketfd);
void handle_see_request(int socketfd);
void handle_delete_request(int socketfd, char user_auth[]);
void handle_find_request(int socketfd);
void extract_data(char buffer[], char filename[], char publisher[], char tahun_pub[]);
char* read_and_format_tsv_line(char line[], char buffer[]);
int is_file_path_exist_in_tsv(char file_path[]);
int is_filename_substring_of_tsv_line(char line[], char filename[]);
void delete_line_in_file_by_line_number(char file_name[], int line_number);
void extract_string_with_delimiter_and_index_pointer(char string[], char extraction[], char delimiter, int *index);
void clear_buffer(char* b);

int main(int argc, char const *argv[]) {
    launch_server();
    return 0;
}

void launch_server() {
    int server_fd, socketfd, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFSIZ] = {0};

    pthread_t tid[100];
    int connections=0;
      
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
      
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
      
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    struct stat st = {0};

    if (stat("FILES", &st) == -1) {
        mkdir("FILES", 0700);
    }

    while (1) {
        if ((socketfd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
            perror("accept failed");
            exit(EXIT_FAILURE);
        }

        pthread_create(&(tid[connections]), NULL, &handle_new_connection, &socketfd);
        pthread_join(tid[connections], NULL);
        connections++;
    }
}

void* handle_new_connection(void *arg) {
    char buffer[BUFSIZ];
    int valread;
    int socketfd = *(int *) arg;

    clear_buffer(buffer);
    valread = read(socketfd , buffer, BUFSIZ);
    fprintf(stdout, "Receive connection handshake %s\n", buffer);

    send(socketfd, SUCCESS, strlen(SUCCESS), 0);
    fprintf(stdout, "Sent response %s\n", SUCCESS);

    handle_menu_request(socketfd);
}

void handle_menu_request(int socketfd) {
    char buffer[BUFSIZ];
    int valread;

    while (1) {
        clear_buffer(buffer);
        valread = read(socketfd, buffer, BUFSIZ);
        
        send(socketfd, SUCCESS, strlen(SUCCESS), 0);

        if (strcmp(buffer, LOGIN_CODE) == 0) {
            char user_auth[50];
            strcpy(user_auth, handle_login(socketfd));
            if  (strcmp(user_auth, FAILED) != 0) {
                handle_client_request(socketfd, user_auth);
            };
        } else if (strcmp(buffer, REGISTER_CODE) == 0) {
            handle_register(socketfd);
        } else if (strcmp(buffer, END_CONN) == 0) {
            break;
        } else {
            perror("BAD REQUEST");
            exit(0);
        }
    }
}

void handle_client_request(int socketfd, char user_auth[]) {
    char buffer[BUFSIZ];
    int valread;
    
    while (1) {
        char request[10];

        clear_buffer(buffer);
        valread = read(socketfd, buffer, BUFSIZ);

        strcpy(request, buffer);

        if (strcmp(request, ADD_CODE) == 0) {
            handle_add_request(socketfd, user_auth);
        } else if (strcmp(request, DOWNLOAD_CODE) == 0) {
            handle_download_request(socketfd);
        } else if (strcmp(request, DELETE_CODE) == 0) {
            handle_delete_request(socketfd, user_auth);
        } else if (strcmp(request, SEE_CODE) == 0) {
            handle_see_request(socketfd);
        } else if (strcmp(request, FIND_CODE) == 0) {
            handle_find_request(socketfd);
        } else if (strcmp(request, LOGOUT_CODE) == 0) {
            send(socketfd, SUCCESS, strlen(SUCCESS), 0);
            break;
        } else {
            perror("BAD REQUEST");
            exit(0);
        }
    }
}

char* handle_login(int socketfd) {
    char *buffer = malloc(sizeof(char) * BUFSIZ);
    int valread;

    FILE *fp = fopen("akun.txt", "r");
    char valid_credential[BUFSIZ];
    int is_credential_valid = 0;
    
    clear_buffer(buffer);
    valread = read(socketfd, buffer, BUFSIZ);

    while (fscanf(fp, "%s", valid_credential) != EOF) {
        if (strcmp(buffer, valid_credential) == 0) {
            is_credential_valid = 1;
            break;
        }
    }
    
    fclose(fp);

    if (is_credential_valid) {
        send(socketfd, SUCCESS, strlen(SUCCESS), 0);
        return buffer;
    } else {
        send(socketfd, FAILED, strlen(FAILED), 0);
        return FAILED;
    }
}

void handle_register(int socketfd) {
    char buffer[BUFSIZ];
    int valread;

    FILE *fp;
    fp = fopen("akun.txt", "a+");

    clear_buffer(buffer);
    valread = read(socketfd, buffer, BUFSIZ);

    fprintf(fp, "%s\n", buffer);

    send(socketfd, SUCCESS, strlen(SUCCESS), 0);

    fclose(fp);
}

void handle_add_request(int socketfd, char user_auth[]) {
    char buffer[BUFSIZ];
    int valread;

    fprintf(stdout, "Enter handle add request\n");

    send(socketfd, SUCCESS, strlen(SUCCESS), 0);
    fprintf(stdout, "Sent status %s\n", SUCCESS);

    FILE *fp;
    fp = fopen("file.tsv", "a+");
    if (fp == NULL) {
        perror("open file failed");
        exit(EXIT_FAILURE);
    }
    
    clear_buffer(buffer);
    valread = read(socketfd, buffer, BUFSIZ);

    char filename[50];
    char publisher[50];
    char tahun_pub[10];

    extract_data(buffer, filename, publisher, tahun_pub);

    char file_path[100];
    sprintf(file_path, "FILES/%s", filename);

    handle_upload_request(socketfd, filename);

    fprintf(fp, "%s\t", file_path);
    fprintf(fp, "%s\t", publisher);
    fprintf(fp, "%s\n", tahun_pub);
    
    send(socketfd, SUCCESS, strlen(SUCCESS), 0);

    FILE *fp_log;
    fp_log = fopen("running.log", "a+");

    fprintf(fp_log, "Tambah: %s (%s)\n", filename, user_auth);

    fclose(fp_log);
    fclose(fp);
}

void handle_upload_request(int socketfd, char filename[]) {
    char buffer[BUFSIZ];
    int valread;

    int file_size;
    int remain_data;
    ssize_t len;
    FILE *received_file;

    send(socketfd, SUCCESS, strlen(SUCCESS), 0);

    clear_buffer(buffer);
    recv(socketfd, buffer, BUFSIZ, 0);
    file_size = atoi(buffer);

    fprintf(stdout, "Received file with size %d bytes\n", file_size);

    char file_path[100];
    sprintf(file_path, "FILES/%s", filename);

    received_file = fopen(file_path, "w");
    if (received_file == NULL) {
            perror("Failed to open file");
            exit(EXIT_FAILURE);
    }

    remain_data = file_size;

    while ((remain_data > 0) && ((len = recv(socketfd, buffer, BUFSIZ, 0)) > 0)) {
            fwrite(buffer, sizeof(char), len, received_file);
            remain_data -= len;
            fprintf(stdout, "Receive %ld bytes and %d bytes remaining\n", len, remain_data);
    }
    fclose(received_file);
}

void handle_download_request(int socketfd) {
    char buffer[BUFSIZ];
    int valread;

    int fd;
    int sent_bytes = 0;
    char file_size[256];
    struct stat file_stat;
    off_t offset;
    int remain_data;

    send(socketfd, SUCCESS, strlen(SUCCESS), 0);

    clear_buffer(buffer);
    valread = read(socketfd, buffer, BUFSIZ);

    char filename[50];
    strcpy(filename, buffer);

    char file_path[100];
    sprintf(file_path, "FILES/%s", filename);

    fprintf(stdout, "%s %ld\n", file_path, strlen(file_path));

    if (is_file_path_exist_in_tsv(file_path)) {
        send(socketfd, SUCCESS, strlen(SUCCESS), 0);
        
        clear_buffer(buffer);
        valread = read(socketfd, buffer, BUFSIZ);

        fd = open(file_path, O_RDONLY);

        if (fstat(fd, &file_stat) < 0) {
                perror("filestat error");
                exit(EXIT_FAILURE);
        }

        sprintf(file_size, "%ld", file_stat.st_size);

        send(socketfd, file_size, sizeof(file_size), 0);
        fprintf(stdout, "Sent file with size %s bytes\n", file_size);

        offset = 0;
        remain_data = file_stat.st_size;

        while (((sent_bytes = sendfile(socketfd, fd, &offset, BUFSIZ)) > 0) && (remain_data > 0)) {   
                remain_data -= sent_bytes;
                fprintf(stdout, "Sent %d bytes from file's data, offset is now %ld and remaining data %d bytes\n", sent_bytes, offset, remain_data);
        }
    } else {
        send(socketfd, FAILED, strlen(FAILED), 0);
    }
}

void handle_see_request(int socketfd) {
    char buffer[BUFSIZ];
    int valread;

    
    FILE *fp;
    fp = fopen("file.tsv", "r");
    char line[256];
    char formatted_data[BUFSIZ];
    formatted_data[0] = '\0';

    while (fgets(line, sizeof line, fp)) {
        read_and_format_tsv_line(line, formatted_data);
    }

    send(socketfd, formatted_data, strlen(formatted_data), 0);

    fclose(fp);
}

void handle_delete_request(int socketfd, char user_auth[]) {
    char buffer[BUFSIZ];
    int valread;

    send(socketfd, SUCCESS, strlen(SUCCESS), 0);

    clear_buffer(buffer);
    valread = read(socketfd, buffer, BUFSIZ);

    char filename_to_delete[50];
    strcpy(filename_to_delete, buffer);

    char line[256];
    char formatted_data[BUFSIZ];
    
    char filename[] = "file.tsv";
    FILE *fp = fopen(filename, "r");
    int line_counter = 0;

    formatted_data[0] = '\0';

    int is_file_exist = 0;

    while (fgets(line, sizeof line, fp)) {
        line_counter++;
        if (is_filename_substring_of_tsv_line(line, filename_to_delete)){
            char file_path[100];
            sprintf(file_path, "FILES/%s", filename_to_delete);

            char file_path_new[100];
            sprintf(file_path_new, "FILES/old-%s", filename_to_delete);

            delete_line_in_file_by_line_number(filename, line_counter);

            rename(file_path, file_path_new);
            is_file_exist = 1;
            break;      
        }
    }

    FILE *fp_log;
    fp_log = fopen("running.log", "a+");

    if (is_file_exist) {
        fprintf(fp_log, "Hapus: %s (%s)\n", filename_to_delete, user_auth);

        send(socketfd, SUCCESS, strlen(SUCCESS), 0);
    } else {
        send(socketfd, FAILED, strlen(FAILED), 0);
    }

    fclose(fp);
    fclose(fp_log);
}

void handle_find_request(int socketfd) {
    char buffer[BUFSIZ];
    int valread;

    send(socketfd, SUCCESS, strlen(SUCCESS), 0);

    clear_buffer(buffer);
    valread = read(socketfd, buffer, BUFSIZ);

    char filename_to_find[50];
    strcpy(filename_to_find, buffer);

    FILE *fp;
    fp = fopen("file.tsv", "r");
    char line[256];
    char formatted_data[BUFSIZ];
    formatted_data[0] = '\0';

    int is_file_exist = 0;

    while (fgets(line, sizeof line, fp) != NULL) {
        if (is_filename_substring_of_tsv_line(line, filename_to_find)) {
            read_and_format_tsv_line(line, formatted_data);
            is_file_exist = 1;
        }
    }

    if (is_file_exist)
        send(socketfd, formatted_data, strlen(formatted_data), 0);
    else 
        send(socketfd, FAILED, strlen(FAILED), 0);

    fclose(fp);
}

void extract_data(char buffer[], char filename[], char publisher[], char tahun_pub[]) {
    int index = 0;
    int len = strlen(buffer);

    extract_string_with_delimiter_and_index_pointer(buffer, filename, ':', &index);

    extract_string_with_delimiter_and_index_pointer(buffer, publisher, ':', &index);

    extract_string_with_delimiter_and_index_pointer(buffer, tahun_pub, ':', &index);
}

char* read_and_format_tsv_line(char line[], char buffer[]) {
    int idx;
    char name[20];
    char publisher[50];
    char tahun_pub[10];
    char ext_file[10];
    char folder_path[20];
    char file_path[50];

    idx = 0;
    extract_string_with_delimiter_and_index_pointer(line, folder_path, '/', &idx);

    extract_string_with_delimiter_and_index_pointer(line, name, '.', &idx);

    extract_string_with_delimiter_and_index_pointer(line, ext_file, '\t', &idx);

    sprintf(file_path, "%s/%s.%s", folder_path, name, ext_file);

    extract_string_with_delimiter_and_index_pointer(line, publisher, '\t', &idx);

    extract_string_with_delimiter_and_index_pointer(line, tahun_pub, '\n', &idx);

    char formatted_data[256];

    sprintf(formatted_data, "Nama: %s\nPublisher: %s\nTahun publishing: %s\nEkstensi file: %s\nFilepath: %s\n\n", name, publisher, tahun_pub, ext_file, file_path);\

    strcat(buffer, formatted_data);

    return buffer;
}

int is_file_path_exist_in_tsv(char file_path[]) {

    int is_file_exist = 0;
    char line[50];
    char file_path_exist[20];
    char filename[] = "file.tsv";
    FILE *fp = fopen(filename, "r");

    while (fgets(line, sizeof line, fp)) {
        int index = 0;
        extract_string_with_delimiter_and_index_pointer(line, file_path_exist, '\t', &index);
        if (strcmp(file_path_exist, file_path) == 0){
            is_file_exist = 1;
            break;
        }
    }

    if (is_file_exist)
        return 1;
    else 
        return 0;
}

int is_filename_substring_of_tsv_line(char line[], char filename[]) {
    char filename_exist[50];
    char delimiter = '\t';
    int idx = 0;
    extract_string_with_delimiter_and_index_pointer(line, filename_exist, delimiter, &idx);
    if (strstr(filename_exist, filename))
        return 1;
    else 
        return 0;
}

void delete_line_in_file_by_line_number(char file_name[], int line_number) {
    FILE *fp, *fp_temp;
    char str[BUFSIZ];
    char file_name_temp[] = "temp_file.txt";
    int counter = 0;
    char ch;

    fp = fopen(file_name, "r");
    fp_temp = fopen(file_name_temp, "w"); 

    while (!feof(fp)) 
    {
        strcpy(str, "\0");
        fgets(str, BUFSIZ, fp);
        if (!feof(fp)) 
        {
            counter++;
            if (counter != line_number) 
            {
                fprintf(fp_temp, "%s", str);
            }
        }
    }
    fclose(fp);
    fclose(fp_temp);
    remove(file_name);
    rename(file_name_temp, file_name);
}

void extract_string_with_delimiter_and_index_pointer(char string[], char extraction[], char delimiter, int *index) {
    int extraction_index = 0;
    while(*index < strlen(string) && string[*index] != delimiter) {
        extraction[extraction_index] = string[*index];
        *index += 1;
        extraction_index++;
    }
    extraction[extraction_index] = '\0';
    *index += 1;
}

void clear_buffer(char* b)
{
    int i;
    for (i = 0; i < BUFSIZ; i++)
        b[i] = '\0';
}
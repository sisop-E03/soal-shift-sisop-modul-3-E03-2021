#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
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

void swap(char *a, char *b) {
    char c = *a;
    *a = *b;
    *b = c;
}

void reverse_string(char string[])
{ 
    int len = strlen(string);
    for (int i = 0; i < len / 2; i++)
        swap(&string[i], &string[len-i-1]);
}

void extract_string_with_delimiter_and_index_pointer(char string[], char extraction[], char delimiter, int *idx) {
    int sec_idx = 0;
    while(*idx < strlen(string) && string[*idx] != delimiter) {
        extraction[sec_idx] = string[*idx];
        *idx += 1;
        sec_idx++;
    }
    extraction[sec_idx] = '\0';
    *idx += 1;
}

/* Extract data from buffer to all corresponding variable */
void extract_data(
    char buffer[], 
    char publisher[], 
    char tahun_pub[],
    char file_path[],
    char name[],
    char ext_file[]
) {
    int idx = 0;
    int len = strlen(buffer);

    extract_string_with_delimiter_and_index_pointer(buffer, publisher, ':', &idx);

    extract_string_with_delimiter_and_index_pointer(buffer, tahun_pub, ':', &idx);

    extract_string_with_delimiter_and_index_pointer(buffer, file_path, '\n', &idx);

    char file_path_rev[100];
    strcpy(file_path_rev, file_path);
    reverse_string(file_path_rev);

    idx = 0;
    extract_string_with_delimiter_and_index_pointer(file_path_rev, ext_file, '.', &idx);
    reverse_string(ext_file);

    extract_string_with_delimiter_and_index_pointer(file_path_rev, name, '/', &idx);
    reverse_string(name);
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

char* read_and_format_tsv_line(char line[], char buffer[]) {
    int idx;
    char name[50];
    char publisher[50];
    char tahun_pub[10];
    char ext_file[10];
    char file_path[50];
    char delimiter = '\t';

    idx = 0;
    strcat(buffer, "Nama: ");
    extract_string_with_delimiter_and_index_pointer(line, name, delimiter, &idx);
    strcat(buffer, name);

    strcat(buffer, "\nPublisher: ");
    extract_string_with_delimiter_and_index_pointer(line, publisher, delimiter, &idx);
    strcat(buffer, publisher);

    strcat(buffer, "\nTahun publishing: ");
    extract_string_with_delimiter_and_index_pointer(line, tahun_pub, delimiter, &idx);
    strcat(buffer, tahun_pub);

    strcat(buffer, "\nEkstensi File: ");
    extract_string_with_delimiter_and_index_pointer(line, ext_file, delimiter, &idx);
    strcat(buffer, ext_file);

    strcat(buffer, "\nFilepath: ");
    extract_string_with_delimiter_and_index_pointer(line, file_path, delimiter, &idx);
    strcat(buffer, file_path);
    strcat(buffer, "\n\n");

    fprintf(stdout, "%s", buffer);

    return buffer;
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

int handle_login(char buffer[], int socketfd, int valread) {
    FILE *fp = fopen("akun.txt", "r");
    char valid_credential[BUFSIZ];
    int is_credential_valid = 0;
    
    clear_buffer(buffer);
    valread = read(socketfd, buffer, BUFSIZ);
    
    fprintf(stdout, "Receive authentication data: %s\n", buffer);

    while (fscanf(fp, "%s", valid_credential) != EOF) {
        if (strstr(buffer, valid_credential)) {
            is_credential_valid = 1;
            break;
        }
    }
    
    fclose(fp);

    if (is_credential_valid) {
        send(socketfd, SUCCESS_CODE, strlen(SUCCESS_CODE), 0);
        fprintf(stdout, "Sent login success code\n");
        return 1;
    } else {
        send(socketfd, FAILED_CODE, strlen(FAILED_CODE), 0);
        fprintf(stdout, "Sent login failed code code\n");
        return 0;
    }
}

void handle_register(char buffer[], int socketfd, int valread) {
    FILE *fp;
    fp = fopen("akun.txt", "a+");

    clear_buffer(buffer);
    valread = read(socketfd, buffer, BUFSIZ);
    fprintf(stdout, "Receive registration data: %s\n", buffer);

    fprintf(fp, "%s\n", buffer);
    fprintf(stdout, "Registration data saved to akun.txt\n");

    send(socketfd, SUCCESS_CODE, strlen(SUCCESS_CODE), 0);
    fprintf(stdout, "Sent registration success code\n");

    fclose(fp);
}

void handle_upload_request(char buffer[], int socketfd, int valread, char filename[]) {
    int file_size;
    int remain_data;
    ssize_t len;
    FILE *received_file;

    send(socketfd, OK, strlen(OK), 0);
    fprintf(stdout, "Sent status %s\n", OK);

    clear_buffer(buffer);
    recv(socketfd, buffer, BUFSIZ, 0);
    file_size = atoi(buffer);

    fprintf(stdout, "Received file size : %d\n", file_size);

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
            fprintf(stdout, "%s\n", buffer);
            remain_data -= len;
            fprintf(stdout, "Receive %ld bytes and %d bytes remaining\n", len, remain_data);
    }
    fclose(received_file);
}

void handle_add_request(char buffer[], int socketfd, int valread) {
    fprintf(stdout, "Enter handle add request\n");

    send(socketfd, OK, strlen(OK), 0);
    fprintf(stdout, "Sent status %s\n", OK);

    FILE *fp;
    fp = fopen("file.tsv", "a+");
    if (fp == NULL) {
        perror("open file failed");
        exit(EXIT_FAILURE);
    }
    
    clear_buffer(buffer);
    valread = read(socketfd, buffer, BUFSIZ);
    fprintf(stdout, "Receive new book data: %s\n", buffer);

    char publisher[50];
    char tahun_pub[10];
    char file_path[50];
    char name[50];
    char ext_file[10];

    extract_data(buffer, publisher, tahun_pub, file_path, name, ext_file);

    char filename[100];
    sprintf(filename, "%s.%s", name, ext_file);

    handle_upload_request(buffer, socketfd, valread, filename);

    fprintf(stdout, "buffer: %s\n", buffer);
    fprintf(stdout, "publisher: %s\n", publisher);
    fprintf(stdout, "tahun_pub: %s\n", tahun_pub);
    fprintf(stdout, "file_path: %s\n", file_path);
    fprintf(stdout, "name: %s\n", name);
    fprintf(stdout, "ext_file: %s\n", ext_file);

    fprintf(fp, "%s\t", name);
    fprintf(fp, "%s\t", publisher);
    fprintf(fp, "%s\t", tahun_pub);
    fprintf(fp, "%s\t", ext_file);
    fprintf(fp, "%s\n", file_path);
    
    send(socketfd, SUCCESS_CODE, strlen(SUCCESS_CODE), 0);
    fprintf(stdout, "Sent login success code\n");

    FILE *fp_log;
    fp_log = fopen("running.log", "a+");

    fprintf(fp_log, "Tambah: %s\n", filename);

    fclose(fp_log);
    fclose(fp);

    fprintf(stdout, "Leaving handle add request\n");
}

void handle_download_request(char buffer[], int socketfd, int valread) {
    int fd;
    int sent_bytes = 0;
    char file_size[256];
    struct stat file_stat;
    off_t offset;
    int remain_data;

    send(socketfd, OK, strlen(OK), 0);
    fprintf(stdout, "Sent status %s\n", OK);

    clear_buffer(buffer);
    valread = read(socketfd, buffer, BUFSIZ);
    fprintf(stdout, "Receive filename to delete %s\n", buffer);
    char file_name[50];
    strcpy(file_name, buffer);
    char file_path[100];
    sprintf(file_path, "FILES/%s", file_name);
    fd = open(file_path, O_RDONLY);

    if (fstat(fd, &file_stat) < 0)
    {
            perror("filestat error");

            exit(EXIT_FAILURE);
    }

    sprintf(file_size, "%ld", file_stat.st_size);

    send(socketfd, file_size, sizeof(file_size), 0);
    fprintf(stdout, "Sent file size %s bytes\n", file_size);

    offset = 0;
    remain_data = file_stat.st_size;

    while (((sent_bytes = sendfile(socketfd, fd, &offset, BUFSIZ)) > 0) && (remain_data > 0))
    {
            fprintf(stdout, "1. Server sent %d bytes from file's data, offset is now : %ld and remaining data = %d\n", sent_bytes, offset, remain_data);
            remain_data -= sent_bytes;
            fprintf(stdout, "2. Server sent %d bytes from file's data, offset is now : %ld and remaining data = %d\n", sent_bytes, offset, remain_data);
    }
}

void handle_see_request(char buffer[], int socketfd, int valread) {
    
    FILE *fp;
    fp = fopen("file.tsv", "r");
    char line[256];
    char formatted_data[BUFSIZ];
    formatted_data[0] = '\0';

    while (fgets(line, sizeof line, fp)) {
        read_and_format_tsv_line(line, formatted_data);
    }

    send(socketfd, formatted_data, strlen(formatted_data), 0);
    fprintf(stdout, "Sent formatted data from tsv file\n");

    fclose(fp);
}

void handle_delete_request(char buffer[], int socketfd, int valread) {
    
    send(socketfd, OK, strlen(OK), 0);
    fprintf(stdout, "Sent status %s\n", OK);

    clear_buffer(buffer);
    valread = read(socketfd, buffer, BUFSIZ);
    fprintf(stdout, "Receive filename to delete %s\n", buffer);

    char filename_to_delete[50];
    strcpy(filename_to_delete, buffer);

    char line[256];
    char formatted_data[BUFSIZ];
    
    char filename[] = "file.tsv";
    FILE *fp = fopen(filename, "r");
    int line_counter = 0;

    formatted_data[0] = '\0';

    while (fgets(line, sizeof line, fp)) {
        line_counter++;
        if (is_filename_substring_of_tsv_line(line, filename_to_delete)){
            char file_path[100];
            sprintf(file_path, "FILES/%s", filename_to_delete);
            char file_path_new[100];
            sprintf(file_path_new, "FILES/old-%s", filename_to_delete);
            rename(file_path, file_path_new);
            break;      
        }
    }

    fclose(fp);

    delete_line_in_file_by_line_number(filename, line_counter);

    FILE *fp_log;
    fp_log = fopen("running.log", "a+");

    fprintf(fp_log, "Hapus: %s\n", filename_to_delete);

    fclose(fp_log);

    send(socketfd, OK, strlen(OK), 0);
    fprintf(stdout, "Sent status %s\n", OK);
}

void handle_find_request(char buffer[], int socketfd, int valread) {

    send(socketfd, OK, strlen(OK), 0);
    fprintf(stdout, "Sent status %s\n", OK);

    clear_buffer(buffer);
    valread = read(socketfd, buffer, BUFSIZ);
    fprintf(stdout, "Receive filename to find %s\n", buffer);

    char filename_to_find[50];
    strcpy(filename_to_find, buffer);

    FILE *fp;
    fp = fopen("file.tsv", "r");
    char line[256];
    char formatted_data[BUFSIZ];
    formatted_data[0] = '\0';

    while (fgets(line, sizeof line, fp)) {
        if (is_filename_substring_of_tsv_line(line, filename_to_find))
            read_and_format_tsv_line(line, formatted_data);
    }

    send(socketfd, formatted_data, strlen(formatted_data), 0);
    fprintf(stdout, "Sent formatted data from tsv file\n");

    fclose(fp);
}

void handle_client_request(char buffer[], int socketfd, int valread) {
    fprintf(stdout, "Enter handle client request\n");
    while (1) {
        char request[10];

        clear_buffer(buffer);
        valread = read(socketfd, buffer, BUFSIZ);
        fprintf(stdout, "Receive request code: %s\n", buffer);

        strcpy(request, buffer);

        if (strcmp(request, ADD_CODE) == 0) {
            handle_add_request(buffer, socketfd, valread);
        } else if (strcmp(request, DONWLOAD_CODE) == 0) {
            handle_download_request(buffer, socketfd, valread);
        } else if (strcmp(request, DELETE_CODE) == 0) {
            handle_delete_request(buffer, socketfd, valread);
        } else if (strcmp(request, SEE_CODE) == 0) {
            handle_see_request(buffer, socketfd, valread);
        } else if (strcmp(request, FIND_CODE) == 0) {
            handle_find_request(buffer, socketfd, valread);
        } else if (strcmp(request, LOGOUT_CODE) == 0) {
            
            send(socketfd, OK, strlen(OK), 0);
            fprintf(stdout, "Sent ok code %s\n", OK);
            
            break;
        } else {
            perror("BAD REQUEST");
            exit(0);
        }
    }
    fprintf(stdout, "Leaving handle client request\n");
}

int main(int argc, char const *argv[]) {

    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFSIZ] = {0};
      
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

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }

    while (1) {
        clear_buffer(buffer);
        valread = read(new_socket, buffer, BUFSIZ);

        fprintf(stdout, "Receive %s code\n", buffer);
        
        send(new_socket, SUCCESS_CODE, strlen(SUCCESS_CODE), 0);
        fprintf(stdout, "Sent success status code\n");

        if (strcmp(buffer, LOGIN_CODE) == 0) {
            if  (handle_login(buffer, new_socket, valread)) {
                handle_client_request(buffer, new_socket, valread);
            };
        } else if (strcmp(buffer, REGISTER_CODE) == 0) {
            handle_register(buffer, new_socket, valread);
        } else {
            perror("BAD REQUEST");
            exit(0);
        }
    }
    return 0;
}
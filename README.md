# soal-shift-sisop-modul-3-E03-2021

## Soal 1
### Soal dan Penyelesaian
Diminta membuat program client dan server yang dapat memenuhi operasi berikut ini:

A. Pada saat client tersambung dengan server, terdapat dua pilihan pertama, yaitu register dan login. Jika memilih register, client akan diminta input id dan passwordnya untuk dikirimkan ke server. User juga dapat melakukan login. Login berhasil jika id dan password yang dikirim dari aplikasi client sesuai dengan list akun yang ada didalam aplikasi server. Sistem ini juga dapat menerima multi-connections. Koneksi terhitung ketika aplikasi client tersambung dengan server. Jika terdapat 2 koneksi atau lebih maka harus menunggu sampai client pertama keluar untuk bisa melakukan login dan mengakses aplikasinya. Keverk menginginkan lokasi penyimpanan id dan password pada file bernama akun.txt

Client:
```c

int handle_login(int socketfd) {
    ...
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
    ...
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
```
Fungsi login dan register pada client menginput data username dan password lalu menggabungkan keduanya dan dikirimkan ke server untuk diproses.

Server:
```c
    ...
    while (1) {
        if ((socketfd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
            perror("accept failed");
            exit(EXIT_FAILURE);
        }

        pthread_create(&(tid[connections]), NULL, &handle_new_connection, &socketfd);
        pthread_join(tid[connections], NULL);
        connections++;
    }
    ...
    
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
```
Agar server dapat menghandle banyak connections maka dibuat thread untuk menghandle setiap koneksi. Agar client menunggu jika ada client lain yang sedang terkoneksi maka digunakan fungsi pthread_join untuk menunggu thread sampai selesai baru membuat thread baru lagi untuk membuat koneksi dengan client baru.
Fungsi login pada server mengecek apakah user:password yang dikirimkan oleh client ada pada akun.txt
Fungsi register pada server langsung menyimpan data user:password yang dikirimkan oleh clientke akun.txt



B. Sistem memiliki sebuah database yang bernama files.tsv. Isi dari files.tsv ini adalah path file saat berada di server, publisher, dan tahun publikasi. Setiap penambahan dan penghapusan file pada folder file yang bernama  FILES pada server akan memengaruhi isi dari files.tsv. Folder FILES otomatis dibuat saat server dijalankan. 

Client:
```c
void handle_add_command(int socketfd) {
    ...
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
```
Fungsi add pada client menginput data buku dari user dan mengirimkannya ke server dengan format dipisah dengan karakter ':'. Lalu file yang bersesuaian dikriimkan dengan fungsi upload.

Server:
```c
void handle_add_request(int socketfd, char user_auth[]) {
    ...
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
```
Fungsi add pada server menerima data buku dari server lalu mengekstrak data tersebut ke variabel-variabel yang bersesuaian dan dimasukkan ke file.tsv. Setelah itu dilakukan penerimaan untuk upload request dari client. Terakhir dilakukan update log pada file running.log dengan informasi berupa operasi tambah, nama file, dan data user yang sedang aktif.


C. Client dapat menambah file baru ke dalam server. Kemudian, dari aplikasi client akan dimasukan data buku tersebut (perlu diingat bahwa Filepath ini merupakan path file yang akan dikirim ke server). Lalu client nanti akan melakukan pengiriman file ke aplikasi server dengan menggunakan socket. Ketika file diterima di server, maka row dari files.tsv akan bertambah sesuai dengan data terbaru yang ditambahkan.

D. Client dapat mendownload file yang telah ada dalam folder FILES di server, sehingga sistem harus dapat mengirim file ke client. Server harus melihat dari files.tsv untuk melakukan pengecekan apakah file tersebut valid. Jika tidak valid, maka mengirimkan pesan error balik ke client. Jika berhasil, file akan dikirim dan akan diterima ke client di folder client tersebut. 

E. Setelah itu, client juga dapat menghapus file yang tersimpan di server. Akan tetapi, Keverk takut file yang dibuang adalah file yang penting, maka file hanya akan diganti namanya menjadi ‘old-NamaFile.ekstensi’. Ketika file telah diubah namanya, maka row dari file tersebut di file.tsv akan terhapus.

F. Client dapat melihat semua isi files.tsv dengan memanggil suatu perintah yang bernama see

G. Aplikasi client juga dapat melakukan pencarian dengan memberikan suatu string. Hasilnya adalah semua nama file yang mengandung string tersebut. Format output seperti format output f.

H. Dikarenakan Keverk waspada dengan pertambahan dan penghapusan file di server, maka Keverk membuat suatu log untuk server yang bernama running.log. Contoh isi dari log ini adalah

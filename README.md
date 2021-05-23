# soal-shift-sisop-modul-3-E03-2021

## Soal 1
### Soal dan Penyelesaian
Diminta membuat program client dan server yang dapat memenuhi operasi berikut ini:

**A.** Pada saat client tersambung dengan server, terdapat dua pilihan pertama, yaitu register dan login. Jika memilih register, client akan diminta input id dan passwordnya untuk dikirimkan ke server. User juga dapat melakukan login. Login berhasil jika id dan password yang dikirim dari aplikasi client sesuai dengan list akun yang ada didalam aplikasi server. Sistem ini juga dapat menerima multi-connections. Koneksi terhitung ketika aplikasi client tersambung dengan server. Jika terdapat 2 koneksi atau lebih maka harus menunggu sampai client pertama keluar untuk bisa melakukan login dan mengakses aplikasinya. Keverk menginginkan lokasi penyimpanan id dan password pada file bernama akun.txt

**Client:**
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

**Server:**
```c    
char* handle_login(int socketfd) {
    ...
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
    ...
    fp = fopen("akun.txt", "a+");

    clear_buffer(buffer);
    valread = read(socketfd, buffer, BUFSIZ);

    fprintf(fp, "%s\n", buffer);
    ...
}
```
Fungsi login pada server mengecek apakah user:password yang dikirimkan oleh client ada pada akun.txt
Fungsi register pada server langsung menyimpan data user:password yang dikirimkan oleh clientke akun.txt
**
```c
void launch_server() {
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
}
```
Agar server dapat menghandle banyak connections maka dibuat thread untuk menghandle setiap koneksi. Agar client menunggu jika ada client lain yang sedang terkoneksi maka digunakan fungsi pthread_join untuk menunggu thread sampai selesai baru membuat thread baru lagi untuk membuat koneksi dengan client baru.

![alt text](https://github.com/sisop-E03/soal-shift-sisop-modul-3-E03-2021/blob/master/images/soal1/a1.png)
![alt text](https://github.com/sisop-E03/soal-shift-sisop-modul-3-E03-2021/blob/master/images/soal1/a2.png)

**B.** Sistem memiliki sebuah database yang bernama files.tsv. Isi dari files.tsv ini adalah path file saat berada di server, publisher, dan tahun publikasi. Setiap penambahan dan penghapusan file pada folder file yang bernama  FILES pada server akan memengaruhi isi dari files.tsv. Folder FILES otomatis dibuat saat server dijalankan. 

**Server**
```c
void handle_add_request(int socketfd, char user_auth[]) {
    ...
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

    fprintf(fp, "%s\t", file_path);
    fprintf(fp, "%s\t", publisher);
    fprintf(fp, "%s\n", tahun_pub);
    ...
}
```
Fungsi add pada server menerima data buku dari server lalu mengekstrak data tersebut ke variabel-variabel yang bersesuaian dan dimasukkan ke file.tsv.

![alt text](https://github.com/sisop-E03/soal-shift-sisop-modul-3-E03-2021/blob/master/images/soal1/c2.png)

**C.** Client dapat menambah file baru ke dalam server. Kemudian, dari aplikasi client akan dimasukan data buku tersebut (perlu diingat bahwa Filepath ini merupakan path file yang akan dikirim ke server). Lalu client nanti akan melakukan pengiriman file ke aplikasi server dengan menggunakan socket. Ketika file diterima di server, maka row dari files.tsv akan bertambah sesuai dengan data terbaru yang ditambahkan.

**Client:**
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

    ...
}
```
Fungsi add pada client menginput data buku dari user dan mengirimkannya ke server dengan format dipisah dengan karakter ':'. Lalu file yang bersesuaian dikirimkan melalui fungsi upload.

**Server:**
```c
void handle_add_request(int socketfd, char user_auth[]) {
    ...

    send(socketfd, SUCCESS, strlen(SUCCESS), 0);
    fprintf(stdout, "Sent status %s\n", SUCCESS);
    
    ...
    /* Extract and concat data to file.tsv*/
    ...

    handle_upload_request(socketfd, filename);
    
    send(socketfd, SUCCESS, strlen(SUCCESS), 0);
    ...
```
Fungsi add pada server menerima data buku dari server lalu mengekstrak data tersebut ke variabel-variabel yang bersesuaian dan dimasukkan ke file.tsv seperti pada **soal 1a**. Setelah itu dilakukan penerimaan untuk upload request dari client pada fungsi upload.

![alt text](https://github.com/sisop-E03/soal-shift-sisop-modul-3-E03-2021/blob/master/images/soal1/c1.png)
![alt text](https://github.com/sisop-E03/soal-shift-sisop-modul-3-E03-2021/blob/master/images/soal1/c2.png)

**D.** Client dapat mendownload file yang telah ada dalam folder FILES di server, sehingga sistem harus dapat mengirim file ke client. Server harus melihat dari files.tsv untuk melakukan pengecekan apakah file tersebut valid. Jika tidak valid, maka mengirimkan pesan error balik ke client. Jika berhasil, file akan dikirim dan akan diterima ke client di folder client tersebut.

**Client**
```c
void handle_download_command(int socketfd) {
    ...
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
```
Pada fungsi update, client pertama akan meminta input nama file yang ingin didownload. Jika nama file tersebut ada pada database server, maka client pertama akan menerima ukuran dari file yang ingin didownload. Selanjutnya client akan menerima data file paket per paket dengan looping.

**Server**
```c
void handle_download_request(int socketfd) {
    ...
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
```
Pada fungsi downlaod, server pertama akan mengecek apakah file yang diminta client ada pada database file.tsv. Jika ada maka pertama akan dikirimkan data ukuran file yang ditransfer. Setelah itu file tersebut akan dikirimkan secara paket per paket untuk mengatasi jika file yang akan ditransfer besar.

![alt text](https://github.com/sisop-E03/soal-shift-sisop-modul-3-E03-2021/blob/master/images/soal1/d1.png)

**E.** Setelah itu, client juga dapat menghapus file yang tersimpan di server. Akan tetapi, Keverk takut file yang dibuang adalah file yang penting, maka file hanya akan diganti namanya menjadi ‘old-NamaFile.ekstensi’. Ketika file telah diubah namanya, maka row dari file tersebut di file.tsv akan terhapus.

**Client**
```c
void handle_delete_command(int socketfd) {
    ...
    char filename[50];
    fscanf(stdin, "%s", filename);

    send(socketfd, filename, strlen(filename), 0);
    ...
}
```
Pada fungsi delete, client akan meminta input dari user nama file yang ingin dihapus, dan nama tersebut dikirim ke server.

**Server**
```c
void handle_delete_request(int socketfd, char user_auth[]) {
   ...
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
    ...
}
```
Pada fungsi delete, server akan menerima nama file yang akan hapus. Selanjutnya server akan mengecek apakah file yang diminta tersebut ada pada database file.tsv, jika ada maka semua data pada file.tsv kecuali pada baris yang ingin dihapus akan dicopy ke sebuah file temporary baru. Selanjutnya file.tsv diahapus dan file termporary baru diganti nama menjadi file.tsv

![alt text](https://github.com/sisop-E03/soal-shift-sisop-modul-3-E03-2021/blob/master/images/soal1/e1.png)
![alt text](https://github.com/sisop-E03/soal-shift-sisop-modul-3-E03-2021/blob/master/images/soal1/e2.png)

**F.** Client dapat melihat semua isi files.tsv dengan memanggil suatu perintah yang bernama see

**Client**
```c
void handle_see_command(int socketfd) {
    ...    
    send(socketfd, SEE_CODE, strlen(SEE_CODE), 0);

    clear_buffer(buffer);
    valread = read(socketfd, buffer, BUFSIZ);
    fprintf(stdout, "%s", buffer);
}
```
Pada fungsi see, client mengirim SEE_CODE dan langsung menampilkan respon dari server

**Server**
```c
void handle_see_request(int socketfd) {
    ...
    while (fgets(line, sizeof line, fp)) {
        read_and_format_tsv_line(line, formatted_data);
    }

    send(socketfd, formatted_data, strlen(formatted_data), 0);
    ...
}
```
Pada fungsi see, setelah server menerima SEE_CODE maka akan membaca isi dari file.tsv dan memformat isi tersebut sesuai requeirement soal.

![alt text](https://github.com/sisop-E03/soal-shift-sisop-modul-3-E03-2021/blob/master/images/soal1/f1.png)

**G.** Aplikasi client juga dapat melakukan pencarian dengan memberikan suatu string. Hasilnya adalah semua nama file yang mengandung string tersebut. Format output seperti format output f.

**Client**
```c
void handle_find_command(int socketfd) {
    ...
    send(socketfd, FIND_CODE, strlen(FIND_CODE), 0);

    clear_buffer(buffer);
    valread = read(socketfd, buffer, BUFSIZ);

    char filename_to_find[50];
    fscanf(stdin, "%s", filename_to_find);

    send(socketfd, filename_to_find, strlen(filename_to_find), 0);

    clear_buffer(buffer);
    valread = read(socketfd, buffer, BUFSIZ);
    if (strlen(buffer) > 0)
        fprintf(stdout, "%s", buffer);
    else 
        fprintf(stdout, "No file exist with substring %s\n", filename_to_find);
}
```
Pada fungsi find, client akan mengirim substring nama file yang ingin dicari pada server. Lalu jika ada respon dari server maka akan ditampilkan isi respon server tersebut.

**Server**
```c
void handle_find_request(int socketfd) {
    ...
    send(socketfd, SUCCESS, strlen(SUCCESS), 0);

    clear_buffer(buffer);
    valread = read(socketfd, buffer, BUFSIZ);

    char filename_to_find[50];
    strcpy(filename_to_find, buffer);
    
    fp = fopen("file.tsv", "r");

    while (fgets(line, sizeof line, fp) != NULL) {
        if (is_filename_substring_of_tsv_line(line, filename_to_find))
            read_and_format_tsv_line(line, formatted_data);
    }

    send(socketfd, formatted_data, strlen(formatted_data), 0);

    fclose(fp);
}
```
Pada fungsi find, server akan menerima string sebagai substring nama file yang ingin dicari. Setelah itu server akan membaca isi dari file.tsv dan mengecek pada setiap baris apakah mengandung substring yang dicari client. Jika ada maka data pada baris tersebut akan diformat dan ditambahkan pada formatted data yang nantinya akan dikirimkan ke client untuk ditampilkan.

![alt text](https://github.com/sisop-E03/soal-shift-sisop-modul-3-E03-2021/blob/master/images/soal1/g1.png)

**H.** Dikarenakan Keverk waspada dengan pertambahan dan penghapusan file di server, maka Keverk membuat suatu log untuk server yang bernama running.log. Contoh isi dari log ini adalah

**Server**
```c
void handle_add_request(int socketfd, char user_auth[]) {
    ...
    FILE *fp_log;
    fp_log = fopen("running.log", "a+");

    fprintf(fp_log, "Tambah: %s (%s)\n", filename, user_auth);

    fclose(fp_log);
    fclose(fp);
}
```
Jika ada penambahan atau penghapusan file akan ditambahkan log pada runnning.log yang menyimpan jenis operasi, nama file, dan akun yang sedang aktif.

![alt text](https://github.com/sisop-E03/soal-shift-sisop-modul-3-E03-2021/blob/master/images/soal1/h1.png)

### Kendala Pengerjaan
1. Karena operasi dari soal nomor 1 cukup banyak, pada awal-awal pembuatan code sempat kesulitan untuk menentukan bagaimana harus menstrukturisasi dari codenya agar mudah dibaca dan dikembangkan
2. Sering berkendala karena antara send dan read pada client dan server tidak singkron
3. Sempat berkendala karena setiap read ke tidak membersihkan buffer terlebih dahulu

## Soal 2
### Soal dan Penyelesaian
Pada soal no 2 ini, diminta untuk membuat tiga program berbeda, yaitu:

**A**
Membuat program perkalian matrix 4x3 dengan 3x6 dan menampilkan hasilnya. Matriks akan berisi angka 1-20 dan diinputkan oleh user. Hasil dari program ini nantinya akan digunakan di program 2B sehingga perlu menggunakan shared memory.

```c
int mat1[4][3];
int mat2[3][6];
int (*mat3)[6];
key_t key = 1234;
int i;
int j;
int k;
```
Pertama, variabel-variabel yang akan dipakai dideklarasikan. `mat1` dan `mat2` akan menjadi tempat matriks yang berasal dari input user. `mat3` akan menjadi tempat hasil perkalian matriks. `key` adalah nilai yang digunakan sebagai key ketika membuat shared memory. `i`, `j`, `k` akan digunakan dalam looping.

```c
int shmid = shmget(key, sizeof(int[4][6]), IPC_CREAT | 0666);
if (shmid == -1)
{
    fprintf(stderr, "shmget() Failed");
    return;
}

mat3 = shmat(shmid, NULL, 0);
if(mat3 == (void *)-1) {
    fprintf(stderr, "shmat() Failed" ); 
    return;
}
```
Kemudian shared memory dibuat dan di-attach ke `mat3`. Apabila terjadi error dari salah satu proses sebelumnya, maka program langsung dihentikan.

```c
printf("Input matrix 4x3\n");
for (i = 0; i < 4; i++)
{
    for (j = 0; j < 3; j++)
    {
        scanf("%d", &mat1[i][j]);
    }
}
printf("Input matrix 3x6\n");
for (i = 0; i < 3; i++)
{
    for (j = 0; j < 6; j++)
    {
        scanf("%d", &mat2[i][j]);
    }
}
```
Nilai-nilai `mat1` dan `mat2` diisikan dengan nilai dari input user.

```c
for (i = 0; i < 4; i++)
{
    for (j = 0; j < 6; j++)
    {
        mat3[i][j] = 0;

        for (k = 0; k < 3; k++)
        {
            mat3[i][j] += mat1[i][k] * mat2[k][j];
        }
    }
}
```
Kemudian dilakukan perhitungan hasil perkalian matriks `mat1` dan `mat2`. Hasil perhitungan tersebut disimpan di `mat3`.

```c
printf("Result\n");
for (i = 0; i < 4; i++)
{
    for (j = 0; j < 6; j++)
    {
        printf("%d ", mat3[i][j]);
    }
    printf("\n");
}
```
Hasil perkalian matriks ditampilkan di terminal.

```c
char ch;
do
{
    printf("Type c to close\n");
    scanf(" %c", &ch);
} while (ch != 'c');
```
Program dibuat untuk menunggu sampai user meng-inputkan karakter `c` sebelum melanjutkan eksekusi.

```c
shmdt(mat3);
shmctl(shmid, IPC_RMID, NULL);
```
Terakhir, shared memory di-dettach dari `mat3` dan shared memory tersebut dihapus.

![Program 2A](https://github.com/sisop-E03/soal-shift-sisop-modul-3-E03-2021/blob/master/images/soal2/a.png)

**B**
Membuat program dengan menggunakan matriks output dari program 2A. Matriks tersebut akan dilakukan perhitungan dengan matrix baru yang berasal dari input user. Perhitungannya adalah setiap cel yang berasal dari matriks A menjadi angka untuk faktorial, lalu cel dari matriks B menjadi batas maksimal faktorialnya dari paling besar ke paling kecil. Perhitungan tiap cell akan dilakukan dalam thread.

```c
typedef struct
{
    long long *cell;
    int a;
    int b;
} CalcArgs;
```
Pertama, sebuah struct bernama `CalcArgs` dideklarasikan. Struct ini akan digunakan untuk memberikan argumen ke thread sehingga isi dari struct ini adalah pointer ke cell, nilai a, dan nilai b.

```c
CalcArgs *makeCalcArgs(long long *cell, int a, int b)
{
    CalcArgs *args = malloc(sizeof(CalcArgs));

    args->cell = cell;
    args->a = a;
    args->b = b;

    return args;
}
```
Sebuah fungsi bernama `makeCalcArgs` yang mengembalikan pointer ke struct `CalcArgs` dideklarasikan. Fungsi ini digunakan untuk membuat sebuah struct baru dan mengisikan nilainya.

```c
void *calculate(void *arguments)
{
    CalcArgs *args = (CalcArgs *)arguments;

    if (args->a == 0 || args->b == 0)
    {
        *(args->cell) = 0;
    }
    else
    {
        int i;
        int x = args->a - args->b > 0 ? args->a - args->b : 1;

        *(args->cell) = args->a;
        for (int i = args->a - 1; i > x; i--)
        {
            *(args->cell) *= i;
        }
    }

    return NULL;
}
```
Sebuah fungsi bernama `calculate` dideklarasikan. Fungsi ini yang nantinya akan digunakan dalam thread. Di awal fungsi ini, pertama dilakukan type casting agar nilai dari `arguments` bisa dibaca. Kemudian argumen `a` dan `b` dicek, apabila 0 maka nilai di `cell` akan dibuat 0 juga. Apabila bukan 0, maka akan dicari nilai batas faktorialnya dan kemudian mengisikan nilai `cell` dengan hasil perkalian nilai-nilai dari `a` sampai batas ditambah 1.

```c
void main()
{
    int (*matA)[6];
    int matB[4][6];
    long long matC[4][6];
    int i;
    int j;
    key_t key = 1234;

    ...
}
```
Di awal fungsi `main`, dideklarasikan variabel-variabel yang akan dipakai. `matA` akan mendapatkan nilainya dari hasil program soal 2a. `matB` akan diisi dari input user. `matC` digunakan untuk tempat menyimpan hasil perhitungan. `i` dan `j` digunakan dalam looping. Dan `key` digunakan sebagai key ketika mengambil shared memory.

```c
int shmid = shmget(key, sizeof(int[4][6]), IPC_CREAT | 0666);
if (shmid == -1)
{
    fprintf(stderr, "shmget() Failed");
    return;
}

matA = shmat(shmid, NULL, 0);
if (matA == (void *)-1)
{
    fprintf(stderr, "shmat() Failed");
    return;
}
```
Mirip seperti soal 2A, shared memory dibuat dan di-attach ke `matA`. Kemudian dilakukan pengecekan error dan program akan dihentikan apabila ada error.

```c
printf("Matrix A\n");
for (i = 0; i < 4; i++)
{
    for (j = 0; j < 6; j++)
    {
        printf("%d ", matA[i][j]);
    }
    printf("\n");
}
```
Matriks yang didapatkan dari shared memory ditampilkan ke terminal.

```c
printf("Input matrix B 4x6\n");
for (i = 0; i < 4; i++)
{
    for (j = 0; j < 6; j++)
    {
        scanf("%d", &matB[i][j]);
    }
}
```
`matB` akan diisikan dari hasil input user.

```c
pthread_t tid[4][6];

for (i = 0; i < 4; i++)
{
    for (j = 0; j < 6; j++)
    {
        pthread_create(&(tid[i][j]), NULL, calculate,
                       (void *)makeCalcArgs(&(matC[i][j]), matA[i][j], matB[i][j]));
    }
}
```
Perhitungan dilakukan dengan cara membuat thread dengan fungsi `calculate` dan argumennya adalah pointer ke cell, nilai a dari `matA`, dan nilai b dari `matB`.  

```c
printf("Result\n");
for (i = 0; i < 4; i++)
{
    for (j = 0; j < 6; j++)
    {
        pthread_join(tid[i][j], NULL);
        printf("%lld ", matC[i][j]);
    }
    printf("\n");
}
```
Hasil perhitungan akan ditampilkan ke terminal. Thread akan di-join terlebih dahulu sebelum di-print agar program tidak menampilkan nilai dari suatu cell sebelum threadnya selesai menghitung.

```c
shmdt(matA);
shmctl(shmid, IPC_RMID, NULL);
```
Juga mirip seperti soal 2A, shared memory di-dettach dari `matA` dan dihapus diakhir program.

![Program 2B](https://github.com/sisop-E03/soal-shift-sisop-modul-3-E03-2021/blob/master/images/soal2/b.png)

**C**
Membuat program untuk mengecek 5 proses teratas apa saja yang memakan resource komputernya dengan command “ps aux | sort -nrk 3,3 | head -5” dan IPC Pipes.

```c
int fd1[2];

if (pipe(fd1)==-1) 
{ 
	fprintf(stderr, "Pipe Failed" ); 
	return; 
}
```
Di awal, pipe pertama bernama `fd1` dibuat. Apabila ada error, program akan langsung dihentikan.

```c
pid_t pid = fork();

if(pid == 0) {
    dup2(fd1[1], STDOUT_FILENO);

    close(fd1[0]);
    close(fd1[1]);

    char *argv[] = {"ps", "aux", NULL};
    execv("/bin/ps", argv);
}
```
Kemudian program akan di-fork. Di dalam child process, file descriptor untuk standard output diganti menjadi ujung tulis dari pipe `fd1` sebelum kedua ujung dari pipe tersebut ditutup. Kemudian `ps aux` dieksekusi dengan bantuan `execv()`.

```c
int fd2[2];

if (pipe(fd2)==-1) 
{ 
	fprintf(stderr, "Pipe Failed" ); 
	return; 
}
```
Kembali di program parent, pipe kedua yang bernama `fd2` dibuat. Sama seperti sebelumnya, juga dilakukan pengecekan error.

```c
pid = fork();

if(pid == 0) {
    dup2(fd1[0], STDIN_FILENO);

    close(fd1[0]);
    close(fd1[1]);

    dup2(fd2[1], STDOUT_FILENO);

    close(fd2[0]);
    close(fd2[1]);

    char *argv[] = {"sort", "-nrk", "3,3", NULL};
    execv("/bin/sort", argv);
} else if (pid > 0) {

    ...

}
```
Program kemudian kembali di-fork. Kali ini di dalam child process, file descriptor untuk standard input diganti menjadi ujung baca dari pipe `fd1` dan kedua ujung dari pipe tersebut ditutup. File descriptor untuk standard output juga diganti menjadi ujung tulis dari pipe `fd2` dan kedua ujung pipe `fd2` juga ditutup. Kemudian `sort -nrk 3,3` dieksekusi dengan bantuan `execv()`.

```c
close(fd1[0]);
close(fd1[1]);

dup2(fd2[0], STDIN_FILENO);

close(fd2[0]);
close(fd2[1]);

char *argv[] = {"head", "-5", NULL};
execv("/bin/head", argv);
```
Di program parent, pipe `fd1` akan ditutup karena sudah tidak digunakan. File descriptor untuk standard input diganti menjadi ujung baca dari pipe `fd2` dan pipe tersebut ditutup. Terahir, `head -5` akan dieksekusi dengan bantuan `execv()`.

![Program 2C](https://github.com/sisop-E03/soal-shift-sisop-modul-3-E03-2021/blob/master/images/soal2/c.png)

### Kendala Pengerjaan
1. Waktu pengerjaan soal shift bersamaan dengan minggu ETS.
2. Sempat bingung ketika membuat shared memory untuk array 2d supaya tetap bisa diakses dengan operator `[][]`, bukan dengan pointer arithmetic.
3. Sempat bingung ketika ingin memberikan banyak argumen ke fungsi thread.

## Soal 3
### Soal dan Penyelesaian

Pada Soal 3 diminta untuk membuat program yang mengategorikan file kedalam folder sesuai dengan eksistensinya.
Untuk mengategorikannya terdapat 3 opsi yaitu `-f`, `-d`,  dan `*`.

```c
int m=0;
    char *arr2[50];
    //ngambil namafile beserta eksistensi
    char *token1 = strtok(things, "/");
    while( token1 != NULL ) {
        // printf( "token %d = %s\n", m , token1 );
        arr2[m] = token1;
        m++;
        token1 = strtok(NULL, "/");
    }
    char namafile[200];
    strcpy(namafile,arr2[m-1]);
```

program diatas untuk mengecek dan mengambil namafile beserta eksistensi yang dimiliki file tersebut.
Setelah itu namafile akan dicek melalui program dibawah ini, jika namafile tidak memiliki eksistensi maka termasuk kedalam folder `Unknown`.
jika namafile pada awalannya memiliki `.` maka termasuk kedalam folder `Hidden`.
namun jika tidak keduanya maka akan dicek termasuk ke folder mana.
karena program tidak case sensitive maka karakter di konversi menjadi huruf kecil dengan command `tolower`.

```c
//cek filenya termasuk dalam folder apa
    char aa[100];
    char *token = strchr(namafile, '.');
    if(token== NULL){
        strcat(aa, "Unknown");
    }
    else if(namafile[0]=='.'){
        strcat(aa, "Hidden");
    }
    else{
        strcpy(aa,token+1);
        for(int i = 0; aa[i]; i++){
            aa[i] = tolower(aa[i]);
        }
    }
```

setelah itu nama folder yang telah didapat akan dibuatkan foldernya dan file akan dipindahkan sesuai dengan eksistensi yang telah didapat.

```c
char tempat2[100];
    strcpy(tempat2,"/home/vika/modul3/");
    strcat(tempat2, aa);
    strcat(tempat2,"/");
    mkdir(tempat2,0750);
    
    char source[1000], target[1000];
    strcpy(source,arg);
    strcpy(target,"/home/vika/modul3/");
    strcat(target,aa);
    strcat(target,"/");
    strcat(target,namafile);

    //pindah file
    rename(source,target);
    return NULL;
}

```

karena disoal berlaku recursive, maka menggunakan directory listing untuk melist file secara recursive.

```c
void listFilesRecursively(char *basePath)
{
	char path[256]={};
	struct dirent *dp;
	DIR *dir = opendir(basePath);

	if (!dir)
	return;

	while ((dp = readdir(dir)) != NULL)
	{
		if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
		{
            \\kalau yang dicek file bukan folder
			if (dp->d_type == DT_REG)
			{
				strcpy(tanda[x], basePath);
				strcat(tanda[x], dp->d_name);
				x++;
			}
			else
			{
				strcpy(path, basePath);
				strcat(path, dp->d_name);
				strcat(path, "/");
				listFilesRecursively(path);
			}
		}
	}
	closedir(dir);
}
```

Opsi -f bisa menambahkan argumen file yang bisa dikategorikan sebanyak yang diinginkan oleh pengguna.
Jika berhasil maka akan mengeluarkan output `file x : Berhasil Dikategorikan` dan jika gagagl akan mengeluarkan output `file x : Sad, gagal :( `.

```c
//menambahkan argumen file yang bisa dikategorikan
    if (strcmp(argv[1],"-f") == 0) {
        for(j = 2 ; j < argc ; j++ ){
            int err;
            //membuat thread
            err=pthread_create(&(thd[i]),NULL,playandcount,argv[j]);
            if (err !=0){
                //kalau error
                printf ("File %d : Sad,gagal :(\n", j-1);
                //return 0;
            }
            else{
                printf("File %d : Berhasil Dikategorikan\n",j-1);
            }

            pthread_join(thd[i],NULL);
             i++;
        }
    }
```
 ![alt text](https://github.com/sisop-E03/soal-shift-sisop-modul-3-E03-2021/blob/master/soal3/soal3_coba%20-f.jpg)
 
 ![alt text](https://github.com/sisop-E03/soal-shift-sisop-modul-3-E03-2021/blob/master/soal3/soal3_coba%20-f%201.jpg)
 
 Untuk opsi -d melakukan pengkategorian pada suatu directory. Namun pada opsi -d ini, user hanya bisa memasukkan input 1 directory saja, tidak seperti file yang bebas menginput file sebanyak mungkin.
Perintah di atas akan mengkategorikan file di `/path/to/directory`, lalu hasilnya akan disimpan di working directory dimana program C tersebut berjalan `/home/vika/modul3`
jika berhasil makan akan mengeluarkan output `Direktori sukses disimpan!` dan jika gagal akan mengeluarkan output `Yah, gagal disimpan :(`.

```c
 //pengkategorian suatu directory
    else if (strcmp(argv[1],"-d") == 0 && argc == 3) {
        i = 0;
	   int err;
	    listFilesRecursively(argv[2]);

	    for (i=0; i<x; i++){
		    err=pthread_create(&(thd[i]),NULL,&playandcount,(void *) tanda[i]);
		    if(err!=0)
		    {
			    printf("Yah, gagal disimpan :(\n");
			    return 0;;
		    }
	    }
	    
        for (i=0; i<x; i++){
		    pthread_join(thd[i],NULL);
        }

    printf("Direktori sukses disimpan!\n");
    }
    
 ```
 
 ![alt text](https://github.com/sisop-E03/soal-shift-sisop-modul-3-E03-2021/blob/master/soal3/soal3_%20coba%20-d.jpg)
 
  
 Untuk Opsi `*` Opsi ini akan mengkategorikan seluruh file yang ada di working directory ketika menjalankan program C tersebut `/home/vika/modul3`.
 
 ```c
 //mengkategorikan seluruh file yang ada di working directory
    else if (strcmp(argv[1],"*") == 0 && argc == 2) {
        i = 0;
	   int err;
	    listFilesRecursively("/home/vika/modul3/");

	    for (i=0; i<x; i++){
		    err=pthread_create(&(thd[i]),NULL,&playandcount,(void *) tanda[i]);
		    
            if(err!=0){
			    return 0;
		    }
	    }

	    for (i=0; i<x; i++){
		    pthread_join(thd[i],NULL);
        }
    
    }
    return 0; 
}
 ```
  ![alt text](https://github.com/sisop-E03/soal-shift-sisop-modul-3-E03-2021/blob/master/soal3/soal3_coba%20bintang.jpg)
  

  ![alt text](https://github.com/sisop-E03/soal-shift-sisop-modul-3-E03-2021/blob/master/soal3/soal3_file%20hidden.jpg) 
  
  
  ![alt text](https://github.com/sisop-E03/soal-shift-sisop-modul-3-E03-2021/blob/master/soal3/soal3_file%20unknown.jpg)
  
  
### Kendala Pengerjaan
1. Pada pengkategorian file, folder tidak terbuat berdasarkan eksistensi  sesuai "." terdepan
2. file tidak berpindah sesuai folder (antara hidden dan unknown)
  

#include <winsock2.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32") /* WinSock使用的库函数   */
#define UFTP_DEF_PORT     1025 /* 缺省端口 */
#define BUF_SIZE   1024 /* 缓冲区的大小 */
#define FILENAME_LEN   256     /* 文件名长度 */


void uftp_put(SOCKET soc, char* filename, struct sockaddr_in * send_addr){

    FILE *file;
    file = fopen(filename, "rb+");
    if(file == NULL){
        printf("[UFTP] The file [%s] is not existed\n", filename);
        exit(1);
    }

    int file_len;
    fseek(file, 0, SEEK_END);
    file_len = ftell(file);
    fseek(file, 0, SEEK_SET);
    /*
    //////////////////
    printf("%d",file_len);
    int send_file_len;
    char data_buf[BUF_SIZE]={0};
    send_file_len = sprintf(data_buf, "%s", "fsdds");
    sendto(soc, data_buf, send_file_len, 0,
               (struct sockaddr *)&send_addr, sizeof(struct sockaddr));
*/
    int read_len;
    char read_buf[BUF_SIZE];
    do /* 发送文件文件*/
    {
        read_len = fread(read_buf, sizeof(char), BUF_SIZE, file);

        if (read_len > 0)
        {
            sendto(soc, read_buf, read_len, 0,
               (struct sockaddr *)send_addr, sizeof(struct sockaddr));
            file_len -= read_len;
        }
    } while ((read_len > 0) && (file_len > 0));
    sendto(soc, read_buf, 0, 0,
               (struct sockaddr *)send_addr, sizeof(struct sockaddr));

    fclose(file);

}

void uftp_get(SOCKET soc, char *file_name, struct sockaddr_in * recv_addr){

    FILE *file_ftp;
    file_ftp = fopen(file_name, "w+");
    if(file_ftp == NULL){
        printf("[UFTP] The file [%s] is not existed\n", file_name);
        exit(1);
    }

    int result = 0;
    char data_buf[BUF_SIZE];
    int addr_len = sizeof(struct sockaddr);
    do /* 接收响应并保存到文件中 */
    {
        result = recvfrom(soc, data_buf, BUF_SIZE, 0,
                              (struct sockaddr *)recv_addr, &addr_len);
        if(result == 0)  break;
        if (result > 0)
        {
            fwrite(data_buf, sizeof(char), result, file_ftp);

            /* 在屏幕上输出 */
            data_buf[result] = 0;
            printf("%s", data_buf);
        }
    } while(result > 0);

    fclose(file_ftp);

}

int main(int argc, char **argv)
{
    WSADATA wsa_data;
    WSAStartup(MAKEWORD(2,0), &wsa_data); /* 初始化 WinSock 资源 */

    if(argc != 3 ){
        printf("usage: UFTPClient commod filename");
        return 0;
    }

    char file_name[FILENAME_LEN];
    strcpy(file_name,argv[2]);

    unsigned short port = UFTP_DEF_PORT;
    char host[FILENAME_LEN] = "127.0.0.1";
    unsigned long addr;
    addr = inet_addr(host);
    struct sockaddr_in serv_addr;  /* 服务器地址 */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = addr;

    SOCKET  uftp_sock = 0;         /* socket 句柄 */
    uftp_sock = socket(AF_INET, SOCK_DGRAM, 0); /* 创建 socket */

    /* 发送 FTP 请求 */
    int send_len;
    char data_buf[BUF_SIZE];
    send_len = sprintf(data_buf, "%s %s", argv[1], argv[2]);
    sendto(uftp_sock, data_buf, send_len, 0,
               (struct sockaddr *)&serv_addr, sizeof(struct sockaddr));

    //printf("%s\n",argv[1]);
    if(!strcmp(argv[1],"get")) uftp_get(uftp_sock, file_name, &serv_addr);
    else if(!strcmp(argv[1], "put")) uftp_put(uftp_sock, file_name, &serv_addr);
    else printf("sdfds1111\n");

    closesocket(uftp_sock);
    WSACleanup();

    return 0;
}

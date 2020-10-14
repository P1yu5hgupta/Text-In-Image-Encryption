#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>   
#include<sys/ioctl.h>
#include<unistd.h>  
#include<iostream>
#include<fstream>
#include<errno.h>
#include <highgui.h>
#include<opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;
bool isBitSet(char ch, int pos) {
	ch = ch >> pos;
	if(ch & 1)
		return true;
	return false;
}

string upgrade_key(string str) {
	int n = str.length();
	for (int i = 0; i < str.length(); i++)
		str[i] = (str[i] + n) % 256;
	return str;
}

void Extract_Text(){

	Mat image = imread("Received/Imagewithtext.jpeg");
	if(image.empty()) {
		cout << "Image Error\n";
		exit(-1);
	}

	char ch=0;
	int bit_count = 0;

	for(int row=0; row < image.rows; row++) {
		for(int col=0; col < image.cols; col++) {
			for(int color=0; color < 3; color++) {

				Vec3b pixel = image.at<Vec3b>(Point(row,col));

				if(isBitSet(pixel.val[color],0))
					ch |= 1;

				bit_count++;

				if(bit_count == 8) {

					if(ch == '\0')
						goto OUT;

					bit_count = 0;
					cout << ch;
					ch = 0;
				}
				else {
					ch = ch << 1;
				}

			}
		}
	}
	OUT:;
}

void Decryt_Image(string key){
    int key_thres = key.length();
	int key_p = 0;
    Mat image = imread("Received/EncrytImage.jpeg");
	for (int r = 0; r < image.rows; r++) {
		for (int c = 0; c < image.cols; c++) {
			for (int p = 0; p < 3; p++) {

				image.at<Vec3b>(r, c)[p] = (image.at<Vec3b>(r, c)[p] - (int)key[key_p] +256) % 256;
				key_p++;
				if (key_p == key_thres) {
					key = upgrade_key(key);
					key_p = 0;
					key_thres--;
					if (key_thres <= 0)
						key_thres = key.length();
				}
			}
		}
	}
    imwrite("Received/Imagewithtext.jpeg",image);

}
int receive_image(int socket){ 

    int buffersize = 0, recv_size = 0,size = 0, read_size, write_size, packet_index =1,stat;

    char imagearray[10241],verify = '1';
    FILE *image;

    do{
        stat = read(socket, &size, sizeof(int));
    }while(stat<0);

    printf("Packet received.\n");
    printf("Packet size: %i\n",stat);
    printf("Image size: %i\n",size);
    printf(" \n");

    char buffer[] = "Got it";

    do{
        stat = write(socket, &buffer, sizeof(int));
    }while(stat<0);

    printf("Reply sent\n");
    printf(" \n");

    image = fopen("Received/EncrytImage.jpeg", "w");

    if( image == NULL) {
        printf("Error has occurred. Image file could not be opened\n");
        return -1; 
    }

    int need_exit = 0;
    struct timeval timeout = {10,0};

    fd_set fds;
    int buffer_fd, buffer_out;

    while(recv_size < size) {

        FD_ZERO(&fds);
        FD_SET(socket,&fds);

        buffer_fd = select(FD_SETSIZE,&fds,NULL,NULL,&timeout);

        if (buffer_fd < 0)
            printf("error: bad file descriptor set.\n");

        if (buffer_fd == 0)
            printf("error: buffer read timeout expired.\n");

        if (buffer_fd > 0)
        {
            do{
                read_size = read(socket,imagearray, 10241);
            }while(read_size <0);

            printf("Packet number received: %i\n",packet_index);
            printf("Packet size: %i\n",read_size);

                write_size = fwrite(imagearray,1,read_size, image);
                printf("Written image size: %i\n",write_size); 

                    if(read_size !=write_size) {
                        printf("error in read write\n");    
                    }
                    
                    recv_size += read_size;
                    packet_index++;
                    printf("Total received image size: %i\n",recv_size);
                    printf(" \n");
                    printf(" \n");
        }

    }


    fclose(image);
    printf("Image successfully Received!\n");
    return 1;
}

int main(int argc , char *argv[]){

    int socket_desc;
    struct sockaddr_in server;
    char *parray;

    socket_desc = socket(AF_INET , SOCK_STREAM , 0);

    if (socket_desc == -1) {
        printf("Could not create socket");
    }

    memset(&server,0,sizeof(server));
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons( 8889 );

    if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0) {
        cout<<strerror(errno);
        close(socket_desc);
        puts("Connect Error");
        return 1;
    }

    puts("Connected\n");
    while(true){
        string key;
        char buffer[256];
        bzero(buffer,256);
        read(socket_desc, buffer, 255);
        key=buffer;
        receive_image(socket_desc);
        Decryt_Image(key);
        Extract_Text()
    }
    close(socket_desc);
    return 0;
}
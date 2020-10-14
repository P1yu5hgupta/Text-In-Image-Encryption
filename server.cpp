#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>   
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
	// 7 6 5 4 3 2 1 0
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

string generate_key() {
	int key_len = 0;
	while (key_len == 0)
		key_len = rand() % 50;

	string key = "";
	for (int i = 0; i < key_len; i++) {
		key += (char)(rand() % 256);
	}
	return key;
}

void AddText(string st,Mat image){
	
    char ch;

	ch=s[0];
	int len=st.length(),cnt=0;

	int bit_count = 0;

	bool last_null_char = false;
	bool encoded = false;

	for(int row=0; row < image.rows; row++) {
		for(int col=0; col < image.cols; col++) {
			for(int color=0; color < 3; color++) {

				Vec3b pixel = image.at<Vec3b>(Point(row,col));

				if(isBitSet(ch,7-bit_count))
					pixel.val[color] |= 1;
				else
					pixel.val[color] &= ~1;

				image.at<Vec3b>(Point(row,col)) = pixel;

				bit_count++;

				if(last_null_char && bit_count == 8) {
					encoded  = true;
					goto OUT;
				}

				if(bit_count == 8) {
					bit_count = 0;
					cnt++;
					ch=st[cnt];

					if(cnt>=len-1) {
						last_null_char = true;
						ch = '\0';
					}
				}

			}
		}
	}
	OUT:;

	if(!encoded) {
		cout << "Message too big. Try with larger image.\n";
		exit(-1);
	}

	imwrite("Images/Imagewithtext.jpeg",image);
}
string EncrytImage(){
    string key =generate_key();
    int key_thres = key.length();
	int key_p = 0;
    Mat image = imread("Images/Imagewithtext.jpeg");
	for (int r = 0; r < image.rows; r++) {
		for (int c = 0; c < image.cols; c++) {
			for (int p = 0; p < 3; p++) {

				image.at<Vec3b>(r, c)[p]= (image.at<Vec3b>(r, c)[p] + (int)key[key_p])%256;
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
    imwrite("Images/EncrytedImage.jpeg",image);

    return key;
}

int send_image(int socket){

FILE *picture;
int size, read_size, stat, packet_index;
char send_buffer[10240], read_buffer[256];
packet_index = 1;

picture = fopen("Images/EncrytedImage.jpeg", "r");

if(picture == NULL) {
    printf("Error Opening Image File"); } 

fseek(picture, 0, SEEK_END);
size = ftell(picture);
fseek(picture, 0, SEEK_SET);
printf("Total Picture size: %i\n",size);

printf("Sending Picture Size\n");
write(socket, (void *)&size, sizeof(int));

printf("Sending Picture as Byte Array\n");

do { 
    stat=read(socket, &read_buffer , 255);
    printf("Bytes read: %i\n",stat);
} while (stat < 0);

printf("Received data in socket\n");
printf("Socket data: %c\n", read_buffer);

while(!feof(picture)) {
    read_size = fread(send_buffer, 1, sizeof(send_buffer)-1, picture);

    do{
        stat = write(socket, send_buffer, read_size);  
    }while (stat < 0);

    printf("Packet Number: %i\n",packet_index);
    printf("Packet Size Sent: %i\n",read_size);     
    printf(" \n");
    printf(" \n");


    packet_index++;  
    bzero(send_buffer, sizeof(send_buffer));
    }
}

int main(int argc , char *argv[])
{
   
	Mat image = imread("Images/image");
	if(image.empty()) {
		cout << "Image Error\n";
		exit(-1);
	}

    int socket_desc , new_socket , c, read_size,buffer = 0;
    struct sockaddr_in server , client;
    char *readin;

    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 8889 );

    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
    puts("bind failed");
    return 1;
    }

    puts("bind done");

    listen(socket_desc , 3);

    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);

    if((new_socket = accept(socket_desc, (struct sockaddr *)&client,(socklen_t*)&c))){
puts("Connection accepted");
        }

fflush(stdout);

if (new_socket<0)
{
    perror("Accept Failed");
    return 1;
}
string st;
while(true){
    cout<<"\n";
    cout<<"Enter the text:";
    cin>>st;
    AddText(st,image);
    string key=EncrytImage();

    send_image(new_socket);
}
close(socket_desc);
fflush(stdout);
return 0;
}
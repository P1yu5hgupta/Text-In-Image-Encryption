g++ server.cpp -o server \-I /usr/include/opencv2 \-L /usr/lib \-lopencv_core \-lopencv_imgproc \-lopencv_imgcodecs
g++ client.cpp -o client \-I /usr/include/opencv2 \-L /usr/lib \-lopencv_core \-lopencv_imgproc \-lopencv_imgcodecs
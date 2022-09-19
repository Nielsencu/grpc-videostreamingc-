#include <iostream>
#include <opencv2/opencv.hpp>

typedef unsigned char byte;

byte * matToBytes(const cv::Mat& image)
{
   int size = image.total() * image.elemSize();
   byte * bytes = new byte[size];  // you will have to delete[] that later
   std::memcpy(bytes, image.data, size * sizeof(byte));
   return bytes;
}

cv::Mat bytesToMat(byte * bytes,int width,int height)
{
    cv::Mat image = cv::Mat(height,width,CV_8UC3,bytes).clone(); // make a copy
    return image;
}

int main(){
    cv::Mat image;
    cv::VideoCapture cap(0);
    if(!cap.isOpened()){
        std::cout << "Cannot open camera";
        return 0;
    }
    while(true){
        cap >> image;
        if(image.empty()){
            break;
        }

        byte* byteArr = matToBytes(image);
        
        cv::imshow("Webcam stream", image);
        char c = static_cast<char>(cv::waitKey(25));
        if(c == 27) break;
    }
    cap.release();
    return 0;
}


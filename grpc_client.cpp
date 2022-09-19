#include <iostream>
#include <opencv2/opencv.hpp>
#include "build/protos/stream.grpc.pb.h"
#include <opencv2/opencv.hpp>
#include <grpcpp/grpcpp.h>
#include <google/protobuf/util/time_util.h>
#include <chrono>

typedef unsigned char byte;
cv::Mat bytesToMat(byte * bytes,int width,int height)
{
    cv::Mat image = cv::Mat(height,width,CV_8UC3,bytes).clone(); // make a copy
    return image;
}

class RobotClient {
    public:
    RobotClient(std::shared_ptr<grpc::Channel> channel) 
    : _stub(VideoStream::NewStub(channel)){

    }

    void Process(){
        StreamReply response;
        StreamRequest req;
        grpc::ClientContext context;
        req.set_status(1);
        std::unique_ptr<grpc::ClientReader<StreamReply>> reader(_stub->Process(&context, req));
        auto now = std::chrono::steady_clock::now();
        auto now_since_epoch = std::chrono::steady_clock::now().time_since_epoch();
        while(reader->Read(&response)){
            auto temp_now = std::chrono::steady_clock::now();
            auto diff_per_loop = std::chrono::duration_cast<std::chrono::milliseconds>((std::chrono::steady_clock::now() - now)).count();
            uint64_t diff_since_sent = (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count() - response.timestamp());
            now = temp_now;
            std::cout << "Time taken per loop using since epoch (ns); " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch() - now_since_epoch).count() << "\n";
            now_since_epoch = std::chrono::steady_clock::now().time_since_epoch();
            std::cout << "Time taken per loop (ns); " << diff_per_loop << "\n";
            std::cout << "FPS; " << static_cast<float>(std::pow(10.0, 3))/diff_per_loop << "\n";
            std::cout << "Time taken since sent in milliseconds: " << diff_since_sent << "\n";
            std::cout << "FPS: " << static_cast<float>(std::pow(10.0, 3))/diff_since_sent << "\n";
            grpc::string mat = response.img();
            byte* bytes = new byte[mat.length()];
            std::memcpy(bytes, mat.data(), mat.length());
            std::cout << "Received length " << mat.length() << "\n";
            std::cout << "Received width " << response.width() << "\n";
            std::cout << "Received height " << response.height() << "\n";
            cv::Mat image = bytesToMat(bytes, response.width(), response.height());
            cv::imshow("Webcam stream", image);
            delete bytes;
            char c = static_cast<char>(cv::waitKey(10));
            if(c == 27) break;
        }
    }
    private:
    std::unique_ptr<VideoStream::Stub> _stub;
};

int main(){
    grpc::ChannelArguments ch_args;
    int max_msg_size{24883328};
    ch_args.SetMaxReceiveMessageSize(max_msg_size);
    RobotClient client(grpc::CreateCustomChannel("localhost:50051", grpc::InsecureChannelCredentials(), ch_args));
    client.Process();
}
#include <iostream>
#include "build/protos/stream.grpc.pb.h"
#include <opencv2/opencv.hpp>
#include <grpcpp/grpcpp.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <google/protobuf/util/time_util.h>

typedef unsigned char byte;
enum class Type { READ, WRITE = 2, CONNECT = 3, DONE = 4, FINISH = 5 };

byte * matToBytes(const cv::Mat& image)
{
   int size = image.total() * image.elemSize();
   byte * bytes = new byte[size];  // you will have to delete[] that later
   std::memcpy(bytes, image.data, size * sizeof(byte));
   return bytes;
}

class StreamService final : public VideoStream::Service {
    public:
    grpc::Status Process(grpc::ServerContext* context,
                            const StreamRequest* request,
                            grpc::ServerWriter<StreamReply>* writer) override {
    cv::Mat image;
    cv::VideoCapture cap(0);
    if(!cap.isOpened()){
        std::cout << "Cannot open camera";
        return grpc::Status::CANCELLED;
    }
    while(true){
        StreamReply response;
                cap >> image;
        if(image.empty()){
            return grpc::Status::CANCELLED;
        }
        // byte* byteArr = matToBytes(image);
        // response.set_height(image.size().height);
        // response.set_width(image.size().width);
        // response.set_img(byteArr, image.total() * image.elemSize() * sizeof(byte));
        byte* byteArr = new byte[3840 * 2160 * 3];
        response.set_height(3840);
        response.set_width(2160);
        response.set_img(byteArr, 3840 * 2160 * 3 * sizeof(byte));
        uint64_t ns_since_epoch = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
        response.set_timestamp(ns_since_epoch);
        if(!writer->Write(response)){
            std::cout << "Stream has been closed\n";
            return grpc::Status::CANCELLED;
        }
        delete byteArr;
    }

    return grpc::Status::OK;
  }
    
    private:
};

void runServer() {
    std::string server_address("localhost:50051");
    StreamService service;

    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    grpc::ServerBuilder builder;
    int max_msg_size{24883328};
    
    builder.SetMaxMessageSize(max_msg_size);
    builder.SetMaxSendMessageSize(max_msg_size);
    builder.SetMaxReceiveMessageSize(max_msg_size);
    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // Register "service" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *synchronous* service.
    builder.RegisterService(&service);
    // Finally assemble the server.
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;

    // Wait for the server to shutdown. Note that some other thread must be
    // responsible for shutting down the server for this call to ever return.
    server->Wait();
}

int main(){
    runServer();
}


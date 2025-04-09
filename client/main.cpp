// client side has its own db which contains key-value fields name-path for images
// ./client load name_of_the_image /path/to/image (load name-path pair to the db) 
// ./client list (list all pairs name-path)
// ./client transform name_of_the_image the_transformation (apply the transformation to the image)

#include <iostream>
#include <memory>
#include <string>
#include "impl.h"
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"

ABSL_FLAG(std::string, command, "", "Command to execute (load, list, transform, remove)");
ABSL_FLAG(std::string, name, "", "Name of the image");
ABSL_FLAG(std::string, path, "", "Path to the image");
ABSL_FLAG(std::string, transformation, "", "Transformation to apply");

void PrintUsage() {
    std::cout << "\nImage Processing Client - Available Commands:\n\n"
              << "1. Load an image:\n"
              << "   ./client --command=load --name=<image_name> --path=/path/to/image.jpg\n"
              << "   Example: ./client --command=load --name=cat --path=/home/user/images/cat.jpg\n\n"
              << "2. List all images:\n"
              << "   ./client --command=list\n\n"
              << "3. Transform an image:\n"
              << "   ./client --command=transform --name=<image_name> --transformation=<transformation>\n"
              << "   Example: ./client --command=transform --name=cat --transformation=rotate:90\n\n"
              << "4. Remove an image:\n"
              << "   ./client --command=remove --name=<image_name>\n"
              << "   Example: ./client --command=remove --name=cat\n\n"
              << "Available transformations:\n"
              << "   - rotate:<angle_degrees>\n"
              << "     Example: rotate:90\n\n"
              << "   - resize:<width>x<height>\n"
              << "     Example: resize:800x600\n\n" 
              << "   - grayscale\n"
              << "     Example: grayscale\n\n"
              << "   - blur:<sigma>\n"
              << "     Example: blur:2.5\n"
              << "     Default: blur (uses sigma=1.5)\n";
}

int main(int argc, char** argv) {
    if (argc == 1) {
        PrintUsage();
        return 0;
    }

    absl::ParseCommandLine(argc, argv);
    
    std::string command = absl::GetFlag(FLAGS_command);
    std::string name = absl::GetFlag(FLAGS_name);
    std::string path = absl::GetFlag(FLAGS_path);
    std::string transformation = absl::GetFlag(FLAGS_transformation);

    // Set maximum message size to 20MB
    grpc::ChannelArguments args;
    args.SetMaxReceiveMessageSize(20 * 1024 * 1024);  // 20MB
    args.SetMaxSendMessageSize(20 * 1024 * 1024);     // 20MB

    // Create gRPC channel and client with the correct port and message size settings
    auto channel = grpc::CreateCustomChannel(
        "localhost:5555",
        grpc::InsecureChannelCredentials(),
        args
    );
    ImageProcessClient client(channel);

    try {
        if (command == "load") {
            if (name.empty() || path.empty()) {
                std::cerr << "Error: Missing required parameters for load command\n"
                         << "Usage: ./client --command=load --name=<image_name> --path=/path/to/image.jpg\n"
                         << "Example: ./client --command=load --name=cat --path=/home/user/images/cat.jpg\n";
                return 1;
            }
            client.LoadImage(name, path);
            std::cout << "Successfully loaded image '" << name << "' from path: " << path << std::endl;
        } else if (command == "list") {
            std::cout << "Listing all stored images:\n";
            client.List();
        } else if (command == "transform") {
            if (name.empty() || transformation.empty()) {
                std::cerr << "Error: Missing required parameters for transform command\n"
                         << "Usage: ./client --command=transform --name=<image_name> --transformation=<transformation>\n"
                         << "Example: ./client --command=transform --name=cat --transformation=rotate:90\n\n"
                         << "Available transformations:\n"
                         << "  - rotate:<angle_degrees>\n"
                         << "  - resize:<width>x<height>\n"
                         << "  - grayscale\n";
            }
            client.Transform(name, transformation);
            std::cout << "Successfully applied transformation '" << transformation 
                      << "' to image '" << name << "'" << std::endl;
        } else if (command == "remove") {
            if (name.empty()) {
                std::cerr << "Error: Missing required parameter for remove command\n"
                         << "Usage: ./client --command=remove --name=<image_name>\n"
                         << "Example: ./client --command=remove --name=cat\n";
                return 1;
            }
            client.Remove(name);
            std::cout << "Successfully removed image '" << name << "'" << std::endl;
        } else {
            std::cerr << "Error: Unknown command '" << command << "'\n\n";
            PrintUsage();
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}


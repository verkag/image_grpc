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

ABSL_FLAG(std::string, command, "", "Command to execute (load, list, transform)");
ABSL_FLAG(std::string, name, "", "Name of the image");
ABSL_FLAG(std::string, path, "", "Path to the image");
ABSL_FLAG(std::string, transformation, "", "Transformation to apply");

int main(int argc, char** argv) {
    absl::ParseCommandLine(argc, argv);
    
    std::string command = absl::GetFlag(FLAGS_command);
    std::string name = absl::GetFlag(FLAGS_name);
    std::string path = absl::GetFlag(FLAGS_path);
    std::string transformation = absl::GetFlag(FLAGS_transformation);

    // Create gRPC channel and client
    auto channel = grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials());
    ImageProcessClient client(channel);

    if (command == "load") {
        if (name.empty() || path.empty()) {
            std::cerr << "Error: Both name and path are required for load command" << std::endl;
            return 1;
        }
        client.LoadImage(name, path);
        std::cout << "Image loaded successfully" << std::endl;
    } else if (command == "list") {
        client.List();
    } else if (command == "transform") {
        if (name.empty() || transformation.empty()) {
            std::cerr << "Error: Both name and transformation are required for transform command" << std::endl;
            return 1;
        }
        client.Transform(name, transformation);
        std::cout << "Transformation applied successfully" << std::endl;
    } else {
        std::cerr << "Error: Unknown command. Use load, list, or transform" << std::endl;
        return 1;
    }

    return 0;
}


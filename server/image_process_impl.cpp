#include "image_process_impl.h"
#include "processor.h"

#include "absl/log/log.h"

// TODO: redesign to bi-directional 
// add timeouts and possibly cancellation
::grpc::ServerUnaryReactor* ImageProcessImpl::UploadAndProcess(::grpc::CallbackServerContext* ctx, const imageservice::ImageRequest* req, imageservice::ImageResponse* res)  {
    class Reactor : public grpc::ServerUnaryReactor {
    public: 
        Reactor(const imageservice::ImageRequest* req, imageservice::ImageResponse* res) : req_(req), res_(res) {
            LOG(INFO) << "Processing request for image: " << req_->filename();
            
            try {
                ImageTransformator t(req_->image_data(), req_->transformation());
                t.apply_transformation();
                
                res_->set_filename(req_->filename());
                res_->set_processed_image_data(t.get_result());
                
                LOG(INFO) << "Successfully processed image: " << req_->filename();
                Finish(grpc::Status::OK);
            } catch (const std::exception& e) {
                LOG(ERROR) << "Error processing image: " << e.what();
                Finish(grpc::Status(grpc::INTERNAL, e.what()));
            }
        }

    private:
        const imageservice::ImageRequest* req_;
        imageservice::ImageResponse* res_;

        void OnDone() override {
            LOG(INFO) << "Request completed for image: " << req_->filename();
            delete this;
        }

        void OnCancel() override {
            LOG(ERROR) << "Request cancelled for image: " << req_->filename();
            delete this;
        }
    };
    return new Reactor(req, res);
}

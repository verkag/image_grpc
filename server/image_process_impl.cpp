#include "image_process_impl.h"
#include "processor.h"

#include "absl/log/log.h"





// TODO: redesign to bi-directional 
// add timeouts and possibly cancellation
::grpc::ServerUnaryReactor* ImageProcessImpl::UploadAndProcess(::grpc::CallbackServerContext* ctx, const ::ImageRequest* req, ::ImageResponse* res)  {
    class Reactor : public grpc::ServerUnaryReactor {
    public: 
        // main logic here
        Reactor (const ::ImageRequest* req, ::ImageResponse* res) {
            ImageTransformator t(req->image_data(),
                                 req->transformation());  
            res->set_processed_image(t.get_result());

            Finish(grpc::Status::OK);
        }

    private:
        void OnDone() override {
            LOG(INFO) << "Upload and Proccess completed";
            delete this;
        }

        void OnCancel() override {
            LOG(ERROR) << "Upload and Proccess cancelled";
        }
    };
    return new Reactor(req, res);
}

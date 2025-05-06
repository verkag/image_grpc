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

::grpc::ServerBidiReactor<::imageservice::ImageMessageChunked, ::imageservice::ImageMessageChunked>* ImageProcessImpl::UploadAndProcessChunked(::grpc::CallbackServerContext* ctx) {
    class BidiReactor : public grpc::ServerBidiReactor<::imageservice::ImageMessageChunked, ::imageservice::ImageMessageChunked> {
    public:
        BidiReactor() {
            StartRead(&chunked_req_);
        }

        void OnReadDone(bool ok) override {
            if (ok) {
                buffer_data_.push_back(chunked_req_);
                StartRead(&chunked_req_);
            }
            else {
                ProcessAndTransform();
                buffer_data_iterator_ = buffer_data_.begin();
                Write(); 
            }
        }

        void OnWriteDone(bool /*ok*/) override {
            Write();
        }

        void OnDone() override {
            LOG(INFO) << "RPC Completed";
        }

        void OnCancel() override {
            LOG(ERROR) << "RPC Cancelled";
        }
                
    private: 
        // unpacks buffer_data_ and apply transformation
        // then packs it back. 
        // First message is metadata
        void ProcessAndTransform() {
            imageservice::ImageMessageChunked meta = buffer_data_[0];
            std::string to_proccess = "";
            to_proccess.reserve(buffer_data_.size() * CHUNK_SIZE);
            for (int i = 1; i < buffer_data_.size(); i++) {
                to_proccess += std::move(*(buffer_data_[i].mutable_chunk()->mutable_data())); 
            }
            ImageTransformator t(to_proccess, meta.meta().transformation()); // config to move 
            t.apply_transformation();

            buffer_data_.clear();
            buffer_data_.push_back(meta);
            Chunkify(t.get_result());
        }

        void Chunkify(const std::string& str) {
            for (size_t offset = 0; offset < str.size(); offset += CHUNK_SIZE) {
                imageservice::ImageMessageChunked temp;
                size_t chunk_size = std::min(CHUNK_SIZE, str.size() - offset);
                temp.mutable_chunk()->set_data(str.substr(offset, chunk_size));
                buffer_data_.push_back(std::move(temp));
            }
        }

        void Write() {
            if (buffer_data_iterator_ != buffer_data_.end()) {
                StartWrite(&*buffer_data_iterator_);
                buffer_data_iterator_++;
            } else {
               Finish(grpc::Status::OK); 
            }
        }
        imageservice::ImageMessageChunked chunked_req_;
        std::vector<imageservice::ImageMessageChunked> buffer_data_; 
        std::vector<imageservice::ImageMessageChunked>::iterator buffer_data_iterator_; 
    };
    return new BidiReactor();
}

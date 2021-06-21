#pragma once

#include <mutex>
#include <queue>

#include <demo.pb.h>

namespace net
{
    enum class EnCommand
    {
        kNull,
        kSendText,
        kSetDisplayName,
        kMax
    };

    struct Header
    {
        EnCommand command = EnCommand::kNull;
        uint64_t body_size = 0;
    };

    struct MessageItem
    {
        Header header;
        std::shared_ptr<const google::protobuf::Message> body = nullptr;
    };

    class MessageQueue
    {
    private:
        std::mutex queue_mutex_;

        std::queue<std::shared_ptr<MessageItem>> queue_;

    public:
        const std::queue<std::shared_ptr<MessageItem>>& GetQueue() const
        {
            return queue_;
        }

        // Push to queue, returns true if queue was empty before push.
        bool Push(std::shared_ptr<MessageItem> item)
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            queue_.push(std::move(item));
            return queue_.empty();
        }

        // Pops from queue, returns true if queue is empty after pop.
        bool Pop()
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            if (queue_.empty())
                return true;
            queue_.pop();
            return queue_.empty();
        }

        std::shared_ptr<MessageItem> Front()
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            if (queue_.empty())
                return nullptr;
            return queue_.front();
        }

        bool Empty()
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            return queue_.empty();
        }
    };
}

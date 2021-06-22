#pragma once

#include <mutex>
#include <queue>

#include <demo.pb.h>

namespace net
{
    struct Header
    {
        uint64_t command = 0;
        uint64_t body_size = 0;
    };

    struct MessageItem
    {
        Header header;
        std::shared_ptr<const google::protobuf::Message> body = nullptr;
    };

    struct Packet
    {
        Header header;
        std::shared_ptr<std::vector<char>> body = nullptr;
    };

    template <typename ItemType>
    class MessageQueue
    {
    private:
        std::mutex queue_mutex_;

        std::queue<std::shared_ptr<ItemType>> queue_;

    public:
        const std::queue<std::shared_ptr<ItemType>>& GetQueue() const
        {
            return queue_;
        }

        // Push to queue, returns true if queue was empty before push.
        bool Push(std::shared_ptr<ItemType> item)
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            auto was_empty = queue_.empty();
            queue_.push(std::move(item));
            return was_empty;
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

        std::shared_ptr<ItemType> Front()
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

        void HandleAll(std::function<void(std::shared_ptr<ItemType>)> handler)
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            while (!queue_.empty())
            {
                auto item = queue_.front();
                handler(std::move(item));
                queue_.pop();
            }
        }
    };
}

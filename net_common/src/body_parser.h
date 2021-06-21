#pragma once

#include "message_queue.h"

namespace net
{
    template <typename ProtoType>
    std::shared_ptr<google::protobuf::Message> Parse(const std::string& data)
    {
        auto item = std::make_shared<ProtoType>();
        if (item->ParseFromString(data))
            return item;
        return nullptr;
    }

    std::shared_ptr<google::protobuf::Message> ParseBody(EnCommand command, const std::string& data)
    {
        switch (command)
        {
        case EnCommand::kSendText:
        case EnCommand::kSetDisplayName:
            return Parse<proto::StringMessage>(data);
        default:
            return nullptr;
        }
    }
}

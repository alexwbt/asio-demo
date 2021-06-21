#include <client.h>

class Client : public net::Client
{
    void OnMessage(
        net::EnCommand command,
        std::shared_ptr<const google::protobuf::Message> body
    ) override {
        std::cout << "You have a message (" << (int)command <<  ")." << std::endl;
        switch (command)
        {
        case net::EnCommand::kSendText:
        {
            auto message = std::dynamic_pointer_cast<const proto::StringMessage>(body);
            std::cout << message->message() << std::endl;
            break;
        }
        //case net::EnCommand::kSetDisplayName:
        }
    }
};

int main()
{
    Client client;
    client.Connect("127.0.0.1", 5625);

    while (true)
    {
        client.HandleMessages();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

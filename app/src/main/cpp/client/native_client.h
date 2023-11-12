#pragma once
#include <cstdint>
#include <string>
#include <vector>

class LtNativeClient {
public:
    struct Params {
        std::string client_id;
        std::string room_id;
        std::string token;
        std::string p2p_username;
        std::string p2p_password;
        std::string signaling_address;
        int32_t signaling_port;
        std::string codec;
        int32_t audio_channels;
        int32_t audio_freq;
        std::vector<std::string> reflex_servers;
        bool validate() const;
    };
public:
    LtNativeClient(const Params& params);
    ~LtNativeClient() = default;

    bool start();

private:
    ;
};
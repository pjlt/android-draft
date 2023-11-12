#include "native_client.h"

bool LtNativeClient::Params::validate() const {
    if (client_id.empty() || room_id.empty() || token.empty() || p2p_username.empty()
        || p2p_password.empty() || signaling_address.empty()) {
        return false;
    }
    if (signaling_port <= 0 || signaling_port > 65535) {
        return false;
    }
    if (codec != "avc" && codec != "hevc") {
        return false;
    }
    if (audio_channels <= 0 || audio_freq <= 0) {
        return false;
    }
    return true;
}

LtNativeClient::LtNativeClient(const Params &params) {
    //
}

bool LtNativeClient::start() {
    return false;
}

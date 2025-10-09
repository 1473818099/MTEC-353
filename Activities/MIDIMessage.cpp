#include "MIDIMessage.h"
#include <iostream>
#include <iomanip>

MIDIMessage::MIDIMessage(uint8_t status, uint8_t channel, const std::vector<uint8_t>& data)
    : status_(status), channel_(channel), data_(data) {}

MIDIMessage::~MIDIMessage() = default;

uint8_t MIDIMessage::status() const { return status_; }
uint8_t MIDIMessage::channel() const { return channel_; }
const std::vector<uint8_t>& MIDIMessage::data() const { return data_; }

std::string MIDIMessage::name() const {
    return "Generic MIDI Message";
}

void MIDIMessage::print(std::ostream& os) const {
    os << name() << " (status=0x" << std::hex << int(status_) << std::dec << ") channel=" << int(channel_);
    if (!data_.empty()) {
        os << " data=[";
        for (size_t i = 0; i < data_.size(); ++i) {
            if (i) os << ", ";
            os << int(data_[i]);
        }
        os << "]";
    }
    os << "\n";
}

std::vector<uint8_t> MIDIMessage::bytes() const {
    std::vector<uint8_t> out;
    out.push_back(status_);
    out.insert(out.end(), data_.begin(), data_.end());
    return out;
}

uint8_t MIDIMessage::statusNibble(uint8_t statusByte) {
    return (statusByte & 0xF0) >> 4;
}

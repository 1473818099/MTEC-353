#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <ostream>

class MIDIMessage {
protected:
    uint8_t status_;
    uint8_t channel_;
    std::vector<uint8_t> data_;
public:
    MIDIMessage(uint8_t status, uint8_t channel, const std::vector<uint8_t>& data = {});
    virtual ~MIDIMessage();

    uint8_t status() const;
    uint8_t channel() const;
    const std::vector<uint8_t>& data() const;

    // Human readable name (can be overridden)
    virtual std::string name() const;

    // Print a one-line description of the message
    virtual void print(std::ostream& os = std::cout) const;

    // Raw bytes that make up the message (status + data)
    std::vector<uint8_t> bytes() const;

    // Helper: upper nibble of a status byte (message type)
    static uint8_t statusNibble(uint8_t statusByte);
};



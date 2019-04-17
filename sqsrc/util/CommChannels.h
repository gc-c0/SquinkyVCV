#pragma once

#include "RingBuffer.h"

/**
 * Protocol:
 *      We send one unit32_t, followed by 'n' zeros
 *      zero is not a legal command
 * 
 *      Top 16 bits are the command, bottom 16 are the data
 */

const uint32_t CommCommand_ClearMute = (100 << 16); 

/**
 * CommChannelSend
 * Sends messages from on VCV Module to another
 */
class CommChannelSend
{
public:
    const static int zeroPad = 3;       // number of zeros to send out after a command
    /**
     * Queues up a message to be sent.
     * Will be sent over several periods.
     */
    void send(uint32_t);

    /**
     * Must be called every sample period to run send 
     * state machine. 
     */
    void go(uint32_t * outputBuffer);
private:
    SqRingBuffer<uint32_t, 4> messageBuffer;
    bool sendingData = false;
    bool sendingZero = false;
    int zeroCount = 0;
};

/**
 * CommChannelReceive
 * Sends messages from on VCV Module to another
 */
class CommChannelReceive
{
public:
    /**
     * returns zero if no data received, otherwise returns data
     */
    uint32_t rx(const uint32_t * inputBuffer);
private:
    uint32_t lastCommand = 0;
};

inline void CommChannelSend::send(uint32_t msg)
{
    assert(!messageBuffer.full());
    messageBuffer.push(msg);
}

inline void CommChannelSend::go(uint32_t* output)
{
    uint32_t x=0;
    if (sendingZero) {
        if (++zeroCount >= zeroPad) {
            sendingZero = false;
        }
    } else if (sendingData) {
        sendingData = false;
        sendingZero = true;
        zeroCount = 1;

        x = 0;
    } else {
        if (!messageBuffer.empty()) {
            sendingData = true;
            x = messageBuffer.pop();
        }
    }
    if (x != 0) printf("output 32: %x\n", x);
    *output = x;
}

inline uint32_t CommChannelReceive::rx(const uint32_t * inputBuffer)
{
    uint32_t ret = inputBuffer[0];
    if (ret == lastCommand) {
        ret = 0;
    } else {
        lastCommand = ret;
    }
    return ret;
};
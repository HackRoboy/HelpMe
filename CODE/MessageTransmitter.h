//
// Created by gecko on 11.12.16.
//

#ifndef FALLDETECTION_MESSAGETRANSMITTER_H
#define FALLDETECTION_MESSAGETRANSMITTER_H

#include <zmq.h>
#include <zmq_utils.h>
#include <string>

class MessageTransmitter {
public:
    MessageTransmitter(){}
    MessageTransmitter(std::string ipstring);
    void sendEmergencyMessage();

    void setup();
    void cleanup();
private:
    void *requester = nullptr;
    void *context = nullptr;
    std::string m_ipString;

};


#endif //FALLDETECTION_MESSAGETRANSMITTER_H

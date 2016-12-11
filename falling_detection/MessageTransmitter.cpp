//
// Created by gecko on 11.12.16.
//

#include <iostream>
#include "MessageTransmitter.h"


MessageTransmitter::MessageTransmitter(std::string ipString) :
        m_ipString( ipString )
{

}

void MessageTransmitter::sendEmergencyMessage()
{
    if(requester == nullptr)
    {
        std::cout << "ERROR: No Connection to the server! " << std::endl;
        return;
    }
    char buffer[32];
    sprintf(buffer, "Emergency Detected!");
    printf("Sender: Sending (%s)\n", buffer);
    int rc = zmq_send(requester, buffer, strlen(buffer), 0);

    int num = zmq_recv(requester, buffer, 128, 0);

    if (num > 0)
    {
        buffer[num] = '\0';
        printf("Receiver: Received (%s)\n", buffer);
    }
}


void MessageTransmitter::setup()
{
    // Socket to talk to clients
    char buffer[32];

    context = zmq_ctx_new();
    requester = zmq_socket(context, ZMQ_REQ);
    //int rc = zmq_connect(requester, "tcp://10.177.255.89:5556");
    int rc = zmq_connect(requester, m_ipString.c_str());

    printf("Sender: Started: RC: %d\n", rc);


    sprintf(buffer, "Hello From Client\n");
    printf("Sender: Sending (%s)\n", buffer);
    rc = zmq_send(requester, buffer, strlen(buffer), 0);

    //std::this_thread::sleep_for(std::chrono::milliseconds(10));


    int num = zmq_recv(requester, buffer, 128, 0);

    if (num > 0)
    {
        buffer[num] = '\0';
        printf("Receiver: Received (%s)\n", buffer);
    }
}

void MessageTransmitter::cleanup()
{
    zmq_close(requester);
    zmq_ctx_destroy(context);
}
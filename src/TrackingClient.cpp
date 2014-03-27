//
//  TrackingClient.cpp
//  kinectTCPServer
//
//  Created by maybites on 14.02.14.
//
//

#include "TrackingClient.h"

TrackingClient::TrackingClient(string _destination, int _port){
	//Client side
	clientDestination = _destination;
	clientSendPort = _port;
}

void TrackingClient::update(long currentMillis){
    aliveUntil = currentMillis;
}

bool TrackingClient::stillAlive(long currentMillis){
    return (currentMillis - STAY_ALIVE > aliveUntil)? false: true;
}



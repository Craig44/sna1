/*
 * Event.h
 *
 *  Created on: 15/06/2018
 *      Author: C.Marsh
 */
#pragma once

#include "Agent.h"

/**
 * A tagging event
 *
 * Copies the fish and adds a time stamp
 */
class Event : public Agent {
 public:
    Time time_;
    Method method_;
    /*
     *
     * Constructor used for both release and
     * recapture events
     */

    Event(const Agent& agent, Time time, Method method):
      Agent(agent),
      time_(time),
      method_(method){
    }


     /*
     * Default constructer used for the recapture
     * event when an entry is made in database.
     * Signified as empty by `time==0`
      */

    Event(void):
        time_(0){
    }
};


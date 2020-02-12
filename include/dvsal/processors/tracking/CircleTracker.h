//---------------------------------------------------------------------------------------------------------------------
//  DVSAL
//---------------------------------------------------------------------------------------------------------------------
//  Copyright 2020 - Marco Montes Grova (a.k.a. mgrova) marrcogrova@gmail.com 
//---------------------------------------------------------------------------------------------------------------------
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
//  and associated documentation files (the "Software"), to deal in the Software without restriction, 
//  including without limitation the rights to use, copy, modify, merge, publish, distribute, 
//  sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is 
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all copies or substantial 
//  portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING 
//  BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
//  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES 
//  OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN 
//  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//---------------------------------------------------------------------------------------------------------------------

#ifndef DV_CIRCLE_TRACKING_H_
#define DV_CIRCLE_TRACKING_H_

#include <libcaer/events/polarity.h>
#include <iostream>
#include <atomic>
#include <csignal>
#include <cmath>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

#define RADIUS 37
#define SIZEX 240
#define SIZEY 180

#include <dv-sdk/processing.hpp>
#include <dv-sdk/config.hpp>
#include <dv-sdk/utils.h>

namespace dvsal{

    struct posCircle{
    	double x, // x-position
    		  y, // y-position
    		  u, // x-speed
    		  v, // y-speed
    		  a, // acc-x
    		  b, //	acc-y
    		  timestamp; // timestamp of the position
    };

    class CircleTracker{
    
    public:
        CircleTracker();

        /// Sort a vector with the last position of the circle and delete the events that are out of a ring
        /// in -> event
        /// pos-> last position of circle
        bool isEventInCircle(dv::Event &in, posCircle pos);

        /// Increase the value of the hough point in the array
        void increaseHoughPoint(u_int32_t x, u_int32_t y, float weight,u_int32_t& maxX, u_int32_t& maxY,u_int32_t& maxValue, u_int32_t** accumulatorArray);
        
        /// This algorithm is copied from the jAER Project of the company Inivation and adapted for C++
        /// Fast inclined ellipse drawing algorithm; ellipse eqn: A*x^2+B*y^2+C*x*y-1 = 0
        /// the algorithm is fast because it uses just integer addition and subtraction
        void accumulate(dv::Event event, float weight, u_int32_t& maxX, u_int32_t& maxY,u_int32_t& maxValue, u_int32_t** accumulatorArray);

        
        /// Generate hough with a vector of events
        /// in -> vector of events to generate hough
        /// maxX -> variable to save max of in X-direction
        void houghCircle(dv::EventStore in,u_int32_t& maxX,u_int32_t& maxY,bool writeToFile);
        
        // Fitting a cylinder in a vector of events with gradient descent.
        // Optimized calculations to make algorithm fast enough.
        // data -> converted events from PolarityEventPacket to a vector -> getXYT
        // pos -> last calculated position
        posCircle cylinderFitting(dv::EventStore data, posCircle pos);
        
        // Generate PolarityEventPacket from vector 
        // sortEvVect -> Vector with events from Type dv::Event        
        caerPolarityEventPacket generatePolarityEventPacket(dv::EventStore sortEvVect);

        /// Generate vector of events from eventPacket 
        /// pPolPack -> PolarityEventPacket from Camera or File
        /// packetSize -> size of EventPacket
        dv::EventStore getVectorFromEventPacket(caerPolarityEventPacket pPolPack,u_int32_t packetSize);

        private:
            int camSizeX_     = 240;
            int camSizeY_     = 180;
            int circleRadius_ = 37;
    };
}

#endif
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

#include "dvsal/processors/tracking/CircleTracker.h"

namespace dvsal{

    CircleTracker::CircleTracker(){

    }

    bool CircleTracker::isEventInCircle(dv::Event &in, posCircle pos){
        const u_int32_t thickness=2;
        double r_min    = (circleRadius_-thickness) * (circleRadius_-thickness);
        double r_max    = (circleRadius_+thickness) * (circleRadius_+thickness);
        double x_d      = abs(in.x() - pos.x);
        double y_d      = abs(in.y() - pos.y);
        double distance = (x_d*x_d) + (y_d*y_d);
        if(distance < r_min || distance > r_max)
                return false;
        return true;
    }

    void CircleTracker::increaseHoughPoint(u_int32_t x, u_int32_t y, float weight,u_int32_t& maxX, u_int32_t& maxY,u_int32_t& maxValue, u_int32_t** accumulatorArray) {
        if ((x < 0) || (x >(camSizeX_ - 1)) || (y < 0) || (y > (camSizeY_ - 1))) {
            return;
        }

        // increase the value of the hough point
        accumulatorArray[x][y] = accumulatorArray[x][y] + weight;

        // check if this is a new maximum
        if (accumulatorArray[x][y] >= maxValue) {
                maxValue = accumulatorArray[x][y];
                maxX = x;
                maxY = y;
        }
        return;
    }


    void CircleTracker::accumulate(dv::Event event, float weight, u_int32_t& maxX, u_int32_t& maxY,u_int32_t& maxValue, u_int32_t** accumulatorArray){

        // TODO: this is a little overhead here, since we only draw circles in
        // Hough space (not ellipses)
        int centerX = (int)event.x();
        int centerY = (int)event.y();
        int aa	  = round(circleRadius_*circleRadius_);
        int bb	  = aa;
        int twoC	= 0;

        int x = 0;
        int y = round((float)sqrt(bb));
        int twoaa = 2*aa;
        int twobb = 2*bb;
        int dx =   (twoaa*y) + (twoC*x);   //slope =dy/dx
        int dy = -((twobb*x) + (twoC*y));
        int ellipseError = aa*((y*y)-bb);

        // first sector: (dy/dx > 1) -> y+1 (x+1)
        // d(x,y+1)   = 2a^2y+a^2+2cx				= dx+aa
        // d(x+1,y+1) = 2b^2x+b^2+2cy+2c+2a^2y+a^2+2cx = d(x,y+1)-dy+bb
        while (dy > dx){
            increaseHoughPoint(centerX+x,centerY+y,weight, maxX, maxY, maxValue,accumulatorArray);
            increaseHoughPoint(centerX-x,centerY-y,weight, maxX, maxY, maxValue,accumulatorArray);
            ellipseError = ellipseError + dx + aa;
            dx = dx + twoaa;
            dy = dy - twoC;
            y = y + 1;
            if ((((2*ellipseError)-dy)+bb) > 0) {
                ellipseError = (ellipseError - dy) + bb ;
                dx = dx + twoC;
                dy = dy - twobb;
                x = x + 1;
            }
        }

        // second sector: (dy/dx > 0) -> x+1 (y+1)
        // d(x+1,y)   = 2b^2x+b^2+2cy				= -dy+bb
        // d(x+1,y+1) = 2b^2x+b^2+2cy+2c+2a^2y+a^2+2cx = d(x+1,y)+dx+aa
        while (dy > 0){
            increaseHoughPoint(centerX+x,centerY+y,weight, maxX, maxY, maxValue,accumulatorArray);
            increaseHoughPoint(centerX-x,centerY-y,weight, maxX, maxY, maxValue,accumulatorArray);
            ellipseError = (ellipseError - dy) + bb;
            dx = dx + twoC;
            dy = dy - twobb;
            x = x + 1;
            if (((2*ellipseError) + dx + aa) < 0){
                ellipseError = ellipseError + dx + aa ;
                dx = dx + twoaa;
                dy = dy - twoC;
                y = y + 1;
            }
        }

        // third sector: (dy/dx > -1) -> x+1 (y-1)
        // d(x+1,y)   = 2b^2x+b^2+2cy				= -dy+bb
        // d(x+1,y-1) = 2b^2x+b^2+2cy-2c-2a^2y+a^2-2cx = d(x+1,y)-dx+aa
        while (dy > - dx){
            increaseHoughPoint(centerX+x,centerY+y,weight, maxX, maxY, maxValue,accumulatorArray);
            increaseHoughPoint(centerX-x,centerY-y,weight, maxX, maxY, maxValue,accumulatorArray);
            ellipseError = (ellipseError - dy) + bb;
            dx = dx + twoC;
            dy = dy - twobb;
            x = x + 1;
            if ((((2*ellipseError) - dx) + aa) > 0){
                ellipseError = (ellipseError - dx) + aa;
                dx = dx - twoaa;
                dy = dy + twoC;
                y = y - 1;
            }
        }

        // fourth sector: (dy/dx < 0) -> y-1 (x+1)
        // d(x,y-1)   = -2a^2y+a^2-2cx			   = -dx+aa
        // d(x+1,y-1) = 2b^2x+b^2+2cy-2c-2a^2y+a^2-2cx = d(x+1,y)-dy+bb
        while (dx > 0){
            increaseHoughPoint(centerX+x,centerY+y,weight, maxX, maxY, maxValue,accumulatorArray);
            increaseHoughPoint(centerX-x,centerY-y,weight, maxX, maxY, maxValue,accumulatorArray);
            ellipseError = (ellipseError - dx) + aa;
            dx = dx - twoaa;
            dy = dy + twoC;
            y = y - 1;
            if ((((2*ellipseError) - dy) + bb) < 0){
                ellipseError = (ellipseError - dy) + bb;
                dx = dx + twoC;
                dy = dy - twobb;
                x = x + 1;
            }
        }

        //fifth sector (dy/dx > 1) -> y-1 (x-1)
        // d(x,y-1)   = -2a^2y+a^2-2cx				= -dx+aa
        // d(x-1,y-1) = -2b^2x+b^2-2cy+2c-2a^2y+a^2-2cx = d(x+1,y)+dy+bb
        while ((dy < dx)&& (x > 0)){
            increaseHoughPoint(centerX+x,centerY+y,weight, maxX, maxY, maxValue,accumulatorArray);
            increaseHoughPoint(centerX-x,centerY-y,weight, maxX, maxY, maxValue,accumulatorArray);
            ellipseError = (ellipseError - dx) + aa;
            dx = dx - twoaa;
            dy = dy + twoC;
            y = y - 1;
            if (((2*ellipseError) + dy + bb) > 0){
                ellipseError = ellipseError  + dy + bb;
                dx = dx - twoC;
                dy = dy + twobb;
                x = x - 1;
            }
        }

        // sixth sector: (dy/dx > 0) -> x-1 (y-1)
        // d(x-1,y)   = -2b^2x+b^2-2cy				= dy+bb
        // d(x-1,y-1) = -2b^2x+b^2-2cy+2c-2a^2y+a^2-2cx = d(x+1,y)-dx+aa
        while ((dy < 0)&& (x > 0)){
            increaseHoughPoint(centerX+x,centerY+y,weight, maxX, maxY, maxValue,accumulatorArray);
            increaseHoughPoint(centerX-x,centerY-y,weight, maxX, maxY, maxValue,accumulatorArray);
            ellipseError = ellipseError + dy + bb;
            dx = dx - twoC;
            dy = dy + twobb;
            x = x - 1;
            if ((((2*ellipseError) - dx) + aa) < 0){
                ellipseError = (ellipseError  - dx) + aa;
                dx = dx - twoaa;
                dy = dy + twoC;
                y = y - 1;
            }
        }

        // seventh sector: (dy/dx > -1) -> x-1 (y+1)
        // d(x-1,y)   = -2b^2x+b^2-2cy				= dy+bb
        // d(x-1,y+1) = -2b^2x+b^2-2cy-2c+2a^2y+a^2+2cx = d(x+1,y)-dx+aa
        while ((dy < - dx)&& (x > 0)){
            increaseHoughPoint(centerX+x,centerY+y,weight, maxX, maxY, maxValue,accumulatorArray);
            increaseHoughPoint(centerX-x,centerY-y,weight, maxX, maxY, maxValue,accumulatorArray);
            ellipseError = ellipseError + dy + bb;
            dx = dx - twoC;
            dy = dy + twobb;
            x = x - 1;
            if (((2*ellipseError) + dx + aa) > 0){
                ellipseError = ellipseError  + dx + aa;
                dx = dx + twoaa;
                dy = dy - twoC;
                y = y + 1;
            }
        }

        // eight sector: (dy/dx < 0) -> y+1 (x-1)
        // d(x,y+1)   = 2a^2y+a^2+2cx				 = dx+aa
        // d(x-1,y+1) = -2b^2x+b^2-2cy-2c+2a^2y+a^2+2cx = d(x,y+1)+dy+bb
        while (((dy > 0) && (dx < 0))&& (x > 0)){
            increaseHoughPoint(centerX+x,centerY+y,weight, maxX, maxY, maxValue,accumulatorArray);
            increaseHoughPoint(centerX-x,centerY-y,weight, maxX, maxY, maxValue,accumulatorArray);
            ellipseError = ellipseError + dx + aa;
            dx = dx + twoaa;
            dy = dy - twoC;
            y = y + 1;
            if (((2*ellipseError) + dy + bb) < 0){
                ellipseError = ellipseError + dy + bb ;
                dx = dx - twoC;
                dy = dy + twobb;
                x = x - 1;
            }
        }
    }

    void CircleTracker::houghCircle(dv::EventStore in,u_int32_t& maxX,u_int32_t& maxY, bool writeToFile){
        u_int32_t maxValue=0;
        u_int32_t**accumulatorArray;
        accumulatorArray = new u_int32_t *[camSizeX_];
        for(int i = 0; i <camSizeX_; i++)
            accumulatorArray[i] = new u_int32_t[camSizeY_];

        for(int i=0;i<camSizeX_;i++){
            for(int j=0;j<camSizeY_;j++){
                accumulatorArray[i][j]=0;
            }
        }
        for (auto ev : in){
            accumulate(ev,1, maxX,maxY,maxValue, accumulatorArray);
        }

        // write hough to file
        if(writeToFile){
            std::ofstream myfile1;
            myfile1.open("hough.csv");
            for(int i=0;i<camSizeY_;i++){
                for(int j=0;j<camSizeX_;j++){
                    myfile1<<accumulatorArray[j][i]<<";";
                }
                myfile1<<"\n";
            }
            myfile1.close();
        }
    }

    dv::EventStore CircleTracker::getVectorFromEventPacket(caerPolarityEventPacket pPolPack,u_int32_t packetSize){
        dv::EventStore dvEvents;
        for(u_int32_t i=0;i<packetSize;i++){

            dv::Event tmpEv(caerPolarityEventGetTimestamp(&pPolPack->events[i]) , 
                            caerPolarityEventGetX(&pPolPack->events[i]) , caerPolarityEventGetY(&pPolPack->events[i]) , 
                            caerPolarityEventGetPolarity(&pPolPack->events[i]));
            dvEvents.add(tmpEv);
        }
        return dvEvents;
    }

    caerPolarityEventPacket CircleTracker::generatePolarityEventPacket(dv::EventStore sortEvVect){
        caerPolarityEventPacket pPolPack;
        pPolPack =  caerPolarityEventPacketAllocate(sortEvVect.size() , 0 , 0);
        
        u_int32_t index = 0;
        for (auto event : sortEvVect){
            caerPolarityEventSetPolarity(&pPolPack->events[index]  , event.polarity());
            caerPolarityEventSetTimestamp(&pPolPack->events[index] , event.timestamp());
            caerPolarityEventSetX(&pPolPack->events[index]         , event.x());
            caerPolarityEventSetY(&pPolPack->events[index]         , event.y());
            caerPolarityEventValidate(&pPolPack->events[index]     , pPolPack);

            index++;
        }

        return pPolPack;
    }

    posCircle CircleTracker::cylinderFitting(dv::EventStore data, posCircle pos){

        u_int32_t k=0;
        u_int32_t n=data.size();
        if(n>1){
            std::vector<double> s;
            std::vector<double> timestamps;
            double timestamp;
            double dx0=0,dy0=0,du0=0,dv0=0,da0=0,db0=0;
            double alpha=5e-8; //schnell 5e-8
            int64_t timestampEnd = data.getHighestTime();
            int64_t timestampBegin = data.getLowestTime();
            for (auto ev : data)
                timestamps.push_back((ev.timestamp()-timestampBegin)/100000.0);
            
            double xtu=0,ytv=0;
            dv::Event tmp;
            while (true) {
                k++;
                dx0=0,dy0=0,du0=0,dv0=0,da0=0,db0=0;
                double circleRadius_2=circleRadius_*circleRadius_;
                int i = 0;
                for (auto ev : data){
                    tmp=ev;
                    timestamp=timestamps[i];
                    xtu=(tmp.x()-pos.x-timestamp*pos.u);
                    ytv=(tmp.y()-pos.y-timestamp*pos.v);
                    s.push_back( (xtu * xtu) + (ytv * ytv) - circleRadius_2);
                    i++;
                }

                double si2=0,sxxtu=0,syytu=0;
                int ii = 0;
                for (auto ev : data){
                    tmp = ev;
                    timestamp = timestamps[ii];
                    si2   = s[ii]*2;
                    sxxtu = si2*(tmp.x()-pos.x-timestamp*pos.u);
                    syytu = si2*(tmp.y()-pos.y-timestamp*pos.v);
                    dx0   = dx0+sxxtu;
                    dy0   = dy0+syytu;
                    du0   = du0+sxxtu*timestamp;
                    dv0   = dv0+syytu*timestamp;
                }

                //position update
                pos.x=pos.x+alpha*dx0;
                pos.y=pos.y+alpha*dy0;
                pos.u=pos.u+alpha*du0;
                pos.v=pos.v+alpha*dv0;

                // abort condition
                if ((abs(dx0)< 1e5) && (abs(dy0)< 1e5) && (abs(du0)< 1e5) && (abs(dv0) < 1e5) /*&& (abs(da0)< 1e5)&& (abs(db0)< 1e5)*/){
                    break;
                }

                //if it doesn't stop break after 20000 iterations
                if(k>20000){
                    break;
                }
            }

            pos.x=pos.x+(timestampEnd-timestampBegin)*pos.u/100000.0;// quadratic cylinderfitting -> +((timestampEnd-timestampBegin)*(timestampEnd-timestampBegin))/2*pos.a/(1000^2);
            pos.y=pos.y+(timestampEnd-timestampBegin)*pos.v/100000.0;// quadratic cylinderfitting -> +((timestampEnd-timestampBegin)*(timestampEnd-timestampBegin))/2*pos.b/(1000^2);
            pos.timestamp=timestampEnd;
        }
        return pos;
    }





}
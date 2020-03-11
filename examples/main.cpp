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

#include <dvsal/streamers/Streamer.h>
#include <dvsal/processors/corner_detectors/FastDetector.h>

#include "thread"

bool run = true;
dvsal::Streamer *streamer = nullptr;
dvsal::Detector *detector = nullptr;

int main(int _argc, char **_argv){

    streamer = dvsal::Streamer::create(dvsal::Streamer::eModel::dataset);
    
    detector = new dvsal::FastDetector();

    std::string datasetPath = "/home/marrcogrova/Downloads/shapes_rotation/events.txt";
    if (!streamer->init(datasetPath)){
        std::cout << "Error creating streamer" << std::endl;
        return 0;
    }

    while(run){
        run = streamer->step();

        dv::EventStore UnfiltEvents;
        streamer->events(UnfiltEvents);
        std::cout << UnfiltEvents.size() << std::endl;

        if (UnfiltEvents.size() > 1000){
            dv::EventStore ev = UnfiltEvents.slice(-1000);
            detector->eventCallback(ev);

            dv::EventStore corners = detector->cornersDetected();
        }
        std::this_thread::sleep_for( std::chrono::milliseconds(1) );
    }

    std::cout << "finished program" << std::endl;

    return 0;
}
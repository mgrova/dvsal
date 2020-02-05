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

#include "dvsal/flow/BlockDvDatasetStreamer.h"

#include <QSpinBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>

namespace dvsal{
    BlockDvDatasetStreamer::BlockDvDatasetStreamer(){
        streamer_ = DvStreamer::create(DvStreamer::eModel::dataset);
        
        createPipe("events", "v_event");
        createPipe("events-size", "int");
    }

    bool BlockDvDatasetStreamer::configure(std::unordered_map<std::string, std::string> _params){
        if(isRunningLoop()) // Cant configure if already running.
            return false;   

        std::string datasetPath = _params["dataset_path"];

        if(!streamer_->init(datasetPath.c_str()))
            return false;
 
        return true;
    }

    std::vector<std::string> BlockDvDatasetStreamer::parameters(){
        return {
            "dataset_path" 
        };
    }

    QWidget * BlockDvDatasetStreamer::customWidget(){
        QGroupBox *box = new QGroupBox;
        QHBoxLayout *layout = new QHBoxLayout;
        box->setLayout(layout);
        QLabel *label = new QLabel("events Hz");
        layout->addWidget(label);

        QSpinBox *rateController = new QSpinBox;
        rateController->setMinimum(1);
        rateController->setMaximum(100000);
        rateController->setValue(int(eventRate_));
        layout->addWidget(rateController);

        QWidget::connect(rateController, QOverload<int>::of(&QSpinBox::valueChanged), [this](int _val){
            eventRate_ = _val;
        });

        return box;
    }


    void BlockDvDatasetStreamer::loopCallback() {
        while(isRunningLoop()){

            streamer_->step();

            dv::EventStore rawEvents;
            streamer_->events(rawEvents);
            
            // 666 must be better way to make it
            int64_t microsec = 1/(eventRate_*0.000001);
            if (rawEvents.getHighestTime() > microsec + lastHighest_){
                streamer_->cutUsingTime(lastHighest_);
                
                dv::EventStore outputEvents;
                streamer_->events(outputEvents);
                
                lastHighest_ = outputEvents.getHighestTime();
                
                if(getPipe("events")->registrations() !=0 ){
                    getPipe("events")->flush(outputEvents);    
                }
            }

            if(getPipe("events-size")->registrations() !=0 ){
                dv::EventStore store;
                streamer_->events(store);
                std::cout <<static_cast<int>(store.size()) <<std::endl;
                getPipe("events-size")->flush(static_cast<int>(store.size()));
            }    
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

}
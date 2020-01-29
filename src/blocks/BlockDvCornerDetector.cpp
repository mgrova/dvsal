//---------------------------------------------------------------------------------------------------------------------
//  DVSAL
//---------------------------------------------------------------------------------------------------------------------
//  Copyright 2019 - Marco Montes Grova (a.k.a. marrcogrova) 
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

#include "dvsal/blocks/BlockDvCornerDetector.h"

namespace dvsal{

    BlockDvCornerDetector::BlockDvCornerDetector(){
        detectorList_ = { new FastDetector,
                          new HarrisDetector };

        initVisualization();

        createPipe("Corners events", "v_event");
        
        createPolicy({{"Unfiltered events", "v_event"}});
        registerCallback({"Unfiltered events"}, 
                                [&](flow::DataFlow _data){
                                    if(idle_){
                                        idle_ = false;
                                        auto UnfiltEvents = _data.get<dv::EventStore>("Unfiltered events");
                                        detectorGuard_.lock();
                                        currentDetector_->eventCallback(UnfiltEvents);
                                        dv::EventStore corners = currentDetector_->cornersDetected();
                                        detectorGuard_.unlock();
                                        if(corners.size() > 0){
                                            getPipe("Corners events")->flush(corners);
                                        }
                                        idle_ = true;
                                    }
                                }
        );
    }

    void BlockDvCornerDetector::initVisualization(){
        visualContainer_ = new QGroupBox();
        mainLayout_ = new QVBoxLayout();
        visualContainer_->setLayout(mainLayout_);

        detectorSelector_ = new QComboBox();
        mainLayout_->addWidget(detectorSelector_);

        for(auto &detector:detectorList()){
            detectorSelector_->addItem(detector->name().c_str());
        }

        QWidget::connect(detectorSelector_, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int _n){ this->changeDetector(_n); });
        currentDetector_ = detectorList_[0];
        detectorSelector_->setCurrentIndex(0);
    }

    std::vector<Detector*> BlockDvCornerDetector::detectorList(){
        return detectorList_;
    }

    void BlockDvCornerDetector::changeDetector(int _index){
        detectorGuard_.lock();

        if(currentDetector_->customWidget()){
            currentDetector_->customWidget()->setVisible(false);
            mainLayout_->removeWidget(currentDetector_->customWidget());
        }
        currentDetector_ = detectorList_[_index];
        auto widget = currentDetector_->customWidget();
        if(widget){
            widget->setVisible(true);
            mainLayout_->addWidget(widget);
        }

        detectorGuard_.unlock();
    }
}

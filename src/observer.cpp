// -*-c++-*-
//******************************************************************
//
// Copyright 2014 Intel Corporation.
// Copyright 2016 Samsung <philippe.coval@osg.samsung.com>
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "common.h"
#include <cstdio>

#include "observer.h"

using namespace std;
using namespace OC;

IoTObserver *IoTObserver::mInstance = nullptr;
const OC::ObserveType IoTObserver::OBSERVE_TYPE_TO_USE = OC::ObserveType::Observe;


IoTObserver::IoTObserver()
{
    cerr << __PRETTY_FUNCTION__ << endl;
    init();
}

IoTObserver *IoTObserver::getInstance()
{
    cerr << __PRETTY_FUNCTION__;
    if (IoTObserver::mInstance == nullptr)
    {
        mInstance = new IoTObserver;
    }
    return mInstance;
}

IoTObserver::~IoTObserver()
{
    cerr << __PRETTY_FUNCTION__ << endl;
}

void IoTObserver::init()
{
    cerr << __PRETTY_FUNCTION__ << endl;

    m_PlatformConfig = make_shared<PlatformConfig>
                       ( ServiceType::InProc, //
                         ModeType::Client, //
                         "0.0.0.0", //
                         0, //
                         OC::QualityOfService::HighQos);

    OCPlatform::Configure(*m_PlatformConfig);
    m_FindCallback = bind(&IoTObserver::onFind, this, placeholders::_1);
}

void IoTObserver::start()
{
    cerr << __PRETTY_FUNCTION__ << endl;
    string uri = string(OC_RSRVD_WELL_KNOWN_URI);

    cerr << "URI: " << uri << endl;
    OCConnectivityType connectivityType(CT_ADAPTER_IP);
    OCPlatform::findResource("",  //
                             uri.c_str(), //
                             connectivityType, // IP
                             m_FindCallback, //cb
                             OC::QualityOfService::HighQos // TODO
                            );
}


void IoTObserver::onFind(shared_ptr<OC::OCResource> resource)
{
    cerr << __PRETTY_FUNCTION__ << endl;
    try
    {
        if (resource)
        {
            print(resource);

            if (resource->uri() == Common::m_endpoint)
            {
                QueryParamsMap test;
                resource->observe(OBSERVE_TYPE_TO_USE, test, &IoTObserver::onObserve);

            }
        }
    }
    catch (OCException &ex)
    {
        cerr << "Caught exception in discoveredResource: " << ex.reason() << endl;
    }
}


void IoTObserver::onObserve(const HeaderOptions headerOptions, const OCRepresentation &rep,
                            const int &eCode, const int &sequenceNumber)
{
    cerr << __PRETTY_FUNCTION__ << endl;
    try
    {
        if (eCode == OC_STACK_OK && sequenceNumber != OC_OBSERVE_NO_OPTION)
        {
            if (sequenceNumber == OC_OBSERVE_REGISTER)
            {
                std::cerr << "Observe registration action is successful" << std::endl;
            }
            else if (sequenceNumber == OC_OBSERVE_DEREGISTER)
            {
                std::cerr << "Observe De-registration action is successful" << std::endl;
            }
            std::cerr << "OBSERVE RESULT:" << std::endl;
            std::cerr << "\tSequenceNumber: " << sequenceNumber << std::endl;
            handle(headerOptions, rep, eCode, sequenceNumber);
        }
        else
        {
            if (sequenceNumber == OC_OBSERVE_NO_OPTION)
            {
                std::cerr << "Observe registration or de-registration action is failed" << std::endl;
            }
            else
            {
                std::cerr << "onObserve Response error: " << eCode << std::endl;
                std::exit(-1);
            }
        }
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << " in onObserve" << std::endl;
    }
}


void IoTObserver::print(shared_ptr<OCResource> resource)
{
    string resourceUri = resource->uri();
    string hostAddress = resource->host();
    cerr << "host: " << hostAddress << endl;


    cerr << "\nFound Resource" << endl << "Resource Types:" << endl;
    for (auto &resourceTypes : resource->getResourceTypes())
    {
        cerr << "\t" << resourceTypes << endl;
    }

    cerr << "Resource Interfaces: " << endl;
    for (auto &resourceInterfaces : resource->getResourceInterfaces())
    {
        cerr << "\t" << resourceInterfaces << endl;
    }
    cerr << "Resource uri: " << resourceUri << endl;

}


// TODO: overide with your business logic
void IoTObserver::handle(const HeaderOptions headerOptions, const OCRepresentation &rep,
                         const int &eCode, const int &sequenceNumber)
{

    bool value = false;
    rep.getValue(Common::m_propname, value);

    std::cerr << Common::m_propname << "=" << value << std::endl;
    std::cout << value << std::endl;
}



int IoTObserver::main(int argc, char *argv[])
{
    IoTObserver::getInstance()->start();

    int choice = 0;
    do
    {
        cin >> choice;
        switch (choice)
        {
            case 9:
            default:
                return 0;
        }
    }
    while (choice != 9);
    return 0;
}


#ifdef CONFIG_OBSERVER_MAIN
int main(int argc, char *argv[])
{
    return IoTObserver::main(argc, argv);
}
#endif

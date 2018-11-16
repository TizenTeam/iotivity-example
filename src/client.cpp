// -*-c++-*-
//******************************************************************
//
// Copyright 2014 Intel Corporation.
// Copyright 2015 Eurogiciel <philippe.coval@eurogiciel.fr>
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
#include <unistd.h>
#include <ctime>
#include "client.h"
#include "platform.h"

using namespace std;
using namespace OC;


Resource::Resource(shared_ptr<OCResource> resource)
{
    LOG();
    m_OCResource = resource;
    m_GETCallback = bind(&Resource::onGet, this,
                         placeholders::_1, placeholders::_2, placeholders::_3);

    m_POSTCallback = bind(&Resource::onPost, this,
                          placeholders::_1, placeholders::_2, placeholders::_3);
}

Resource::~Resource()
{
}


void Resource::onGet(const HeaderOptions &headerOptions,
                     const OCRepresentation &representation, int eCode)
{
    LOG();
    if (eCode < OC_STACK_INVALID_URI)
    {
        bool value;
        representation.getValue(Common::m_propname, value);
        cout << value << endl;
    }
    else
    {
        cerr << "errror:: in GET response:" << eCode << endl;
    }
    IoTClient::menu();
}

void Resource::onPost(const HeaderOptions &headerOptions,
                      const OCRepresentation &representation, int eCode)
{
    LOG();
    if (eCode < OC_STACK_INVALID_URI)
    {
        bool value;
        representation.getValue(Common::m_propname, value);
        Platform::getInstance().setValue(value);
    }
    else
    {
        cerr << "error: in POST response: " << eCode <<  endl;
    }
    IoTClient::menu();
}


void Resource::get()
{
    LOG();
    QueryParamsMap params;
    m_OCResource->get(params, m_GETCallback);
}

void Resource::post(bool value)
{
    LOG();
    QueryParamsMap params;
    OCRepresentation rep;
    rep.setValue(Common::m_propname, value);
    m_OCResource->post(rep, params, m_POSTCallback);
}


IoTClient::IoTClient()
{
    LOG();
    init();
}

IoTClient::~IoTClient()
{
    LOG();
}


IoTClient *IoTClient::mInstance = nullptr;

IoTClient *IoTClient::getInstance()
{
    if (!IoTClient::mInstance)
    {
        mInstance = new IoTClient;
    }
    return mInstance;
}

void IoTClient::init()
{
    LOG();

    m_platformConfig = make_shared<PlatformConfig>
                       (ServiceType::InProc, //
                        ModeType::Both, //
                        "0.0.0.0", //
                        0, //
                        OC::QualityOfService::LowQos //
                       );
    OCPlatform::Configure(*m_platformConfig);
    m_FindCallback = bind(&IoTClient::onFind, this, placeholders::_1);
}


void IoTClient::start()
{
    LOG();
    string coap_multicast_discovery = string(OC_RSRVD_WELL_KNOWN_URI);

    OCConnectivityType connectivityType(CT_ADAPTER_IP);
    try
    {
        OCPlatform::findResource("", //
                                 coap_multicast_discovery.c_str(),
                                 connectivityType,
                                 m_FindCallback,
                                 OC::QualityOfService::LowQos);
    }
    catch (OCException &e)
    {
        cerr << "error: Exception: " << e.what();
        exit(1);
    }
}

shared_ptr<Resource> IoTClient::getResource()
{
    return m_Resource;
}


void IoTClient::onFind(shared_ptr<OCResource> resource)
{
    LOG();
    try
    {
        if (resource)
        {
            print(resource);

            string resourceUri = resource->uri();
            if (Common::m_endpoint == resourceUri)
            {
                cerr << "resourceUri=" << resourceUri << endl;
                m_Resource = make_shared<Resource>(resource);

                if (true)   // multi client need observe (for flip/flop)
                {
                    QueryParamsMap test;
                    resource->observe(OC::ObserveType::Observe, test, &IoTClient::onObserve);
                }
                else     // simple client can only use get once
                {
                    m_Resource->get();
                }
                menu();
            }

        }
    }
    catch (OCException &ex)
    {
        cerr << "error: Caught exception in discoveredResource: " << ex.reason() << endl;
    }
}


void IoTClient::print(shared_ptr<OCResource> resource)
{
    LOG();
    cerr << "log: Resource: uri: " << resource->uri() << endl;
    cerr << "log: Resource: host: " << resource->host() << endl;

    for (auto &type : resource->getResourceTypes())
    {
        cerr << "log: Resource: type: " << type << endl;
    }

    for (auto &interface : resource->getResourceInterfaces())
    {
        cerr << "log: Resource: interface: " << interface << endl;
    }
}

void IoTClient::onObserve(const HeaderOptions headerOptions, const OCRepresentation &rep,
                          const int &eCode, const int &sequenceNumber)
{
    LOG();
    try
    {
        if (eCode == OC_STACK_OK && sequenceNumber != OC_OBSERVE_NO_OPTION)
        {
            if (sequenceNumber == OC_OBSERVE_REGISTER)
            {
                cerr << "Observe registration action is successful" << endl;
            }
            else if (sequenceNumber == OC_OBSERVE_DEREGISTER)
            {
                cerr << "Observe De-registration action is successful" << endl;
            }
            cerr << "log: observe: sequenceNumber=" << sequenceNumber << endl;

            bool value;
            rep.getValue(Common::m_propname, value);
            cerr << "log: " << Common::m_propname << "=" << value << endl;
            IoTClient::getInstance()->m_value = value;
        }
        else
        {
            if (sequenceNumber == OC_OBSERVE_NO_OPTION)
            {
                cerr << "warning: Observe registration or de-registration action is failed" << endl;
            }
            else
            {
                cerr << "error: onObserve Response error=" << eCode << endl;
                //exit(-1);
            }
        }
    }
    catch (exception &e)
    {
        cerr << "warning: Exception: " << e.what() << " in onObserve" << endl;
    }

}

void IoTClient::menu()
{
    cerr << endl << "menu: "
         << "  0) Set value off"
         << "  1) Set value on"
         << "  2) Toggle value"
         << "  9) Quit"
         << "  *) Display this menu"
         << endl;
}


bool IoTClient::toggle()
{
    Common::log(__PRETTY_FUNCTION__);

    bool value = m_value;
    if (m_Resource)
    {
        m_Resource->post(!value);
    }
    else
    {
        cerr << "log: resource not yet discovered" << endl;
        Common::log("log: resource not yet discovered");
    }

    return value;
}


bool IoTClient::setValue(bool value)
{
    Common::log(__PRETTY_FUNCTION__);

    if (m_Resource)
    {
        m_Resource->post(value);
    }
    else
    {
        cerr << "log: resource not yet discovered" << endl;
        Common::log("log: resource not yet discovered");
    }

    return m_value;
}


int IoTClient::main(int argc, char *argv[])
{
    LOG();

    for (int i = 1; i < argc; i++)
    {
        if (0 == strcmp("-v", argv[i]))
        {
            Common::m_logLevel++;
        }
    }
    cerr<<Common::m_logLevel<<endl;
    IoTClient::getInstance()->start();

    int choice;
    do
    {
        cin >> choice;
        switch (choice)
        {
            case 0:
                IoTClient::getInstance()->setValue(false);
                break;
            case 1:
                IoTClient::getInstance()->setValue(true);
                break;
            case 2:
                IoTClient::getInstance()->toggle();
                break;
            case 9:
                return 0;
            default:
                IoTClient::menu();
                break;
        }
    }
    while (choice != 9);
    return 0;
}


#ifdef CONFIG_CLIENT_MAIN
int main(int argc, char *argv[])
{
    LOG();
    return IoTClient::main(argc, argv);
}
#endif

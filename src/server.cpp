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

#include <csignal>
#include <functional>

#include "server.h"
#include "platform.h"

using namespace std;
using namespace OC;


bool IoTServer::m_over = false;

IoTServer::IoTServer(string property, bool value)
{
    LOG();
    init();
    setup();
    m_Representation.setValue(property, value);
}

IoTServer::~IoTServer()
{
    LOG();
}

void IoTServer::init()
{
    LOG();

    m_platformConfig = make_shared<PlatformConfig>
                       (ServiceType::InProc, // different service ?
                        ModeType::Server, // other is Client or Both
                        "0.0.0.0", // default ip
                        0, // default random port
                        OC::QualityOfService::LowQos// qos
                       );
    OCPlatform::Configure(*m_platformConfig);
}

void IoTServer::setup()
{
    LOG();
    OCStackResult result ;
    EntityHandler handler = bind(&IoTServer::handleEntity, this, placeholders::_1);

    result = createResource(Common::m_endpoint, Common::m_type, handler, m_ResourceHandle);
    if (OC_STACK_OK != result)
    {
        cerr << "error: Error on createResource" << endl;
        throw OC::InitializeException(__PRETTY_FUNCTION__, result);
    }
}

OCStackResult IoTServer::createResource(string uri, string type, EntityHandler handler,
                                        OCResourceHandle &handle)
{
    LOG();
    OCStackResult result;
    string resourceUri = uri;
    string resourceType = type;
    string resourceInterface = Common::m_interface;
    uint8_t resourceFlag = OC_DISCOVERABLE | OC_OBSERVABLE;
    try
    {
        result = OCPlatform::registerResource // resource to be discovered
                 (handle,
                  resourceUri, resourceType,
                  resourceInterface, //
                  handler, resourceFlag);

        if (result != OC_STACK_OK)
            cerr << "error: Could not create " << type << " resource" << endl;
        else
            cerr << "log: Successfully created " << type << " resource" << endl;
    }

    catch (OC::OCException &e)
    {
        cerr << "error: OCException " <<  e.reason().c_str() << " " << hex << e.code();
        cerr << "error: @" << __PRETTY_FUNCTION__ << endl;
        result = OC_STACK_ERROR;
    }

    return result;
}


void IoTServer::postResourceRepresentation()
{
    LOG();
    bool value = 0;
    m_Representation.getValue(Common::m_propname, value);
    OCStackResult result = OCPlatform::notifyAllObservers(m_ResourceHandle);
}

OCStackResult IoTServer::respond(std::shared_ptr<OC::OCResourceResponse> response)
{
    OCStackResult result =  OC_STACK_ERROR;
    LOG();

    if (response)
    {
        response->setErrorCode(200);
        response->setResponseResult(OC_EH_OK);
        response->setResourceRepresentation(m_Representation);
        result = OCPlatform::sendResponse(response);
    }
    return result;
}

OCStackResult IoTServer::handlePost(shared_ptr<OCResourceRequest> request)
{
    LOG();
    OCStackResult result = OC_STACK_OK;

    OCRepresentation requestRep = request->getResourceRepresentation();
    if (requestRep.hasAttribute(Common::m_propname))
    {
        try
        {
            bool value = requestRep.getValue<bool>(Common::m_propname);
            Platform::getInstance().setValue(value);
        }
        catch (...)
        {
            cerr << "error: Client sent invalid resource value type" << endl;
            return result;
        }
    }
    else
    {
        cerr << "error: Client sent invalid resource property" << endl;
        return result;
    }
    m_Representation = requestRep;
    postResourceRepresentation();

    return result;
}

OCStackResult IoTServer::handleGet(shared_ptr<OCResourceRequest> request)
{
    LOG();
    OCStackResult result = OC_STACK_OK;

    return result;
}

OCEntityHandlerResult IoTServer::handleEntity(shared_ptr<OCResourceRequest> request)
{
    LOG();

    OCEntityHandlerResult result = OC_EH_ERROR;
    if (request)
    {
        string requestType = request->getRequestType();
        int requestFlag = request->getRequestHandlerFlag();
        if (requestFlag & RequestHandlerFlag::RequestFlag)
        {
            auto response = std::make_shared<OC::OCResourceResponse>();
            response->setRequestHandle(request->getRequestHandle());
            response->setResourceHandle(request->getResourceHandle());

            if (requestType == "POST")
            {
                if (handlePost(request) == OC_STACK_OK)
                {
                    if (respond(response) == OC_STACK_OK)
                    {
                        result = OC_EH_OK;
                    }
                }
                else
                {
                    response->setResponseResult(OC_EH_ERROR);
                    OCPlatform::sendResponse(response);
                }
                ;;
            }
            else if (requestType == "GET")
            {
                if (handleGet(request) == OC_STACK_OK)
                {
                    if (respond(response) == OC_STACK_OK)
                    {
                        result = OC_EH_OK;
                    }
                }
                else
                {
                    response->setResponseResult(OC_EH_ERROR);
                    OCPlatform::sendResponse(response);
                }

            }
            else
            {
                cerr << "error: unsupported " << requestType << endl;
                response->setResponseResult(OC_EH_ERROR);
                OCPlatform::sendResponse(response);
            }
        }
    }
    return result;
}


void IoTServer::handle_signal(int signal)
{
    LOG();
    IoTServer::m_over = true;
}


int IoTServer::main(int argc, char *argv[])
{
    LOG();
    struct sigaction sa;
    sigfillset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = IoTServer::handle_signal;
    sigaction(SIGINT, &sa, nullptr);

    cerr << "log: Server: " << endl
         << "Press Ctrl-C to quit...." << endl;

    for (int i = 1; i < argc; i++)
    {
        if (0 == strcmp("-v", argv[i]))
        {
            Common::m_logLevel++;
        }
    }

    Platform::getInstance().setup(argc, argv);

    IoTServer server;
    try
    {
        do
        {
            usleep(2000000);
        }
        while (!IoTServer::m_over );
    }
    catch (...)
    {
        exit(1);
    }
    return 0;
}

#ifdef CONFIG_SERVER_MAIN
int main(int argc, char *argv[])
{
    return IoTServer::main(argc, argv);
}
#endif

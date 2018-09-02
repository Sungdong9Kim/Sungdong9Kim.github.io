//******************************************************************
//
// Copyright 2017 Open Connectivity Foundation
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
#include <signal.h>
#include <functional>
#include <string>
#include <iostream>
#include <memory>
#include <exception>

#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif

#include "ocstack.h"
#include "OCPlatform.h"
#include "OCApi.h"
#include "ocpayload.h"

// code for dht22(Humidity and Temperature)--------------------------

#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MAX_TIMINGS    85
#define DHT_PIN        11    /* GPIO-22 */

int data[5] = { 0, 0, 0, 0, 0 };

float read_dht_data_CelTemp()
{
    uint8_t laststate    = HIGH;
    uint8_t counter        = 0;
    uint8_t j            = 0, i;

    data[0] = data[1] = data[2] = data[3] = data[4] = 0;

    /* pull pin down for 18 milliseconds */
    pinMode( DHT_PIN, OUTPUT );
    digitalWrite( DHT_PIN, LOW );
    delay( 18 );

    /* prepare to read the pin */
    pinMode( DHT_PIN, INPUT );

    /* detect change and read data */
    for ( i = 0; i < MAX_TIMINGS; i++ )
    {
        counter = 0;
        while ( digitalRead( DHT_PIN ) == laststate )
        {
            counter++;
            delayMicroseconds( 1 );
            if ( counter == 255 )
            {
                break;
            }
        }
        laststate = digitalRead( DHT_PIN );

        if ( counter == 255 )
            break;

        /* ignore first 3 transitions */
        if ( (i >= 4) && (i % 2 == 0) )
        {
            /* shove each bit into the storage bytes */
            data[j / 8] <<= 1;
            if ( counter > 16 )
                data[j / 8] |= 1;
            j++;
        }
    }

    /*
     * check we read 40 bits (8bit x 5 ) + verify checksum in the last byte
     * print it out if data is good
     */
    if ( (j >= 40) &&
         (data[4] == ( (data[0] + data[1] + data[2] + data[3]) & 0xFF) ) )
    {
        float c = (float)(((data[2] & 0x7F) << 8) + data[3]) / 10;
        if ( c > 125 )
        {
            c = data[2];    // for DHT11
        }
        if ( data[2] & 0x80 )
        {
            c = -c;
        }
        return c;
    }else  {
        printf( "Data not good, skip\n" );
        return 0;
    }
}

float read_dht_data_Humi()
{
    uint8_t laststate    = HIGH;
    uint8_t counter        = 0;
    uint8_t j            = 0, i;

    data[0] = data[1] = data[2] = data[3] = data[4] = 0;

    /* pull pin down for 18 milliseconds */
    pinMode( DHT_PIN, OUTPUT );
    digitalWrite( DHT_PIN, LOW );
    delay( 18 );

    /* prepare to read the pin */
    pinMode( DHT_PIN, INPUT );

    /* detect change and read data */
    for ( i = 0; i < MAX_TIMINGS; i++ )
    {
        counter = 0;
        while ( digitalRead( DHT_PIN ) == laststate )
        {
            counter++;
            delayMicroseconds( 1 );
            if ( counter == 255 )
            {
                break;
            }
        }
        laststate = digitalRead( DHT_PIN );

        if ( counter == 255 )
            break;

        /* ignore first 3 transitions */
        if ( (i >= 4) && (i % 2 == 0) )
        {
            /* shove each bit into the storage bytes */
            data[j / 8] <<= 1;
            if ( counter > 16 )
                data[j / 8] |= 1;
            j++;
        }
    }

    /*
     * check we read 40 bits (8bit x 5 ) + verify checksum in the last byte
     * print it out if data is good
     */
    if ( (j >= 40) &&
         (data[4] == ( (data[0] + data[1] + data[2] + data[3]) & 0xFF) ) )
    {
        float h = (float)((data[0] << 8) + data[1]) / 10;
        if ( h > 100 )
        {
            h = data[0];    // for DHT11
        }
        return h;
    }else  {
        printf( "Data not good, skip\n" );
        return 0;
    }
}

//---------------------------------------------


using namespace OC;
namespace PH = std::placeholders;

/*
 tool_version          : 20171123
 input_file            : ../device_output/out_codegeneration_merged.swagger.json
 version of input_file : v1.1.0-20160519
 title of input_file   : Binary Switch
*/

#define INTERFACE_KEY "if"

/*
* default class, so that we have to define less variables/functions.
*/
class Resource
{
    protected:
    OCResourceHandle m_resourceHandle;
    OC::OCRepresentation m_rep;
    virtual OCEntityHandlerResult entityHandler(std::shared_ptr<OC::OCResourceRequest> request)=0;

};


/*
 * class definition for class that handles /binaryswitch
 *
 * This resource describes a binary switch (on/off).
 * The value is a boolean.
 * A value of 'true' means that the switch is on.
 * A value of 'false' means that the switch is off.
*/
class BinaryswitchResource : public Resource
{
    public:
        /*
         * constructor
         *
         * @param resourceUri the uri for this resource
         */
        BinaryswitchResource(std::string resourceUri = "/binaryswitch");

        /*
         * destructor
         */
         virtual ~BinaryswitchResource(void);

        /*
         * Register the resource with the server
         *
         * setting resourceProperty as OC_DISCOVERABLE will allow Discovery of this resource
         * setting resourceProperty as OC_OBSERVABLE will allow observation
         * setting resourceProperty as OC_DISCOVERABLE | OC_OBSERVABLE will allow both discovery and observation
         * setting resourceProperty as OC_SECURE the resource supports access via secure endpoints
         * setting resourceProperty as OC_NONSECURE the resource supports access via non-secure endpoints
         * setting resourceProperty as OC_SECURE | OC_NONSECURE will allow access via secure and non-secure endpoints
         *
         * @param resourceProperty indicates the property of the resource. Defined in octypes.h.
         */
        OCStackResult registerResource(uint8_t resourceProperty = OC_DISCOVERABLE | OC_OBSERVABLE | OC_SECURE);

        /*
         * Attempt to send out notifications to observing clients
         * if no value on the device has been changed no notification
         * will be sent.
         *
         * @return OC_STACK_OK on success
         */
        OCStackResult sendNotification();
        OCStackResult sendNotification(const std::shared_ptr< OCResourceResponse > pResponse);
    private:

        /*
         * Make the payload for the retrieve function (e.g. GET) /binaryswitch
         * This resource describes a binary switch (on/off).
         * The value is a boolean.
         * A value of 'true' means that the switch is on.
         * A value of 'false' means that the switch is off.
         * @param queries  the query parameters for this call
         */
        OCRepresentation get(OC::QueryParamsMap queries);

        /*
         * Parse the payload for the update function (e.g. POST) /binaryswitch

         * @param queries  the query parameters for this call
         * @param rep  the response to get the property values from
         * @return OCEntityHandlerResult ok or not ok indication
         */
        OCEntityHandlerResult post(OC::QueryParamsMap queries, const OC::OCRepresentation& rep);


        std::string m_resourceUri;
        // resource types and interfaces as array..
        std::string m_RESOURCE_TYPE[1] = {"oic.r.switch.binary"}; // rt value (as an array)
        std::string m_RESOURCE_INTERFACE[2] = {"oic.if.a","oic.if.baseline"}; // interface if (as an array)
        std::string m_IF_UPDATE[3] = {"oic.if.a", "oic.if.rw", "oic.if.baseline"}; // updateble interfaces
        ObservationIds m_interestedObservers;

        // member variables for path: "/binaryswitch"
        std::vector<std::string>  m_var_value_if; // the value for the array attribute "if": The interface set supported by this resource
        std::string m_var_name_if = "if"; // the name for the attribute "if"
        std::string m_var_value_n; // the value for the attribute "n": Friendly name of the resource
        std::string m_var_name_n = "n"; // the name for the attribute "n"
        std::vector<std::string>  m_var_value_rt; // the value for the array attribute "rt": Resource Type
        std::string m_var_name_rt = "rt"; // the name for the attribute "rt"
        bool m_var_value_value; // the value for the attribute "value": Status of the switch
        std::string m_var_name_value = "value"; // the name for the attribute "value"

    protected:
        /*
         * Check if the interface is
         * @param  interface_name the interface name used during the request
         * @return true: updatable interface
         */
        bool in_updatable_interfaces(std::string interface_name);

        /*
         * the entity handler for this resource
         * @param request the incoming request to handle
         * @return OCEntityHandlerResult ok or not ok indication
         */
        virtual OCEntityHandlerResult entityHandler(std::shared_ptr<OC::OCResourceRequest> request);
};

/*
* Constructor code
*/
BinaryswitchResource::BinaryswitchResource(std::string resourceUri)
{
    std::cout << "- Running: BinaryswitchResource constructor" << std::endl;

    m_resourceUri = resourceUri;
    // initialize member variables /binaryswitch
    // initialize vector if  The interface set supported by this resource
    m_var_value_if.push_back("oic.if.a");
    m_var_value_if.push_back("oic.if.baseline");
    m_var_value_n = "";  // current value of property "n" Friendly name of the resource
    // initialize vector rt  Resource Type
    m_var_value_rt.push_back("oic.r.switch.binary");
    m_var_value_value = false; // current value of property "value" Status of the switch
    }

/*
* Destructor code
*/
BinaryswitchResource::~BinaryswitchResource() { }

OCStackResult BinaryswitchResource::registerResource(uint8_t resourceProperty)
{
    OCStackResult result = OC_STACK_ERROR;
    EntityHandler cb = std::bind(&BinaryswitchResource::entityHandler, this,PH::_1);
    result = OCPlatform::registerResource(m_resourceHandle,
                                          m_resourceUri,
                                          m_RESOURCE_TYPE[0],
                                          m_RESOURCE_INTERFACE[0],
                                          cb,
                                          resourceProperty);
    if(OC_STACK_OK != result)
    {
        std::cerr << "Failed to register BinaryswitchResource." << std::endl;
        return result;
    }

    /// add the additional resource types
    for( unsigned int a = 1; a < (sizeof(m_RESOURCE_TYPE)/sizeof(m_RESOURCE_TYPE[0])); a++ )
    {
        result = OCPlatform::bindTypeToResource(m_resourceHandle, m_RESOURCE_TYPE[a].c_str());
        if(OC_STACK_OK != result)
        {
            std::cerr << "Could not bind resource type:" << m_RESOURCE_INTERFACE[a] << std::endl;
            return result;
        }
    }
    // add the additional interfaces
    for( unsigned int a = 1; a < (sizeof(m_RESOURCE_INTERFACE)/sizeof(m_RESOURCE_INTERFACE[0])); a++)
    {
        result = OCPlatform::bindInterfaceToResource(m_resourceHandle, m_RESOURCE_INTERFACE[a].c_str());
        if(OC_STACK_OK != result)
        {
            std::cerr << "Could not bind interface:" << m_RESOURCE_INTERFACE[a] << std::endl;
            return result;
        }
    }

    std::cout << "BinaryswitchResource:" << std::endl;
    std::cout << "\t" << "# resource interfaces: "
              << sizeof(m_RESOURCE_INTERFACE)/sizeof(m_RESOURCE_INTERFACE[0]) << std::endl;
    std::cout << "\t" << "# resource types     : "
              << sizeof(m_RESOURCE_TYPE)/sizeof(m_RESOURCE_TYPE[0]) << std::endl;

    return result;
}

/*
* Make the payload for the observe function (e.g. GET) /binaryswitch
*/
OCStackResult BinaryswitchResource::sendNotification(void)
{
    OCStackResult sResult = OC_STACK_OK;
    if ( m_interestedObservers.size() > 0) {
        std::cout << "Notifying list "  << m_interestedObservers.size() << " of observers\n";
        auto pResponse = std::make_shared<OC::OCResourceResponse>();
        sResult = OCPlatform::notifyListOfObservers(m_resourceHandle,
                                                    m_interestedObservers,
                                                    pResponse);
    }
    return sResult;
}

/*
* Make the payload for the observe function (e.g. GET) /binaryswitch
* @param pResponse  the response to use for the observe
*/
OCStackResult BinaryswitchResource::sendNotification(const std::shared_ptr< OCResourceResponse > pResponse)
{
    OCStackResult sResult = OC_STACK_OK;
    if ( m_interestedObservers.size() > 0) {
        std::cout << "Notifying list "  << m_interestedObservers.size() << " of observers\n";
        sResult = OCPlatform::notifyListOfObservers(m_resourceHandle,
                                                    m_interestedObservers,
                                                    pResponse);
    }
    return sResult;
}


/*
* Make the payload for the retrieve function (e.g. GET) /binaryswitch
* @param queries  the query parameters for this call
*/
OCRepresentation BinaryswitchResource::get(QueryParamsMap queries)
{
    OC_UNUSED(queries);

	// TODO: SENSOR add here the code to talk to the HW if one implements a sensor.
	// the calls needs to fill in the member variable before it is returned.
	// alternative is to have a callback from the hardware that sets the member variables

    std::cout << "\t\t" << "property 'n' : "<< m_var_value_n << std::endl;
    std::cout << "\t\t" << "property 'value' : "<< ((m_var_value_value) ? "true" : "false") << std::endl;

    m_rep.setValue(m_var_name_if,  m_var_value_if );
    m_rep.setValue(m_var_name_n, m_var_value_n );
    m_rep.setValue(m_var_name_rt,  m_var_value_rt );
    m_rep.setValue(m_var_name_value, m_var_value_value );

    return m_rep;
}

/*
* Parse the payload for the update function (e.g. POST) /binaryswitch
* @param queries  the query parameters for this call
* @param rep  the response to get the property values from
* @return OCEntityHandlerResult ok or not ok indication
*/
OCEntityHandlerResult BinaryswitchResource::post(QueryParamsMap queries, const OCRepresentation& rep)
{
    OCEntityHandlerResult ehResult = OC_EH_OK;
    OC_UNUSED(queries);

    // TODO: missing code: add check on array contents out of range
	// such a check is resource specific
    try {
        if (rep.hasAttribute(m_var_name_if))
        {
            // value exist in payload

            // check if "if" is read only
            ehResult = OC_EH_ERROR;
            std::cout << "\t\t" << "property 'if' is readOnly "<< std::endl;

        }
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }

    try {
        if (rep.hasAttribute(m_var_name_n))
        {
            // value exist in payload

            // check if "n" is read only
            ehResult = OC_EH_ERROR;
            std::cout << "\t\t" << "property 'n' is readOnly "<< std::endl;

        }
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }

    // TODO: missing code: add check on array contents out of range
	// such a check is resource specific
    try {
        if (rep.hasAttribute(m_var_name_rt))
        {
            // value exist in payload

            // check if "rt" is read only
            ehResult = OC_EH_ERROR;
            std::cout << "\t\t" << "property 'rt' is readOnly "<< std::endl;

        }
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
    try {
        if (rep.hasAttribute(m_var_name_value))
        {
            // value exist in payload

        }
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
    if (ehResult == OC_EH_OK)
    {
        // no error: assign the variables
        // array only works for integer, boolean, numbers and strings
        // TODO: missing code, make it also work with array of objects
        try {
            if (rep.hasAttribute(m_var_name_if))
            {
                rep.getValue(m_var_name_if, m_var_value_if);
                int first = 1;
                std::cout << "\t\t" << "property 'if' UPDATED: " ;
                for(auto myvar: m_var_value_if)
                {
                    if(first)
                    {
                        std::cout << myvar;
                        first = 0;
                    }
                    else
                    {
                        std::cout << "," << myvar;
                    }
                }
                std::cout <<  std::endl;
            }
            else
            {
                std::cout << "\t\t" << "property 'if' not found in the representation" << std::endl;
            }
        }
        catch (std::exception& e)
        {
            std::cout << e.what() << std::endl;
        }
        try {
            // value exist in payload
            std::string temp;
            if (rep.getValue(m_var_name_n, temp ))
            {
                m_var_value_n = temp;
                std::cout << "\t\t" << "property 'n' UPDATED: " << m_var_value_n << std::endl;
            }
            else
            {
                std::cout << "\t\t" << "property 'n' not found in the representation" << std::endl;
            }
        }
        catch (std::exception& e)
        {
            std::cout << e.what() << std::endl;
        }// array only works for integer, boolean, numbers and strings
        // TODO: missing code, make it also work with array of objects
        try {
            if (rep.hasAttribute(m_var_name_rt))
            {
                rep.getValue(m_var_name_rt, m_var_value_rt);
                int first = 1;
                std::cout << "\t\t" << "property 'rt' UPDATED: " ;
                for(auto myvar: m_var_value_rt)
                {
                    if(first)
                    {
                        std::cout << myvar;
                        first = 0;
                    }
                    else
                    {
                        std::cout << "," << myvar;
                    }
                }
                std::cout <<  std::endl;
            }
            else
            {
                std::cout << "\t\t" << "property 'rt' not found in the representation" << std::endl;
            }
        }
        catch (std::exception& e)
        {
            std::cout << e.what() << std::endl;
        }
        try {
            bool temp;
            if (rep.getValue(m_var_name_value, temp ))
            {
                m_var_value_value = temp;
                std::cout << "\t\t" << "property 'value' UPDATED: " << ((m_var_value_value) ? "true" : "false") << std::endl;
            }
            else
            {
                std::cout << "\t\t" << "property 'value' not found in the representation" << std::endl;
            }
        }
        catch (std::exception& e)
        {
            std::cout << e.what() << std::endl;
        }
	// TODO: ACTUATOR add here the code to talk to the HW if one implements an actuator.
	// one can use the member variables as input to those calls
	// the member values have been updated already with the request data
    }
    return ehResult;
}
/*
* Check if the interface name is an registered interface name
*/
bool BinaryswitchResource::in_updatable_interfaces(std::string interface_name)
{
    for (unsigned int i=0; i < (sizeof(m_IF_UPDATE)/sizeof(m_IF_UPDATE[0])); i++)
    {
        if (m_IF_UPDATE[i].compare(interface_name) == 0)
            return true;
    }
    return false;
}

/*
* the entity handler
*/
OCEntityHandlerResult BinaryswitchResource::entityHandler(std::shared_ptr<OCResourceRequest> request)
{
    OCEntityHandlerResult ehResult = OC_EH_ERROR;
    //std::cout << "In entity handler for BinaryswitchResource " << std::endl;

    if(request)
    {
        std::cout << "In entity handler for BinaryswitchResource, URI is : "
                  << request->getResourceUri() << std::endl;

        // Check for query params (if any)
        QueryParamsMap queries = request->getQueryParameters();
        if (!queries.empty())
        {
            std::cout << "\nQuery processing up to entityHandler" << std::endl;
        }
        for (auto it : queries)
        {
            std::cout << "Query key: " << it.first << " value : " << it.second
                    << std::endl;
        }
        // get the value, so that we can AND it to check which flags are set
        int requestFlag = request->getRequestHandlerFlag();

        if(requestFlag & RequestHandlerFlag::RequestFlag)
        {
            // request flag is set
            auto pResponse = std::make_shared<OC::OCResourceResponse>();
            pResponse->setRequestHandle(request->getRequestHandle());
            pResponse->setResourceHandle(request->getResourceHandle());

            if(request->getRequestType() == "GET")
            {
                std::cout<<"BinaryswitchResource Get Request"<< std::endl;

                pResponse->setResourceRepresentation(get(queries), "");
                if(OC_STACK_OK == OCPlatform::sendResponse(pResponse))
                {
                    ehResult = OC_EH_OK;
                }
            }

            else if(request->getRequestType() == "POST")
            {
                std::cout <<"BinaryswitchResource Post Request"<<std::endl;
                bool  handle_post = true;

                if (queries.size() > 0)
                {
                    for (const auto &eachQuery : queries)
                    {
                        std::string key = eachQuery.first;
                        if (key.compare(INTERFACE_KEY) == 0)
                        {
                            std::string value = eachQuery.second;
                            if (in_updatable_interfaces(value) == false)
                            {
                                std::cout << "Update request received via interface: " << value
                                            << " . This interface is not authorized to update resource!!" << std::endl;
                                pResponse->setResponseResult(OCEntityHandlerResult::OC_EH_FORBIDDEN);
                                handle_post = false;
                                ehResult = OC_EH_ERROR;
                                break;
                            }
                        }
                    }
                }
                if (handle_post)
                {
                    ehResult = post(queries, request->getResourceRepresentation());
                    if (ehResult == OC_EH_OK)
                    {
                        pResponse->setResourceRepresentation(get(queries), "");
                        if (OC_STACK_OK == OCPlatform::sendResponse(pResponse))
                        {
                            if (OC_STACK_OK != sendNotification(pResponse) )
                            {
                                std::cerr << "NOTIFY failed." << std::endl;
                            }
                        }
                    }
                    else
                    {
                         pResponse->setResponseResult(OCEntityHandlerResult::OC_EH_ERROR);
                    }
                }
            }
            else
            {
                std::cout << "BinaryswitchResource unsupported request type (delete,put,..)"
                    << request->getRequestType() << std::endl;
                pResponse->setResponseResult(OC_EH_ERROR);
                OCPlatform::sendResponse(pResponse);
                ehResult = OC_EH_ERROR;
            }
        }

        if(requestFlag & RequestHandlerFlag::ObserverFlag)
        {
            // observe flag is set
            ObservationInfo observationInfo = request->getObservationInfo();
            std::cout << "\t\trequestFlag : observer ";
            if (ObserveAction::ObserveRegister == observationInfo.action)
            {
                std::cout << "register" << std::endl;
            }
            else
            {
                std::cout << "unregister" << std::endl;
            }

            if(ObserveAction::ObserveRegister == observationInfo.action)
            {
                // add observer
                m_interestedObservers.push_back(observationInfo.obsId);
            }
            else if(ObserveAction::ObserveUnregister == observationInfo.action)
            {
                // delete observer
                m_interestedObservers.erase(std::remove(
                                            m_interestedObservers.begin(),
                                            m_interestedObservers.end(),
                                            observationInfo.obsId),
                                            m_interestedObservers.end());
            }
            ehResult = OC_EH_OK;
        }
    }
    return ehResult;
}


/*
 * class definition for class that handles /illuminance
 *
 * This resource describes an illuminance sensor.
 * Illuminance is a float and represents the sensed luminous flux per unit area in lux.
*/
class IlluminanceResource : public Resource
{
    public:
        /*
         * constructor
         *
         * @param resourceUri the uri for this resource
         */
        IlluminanceResource(std::string resourceUri = "/illuminance");

        /*
         * destructor
         */
         virtual ~IlluminanceResource(void);

        /*
         * Register the resource with the server
         *
         * setting resourceProperty as OC_DISCOVERABLE will allow Discovery of this resource
         * setting resourceProperty as OC_OBSERVABLE will allow observation
         * setting resourceProperty as OC_DISCOVERABLE | OC_OBSERVABLE will allow both discovery and observation
         * setting resourceProperty as OC_SECURE the resource supports access via secure endpoints
         * setting resourceProperty as OC_NONSECURE the resource supports access via non-secure endpoints
         * setting resourceProperty as OC_SECURE | OC_NONSECURE will allow access via secure and non-secure endpoints
         *
         * @param resourceProperty indicates the property of the resource. Defined in octypes.h.
         */
        OCStackResult registerResource(uint8_t resourceProperty = OC_DISCOVERABLE | OC_OBSERVABLE | OC_SECURE);

        /*
         * Attempt to send out notifications to observing clients
         * if no value on the device has been changed no notification
         * will be sent.
         *
         * @return OC_STACK_OK on success
         */
        OCStackResult sendNotification();
        OCStackResult sendNotification(const std::shared_ptr< OCResourceResponse > pResponse);
    private:

        /*
         * Make the payload for the retrieve function (e.g. GET) /illuminance
         * This resource describes an illuminance sensor.
         * Illuminance is a float and represents the sensed luminous flux per unit area in lux.
         * @param queries  the query parameters for this call
         */
        OCRepresentation get(OC::QueryParamsMap queries);


        std::string m_resourceUri;
        // resource types and interfaces as array..
        std::string m_RESOURCE_TYPE[1] = {"oic.r.sensor.illuminance"}; // rt value (as an array)
        std::string m_RESOURCE_INTERFACE[2] = {"oic.if.a","oic.if.baseline"}; // interface if (as an array)
        std::string m_IF_UPDATE[3] = {"oic.if.a", "oic.if.rw", "oic.if.baseline"}; // updateble interfaces
        ObservationIds m_interestedObservers;

        // member variables for path: "/illuminance"
        std::vector<std::string>  m_var_value_if; // the value for the array attribute "if": The interface set supported by this resource
        std::string m_var_name_if = "if"; // the name for the attribute "if"
        double m_var_value_illuminance; // the value for the attribute "illuminance": Sensed luminous flux per unit area in lux.
        std::string m_var_name_illuminance = "illuminance"; // the name for the attribute "illuminance"
        std::string m_var_value_n; // the value for the attribute "n": Friendly name of the resource
        std::string m_var_name_n = "n"; // the name for the attribute "n"
        std::vector<std::string>  m_var_value_rt; // the value for the array attribute "rt": Resource Type
        std::string m_var_name_rt = "rt"; // the name for the attribute "rt"
        void* m_var_value_value; // the value for the attribute "value": The value sensed or actuated by this Resource
        std::string m_var_name_value = "value"; // the name for the attribute "value"

    protected:
        /*
         * Check if the interface is
         * @param  interface_name the interface name used during the request
         * @return true: updatable interface
         */
        bool in_updatable_interfaces(std::string interface_name);

        /*
         * the entity handler for this resource
         * @param request the incoming request to handle
         * @return OCEntityHandlerResult ok or not ok indication
         */
        virtual OCEntityHandlerResult entityHandler(std::shared_ptr<OC::OCResourceRequest> request);
};

/*
* Constructor code
*/
IlluminanceResource::IlluminanceResource(std::string resourceUri)
{
    std::cout << "- Running: IlluminanceResource constructor" << std::endl;

    m_resourceUri = resourceUri;
    // initialize member variables /illuminance
    // initialize vector if  The interface set supported by this resource
    m_var_value_if.push_back("oic.if.a");
    m_var_value_if.push_back("oic.if.baseline");
    m_var_value_illuminance = 450.0; // current value of property "illuminance"  Sensed luminous flux per unit area in lux.
    m_var_value_n = "";  // current value of property "n" Friendly name of the resource
    // initialize vector rt  Resource Type
    m_var_value_rt.push_back("oic.r.sensor.illuminance");
    }

/*
* Destructor code
*/
IlluminanceResource::~IlluminanceResource() { }

OCStackResult IlluminanceResource::registerResource(uint8_t resourceProperty)
{
    OCStackResult result = OC_STACK_ERROR;
    EntityHandler cb = std::bind(&IlluminanceResource::entityHandler, this,PH::_1);
    result = OCPlatform::registerResource(m_resourceHandle,
                                          m_resourceUri,
                                          m_RESOURCE_TYPE[0],
                                          m_RESOURCE_INTERFACE[0],
                                          cb,
                                          resourceProperty);
    if(OC_STACK_OK != result)
    {
        std::cerr << "Failed to register IlluminanceResource." << std::endl;
        return result;
    }

    /// add the additional resource types
    for( unsigned int a = 1; a < (sizeof(m_RESOURCE_TYPE)/sizeof(m_RESOURCE_TYPE[0])); a++ )
    {
        result = OCPlatform::bindTypeToResource(m_resourceHandle, m_RESOURCE_TYPE[a].c_str());
        if(OC_STACK_OK != result)
        {
            std::cerr << "Could not bind resource type:" << m_RESOURCE_INTERFACE[a] << std::endl;
            return result;
        }
    }
    // add the additional interfaces
    for( unsigned int a = 1; a < (sizeof(m_RESOURCE_INTERFACE)/sizeof(m_RESOURCE_INTERFACE[0])); a++)
    {
        result = OCPlatform::bindInterfaceToResource(m_resourceHandle, m_RESOURCE_INTERFACE[a].c_str());
        if(OC_STACK_OK != result)
        {
            std::cerr << "Could not bind interface:" << m_RESOURCE_INTERFACE[a] << std::endl;
            return result;
        }
    }

    std::cout << "IlluminanceResource:" << std::endl;
    std::cout << "\t" << "# resource interfaces: "
              << sizeof(m_RESOURCE_INTERFACE)/sizeof(m_RESOURCE_INTERFACE[0]) << std::endl;
    std::cout << "\t" << "# resource types     : "
              << sizeof(m_RESOURCE_TYPE)/sizeof(m_RESOURCE_TYPE[0]) << std::endl;

    return result;
}

/*
* Make the payload for the observe function (e.g. GET) /illuminance
*/
OCStackResult IlluminanceResource::sendNotification(void)
{
    OCStackResult sResult = OC_STACK_OK;
    if ( m_interestedObservers.size() > 0) {
        std::cout << "Notifying list "  << m_interestedObservers.size() << " of observers\n";
        auto pResponse = std::make_shared<OC::OCResourceResponse>();
        sResult = OCPlatform::notifyListOfObservers(m_resourceHandle,
                                                    m_interestedObservers,
                                                    pResponse);
    }
    return sResult;
}

/*
* Make the payload for the observe function (e.g. GET) /illuminance
* @param pResponse  the response to use for the observe
*/
OCStackResult IlluminanceResource::sendNotification(const std::shared_ptr< OCResourceResponse > pResponse)
{
    OCStackResult sResult = OC_STACK_OK;
    if ( m_interestedObservers.size() > 0) {
        std::cout << "Notifying list "  << m_interestedObservers.size() << " of observers\n";
        sResult = OCPlatform::notifyListOfObservers(m_resourceHandle,
                                                    m_interestedObservers,
                                                    pResponse);
    }
    return sResult;
}


/*
* Make the payload for the retrieve function (e.g. GET) /illuminance
* @param queries  the query parameters for this call
*/
OCRepresentation IlluminanceResource::get(QueryParamsMap queries)
{
    OC_UNUSED(queries);

	// TODO: SENSOR add here the code to talk to the HW if one implements a sensor.
	// the calls needs to fill in the member variable before it is returned.
	// alternative is to have a callback from the hardware that sets the member variables

    std::cout << "\t\t" << "property 'illuminance' : "<< m_var_value_illuminance << std::endl;
    std::cout << "\t\t" << "property 'n' : "<< m_var_value_n << std::endl;

    m_rep.setValue(m_var_name_if,  m_var_value_if );
    m_rep.setValue(m_var_name_illuminance, m_var_value_illuminance );
    m_rep.setValue(m_var_name_n, m_var_value_n );
    m_rep.setValue(m_var_name_rt,  m_var_value_rt );

    return m_rep;
}
/*
* Check if the interface name is an registered interface name
*/
bool IlluminanceResource::in_updatable_interfaces(std::string interface_name)
{
    for (unsigned int i=0; i < (sizeof(m_IF_UPDATE)/sizeof(m_IF_UPDATE[0])); i++)
    {
        if (m_IF_UPDATE[i].compare(interface_name) == 0)
            return true;
    }
    return false;
}

/*
* the entity handler
*/
OCEntityHandlerResult IlluminanceResource::entityHandler(std::shared_ptr<OCResourceRequest> request)
{
    OCEntityHandlerResult ehResult = OC_EH_ERROR;
    //std::cout << "In entity handler for IlluminanceResource " << std::endl;

    if(request)
    {
        std::cout << "In entity handler for IlluminanceResource, URI is : "
                  << request->getResourceUri() << std::endl;

        // Check for query params (if any)
        QueryParamsMap queries = request->getQueryParameters();
        if (!queries.empty())
        {
            std::cout << "\nQuery processing up to entityHandler" << std::endl;
        }
        for (auto it : queries)
        {
            std::cout << "Query key: " << it.first << " value : " << it.second
                    << std::endl;
        }
        // get the value, so that we can AND it to check which flags are set
        int requestFlag = request->getRequestHandlerFlag();

        if(requestFlag & RequestHandlerFlag::RequestFlag)
        {
            // request flag is set
            auto pResponse = std::make_shared<OC::OCResourceResponse>();
            pResponse->setRequestHandle(request->getRequestHandle());
            pResponse->setResourceHandle(request->getResourceHandle());

            if(request->getRequestType() == "GET")
            {
                std::cout<<"IlluminanceResource Get Request"<< std::endl;

                pResponse->setResourceRepresentation(get(queries), "");
                if(OC_STACK_OK == OCPlatform::sendResponse(pResponse))
                {
                    ehResult = OC_EH_OK;
                }
            }
else
            {
                std::cout << "IlluminanceResource unsupported request type (delete,put,..)"
                    << request->getRequestType() << std::endl;
                pResponse->setResponseResult(OC_EH_ERROR);
                OCPlatform::sendResponse(pResponse);
                ehResult = OC_EH_ERROR;
            }
        }

        if(requestFlag & RequestHandlerFlag::ObserverFlag)
        {
            // observe flag is set
            ObservationInfo observationInfo = request->getObservationInfo();
            std::cout << "\t\trequestFlag : observer ";
            if (ObserveAction::ObserveRegister == observationInfo.action)
            {
                std::cout << "register" << std::endl;
            }
            else
            {
                std::cout << "unregister" << std::endl;
            }

            if(ObserveAction::ObserveRegister == observationInfo.action)
            {
                // add observer
                m_interestedObservers.push_back(observationInfo.obsId);
            }
            else if(ObserveAction::ObserveUnregister == observationInfo.action)
            {
                // delete observer
                m_interestedObservers.erase(std::remove(
                                            m_interestedObservers.begin(),
                                            m_interestedObservers.end(),
                                            observationInfo.obsId),
                                            m_interestedObservers.end());
            }
            ehResult = OC_EH_OK;
        }
    }
    return ehResult;
}


/*
 * class definition for class that handles /temperature
 *
 * This resource describes a sensed or actuated Temperature value.
 * The temperature describes the current value measured.
 * The units is a single value that is one of C, F or K.
 * It provides the unit of measurement for the temperature value.
 * It is a read-only value that is provided by the server.
 * If the units Property is missing the default is Celsius [C].
 * When range (from oic.r.baseresource) is omitted the default is +/- MAXINT.
 * Retrieves the current temperature value.
 * A client can specify the units for the requested temperature by use of a query parameter.
 * If no query parameter is provided the server provides its default measure or set value.
 * It is recommended to return always the units Property in the result.
*/
class TemperatureResource : public Resource
{
    public:
        /*
         * constructor
         *
         * @param resourceUri the uri for this resource
         */
        TemperatureResource(std::string resourceUri = "/temperature");

        /*
         * destructor
         */
         virtual ~TemperatureResource(void);

        /*
         * Register the resource with the server
         *
         * setting resourceProperty as OC_DISCOVERABLE will allow Discovery of this resource
         * setting resourceProperty as OC_OBSERVABLE will allow observation
         * setting resourceProperty as OC_DISCOVERABLE | OC_OBSERVABLE will allow both discovery and observation
         * setting resourceProperty as OC_SECURE the resource supports access via secure endpoints
         * setting resourceProperty as OC_NONSECURE the resource supports access via non-secure endpoints
         * setting resourceProperty as OC_SECURE | OC_NONSECURE will allow access via secure and non-secure endpoints
         *
         * @param resourceProperty indicates the property of the resource. Defined in octypes.h.
         */
        OCStackResult registerResource(uint8_t resourceProperty = OC_DISCOVERABLE | OC_OBSERVABLE | OC_SECURE);

        /*
         * Attempt to send out notifications to observing clients
         * if no value on the device has been changed no notification
         * will be sent.
         *
         * @return OC_STACK_OK on success
         */
        OCStackResult sendNotification();
        OCStackResult sendNotification(const std::shared_ptr< OCResourceResponse > pResponse);
    private:

        /*
         * Make the payload for the retrieve function (e.g. GET) /temperature
         * This resource describes a sensed or actuated Temperature value.
         * The temperature describes the current value measured.
         * The units is a single value that is one of C, F or K.
         * It provides the unit of measurement for the temperature value.
         * It is a read-only value that is provided by the server.
         * If the units Property is missing the default is Celsius [C].
         * When range (from oic.r.baseresource) is omitted the default is +/- MAXINT.
         * Retrieves the current temperature value.
         * A client can specify the units for the requested temperature by use of a query parameter.
         * If no query parameter is provided the server provides its default measure or set value.
         * It is recommended to return always the units Property in the result.
         * @param queries  the query parameters for this call
         */
        OCRepresentation get(OC::QueryParamsMap queries);

        /*
         * Parse the payload for the update function (e.g. POST) /temperature
         * Sets the desired temperature value.
         * If a unit is included and the server does not support the unit indicated the request will fail.
         * If the units are omitted value is taken to be in C.
         * @param queries  the query parameters for this call
         * @param rep  the response to get the property values from
         * @return OCEntityHandlerResult ok or not ok indication
         */
        OCEntityHandlerResult post(OC::QueryParamsMap queries, const OC::OCRepresentation& rep);


        std::string m_resourceUri;
        // resource types and interfaces as array..
        std::string m_RESOURCE_TYPE[1] = {"oic.r.temperature"}; // rt value (as an array)
        std::string m_RESOURCE_INTERFACE[2] = {"oic.if.a","oic.if.baseline"}; // interface if (as an array)
        std::string m_IF_UPDATE[3] = {"oic.if.a", "oic.if.rw", "oic.if.baseline"}; // updateble interfaces
        ObservationIds m_interestedObservers;

        // member variables for path: "/temperature"
        std::vector<std::string>  m_var_value_if; // the value for the array attribute "if": The interface set supported by this resource
        std::string m_var_name_if = "if"; // the name for the attribute "if"
        std::string m_var_value_n; // the value for the attribute "n": Friendly name of the resource
        std::string m_var_name_n = "n"; // the name for the attribute "n"
        std::vector<std::string>  m_var_value_rt; // the value for the array attribute "rt": Resource Type
        std::string m_var_name_rt = "rt"; // the name for the attribute "rt"
        double m_var_value_temperature; // the value for the attribute "temperature": Current temperature setting or measurement
        std::string m_var_name_temperature = "temperature"; // the name for the attribute "temperature"
        std::string m_var_value_units; // the value for the attribute "units": Units for the temperature value
        std::string m_var_name_units = "units"; // the name for the attribute "units"
        void* m_var_value_value; // the value for the attribute "value": The value sensed or actuated by this Resource
        std::string m_var_name_value = "value"; // the name for the attribute "value"

    protected:
        /*
         * Check if the interface is
         * @param  interface_name the interface name used during the request
         * @return true: updatable interface
         */
        bool in_updatable_interfaces(std::string interface_name);

        /*
         * the entity handler for this resource
         * @param request the incoming request to handle
         * @return OCEntityHandlerResult ok or not ok indication
         */
        virtual OCEntityHandlerResult entityHandler(std::shared_ptr<OC::OCResourceRequest> request);
};

/*
* Constructor code
*/
TemperatureResource::TemperatureResource(std::string resourceUri)
{
    std::cout << "- Running: TemperatureResource constructor" << std::endl;

    m_resourceUri = resourceUri;
    // initialize member variables /temperature
    // initialize vector if  The interface set supported by this resource
    m_var_value_if.push_back("oic.if.a");
    m_var_value_if.push_back("oic.if.baseline");
    m_var_value_n = "";  // current value of property "n" Friendly name of the resource
    // initialize vector rt  Resource Type
    m_var_value_rt.push_back("oic.r.temperature");
    m_var_value_temperature = read_dht_data_CelTemp(); // current value of property "temperature"  Current temperature setting or measurement
    m_var_value_units = "C";  // current value of property "units" Units for the temperature value
    }

/*
* Destructor code
*/
TemperatureResource::~TemperatureResource() { }

OCStackResult TemperatureResource::registerResource(uint8_t resourceProperty)
{
    OCStackResult result = OC_STACK_ERROR;
    EntityHandler cb = std::bind(&TemperatureResource::entityHandler, this,PH::_1);
    result = OCPlatform::registerResource(m_resourceHandle,
                                          m_resourceUri,
                                          m_RESOURCE_TYPE[0],
                                          m_RESOURCE_INTERFACE[0],
                                          cb,
                                          resourceProperty);
    if(OC_STACK_OK != result)
    {
        std::cerr << "Failed to register TemperatureResource." << std::endl;
        return result;
    }

    /// add the additional resource types
    for( unsigned int a = 1; a < (sizeof(m_RESOURCE_TYPE)/sizeof(m_RESOURCE_TYPE[0])); a++ )
    {
        result = OCPlatform::bindTypeToResource(m_resourceHandle, m_RESOURCE_TYPE[a].c_str());
        if(OC_STACK_OK != result)
        {
            std::cerr << "Could not bind resource type:" << m_RESOURCE_INTERFACE[a] << std::endl;
            return result;
        }
    }
    // add the additional interfaces
    for( unsigned int a = 1; a < (sizeof(m_RESOURCE_INTERFACE)/sizeof(m_RESOURCE_INTERFACE[0])); a++)
    {
        result = OCPlatform::bindInterfaceToResource(m_resourceHandle, m_RESOURCE_INTERFACE[a].c_str());
        if(OC_STACK_OK != result)
        {
            std::cerr << "Could not bind interface:" << m_RESOURCE_INTERFACE[a] << std::endl;
            return result;
        }
    }

    std::cout << "TemperatureResource:" << std::endl;
    std::cout << "\t" << "# resource interfaces: "
              << sizeof(m_RESOURCE_INTERFACE)/sizeof(m_RESOURCE_INTERFACE[0]) << std::endl;
    std::cout << "\t" << "# resource types     : "
              << sizeof(m_RESOURCE_TYPE)/sizeof(m_RESOURCE_TYPE[0]) << std::endl;

    return result;
}

/*
* Make the payload for the observe function (e.g. GET) /temperature
*/
OCStackResult TemperatureResource::sendNotification(void)
{
    OCStackResult sResult = OC_STACK_OK;
    if ( m_interestedObservers.size() > 0) {
        std::cout << "Notifying list "  << m_interestedObservers.size() << " of observers\n";
        auto pResponse = std::make_shared<OC::OCResourceResponse>();
        sResult = OCPlatform::notifyListOfObservers(m_resourceHandle,
                                                    m_interestedObservers,
                                                    pResponse);
    }
    return sResult;
}

/*
* Make the payload for the observe function (e.g. GET) /temperature
* @param pResponse  the response to use for the observe
*/
OCStackResult TemperatureResource::sendNotification(const std::shared_ptr< OCResourceResponse > pResponse)
{
    OCStackResult sResult = OC_STACK_OK;
    if ( m_interestedObservers.size() > 0) {
        std::cout << "Notifying list "  << m_interestedObservers.size() << " of observers\n";
        sResult = OCPlatform::notifyListOfObservers(m_resourceHandle,
                                                    m_interestedObservers,
                                                    pResponse);
    }
    return sResult;
}


/*
* Make the payload for the retrieve function (e.g. GET) /temperature
* @param queries  the query parameters for this call
*/
OCRepresentation TemperatureResource::get(QueryParamsMap queries)
{
    OC_UNUSED(queries);

	// TODO: SENSOR add here the code to talk to the HW if one implements a sensor.
	// the calls needs to fill in the member variable before it is returned.
	// alternative is to have a callback from the hardware that sets the member variables

    std::cout << "\t\t" << "property 'n' : "<< m_var_value_n << std::endl;
    std::cout << "\t\t" << "property 'temperature' : "<< m_var_value_temperature << std::endl;
    std::cout << "\t\t" << "property 'units' : "<< m_var_value_units << std::endl;

    m_rep.setValue(m_var_name_if,  m_var_value_if );
    m_rep.setValue(m_var_name_n, m_var_value_n );
    m_rep.setValue(m_var_name_rt,  m_var_value_rt );
    m_rep.setValue(m_var_name_temperature, m_var_value_temperature );
    m_rep.setValue(m_var_name_units, m_var_value_units );

    return m_rep;
}

/*
* Parse the payload for the update function (e.g. POST) /temperature
* @param queries  the query parameters for this call
* @param rep  the response to get the property values from
* @return OCEntityHandlerResult ok or not ok indication
*/
OCEntityHandlerResult TemperatureResource::post(QueryParamsMap queries, const OCRepresentation& rep)
{
    OCEntityHandlerResult ehResult = OC_EH_OK;
    OC_UNUSED(queries);

    // TODO: missing code: add check on array contents out of range
	// such a check is resource specific
    try {
        if (rep.hasAttribute(m_var_name_if))
        {
            // value exist in payload

            // check if "if" is read only
            ehResult = OC_EH_ERROR;
            std::cout << "\t\t" << "property 'if' is readOnly "<< std::endl;

        }
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }

    try {
        if (rep.hasAttribute(m_var_name_n))
        {
            // value exist in payload

            // check if "n" is read only
            ehResult = OC_EH_ERROR;
            std::cout << "\t\t" << "property 'n' is readOnly "<< std::endl;

        }
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }

    // TODO: missing code: add check on array contents out of range
	// such a check is resource specific
    try {
        if (rep.hasAttribute(m_var_name_rt))
        {
            // value exist in payload

            // check if "rt" is read only
            ehResult = OC_EH_ERROR;
            std::cout << "\t\t" << "property 'rt' is readOnly "<< std::endl;

        }
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
    try {
        if (rep.hasAttribute(m_var_name_temperature))
        {
            // value exist in payload
            // allocate the variable
            double value;
            // get the actual value from the payload
            rep.getValue(m_var_name_temperature, value);



        }
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }


    try {
        if (rep.hasAttribute(m_var_name_units))
        {
            // value exist in payload

            // check if "units" is read only
            ehResult = OC_EH_ERROR;
            std::cout << "\t\t" << "property 'units' is readOnly "<< std::endl;

        }
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
    if (ehResult == OC_EH_OK)
    {
        // no error: assign the variables
        // array only works for integer, boolean, numbers and strings
        // TODO: missing code, make it also work with array of objects
        try {
            if (rep.hasAttribute(m_var_name_if))
            {
                rep.getValue(m_var_name_if, m_var_value_if);
                int first = 1;
                std::cout << "\t\t" << "property 'if' UPDATED: " ;
                for(auto myvar: m_var_value_if)
                {
                    if(first)
                    {
                        std::cout << myvar;
                        first = 0;
                    }
                    else
                    {
                        std::cout << "," << myvar;
                    }
                }
                std::cout <<  std::endl;
            }
            else
            {
                std::cout << "\t\t" << "property 'if' not found in the representation" << std::endl;
            }
        }
        catch (std::exception& e)
        {
            std::cout << e.what() << std::endl;
        }
        try {
            // value exist in payload
            std::string temp;
            if (rep.getValue(m_var_name_n, temp ))
            {
                m_var_value_n = temp;
                std::cout << "\t\t" << "property 'n' UPDATED: " << m_var_value_n << std::endl;
            }
            else
            {
                std::cout << "\t\t" << "property 'n' not found in the representation" << std::endl;
            }
        }
        catch (std::exception& e)
        {
            std::cout << e.what() << std::endl;
        }// array only works for integer, boolean, numbers and strings
        // TODO: missing code, make it also work with array of objects
        try {
            if (rep.hasAttribute(m_var_name_rt))
            {
                rep.getValue(m_var_name_rt, m_var_value_rt);
                int first = 1;
                std::cout << "\t\t" << "property 'rt' UPDATED: " ;
                for(auto myvar: m_var_value_rt)
                {
                    if(first)
                    {
                        std::cout << myvar;
                        first = 0;
                    }
                    else
                    {
                        std::cout << "," << myvar;
                    }
                }
                std::cout <<  std::endl;
            }
            else
            {
                std::cout << "\t\t" << "property 'rt' not found in the representation" << std::endl;
            }
        }
        catch (std::exception& e)
        {
            std::cout << e.what() << std::endl;
        }
        try {
            // value exist in payload
            double temp;
            if (rep.getValue(m_var_name_temperature, temp ))
            {
                m_var_value_temperature = temp;
                std::cout << "\t\t" << "property 'temperature' UPDATED: " << m_var_value_temperature << std::endl;
            }
            else
            {
                std::cout << "\t\t" << "property 'temperature' not found in the representation" << std::endl;
            }
        }
        catch (std::exception& e)
        {
            std::cout << e.what() << std::endl;
        }
        try {
            // value exist in payload
            std::string temp;
            if (rep.getValue(m_var_name_units, temp ))
            {
                m_var_value_units = temp;
                std::cout << "\t\t" << "property 'units' UPDATED: " << m_var_value_units << std::endl;
            }
            else
            {
                std::cout << "\t\t" << "property 'units' not found in the representation" << std::endl;
            }
        }
        catch (std::exception& e)
        {
            std::cout << e.what() << std::endl;
        }
	// TODO: ACTUATOR add here the code to talk to the HW if one implements an actuator.
	// one can use the member variables as input to those calls
	// the member values have been updated already with the request data
    }
    return ehResult;
}
/*
* Check if the interface name is an registered interface name
*/
bool TemperatureResource::in_updatable_interfaces(std::string interface_name)
{
    for (unsigned int i=0; i < (sizeof(m_IF_UPDATE)/sizeof(m_IF_UPDATE[0])); i++)
    {
        if (m_IF_UPDATE[i].compare(interface_name) == 0)
            return true;
    }
    return false;
}

/*
* the entity handler
*/
OCEntityHandlerResult TemperatureResource::entityHandler(std::shared_ptr<OCResourceRequest> request)
{
    OCEntityHandlerResult ehResult = OC_EH_ERROR;
    //std::cout << "In entity handler for TemperatureResource " << std::endl;

    if(request)
    {
        std::cout << "In entity handler for TemperatureResource, URI is : "
                  << request->getResourceUri() << std::endl;

        // Check for query params (if any)
        QueryParamsMap queries = request->getQueryParameters();
        if (!queries.empty())
        {
            std::cout << "\nQuery processing up to entityHandler" << std::endl;
        }
        for (auto it : queries)
        {
            std::cout << "Query key: " << it.first << " value : " << it.second
                    << std::endl;
        }
        // get the value, so that we can AND it to check which flags are set
        int requestFlag = request->getRequestHandlerFlag();

        if(requestFlag & RequestHandlerFlag::RequestFlag)
        {
            // request flag is set
            auto pResponse = std::make_shared<OC::OCResourceResponse>();
            pResponse->setRequestHandle(request->getRequestHandle());
            pResponse->setResourceHandle(request->getResourceHandle());

            if(request->getRequestType() == "GET")
            {
                std::cout<<"TemperatureResource Get Request"<< std::endl;

                pResponse->setResourceRepresentation(get(queries), "");
                if(OC_STACK_OK == OCPlatform::sendResponse(pResponse))
                {
                    ehResult = OC_EH_OK;
                }
            }

            else if(request->getRequestType() == "POST")
            {
                std::cout <<"TemperatureResource Post Request"<<std::endl;
                bool  handle_post = true;

                if (queries.size() > 0)
                {
                    for (const auto &eachQuery : queries)
                    {
                        std::string key = eachQuery.first;
                        if (key.compare(INTERFACE_KEY) == 0)
                        {
                            std::string value = eachQuery.second;
                            if (in_updatable_interfaces(value) == false)
                            {
                                std::cout << "Update request received via interface: " << value
                                            << " . This interface is not authorized to update resource!!" << std::endl;
                                pResponse->setResponseResult(OCEntityHandlerResult::OC_EH_FORBIDDEN);
                                handle_post = false;
                                ehResult = OC_EH_ERROR;
                                break;
                            }
                        }
                    }
                }
                if (handle_post)
                {
                    ehResult = post(queries, request->getResourceRepresentation());
                    if (ehResult == OC_EH_OK)
                    {
                        pResponse->setResourceRepresentation(get(queries), "");
                        if (OC_STACK_OK == OCPlatform::sendResponse(pResponse))
                        {
                            if (OC_STACK_OK != sendNotification(pResponse) )
                            {
                                std::cerr << "NOTIFY failed." << std::endl;
                            }
                        }
                    }
                    else
                    {
                         pResponse->setResponseResult(OCEntityHandlerResult::OC_EH_ERROR);
                    }
                }
            }
            else
            {
                std::cout << "TemperatureResource unsupported request type (delete,put,..)"
                    << request->getRequestType() << std::endl;
                pResponse->setResponseResult(OC_EH_ERROR);
                OCPlatform::sendResponse(pResponse);
                ehResult = OC_EH_ERROR;
            }
        }

        if(requestFlag & RequestHandlerFlag::ObserverFlag)
        {
            // observe flag is set
            ObservationInfo observationInfo = request->getObservationInfo();
            std::cout << "\t\trequestFlag : observer ";
            if (ObserveAction::ObserveRegister == observationInfo.action)
            {
                std::cout << "register" << std::endl;
            }
            else
            {
                std::cout << "unregister" << std::endl;
            }

            if(ObserveAction::ObserveRegister == observationInfo.action)
            {
                // add observer
                m_interestedObservers.push_back(observationInfo.obsId);
            }
            else if(ObserveAction::ObserveUnregister == observationInfo.action)
            {
                // delete observer
                m_interestedObservers.erase(std::remove(
                                            m_interestedObservers.begin(),
                                            m_interestedObservers.end(),
                                            observationInfo.obsId),
                                            m_interestedObservers.end());
            }
            ehResult = OC_EH_OK;
        }
    }
    return ehResult;
}



class IoTServer
{
    public:
        /**
         *  constructor
         *  creates all resources from the resource classes.
         */
        IoTServer();

        /**
         *  destructor
         */
        ~IoTServer();

        /*
         * Register the resources with the server
         *
         * setting resourceProperty as OC_DISCOVERABLE will allow Discovery of this resource
         * setting resourceProperty as OC_OBSERVABLE will allow observation
         * setting resourceProperty as OC_DISCOVERABLE | OC_OBSERVABLE will allow both discovery and observation
         * setting resourceProperty as OC_SECURE the resource supports access via secure endpoints
         * setting resourceProperty as OC_NONSECURE the resource supports access via non-secure endpoints
         * setting resourceProperty as OC_SECURE | OC_NONSECURE will allow access via secure and non-secure endpoints
         *
         * @param resourceProperty indicates the property of the resources. Defined in octypes.h.
         */
        OCStackResult registerResources(uint8_t resourceProperty = OC_DISCOVERABLE | OC_OBSERVABLE | OC_SECURE);

    private:
        BinaryswitchResource  m_binaryswitchInstance;
        IlluminanceResource  m_illuminanceInstance;
        TemperatureResource  m_temperatureInstance;};

IoTServer::IoTServer()
    :m_binaryswitchInstance(),
     m_illuminanceInstance(),
     m_temperatureInstance()
{
    std::cout << "Running IoTServer constructor" << std::endl;
}

IoTServer::~IoTServer()
{
    std::cout << "Running IoTServer destructor" << std::endl;
}

OCStackResult IoTServer::registerResources(uint8_t resourceProperty)
{
    OCStackResult result = OC_STACK_ERROR;
    result = m_binaryswitchInstance.registerResource(resourceProperty);
    if(OC_STACK_OK != result)
    {
        return result;
    }
    result = m_illuminanceInstance.registerResource(resourceProperty);
    if(OC_STACK_OK != result)
    {
        return result;
    }
    result = m_temperatureInstance.registerResource(resourceProperty);
    if(OC_STACK_OK != result)
    {
        return result;
    }return result;
}

class Platform
{
public:
    /*
     * Platform constructor
     */
    Platform(void);

    /*
     * Platform destructor
     */
    virtual ~Platform();

    /*
     * Start the platform
     */
    OCStackResult start();

    /*
     * Register all platform info
     * This will register the platformId, manufaturerName, manufacturerUrl,
     * modelNumber, dateOfManufacture, platformVersion, operatingSystemVersion,
     * hardwareVersion, firmwareVersion, supportUrl, and systemTime
     */
    OCStackResult registerPlatformInfo(void);

    /*
     * Get OCPlatformInfo pointer
     */
    OCPlatformInfo* getPlatformInfo(void);

    /*
     * Stop the platform
     */
    OCStackResult stop(void);

    /*
     * SetDeviceInfo
     * Sets the device information ("/oic/d")
     *
     * @return OC_STACK_OK on success OC_STACK_ERROR on failure
     */
    OCStackResult setDeviceInfo(void);

    // Set of strings for each of device info fields
    std::string  deviceName = "Binary Switch";
    std::string  deviceType = "oic.d.light";
    std::string  specVersion = "ocf.1.0.0";
    std::vector<std::string> dataModelVersions;

    std::string  protocolIndependentID = "fa008167-3bbf-4c9d-8604-c9bcb96cb712";

private:
    // Set of strings for each of platform Info fields
    std::string m_platformId = "0A3E0D6F-DBF5-404E-8719-D6880042463A";
    std::string m_manufacturerName = "ocf";
    std::string m_manufacturerLink = "https://ocf.org/";
    std::string m_modelNumber = "ModelNumber";
    std::string m_dateOfManufacture = "2017-12-01";
    std::string m_platformVersion = "1.0";
    std::string m_operatingSystemVersion = "myOS";
    std::string m_hardwareVersion = "1.0";
    std::string m_firmwareVersion = "1.0";
    std::string m_supportLink = "https://ocf.org/";
    std::string m_systemTime = "2017-12-01T12:00:00.52Z";

    /*
    * duplicateString
    *
    * @param targetString  destination string, will be allocated
    * @param sourceString  source string, e.g. will be copied
    */
    void duplicateString(char ** targetString, std::string sourceString);

    /**
     *  SetPlatformInfo
     *  Sets the platform information ("oic/p")
     *
     * @param platformID the platformID
     * @param manufacturerName the manufacturerName
     * @param manufacturerUrl the manufacturerUrl
     * @param modelNumber the modelNumber
     * @param platformVersion the platformVersion
     * @param operatingSystemVersion the operatingSystemVersion
     * @param hardwareVersion the hardwareVersion
     * @param firmwareVersion the firmwareVersion
     * @param supportUrl the supportUrl
     * @param systemTime the systemTime
     * @return OC_STACK_ERROR or OC_STACK_OK
     */
    void setPlatformInfo(std::string platformID,
                         std::string manufacturerName,
                         std::string manufacturerUrl,
                         std::string modelNumber,
                         std::string dateOfManufacture,
                         std::string platformVersion,
                         std::string operatingSystemVersion,
                         std::string hardwareVersion,
                         std::string firmwareVersion,
                         std::string supportUrl,
                         std::string systemTime);

    /*
     * deletePlatformInfo
     * Deleted the allocated platform information
     */
    void deletePlatformInfo(void);

    // OCPlatformInfo Contains all the platform info
    OCPlatformInfo platformInfo;
};

/**
*  server_fopen
*  opens file
*  implements redirection to open:
* - initial security settings
* - introspection file
* @param path path+filename of the file to open
* @param mode mode of the file to open
* @return the filehandle of the opened file (or error)
*/
FILE* server_fopen(const char* path, const char* mode)
{
    FILE* fileptr = NULL;

    if (0 == strcmp(path, OC_SECURITY_DB_DAT_FILE_NAME))
    {
        // reading the security initial setup file
        fileptr = fopen("server_security.dat", mode);
        std::cout << "reading security file 'server_security.dat' ptr: " << fileptr << std::endl;
        return fileptr;
    }
    else if (0 == strcmp(path, OC_INTROSPECTION_FILE_NAME))
    {
        // reading the introspection file
        fileptr = fopen("server_introspection.dat", mode);
        std::cout << "reading introspection file  'server_introspection.dat' ptr: " << fileptr << std::endl;
        return fileptr;
    }
    else
    {
        std::cout << "persistent storage - server_fopen: " << path << std::endl;
        return fopen(path, mode);
    }
}

// Create persistent storage handlers
OCPersistentStorage ps{server_fopen, fread, fwrite, fclose, unlink};

/*
* Platform Constructor
*/
Platform::Platform(void)
{
    std::cout << "Running Platform constructor" << std::endl;
    dataModelVersions.push_back("ocf.res.1.3.0");
    dataModelVersions.push_back("ocf.sh.1.3.0");

    // create the platform
    PlatformConfig cfg
    {
        ServiceType::InProc,
        ModeType::Server,
        &ps
    };
    OCPlatform::Configure(cfg);
    setPlatformInfo(m_platformId, m_manufacturerName, m_manufacturerLink,
                    m_modelNumber, m_dateOfManufacture, m_platformVersion,
                    m_operatingSystemVersion, m_hardwareVersion,
                    m_firmwareVersion, m_supportLink, m_systemTime);
}

/*
* Platform Destructor
*/
Platform::~Platform(void)
{
    std::cout << "Running Platform destructor" << std::endl;
    deletePlatformInfo();
}

OCStackResult Platform::start(void)
{
    return OCPlatform::start();
}

OCStackResult Platform::registerPlatformInfo(void)
{
    OCStackResult result = OC_STACK_ERROR;
    result = OCPlatform::registerPlatformInfo(platformInfo);
    return result;
}

OCPlatformInfo* Platform::getPlatformInfo(void)
{
    return &platformInfo;
}

OCStackResult Platform::stop(void)
{
    return OCPlatform::stop();
}

void Platform::duplicateString(char ** targetString, std::string sourceString)
{
    *targetString = new char[sourceString.length() + 1];
    strncpy(*targetString, sourceString.c_str(), (sourceString.length() + 1));
}

void Platform::setPlatformInfo(std::string platformID,
                               std::string manufacturerName,
                               std::string manufacturerUrl,
                               std::string modelNumber,
                               std::string dateOfManufacture,
                               std::string platformVersion,
                               std::string operatingSystemVersion,
                               std::string hardwareVersion,
                               std::string firmwareVersion,
                               std::string supportUrl,
                               std::string systemTime)
{
    duplicateString(&platformInfo.platformID, platformID);
    duplicateString(&platformInfo.manufacturerName, manufacturerName);
    duplicateString(&platformInfo.manufacturerUrl, manufacturerUrl);
    duplicateString(&platformInfo.modelNumber, modelNumber);
    duplicateString(&platformInfo.dateOfManufacture, dateOfManufacture);
    duplicateString(&platformInfo.platformVersion, platformVersion);
    duplicateString(&platformInfo.operatingSystemVersion, operatingSystemVersion);
    duplicateString(&platformInfo.hardwareVersion, hardwareVersion);
    duplicateString(&platformInfo.firmwareVersion, firmwareVersion);
    duplicateString(&platformInfo.supportUrl, supportUrl);
    duplicateString(&platformInfo.systemTime, systemTime);
}

void Platform::deletePlatformInfo()
{
    delete[] platformInfo.platformID;
    delete[] platformInfo.manufacturerName;
    delete[] platformInfo.manufacturerUrl;
    delete[] platformInfo.modelNumber;
    delete[] platformInfo.dateOfManufacture;
    delete[] platformInfo.platformVersion;
    delete[] platformInfo.operatingSystemVersion;
    delete[] platformInfo.hardwareVersion;
    delete[] platformInfo.firmwareVersion;
    delete[] platformInfo.supportUrl;
    delete[] platformInfo.systemTime;
}

/**
*  SetDeviceInfo
*  Sets the device information ("oic/d"), from the globals

* @return OC_STACK_ERROR or OC_STACK_OK
*/
OCStackResult Platform::setDeviceInfo()
{
    OCStackResult result = OC_STACK_ERROR;

    OCResourceHandle handle = OCGetResourceHandleAtUri(OC_RSRVD_DEVICE_URI);
    if (handle == NULL)
    {
        std::cout << "Failed to find resource " << OC_RSRVD_DEVICE_URI << std::endl;
        return result;
    }
    result = OCBindResourceTypeToResource(handle, deviceType.c_str());
    if (result != OC_STACK_OK)
    {
        std::cout << "Failed to add device type" << std::endl;
        return result;
    }
    result = OCPlatform::setPropertyValue(PAYLOAD_TYPE_DEVICE, OC_RSRVD_DEVICE_NAME, deviceName);
    if (result != OC_STACK_OK)
    {
        std::cout << "Failed to set device name" << std::endl;
        return result;
    }
    result = OCPlatform::setPropertyValue(PAYLOAD_TYPE_DEVICE, OC_RSRVD_DATA_MODEL_VERSION,
                                          dataModelVersions);
    if (result != OC_STACK_OK)
    {
        std::cout << "Failed to set data model versions" << std::endl;
        return result;
    }
    result = OCPlatform::setPropertyValue(PAYLOAD_TYPE_DEVICE, OC_RSRVD_SPEC_VERSION, specVersion);
    if (result != OC_STACK_OK)
    {
        std::cout << "Failed to set spec version" << std::endl;
        return result;
    }
    result = OCPlatform::setPropertyValue(PAYLOAD_TYPE_DEVICE, OC_RSRVD_PROTOCOL_INDEPENDENT_ID,
                                          protocolIndependentID);
    if (result != OC_STACK_OK)
    {
        std::cout << "Failed to set piid" << std::endl;
        return result;
    }

    return OC_STACK_OK;
}

#ifdef __unix__
// global needs static, otherwise it can be compiled out and then Ctrl-C does not work
static int quit = 0;
// handler for the signal to stop the application
void handle_signal(int signal)
{
    OC_UNUSED(signal);
    quit = 1;
}
#endif

// main application
// starts the server
int main(void)
{
    Platform platform;
    if(OC_STACK_OK != platform.start())
    {
        std::cerr << "Failed to start the IoTivity platform." << std::endl;
        return 1;
    }

    std::cout << "/oic/p" << std::endl;
    // initialize "/oic/p"
    if (OC_STACK_OK != platform.registerPlatformInfo())
    {
        std::cerr << "Failed platform registration (/oic/p)." << std::endl;
    }
    // initialize "/oic/d"
    std::cout << "/oic/d" << std::endl;
    if (OC_STACK_OK != platform.setDeviceInfo())
    {
        std::cerr << "Failed device registration (/oic/d)." << std::endl;
    }

    std::cout << "device type: " <<  platform.deviceType << std::endl;
    std::cout << "platformID: " <<  platform.getPlatformInfo()->platformID << std::endl;
    std::cout << "platform independent: " <<  platform.protocolIndependentID << std::endl;

    // create the server
    IoTServer server;
    if (OC_STACK_OK != server.registerResources())
    {
        std::cerr << "Failed to register server resources." << std::endl;
        return 1;
    }

#ifdef __unix__
    struct sigaction sa;
    sigfillset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    std::cout << "Press Ctrl-C to quit...." << std::endl;
    do
    {
        usleep(2000000);
    }
    while (quit != 1);
#endif


#if defined(_WIN32)
    std::cout << "Press Ctrl-C to quit...." << std::endl;
    // we will keep the server alive for at most 30 minutes
    std::this_thread::sleep_for(std::chrono::minutes(30));
    OC_VERIFY(OCPlatform::stop() == OC_STACK_OK);
#endif

    return 0;
}

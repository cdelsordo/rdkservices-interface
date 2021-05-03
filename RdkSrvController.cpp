/*******************************************************************************
# Copyright 2021 CommScope, Inc., All rights reserved.
#
# This program is confidential and proprietary to CommScope, Inc.
# (CommScope), and may not be copied, reproduced, modified,
# disclosed to others, published or used, in whole or in part,
# without the express prior written permission of CommScope.
#*******************************************************************************/
#include "RdkSrvController.h"
static int m_idActivateRdkServices = 1111;
static int m_idDSGetCurrentResolution = 2222;
static int m_idDISystemInfo = 3333;
static RdkSrvController* m_controller = NULL;
 
void WebSocketClientProcessServerReponse(std::string response)
{
 // printf("RdkSrvController::WebSocketClientProcessServerReponse entered response: %s lenght:%d \n",response.c_str(),response.size());
  cJSON *serverResponseId = NULL;
  cJSON *serverResponse = NULL;
  
  // It will parse the JSON and allocate a tree of cJSON items that represents it.
  // Once it returns, you are fully responsible for deallocating it after use with cJSON_Delete.
  serverResponse = cJSON_ParseWithLength(response.c_str(), response.size());
  if (serverResponse == NULL)
  {
    printf("RdkSrvController::WebSocketClientProcessServerReponse FAILED serverResponse is NULL \n");
  }
  else {
    serverResponseId = cJSON_GetObjectItemCaseSensitive(serverResponse, "id");
    if (cJSON_IsNumber(serverResponseId))
    {
	  if (serverResponseId->valueint == m_idActivateRdkServices)  {
		if (m_controller->ProcessActivateDSReponse(response)) {
		  m_controller->m_displaySettingsActivated = true;
		}
		else {
		  printf("RdkSrvController::WebSocketClientProcessServerReponse ProcessActivateDSReponse FAILED \n");
	    }
	  } else if (serverResponseId->valueint == m_idDSGetCurrentResolution) {
		if (m_controller->ProcessGetCurrentResolutionDSReponse(response)) {
		  m_controller->m_receivedCurrentResolution = true;
		}
		else {
		  printf("RdkSrvController::WebSocketClientProcessServerReponse ProcessGetCurrentResolutionDSReponse FAILED \n");
		}
      } else if (serverResponseId->valueint == m_idDISystemInfo) {
		 if (m_controller->ProcessSystemInfoDIResponse(response)) {
		   m_controller->m_receivedSerialnumber = true;
		   m_controller->m_receivedDeviceName = true;
         }
         else {
		   printf("RdkSrvController::WebSocketClientProcessServerReponse ProcessSystemInfoDIResponse FAILED \n");
         }		  
	  } else {
	    printf("RdkSrvController::WebSocketClientProcessServerReponse FAILED serverResponseId NOT FOUND \n");
	  }
    }   // End Check serverResponseId
	else {
	  printf("RdkSrvController::WebSocketClientProcessServerReponse serverResponseId  Number check FAILED \n");
	}
    cJSON_Delete(serverResponse);
  }  // End Check serverResponse
}

RdkSrvController::RdkSrvController()
{
  bool retVal = false;
  printf("RdkSrvController::RdkSrvController entered \n");
  m_controller = this;
  RdkSrvCallSigns = {{ACTIVITY_MONITOR, "org.rdk.ActivityMonitor.1" },
                     {AV_INPUT, "org.rdk.AVInput.1"},
                     {BLUETOOTH, "org.rdk.Bluetooth.2"},
                     {CONTINUE_WATCHING, ""},
                     {CONTROL_SERVICE, ""},
                     {DEVICE_DIAGNOSTICS, "org.rdk.DeviceDiagnostics"},
                     {DEVICE_IDENTIFICATION, ""},
                     {DEVICE_INFO,"DeviceInfo"},
                     {DISPLAY_INFO,""},
                     {DISPLAY_SETTINGS,"org.rdk.DisplaySettings.1"},
                     {FRAME_RATE, ""},
                     {FRONT_PANEL, ""},
                     {HDCP_PROFILE, ""},
                     {HDMI_CEC, ""},
                     {HDMI_CEC_2, ""},
                     {HDMI_CEC_SINK, ""},
                     {HDMI_INPUT, ""},
                     {LOCATION_SYNC, ""},
                     {LOGGING_PREFERENCES, ""},
                     {MESSENGER, ""},
                     {MONITOR, ""},
                     {MOTION_DETECTION, ""},
                     {NETWORK, ""},
                     {OPEN_CDMI, ""},
                     {PACKAGER, ""},
                     {PERSISTENT_STORE, ""},
                     {PLAYER_INFO, ""},
                     {RDK_SHELL, ""},
                     {REMOTE_ACTION_MAPPING, ""},
                     {SCREEN_CAPTURE, ""},
                     {SECURITY_AGENT, ""},
                     {STATE_OBSERVER, ""},
                     {SYSTEM_SERVICES, ""},
                     {TEXT_TO_SPEECH, ""},
                     {TIMER, ""},
                     {TRACE_CONTROL, ""},
                     {USER_PREFERENCES, ""},
                     {WAREHOUSE, ""},
                     {WEBKITBROWSER, ""},
                     {WIFI_MANAGER, ""},
                     {XCAST, ""}
  };
	
  RdkSrvControllerMethods = {{ACTIVATE, "Controller.1.activate"},
                             {DEACTIVATE, "Controller.1.deactivate"},
					         {STARTDISCOVERY, "Controller.1.startdiscovery"},
                             {STORECONFIG, "Controller.1.storeconfig"},
                             {DELETE, "Controller.1.delete"},
                             {HARIKARI, "Controller.1.harakiri"}
  };
  // Initialize connection to Service Controller
  retVal = Initialize();
}

RdkSrvController::~RdkSrvController()
{
  printf("RdkSrvController::~RdkSrvController entered \n");
  if (m_endpoint != NULL) {
    delete m_endpoint;
	m_endpoint = NULL;
  }
}

std::string RdkSrvController::DSControllerActivateCmd()
{
  // find Controller Activate method from map and use it in cjson item
  /* cstrings to build for Controller Activation
	const char *activateDisplaySettings = {
      "jsonrpc": "2.0", 
      "id": "1234567890", 
      "method": "Controller.1.activate", 
      "params": {
        "callsign": "org.rdk.DisplaySettings"
      }
	};
  */
  std::string strActivateObjectJson = "";
  std::string controllerActivateMethod = RdkSrvControllerMethods.find(ACTIVATE)->second;
  if (RdkSrvControllerMethodsIter != RdkSrvControllerMethods.end()) {
    printf("RdkSrvController::DSControllerActivateCmd entered found Controller Activate method \n");
  }
  else {
	return strActivateObjectJson;
  }
  std::string jsonRpcVersion = "2.0";
  // Each message id is specific to the call made so we can
  // recognize it when the endpoint OnMessage fires a response.
  std::string activateCallsign;
  cJSON *activateObject;
  activateObject = cJSON_CreateObject();
  if (activateObject == NULL) {
    printf("RdkSrvController::DSControllerActivateCmd activateObject is NULL \n");
  }
  cJSON_AddStringToObject(activateObject, "jsonrpc", jsonRpcVersion.c_str());
  cJSON_AddNumberToObject(activateObject, "id", m_idActivateRdkServices);
  // set up Service Controller Activate Method
  cJSON_AddStringToObject(activateObject, "method", controllerActivateMethod.c_str());
  cJSON *params;
  cJSON_AddItemToObject(activateObject,"params", params = cJSON_CreateObject());
  // before assigning a string to callsign do lookup.
  // find callsign from map and use it in cjson item
  activateCallsign =  RdkSrvCallSigns.find(DISPLAY_SETTINGS)->second;
  if (RdvSrvsCallSignsIter != RdkSrvCallSigns.end()) {
    printf("RdkSrvController::DSControllerActivateCmd entered found displaysettings callsign \n");
	cJSON_AddStringToObject(params, "callsign", activateCallsign.c_str());
  }
  else {
    printf("RdkSrvController::DSControllerActivateCmd entered DID NOT fIND displaysettings callsign \n");
	return strActivateObjectJson;
  }
  // output string
  char *pActivateObjectStr = cJSON_Print(activateObject);
  strActivateObjectJson = pActivateObjectStr;
  cJSON_Delete(activateObject);
  if (NULL != pActivateObjectStr) {
    free(pActivateObjectStr);			
    pActivateObjectStr = NULL;
  }
  return strActivateObjectJson;
}

std::string RdkSrvController::DSGetCurrentResolutionCmd()
{
  /*
	getCurrentResolution (v1)
    Description: Returns the current resolution on the selected video display port.

    Arguments:

      videoDisplay : string - video display port name (HDMI0 if no argument specified)
    Returns:

      resolution : string - current resolution
    success : bool - result of the request


    Request : {"jsonrpc":"2.0", "id":3, "method":"org.rdk.DisplaySettings.1.getCurrentResolution", "params":{"videoDisplay":"HDMI0"}}
 
    Response: {"jsonrpc":"2.0","id":3,"result":{"resolution":"720p60","success":true}}
  */
  std::string strDSGetCurrentResolutionObjectJson = "";
  std::string jsonRpcVersion = "2.0";

  std::string methodDSGetCurrentResolution = "org.rdk.DisplaySettings.1.getCurrentResolution";
  cJSON *objectDSGetCurrentResolution = cJSON_CreateObject();
  cJSON_AddStringToObject(objectDSGetCurrentResolution, "jsonrpc", jsonRpcVersion.c_str());
  cJSON_AddNumberToObject(objectDSGetCurrentResolution, "id", m_idDSGetCurrentResolution);
  // set up Method
  cJSON_AddStringToObject(objectDSGetCurrentResolution, "method", methodDSGetCurrentResolution.c_str());
  cJSON *params;
  cJSON_AddItemToObject(objectDSGetCurrentResolution,"params", params = cJSON_CreateObject());
  cJSON_AddStringToObject(params, "videoDisplay", "HDMI0");
  // output string
  char *pObjectDSGetCurrentResolutionStr = cJSON_Print(objectDSGetCurrentResolution);
  strDSGetCurrentResolutionObjectJson = pObjectDSGetCurrentResolutionStr;
  cJSON_Delete(objectDSGetCurrentResolution);
  if (NULL != pObjectDSGetCurrentResolutionStr) {
    free(pObjectDSGetCurrentResolutionStr);			
    pObjectDSGetCurrentResolutionStr = NULL;
  }
  return strDSGetCurrentResolutionObjectJson;
}

bool RdkSrvController::IssueActivateDS()
{
  bool retVal = false;
  std::string dsControllerActivateCmd = DSControllerActivateCmd();
  if (m_current_connectid != INVALID_CONNNECT_ID) {
	if (WebSocketClientSend(m_current_connectid, dsControllerActivateCmd)) {
	  return true;
	}
	else {
	  printf("RdkSrvController::IssueActivateDS WebSocketClientSend failed  \n");
      return retVal;
	}
  }
  else {
    printf("RdkSrvController::IssueActivateDS failed INVALID_CONNECTID \n");
    return retVal;
  }
}

bool RdkSrvController::ProcessActivateDSReponse(std::string response)
{
	/*
  ```json
{
    "jsonrpc": "2.0", 
    "id": 1234567890, 
    "result": null
}
*/
  bool retVal = false;
  cJSON *activateDSReponse = NULL;
  cJSON *activateDSReponseResult = NULL;
  // It will parse the JSON and allocate a tree of cJSON items that represents it.
  // Once it returns, you are fully responsible for deallocating it after use with cJSON_Delete.
  // any cJSON_GetObjectItemCaseSensitive object does not get deleted with cJSON_Delete
  activateDSReponse = cJSON_ParseWithLength(response.c_str(), response.size());
  if (activateDSReponse == NULL)
  {
    printf("RdkSrvController::ProcessActivateDSReponse FAILED activateDSReponse is NULL \n");
  }
  else {
    activateDSReponseResult = cJSON_GetObjectItemCaseSensitive(activateDSReponse, "result");
	if (activateDSReponseResult->valuestring == NULL) {
	  printf("RdkSrvController::ProcessActivateDSReponse activateDSReponseResult->valuestring == NULL \n");
	}
	cJSON_Delete(activateDSReponse);
	retVal = true;
  }
  return retVal;
}		

bool RdkSrvController::IssueGetCurrentResolutionDS()
{
  bool retVal = false;
  std::string dsGetCurrentResolutionCmd = DSGetCurrentResolutionCmd();
  if (m_current_connectid != INVALID_CONNNECT_ID) {
	if (WebSocketClientSend(m_current_connectid, dsGetCurrentResolutionCmd)) {
	  retVal = true;
	}
	else {
	  printf("RdkSrvController::IssueDisplaySettingsGetCurrentReolution WebSocketClientSend failed  \n");
	}
  }
  else {
    printf("RdkSrvController::IssueDisplaySettingsGetCurrentReolution failed INVALID_CONNECTID \n");
  }
  return retVal;
}
	
bool RdkSrvController::ProcessGetCurrentResolutionDSReponse(std::string response)
{
  /*
	getCurrentResolution (v1)
    Description: Returns the current resolution on the selected video display port.

    Arguments:

      videoDisplay : string - video display port name (HDMI0 if no argument specified)
    Returns:

      resolution : string - current resolution
    success : bool - result of the request


    Request : {"jsonrpc":"2.0", "id":3, "method":"org.rdk.DisplaySettings.1.getCurrentResolution", "params":{"videoDisplay":"HDMI0"}}
 
    Response: {"jsonrpc":"2.0","id":3,"result":{"resolution":"720p60","success":true}}
  */
  bool retVal = false;
  cJSON *getCurrentResolutionDSResponse = NULL;
  cJSON *getCurrentResolution = NULL;
  cJSON *getCurrentResolutionResult = NULL;
  cJSON *getCurrentResolutionResultSuccess = NULL;

  // It will parse the JSON and allocate a tree of cJSON items that represents it.
  // Once it returns, you are fully responsible for deallocating it after use with cJSON_Delete.
  getCurrentResolutionDSResponse = cJSON_ParseWithLength(response.c_str(), response.size());
  if (getCurrentResolutionDSResponse == NULL) {
    printf("RdkSrvController::ProcessGetCurrentResolutionDSReponse FAILED getCurrentResolutionDSResponse is NULL \n");
  }
  else {
	getCurrentResolutionResult = cJSON_GetObjectItemCaseSensitive(getCurrentResolutionDSResponse, "result");
	getCurrentResolutionResultSuccess = cJSON_GetObjectItemCaseSensitive(getCurrentResolutionResult, "success");
	getCurrentResolution = cJSON_GetObjectItemCaseSensitive(getCurrentResolutionResult, "resolution");
	if (cJSON_IsTrue(getCurrentResolutionResultSuccess) == 1) {    
	  m_currentResolution = getCurrentResolution->valuestring;
	  printf("RdkSrvController::ProcessGetCurrentResolutionDSReponse resultsuccess is true m_currentResolution:%s \n",m_currentResolution.c_str());
	  retVal = true;
	}  // End check getCurrentResolutionResultSuccess valueString
	else {
	  printf("RdkSrvController::ProcessGetCurrentResolutionDSReponse FAILED resultsuccess is false  \n");
	}
	cJSON_Delete(getCurrentResolutionDSResponse);
  }   // End check getCurrentResolutionDSReponse
  return retVal;
}

bool RdkSrvController::IssueDIsystemInfo()
{
  bool retVal = false;
  std::string diSystemInfoCmd = DISystemInfoCmd();

  if (m_current_connectid != INVALID_CONNNECT_ID) {
	if (WebSocketClientSend(m_current_connectid, diSystemInfoCmd)) {
	  retVal = true;
	}
	else {
	  printf("RdkSrvController::IssueDIsystemInfo WebSocketClientSend failed  \n");
	}
  }
  else {
    printf("RdkSrvController::IssueDIsystemInfo failed INVALID_CONNECTID \n");
  }
  return retVal;
}

std::string RdkSrvController::DISystemInfoCmd()
{
 /*
#### Get Request

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "method": "DeviceInfo.1.systeminfo"
}
```
#### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "version": "1.0#14452f612c3747645d54974255d11b8f3b4faa54",
        "uptime": 120,
        "totalram": 655757312,
        "freeram": 563015680,
        "devicename": "buildroot",
        "cpuload": "2",
        "serialnumber": "WPEuCfrLF45",
        "time": "Mon, 11 Mar 2019 14:38:18"
    }
}
*/
  std::string strDISystemInfoObjectJson = "";
  std::string jsonRpcVersion = "2.0";

  std::string methodDISystemInfo = "DeviceInfo.1.systeminfo";
  cJSON *objectDISystemInfo = cJSON_CreateObject();
  cJSON_AddStringToObject(objectDISystemInfo, "jsonrpc", jsonRpcVersion.c_str());
  cJSON_AddNumberToObject(objectDISystemInfo, "id", m_idDISystemInfo);
  // set up Method
  cJSON_AddStringToObject(objectDISystemInfo, "method", methodDISystemInfo.c_str());
 
  // output string
  char *pObjectDISystemInfoStr = cJSON_Print(objectDISystemInfo);
  strDISystemInfoObjectJson = pObjectDISystemInfoStr;
  cJSON_Delete(objectDISystemInfo);
  if (NULL != pObjectDISystemInfoStr) {
    free(pObjectDISystemInfoStr);			
    pObjectDISystemInfoStr = NULL;
  }
  return strDISystemInfoObjectJson;
}

bool RdkSrvController::ProcessSystemInfoDIResponse(std::string response)
{
/*
  #### Get Response

```json
{
    "jsonrpc": "2.0",
    "id": 1234567890,
    "result": {
        "version": "1.0#14452f612c3747645d54974255d11b8f3b4faa54",
        "uptime": 120,
        "totalram": 655757312,
        "freeram": 563015680,
        "devicename": "buildroot",
        "cpuload": "2",
        "serialnumber": "WPEuCfrLF45",
        "time": "Mon, 11 Mar 2019 14:38:18"
    }
}
*/
  bool retVal = false;
  cJSON *systemInfoDIResponse = NULL;
  cJSON *systemInfoResult = NULL;
  cJSON *systemInfoResultSerialNumber = NULL;
  cJSON *systemInfoResultDeviceName = NULL;
  // It will parse the JSON and allocate a tree of cJSON items that represents it.
  // Once it returns, you are fully responsible for deallocating it after use with cJSON_Delete.
  systemInfoDIResponse = cJSON_ParseWithLength(response.c_str(), response.size());
  if (systemInfoDIResponse == NULL)
  {
    printf("RdkSrvController::ProcessSystemInfoDIResponse FAILED systemInfoDIResponse is NULL \n");
  }
  else {
	systemInfoResult = cJSON_GetObjectItemCaseSensitive(systemInfoDIResponse, "result");
	systemInfoResultSerialNumber = cJSON_GetObjectItemCaseSensitive(systemInfoResult, "serialnumber");
	systemInfoResultDeviceName = cJSON_GetObjectItemCaseSensitive(systemInfoResult, "devicename");
	m_serialnumber = systemInfoResultSerialNumber->valuestring;
	m_deviceName = systemInfoResultDeviceName->valuestring;
	printf("RdkSrvController::ProcessSystemInfoDIResponse m_serialnumber:%s m_deviceName:%s \n",
		    m_serialnumber.c_str(), m_deviceName.c_str());
	cJSON_Delete(systemInfoDIResponse);
	retVal = true;
  }    // End check systemInfoDIResponse
  return retVal;
}

int RdkSrvController::WebSocketClientOpen(std::string uri)
{
  // create socket connection
  int m_connect_id = -1;
  m_endpoint = new websocket_endpoint();
  if (m_endpoint != NULL) {
    m_connect_id = m_endpoint->connect(uri);

    if (m_connect_id != -1) {
      printf("RdkSrvController::WebSocketClientOpen connect SUCCESS m_connection_id: %d \n", m_connect_id );
    }
    else {
      printf("RdkSrvController::WebSocketClientOpen FAILED \n");
	  delete m_endpoint;
	  m_endpoint = NULL;
    }
  }  // check endpoint == NULL
  return m_connect_id; 
}

bool RdkSrvController::WebSocketClientClose(int connect_id)
{
  bool retVal = false;
  // close socket connection
  websocketpp::lib::error_code ec;
  if (m_endpoint != NULL) {
    //void close(int id, websocketpp::close::status::value code, std::string reason);
	// m_endpoint.close(it->second->get_hdl(), websocketpp::close::status::going_away, "", ec);
    m_endpoint->close(connect_id, websocketpp::close::status::going_away, "going away");
	retVal = true;
  }  // check endpoint == NULL
  else {
    printf(" RdkSrvController::WebSocketClientClose FAILED check endpoint == NULL \n");
  }
  return retVal; 
}

bool RdkSrvController::WebSocketClientSend(int connectId, std::string sendString)
{
  bool retVal = false;
  if ((m_endpoint != NULL) && (connectId!= -1)) {
//	printf("RdkSrvController::WebSocketClientSend connectId: %d, sendString: %s \n",
//	        connectId, 
//			sendString.c_str());
    m_endpoint->send(connectId, sendString);
	return true;
  }
  else {
    printf("RdkSrvController::WebSocketClientSend failed bad params \n");
  }

  return retVal;
}

bool RdkSrvController::Initialize()
{
  currentState = ACTIVATE_DISPLAYSETTINGS;
  m_displaySettingsActivated = false;
  m_receivedCurrentResolution = false;
  m_receivedSerialnumber = false;
  m_receivedDeviceName = false;
  m_currentResolution = "";
  m_deviceName = "";
  m_serialnumber = "";

  // initialize on_message callback function pointer for registration
  f_on_message_t on_message_p = &WebSocketClientProcessServerReponse;
  printf("RdkSrvController::Initialize before WebSocketClientOpen on_message_p %p \n", on_message_p);
  bool retVal = false;
  m_current_connectid = INVALID_CONNNECT_ID;
  m_current_connectid = WebSocketClientOpen(m_jsonrpc_ws_uri);
  m_metadata = m_endpoint->get_metadata(m_current_connectid);
  
  // register callback with the connection_metadata for the connection to the controller
  m_metadata->registerOnMessageCallback(on_message_p);
  
  //activate rdkservices DisplaySettings plugin
  retVal = IssueActivateDS();
  int count = 4;
  if (retVal != false) {
    while (count != 0) {
	  if (m_displaySettingsActivated == true) {
		break;
	  }
	  sleep(2);
	  count -= 1;
	}
  }
  //request currentResolution from rdkservices DisplaySettings plugin
  count = 4;
  retVal = IssueGetCurrentResolutionDS();
  if (retVal != false) {
    while (count != 0) {
	  if (m_receivedCurrentResolution == true) {
		break;
	  }
	  sleep(2);
	  count -= 1;
	}
  }
  //get serialnumber and device name from rdkservices deviceInfo systemInfo property
  count = 4;
  retVal = IssueDIsystemInfo();
  if (retVal != false) {
    while (count != 0) {
	  if (m_receivedSerialnumber == true) {
		break;
	  }
	  sleep(2);
	  count -= 1;
	}
  }
 
  retVal = WebSocketClientClose(m_current_connectid);
  if (retVal == true)
  {
    printf("RdkSrvController::Initialize WebSocketClientClose SUCCESS \n");
  }
  else {
    printf("RdkSrvController::Initialize WebSocketClientClose FAILED \n");
  }
  
  return(retVal);
}

std::string RdkSrvController::GetResolution()
{
  return m_currentResolution;
}

std::string RdkSrvController::GetDeviceName()
{
  return m_deviceName;
}

std::string RdkSrvController::GetDeviceSerialnumber()
{
  return m_serialnumber;
}

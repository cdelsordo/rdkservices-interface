/*******************************************************************************
# Copyright 2021 CommScope, Inc., All rights reserved.
#
# This program is confidential and proprietary to CommScope, Inc.
# (CommScope), and may not be copied, reproduced, modified,
# disclosed to others, published or used, in whole or in part,
# without the express prior written permission of CommScope.
#*******************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <map>
#include <string>
#include <stdint.h>
#include <fstream>
#include <vector>
#include <cjson/cJSON.h>
#include "WSClientwebsocketendpoint.h"

#define INVALID_CONNNECT_ID -1
class RdkSrvController{

// This class is meant to be called from users porting layer. It requires a websockets class which
// abstracts use of libwebsockets.  This class will be used to hand out instances of each rdkservices plugin.
// For the time being assume a single websocket will be used for all communication. In the future
// some classes live BLE voice may require a separate socket
// Interface defini can be found aith link below
// https://github.com/rdkcentral/Thunder/blob/master/Source/WPEFramework/ControllerPlugin.md
/*A Websocket can be opened directly to the plugin or it can be opened to the Controller. 
 *Directly means that socket can only communicate with the designated (Callsign) plugin. 
 *The method, in the JSONRPC message, can be issued without the callsign, if the websocket is opened directly.
 *If the socket is opened towards the controller, the method should use the designator definition, 
 * as found in §2.1.1 Designator definition) to reach a specific plugin for method invocation.
*/

public:

    typedef enum _RdkSrvControllerStates_t {
		 ACTIVATE_DISPLAYSETTINGS,
		 GET_CURRENT_RESOLUTION,
		 GET_SERIAL_NUMBER,
		 GET_MODEL_NUMBER,
		 RDKSERVICES_SETUP_COMPLETE
     } RdkSrvControllerStates_t;
	 
    /* _Plugin Identifiers as of 0218201 October tag of RDK-One this will change so
     	keep up with it */
    typedef enum _RdkServices_Id_t {
      ACTIVITY_MONITOR,
      AV_INPUT,
	  BLUETOOTH,
      CONTINUE_WATCHING,
	  CONTROL_SERVICE,
      DATA_CAPTURE,
	  DEVICE_DIAGNOSTICS,
	  DEVICE_IDENTIFICATION,
	  DEVICE_INFO,
	  DISPLAY_INFO,
	  DISPLAY_SETTINGS,
	  FRAME_RATE,
	  FRONT_PANEL,
	  HDCP_PROFILE,
	  HDMI_CEC,  // use cec 2 easier
	  HDMI_CEC_2,
	  HDMI_CEC_SINK,
	  HDMI_INPUT,
	  LOCATION_SYNC,
	  LOGGING_PREFERENCES,
	  MESSENGER,
	  MONITOR,
	  MOTION_DETECTION,
	  NETWORK,
	  OPEN_CDMI,
	  PACKAGER,
	  PERSISTENT_STORE,
	  PLAYER_INFO,
	  RDK_SHELL,
	  REMOTE_ACTION_MAPPING,
	  SCREEN_CAPTURE,
	  SECURITY_AGENT,
	  STATE_OBSERVER,
	  SYSTEM_SERVICES,
	  TEXT_TO_SPEECH,
	  TIMER,
	  TRACE_CONTROL,
	  USER_PREFERENCES,
	  WAREHOUSE,
	  WEBKITBROWSER,
	  WIFI_MANAGER,
	  XCAST
    } RdkServices_Id_t;
	
	websocket_endpoint *m_endpoint;
	int m_current_connectid;
	connection_metadata::ptr m_metadata;
	
	/*  used by all RdkServices Classes for callsign translation as of 0218201 October tag of 
	    RDK-One this will change so keep up with it */
	typedef std::map<RdkServices_Id_t, std::string> TRdvSrvsCallSigns;
	TRdvSrvsCallSigns RdkSrvCallSigns;
	typedef std::map<RdkServices_Id_t, std::string>::iterator TRdvSrvsCallSignsIter;
	TRdvSrvsCallSignsIter RdvSrvsCallSignsIter;
	
	typedef enum _RdkSrvControllerMethods_Id_t {
		 ACTIVATE,
		 DEACTIVATE,
		 STARTDISCOVERY,
		 STORECONFIG,
		 DELETE,
		 HARIKARI
     } RdkSrvControllerMethods_Id_t;
	 
	 typedef enum _RdkSrvDisplaySettingsMethods_Id_t {
		 GET_SUPPORTED_DISPLAYRESAOLUTIONS
     } RdkSrvDisplaySettingsMethods_Id_t;
		 
	
	/* Contoller method callsigns */
	typedef std::map<RdkSrvControllerMethods_Id_t, std::string> TRdkSrvControllerMethods;
	TRdkSrvControllerMethods RdkSrvControllerMethods;
	typedef std::map<RdkSrvControllerMethods_Id_t, std::string>::iterator TRdkSrvControllerMethodsIter;
	TRdkSrvControllerMethodsIter RdkSrvControllerMethodsIter;
	// i.e. f_on_message_t is a type: function pointer taking one std::string argument, returning void
    typedef void (*f_on_message_t) (std::string);
//	using f_on_message_t = void(*)(std::string);
	
    RdkSrvController();
    ~RdkSrvController();
	std::string DSControllerActivateCmd();
    std::string DSGetCurrentResolutionCmd();
	bool m_displaySettingsActivated;
    bool m_receivedCurrentResolution;
    bool m_receivedSerialnumber;
    bool m_receivedDeviceName;
	bool IssueActivateDS();
	bool GetActivateDSReponse();
	bool ProcessActivateDSReponse(std::string response);
	bool IssueGetCurrentResolutionDS();
	bool ProcessGetCurrentResolutionDSReponse(std::string response);
	bool IssueDIsystemInfo();
	std::string DISystemInfoCmd();
	bool ProcessSystemInfoDIResponse(std::string response);
	int WebSocketClientOpen(std::string uri);
    bool WebSocketClientClose(int connectId);
	bool WebSocketClientSend(int connectId, std::string sendString);
	std::string GetResolution();
	std::string GetDeviceName();
	std::string GetDeviceSerialnumber();

//callbacks for RdkServices Controller Plugin 
public:
 
   std::string m_controller_ws_uri = "ws://127.0.0.1:9998/jsonrpc/Service/Controller";
   std::string m__ws_display_settings_uri = "ws://127.0.0.1:9998/jsonrpc/org.rdk.DisplaySettings";
   std::string m_jsonrpc_ws_uri = "ws://127.0.0.1:9998/jsonrpc";
//   ws://<ip>:9998/jsonrpc
   // for ws connections directly to the plugin's callsign, then we send a c string method call
   std::string m_ws_uri_DeviceSettings = "ws://127.0.0.1:9998/jsonrpc/org.rdk.DeviceSettings.1";
   std::string m_ws_uri_DeviceInfo = "ws://127.0.0.1:9998/jsonrpc/DeviceInfo.1";
   std::string m_ws_uri_ActiveVideoClient = "ws://127.0.0.1:9998/ActiveVideoClient";
   RdkSrvControllerStates_t currentState;
   std::string m_currentResolution;
   std::string m_deviceName;
   std::string m_serialnumber;
   
private:
  std::string m_activateDSReponse;
  std::string m_currentResolutionDSReponse;
  bool Initialize();

};
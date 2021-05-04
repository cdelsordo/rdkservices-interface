# rdkservices-interface
rdkservices interface using websocketspp is a initial cut of implementation from CommScope to perform websockets based JSONRPC to rdkservices interface.
Please note: this implementation only includes what was needed for CommScope ActiveVideo embedded application and was not meant for a full featured rdkservices interface. 

This code represents a start but would and would need to be refactored and completed for a robust and complete rdkservices interface. 

The bitbake recipe ithat is included is designed to work in the latest Rdk environment; it builds both the RdkSrvController class plus the underlying websockets client 
endpoint classes. 

The url http://127.0.0.1:9998/Service/Controller used in RdkSrvController matches what is needed to communicate to the rdk wpeframwork Controller plugin. The interface
definition can be found at the link https://github.com/rdkcentral/Thunder/blob/master/Source/WPEFramework/ControllerPlugin.md
After instantiation of thr RdkSrvController class, the class automatically carries out the following functionality, First it ACTIVATE_DISPLAYSETTINGS via the controller plugin,
then it issues the rdkservices DisplaySettings GET_CURRENT_RESOLUTION call, then it issues rdkservices DisplayInfo systeminfo get property to obtain serial number and device name.

An application that uses this code would include RdkSrvController.h header file from the sysroots and will use a DEPENDS rdkservices-cs-interface in their application bitbake recipe.
After the application instantiates the RdkSrvController class and then can call RdkSrvController::GetResolution(), RdkSrvController::GetDeviceName() and RdkSrvController::GetDeviceSerialnumber()
Here is some example code that the application can use.
m_controller = new RdkSrvController();
if (m_controller != NULL) {	 
  mCurrentResolution = m_controller->GetResolution();
  if ((mCurrentResolution == "1080p60") || (mCurrentResolution == "1080p30")) {
    printf("activeVideoClientImpl::activeVideoClientImpl mCurrentResolution is 1080p setting width and height environment variables \n");
	setenv("WIDTH","1920",1); // 1 means overwrite
	   setenv("HEIGHT","1080",1); 
  }
  mSerialnumber = m_controller->GetDeviceSerialnumber(); 
  mDevicename = m_controller->GetDeviceName();
  printf("mCurrentResolution:%s mSerialnumber:%s mDevicename:%s \n",
	      mCurrentResolution.c_str(),
		  mSerialnumber.c_str(),
		  mDevicename.c_str());
}
else {
  printf("new RdkSrvController FAILED \n");
}
 



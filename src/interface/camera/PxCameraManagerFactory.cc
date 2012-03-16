#include "PxCameraManagerFactory.h"

#undef __linux

#ifdef __linux
#include "PxBluefoxCameraManager.h"
#endif
#include "PxFireflyCameraManager.h"
//#include "PxOpenCVCameraManager.h"

#ifdef __linux
PxCameraManagerPtr PxCameraManagerFactory::bluefoxCameraManager;
#endif
PxCameraManagerPtr PxCameraManagerFactory::fireflyCameraManager;
//PxCameraManagerPtr PxCameraManagerFactory::opencvCameraManager;

PxCameraManagerPtr
PxCameraManagerFactory::generate(const std::string& type)
{
	bool blueFoxSupported = false;
#ifdef __linux
	blueFoxSupported = true;
#endif

	// return singleton instance of device-specific camera manager
        if (type.compare("firefly") == 0)
        {
                if (fireflyCameraManager.get() == 0)
                {
                        fireflyCameraManager = PxCameraManagerPtr(new PxFireflyCameraManager);
                }
                return fireflyCameraManager;
        }
#ifdef __linux
        else if (type.compare("bluefox") == 0 && blueFoxSupported)
        {
                if (bluefoxCameraManager.get() == 0)
                {
                        bluefoxCameraManager = PxCameraManagerPtr(new PxBluefoxCameraManager);
                }
                return bluefoxCameraManager;
        }
#endif
        //	else if (type.compare("opencv") == 0)
//		{
//			if (opencvCameraManager.get() == 0)
//			{
//				opencvCameraManager = PxCameraManagerPtr(new PxOpenCVCameraManager);
//			}
//			return opencvCameraManager;
//		}
	else
	{
		return PxCameraManagerPtr();
	}
}

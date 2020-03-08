#pragma once


/**
*
* \file
* \brief
*
* Details
*
*/


namespace vve::syswin {

	std::vector<const char*> getRequiredInstanceExtensions();
	bool createSurface(VkInstance instance, VkSurfaceKHR* pSurface);

	void init();
	void tick();
	void cleanUp();
	void close();

	void windowSizeChanged();
}



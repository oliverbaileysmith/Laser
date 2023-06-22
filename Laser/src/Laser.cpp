#include "Application.h"

#define VERIFY(x) if (!x) return -1

int main()
{
	Application application;

	VERIFY(application.Init());
	VERIFY(application.GenBuffers());
	VERIFY(application.SetKernelArgs());
	VERIFY(application.Render());
	VERIFY(application.WriteOutput());

	return 0;
}
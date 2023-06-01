// handles everything
#include "Application.h"
// program entry point
int main()
{
	Application adventura;
	if (adventura.Init()) {
		if (adventura.Run()) {
			return adventura.Shutdown() ? 0 : 1;
		}
	}
	return 1;
}
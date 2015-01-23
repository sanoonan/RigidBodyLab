#include "Effector.h"

using namespace std;

Effector :: Effector()
{
	position = force = glm::vec3(0.0f);
	time = 0.0f;
}


void Effector :: addTBar(TwBar *bar)
{
	TwAddVarRW(bar, "Effector Position", TW_TYPE_DIR3F, &position, "");
	TwAddVarRW(bar, "Effector Force", TW_TYPE_DIR3F, &force, "");
	TwAddVarRW(bar, "Effector Time", TW_TYPE_FLOAT, &time, "");
}

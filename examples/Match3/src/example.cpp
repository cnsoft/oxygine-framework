#include "oxygine-framework.h"

#include "gameframe.h"
#include "shared.h"

using namespace oxygine;
void example_init()
{
	//load xml file with resources definition
	res.loadXML("resources.xml");

	RootActor::instance->addChild(new GameFrame());
}

void example_destroy()
{
	res.free();
}
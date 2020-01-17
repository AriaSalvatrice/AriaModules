#include "plugin.hpp"

Plugin* pluginInstance;

void init(Plugin* p) {
	pluginInstance = p;

	// Split series
	p->addModel(modelSplort);
	p->addModel(modelSmerge);
	p->addModel(modelSpleet);
	p->addModel(modelSwerge);
	p->addModel(modelSplirge);

	// Sequencer
	p->addModel(modelDarius);

	// Blank plate
	p->addModel(modelBlank);

#ifdef ARIA_DEV_MODULES
	// Bend series
	p->addModel(modelBendlet);
#endif

}

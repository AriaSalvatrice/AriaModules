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
	
	// Arcane
	p->addModel(modelArcane);
	p->addModel(modelAtout);
	p->addModel(modelAleister);

	// Blank plate
	p->addModel(modelBlank);

#ifdef ARIA_DEV_MODULES
	p->addModel(modelBendlet);
	p->addModel(modelUndular);
	p->addModel(modelTest);
#endif

}

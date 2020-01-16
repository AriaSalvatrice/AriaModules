#include "plugin.hpp"

Plugin* pluginInstance;

void init(Plugin* p) {
	pluginInstance = p;

	// Add modules here
	// Split series
	p->addModel(modelSplort);
	p->addModel(modelSmerge);
	p->addModel(modelSpleet);
	p->addModel(modelSwerge);
	p->addModel(modelSplirge);
	
#ifdef ARIA_DEV_MODULES
	// Bend series
	p->addModel(modelBendlet);
	
	// Sequencer
	p->addModel(modelDarius);
#endif

	// Blank plate
	p->addModel(modelBlank);
	
	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}

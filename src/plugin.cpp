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
    p->addModel(modelSrot);

    // Sequencer
    p->addModel(modelDarius);
    
    // Arcane
    p->addModel(modelArcane);
    p->addModel(modelAtout);
    p->addModel(modelAleister);
    
    // Live performance
    p->addModel(modelUndular);

    // Blank plate
    p->addModel(modelBlank);

    p->addModel(modelBendlet);
    p->addModel(modelTest);

}

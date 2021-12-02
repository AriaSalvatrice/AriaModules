/*  Copyright (C) 2019-2020 Aria Salvatrice
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/
#include "plugin.hpp"

Plugin* pluginInstance;

void init(Plugin* p) {
    pluginInstance = p;

    // Split & Merge series
    p->addModel(modelSplort);
    p->addModel(modelSmerge);
    p->addModel(modelSpleet);
    p->addModel(modelSwerge);
    p->addModel(modelSplirge);
    // p->addModel(modelSrot);

    // Quantizers
    p->addModel(modelQqqq);
    p->addModel(modelQuack);
    p->addModel(modelQ);
    p->addModel(modelQuale);

    // Sequencers
    p->addModel(modelDarius);
    p->addModel(modelSolomon4);
    p->addModel(modelSolomon8);
    p->addModel(modelSolomon16);
    
    // Arcane
    p->addModel(modelArcane);
    p->addModel(modelAtout);
    p->addModel(modelAleister);

    // Psychopump
    p->addModel(modelPsychopump);

    // Remote Controllers
    p->addModel(modelPokies4);
    p->addModel(modelGrabby);
    p->addModel(modelRotatoes4);

    // Live performance
    p->addModel(modelUndular);

    // Blank plate
    p->addModel(modelBlank);

    // p->addModel(modelBendlet);
    // p->addModel(modelTest);

}

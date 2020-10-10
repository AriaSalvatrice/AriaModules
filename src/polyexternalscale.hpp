/*             DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
                    Version 2, December 2004

 Copyright (C) 2004 Sam Hocevar <sam@hocevar.net>

 Everyone is permitted to copy and distribute verbatim or modified
 copies of this license document, and changing it is allowed as long
 as the name is changed.

            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION

  0. You just DO WHAT THE FUCK YOU WANT TO.
*/

// For now, this only contains the Struct I use to share across expanders.

namespace PolyExternalScale {

struct PESExpanderMessage {
    std::array<bool, 12> scale;
    bool hasRootNote = false;
    // Only if hasRootNote
    size_t rootNote = 0;
};

} // namespace PolyExternalScale

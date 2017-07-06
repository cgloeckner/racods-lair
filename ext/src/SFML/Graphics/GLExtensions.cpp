////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2015 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics/GLExtensions.hpp>
#include <SFML/System/Err.hpp>


namespace sf
{
namespace priv
{
////////////////////////////////////////////////////////////
void ensureExtensionsInit()
{
#if !defined(SFML_OPENGL_ES)
    static bool initialized = false;
    if (!initialized)
    {
        sfogl_LoadFunctions();

        if (!sfogl_IsVersionGEQ(1, 1))
        {
            err() << "sfml-graphics requires support for OpenGL 1.1 or greater" << std::endl;
            err() << "Ensure that hardware acceleration is enabled if available" << std::endl;
        }

        initialized = true;
    }
#endif
}

} // namespace priv

} // namespace sf

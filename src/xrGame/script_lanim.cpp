////////////////////////////////////////////////////////////////////////////
//	Module 		: script_ini_file.cpp
//	Created 	: 21.05.2004
//  Modified 	: 21.05.2004
//	Author		: Dmitriy Iassenev
//	Description : Script ini file class
////////////////////////////////////////////////////////////////////////////

#include "pch_script.h"
#include "xrEngine/LightAnimLibrary.h"
#include "xrScriptEngine/ScriptExporter.hpp"

struct lanim_wrapper
{
    CLAItem* item;

public:
    lanim_wrapper(pcstr name)
    {
        load(name);
    }

    void load(pcstr name)
    {
        item = LALib.FindItem(name);
        R_ASSERT3(item, "Can't find color anim:", name);
    }

    u32 length()
    {
        VERIFY(item);
        return item->Length_ms();
    }

    Fcolor calculate(float T)
    {
        int frame;
        VERIFY(item);
        return Fcolor(item->CalculateRGB(T, frame));
    }
};

SCRIPT_EXPORT(lanim_wrapper, (),
{
    using namespace luabind;

    module(luaState)
    [
        class_<lanim_wrapper>("color_animator")
            .def(constructor<pcstr>())
            .def("load", &lanim_wrapper::load)
            .def("calculate", &lanim_wrapper::calculate)
            .def("length", &lanim_wrapper::length)
    ];
});

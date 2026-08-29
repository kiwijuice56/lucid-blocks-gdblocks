
#pragma once


#include "macros.h"
#include "cellular_structure.h"

namespace godot {

class SkyThing : public CellularStructure {
	DECLARE_CLASS(SkyThing, CellularStructure);

    int update_rule(int x, int y, int z, int type) override;
};

}
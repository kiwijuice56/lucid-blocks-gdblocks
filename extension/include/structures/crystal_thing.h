
#pragma once


#include "macros.h"
#include "cellular_structure.h"

namespace godot {

class CrystalThing : public CellularStructure {
	DECLARE_CLASS(CrystalThing, CellularStructure);

    int update_rule(int x, int y, int z, int type) override;
};

}
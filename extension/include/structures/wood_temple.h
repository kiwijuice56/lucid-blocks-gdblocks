
#pragma once


#include "macros.h"
#include "cellular_structure.h"

namespace godot {

class WoodTemple : public CellularStructure {
	DECLARE_CLASS(WoodTemple, CellularStructure);

    int update_rule(int x, int y, int z, int type) override;
};

}
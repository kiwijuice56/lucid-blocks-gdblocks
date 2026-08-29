
#pragma once


#include "macros.h"
#include "cellular_structure.h"

namespace godot {

class ConcreteJungle : public CellularStructure {
	DECLARE_CLASS(ConcreteJungle, CellularStructure);

    int update_rule(int x, int y, int z, int type) override;
};

}
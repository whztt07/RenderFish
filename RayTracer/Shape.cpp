#include "Shape.hpp"
#include "DifferentialGeometry.hpp"

uint32_t Shape::next_shape_id = 1;

void Shape::get_shading_geometry(const Transform &obj2world, const DifferentialGeometry &dg, DifferentialGeometry *dg_shading) const
{
	*dg_shading = dg;
}

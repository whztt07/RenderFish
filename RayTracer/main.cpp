#include "RenderFish.hpp"
#include "World.hpp"
using namespace std;

const int width = 800;
const int height = 600;

int main()
{
	log_system_init();

	World w;
	w.build(width, height);
	w.render_scene();

	return 0;
}

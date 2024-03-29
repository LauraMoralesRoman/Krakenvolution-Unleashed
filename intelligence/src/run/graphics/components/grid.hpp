#pragma once

#include "../context.hpp"

namespace run::graphics::components {
	class Grid {
		public:
			Grid(Context& ctx);
			void update(Context& ctx);
			void render(Context& ctx);

		private:
			sf::Shader grid_shader;
			sf::RectangleShape backdrop;
	};
}

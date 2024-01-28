#pragma once

#include "config/err.hpp"

#include <string>
#include <expected>
#include <yaml/Yaml.hpp>

namespace core::config {
	using LoadResult = std::expected<Yaml::Node, LoadErr>;
	using Config = Yaml::Node;

	const LoadResult load_config(const std::string& path) noexcept;
}
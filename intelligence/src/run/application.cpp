#include "flag/flag.hpp"
#include "run.hpp"
#include "application.hpp"

#include <serial_topics.hpp>
#include <nodes/nodes.hpp>

#include <easylogging/easylogging++.h>
#include <functional>

//
// This has to be this way because the signal function only takes a void(*)() function pointer
std::function<void(int signum)> sigint_func; // <-- Real callback that will be called upon SIGINT
auto sigint_handler(int signum) -> void {
	sigint_func(signum);
}

void run::run(int argc, char **argv) {
	run::config_easylogging(argc, argv);

	// Main thread control variable
	std::atomic<bool> running = true;

	core::topics::GlobalContext global_context(running);

	// auto nodes = core::nodes::create_all_nodes();
	auto& nodes = core::nodes::static_pool();

	auto options = run::parse_cli(argc, argv);
	if (options == nullptr) {
		LOG(ERROR) << "󰆍 Failed to process CLI args";
		exit(1);
	};

	// ///////////////////////////////// //
	// Interpreting command line options //
	// ///////////////////////////////// //
	if (!options->quiet) {
		auto flag = run::flag::flag();
		std::cout << flag << std::endl << std::endl;
	}

	LOG(INFO) << "🐈 Welcome to \x1b[1\x1b[32mKRAKENVOLUTION\x1b[0m!";

	global_context.topics.serialized.merge(user::serial::mapping(global_context));
	LOG(INFO) << "󰑓 Loaded serde topics: ";
	for (const auto& [key, value] : global_context.topics.serialized) {
		LOG(INFO) << "󰛳 " << key;
	}

	std::optional<std::thread> gui_thread = std::nullopt;
	if (options->graphics) {
		gui_thread = run::init_graphics_thread(global_context);
		if (not gui_thread.has_value()) { LOG(WARNING) << "❌Failed to init GUI, no SFML support in this binary"; }
	}

	core::config::Config config = options->config_path.has_value()
		? run::config_from_path(options->config_path.value())
		: core::config::Config{};
	if (not options->config_path.has_value()) { LOG(WARNING) << " No configuration specifided"; }

	// ////////////////////// //
	// Initialize application //
	// ////////////////////// //
		// Configure SIGINT function
	sigint_func = [&](int signum) {
		LOG(INFO) << " Received closing signal from user <Ctrl-C>";
		running = false;
		running.notify_all();
	};
	std::signal(SIGINT, sigint_handler);

	run::init_nodes(nodes, global_context, config);

	running.wait(true); // Wait here until the program stops running

	run::teardown_nodes(nodes, global_context, config);

	LOG(INFO) << "󰔟 Stopping threads";
	if (options->graphics && gui_thread.has_value()) {
		gui_thread->join();
		LOG(INFO) << "GUI stopped";
	}
}

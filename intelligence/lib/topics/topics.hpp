#pragma once

#include <atomic>
#include <rxcpp/rx.hpp>
#include <unordered_map>

// Messages
#include "rxcpp/rx-observable.hpp"
#include "rxcpp/subjects/rx-subject.hpp"
// Protobuf messages

namespace core::topics {
	/*=============
	 System topics
	=============*/
	struct Topics {
		Topics();
		// rxcpp::subjects::subject<messages::Detection> detections;
		rxcpp::subjects::subject<int> test;
		std::unordered_map<std::string, rxcpp::observable<std::string>> serialized;
	};

	struct GlobalContext {
		Topics topics;
		const std::atomic<bool>& running;
		rxcpp::subjects::subject<bool> stop_subject;
		rxcpp::observable<bool> stop_signal;

		GlobalContext(const std::atomic<bool>* running) 
			: running(*running), stop_signal(stop_subject.get_observable()) {
			// Start a separate thread for the running.wait logic
			std::thread([this]() {
				this->running.wait(true);
				stop_subject.get_subscriber().on_next(static_cast<bool>(this->running));
				stop_subject.get_subscriber().on_completed();
			}).detach(); // Detach the thread to run independently
		}
	};
// Define custom nodes
#define INIT_NODE(name, context_class) \
	void setup(::core::topics::GlobalContext& global, context_class& ctx, const ::core::config::Config& cfg); \
	void end(::core::topics::GlobalContext& global, context_class& ctx, const ::core::config::Config& cfg); \
	::core::nodes::ApplicationNode create_node();
}
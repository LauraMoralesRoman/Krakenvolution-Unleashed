#include "remote.hpp"
#include "easylogging/easylogging++.h"
#include "nodes/node.hpp"
#include "channel_linking.hpp"

#include "messages/remote.pb.h"

#include "sources/tcp/tcp.hpp"

auto core::extensions::remote::create_node() -> core::nodes::ApplicationNode {
	return nodes::ApplicationNode {
		.node = nodes::create_node<Context>(setup, end),
		.name = "remote"
	};
}

auto core::extensions::remote::setup(::core::topics::GlobalContext &global, Context &ctx, const ::core::config::Config &cfg) -> void {
	LOG(INFO) << " Initializing remote channels";

	ctx.source_shared_ctx.conn_manager.on_new_channel([&global, &ctx](const core::serial::Channel& channel) {
		std::string channel_name = ctx.source_shared_ctx.conn_manager.find_channel_name(channel).value_or("Unnamed channel");
		LOG(DEBUG) << "󰟥 Created channel [" << channel_name << ']';
		channel.rx
			.take_until(global.stop_signal)
			.subscribe([channel_name, &global, &channel, &ctx](const std::string& data) {
				Subscribe subscription;
				Publish publish;
				if (subscription.ParseFromString(data)) {
					auto result = create_subscription(subscription, global.topics.serialized, channel);
					if (result.has_value()) {
						LOG(INFO) << "󰄐 Subscription from " << channel_name << " to " << subscription.topic();
						ctx.channel_subscriptions[channel.UID].push_back(result.value());
						LOG(INFO) << "Channel " << channel_name << " has now " << ctx.channel_subscriptions[channel.UID].size() << " subscriptions";
					} else {
						LOG(WARNING) << " Failed subscription from " << channel_name << " to " << subscription.topic();
					}
				} else if (publish.ParseFromString(data)) {
					if (manage_publish(publish, global.topics.serialized)) {
						LOG(INFO) << "󰍩 Publish from " << channel_name << " to " << publish.topic();
					} else {
						LOG(WARNING) << " Failed to publish from " << channel_name << " to " << publish.topic();
					}
				} else {
					LOG(DEBUG) << "󰍩 Untyped message from " << channel_name << ": " << data;
					// TODO: add topic for these kind of messages
				}
			});
	});

	ctx.source_shared_ctx.conn_manager.on_removed_channel([&ctx](const core::serial::Channel& channel) {
		std::string channel_name = ctx.source_shared_ctx.conn_manager.find_channel_name(channel).value_or("Unnamed channel");

		auto subscriptions_iter = ctx.channel_subscriptions.find(channel.UID);
		if (subscriptions_iter != ctx.channel_subscriptions.end()) {
			const auto& [key, subscriptions] = *subscriptions_iter;

			LOG(DEBUG) << "Removing " << subscriptions.size() << " subscriptions from " << channel_name;
			for (const auto& sub : subscriptions) {
				sub.unsubscribe();
			}
		}

		LOG(DEBUG) << "󰟦 Removed channel [" << channel_name << ']';
	});

	// TCP
	if (cfg["tcp"].IsDefined()) {
		ctx.tcp_server_thread = sources::tcp::init_server(global, ctx.tcp_ctx, ctx.source_shared_ctx, cfg["tcp"]);
	}
	if (cfg["uart"].IsDefined()) {
		
	}
}

auto core::extensions::remote::end(::core::topics::GlobalContext &global, Context &ctx, const ::core::config::Config &cfg) -> void {
	if (ctx.tcp_server_thread.has_value()) ctx.tcp_server_thread->join();
}

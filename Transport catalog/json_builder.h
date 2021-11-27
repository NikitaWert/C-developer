#include "json.h"

#include <vector>
#include <string>
#include <optional>

namespace json {

	class KeyItemContext;
	class ValueItemContext;
	class ArrayItemContext;

	class Builder {
	public:

		KeyItemContext Key(std::string&& key);
		Builder& Value(json::Node::Value value);
		ValueItemContext StartDict();
		ArrayItemContext StartArray();
		Builder& EndDict();
		Builder& EndArray();
		Node& Build();

		virtual ~Builder() {}

	private:

		Node root_ = nullptr;
		std::vector<Node*> nodes_stack_;
		std::optional<std::string> keys_;

		void AddItem(Node::Value value);
		void AddRoot(Node::Value value);
	};

	class KeyItemContext {
	public:

		explicit KeyItemContext(Builder& b) : build_(b) {}

		ValueItemContext Value(json::Node::Value value);

		virtual ValueItemContext StartDict();

		virtual ArrayItemContext StartArray();

		virtual ~KeyItemContext() = default;

	private:
		Builder& build_;
	};

	class ValueItemContext {
	public:
		explicit ValueItemContext(Builder& b) : build_(b) {}

		KeyItemContext Key(std::string&& key);

		Builder& EndDict();

	private:
		Builder& build_;
	};

	class ArrayItemContext : public KeyItemContext {
	public:

		explicit ArrayItemContext(Builder& b) : KeyItemContext(b), build_(b) {}

		ArrayItemContext& Value(json::Node::Value value);

		Builder& EndArray();

	private:
		Builder& build_;
	};
}


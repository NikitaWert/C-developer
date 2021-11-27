#include "json_builder.h"

#include <stdexcept>
using namespace json;

KeyItemContext Builder::Key(std::string&& key) {
	if (nodes_stack_.empty() || !nodes_stack_.back()->IsDict() || keys_.has_value()) {
		throw std::logic_error("");
	}

	keys_.emplace(key);
	return KeyItemContext(*this);
}

Builder& Builder::Value(json::Node::Value value) {

	if (root_ == nullptr) {
		root_.GetValue() = std::move(value);
		return *this;
	}

	if(nodes_stack_.empty()) throw std::logic_error("");

	if (nodes_stack_.back()->IsArray()) {
		nodes_stack_.back()->AsArray().emplace_back(value);
	}
	else if (nodes_stack_.back()->IsDict() && keys_.has_value()) {
		nodes_stack_.back()->AsDict().emplace(*keys_, std::move(value));
		keys_ = std::nullopt;
	}
	else {
		throw std::logic_error("");
	}

	return *this;
}

ValueItemContext Builder::StartDict() {
	if (root_ == nullptr) {
		AddRoot(Dict());
		return ValueItemContext(*this);
	}
	AddItem(Dict());
	return ValueItemContext(*this);
}

ArrayItemContext Builder::StartArray() {
	if (root_ == nullptr) {
		AddRoot(Array());
		return ArrayItemContext(*this);
	}
	AddItem(Array());
	return ArrayItemContext(*this);
}

Builder& Builder::EndDict() {
	if (nodes_stack_.empty() || !nodes_stack_.back()->IsDict()) throw std::logic_error("");
	nodes_stack_.pop_back();
	return *this;
}

Builder& Builder::EndArray() {
	if (nodes_stack_.empty() || !nodes_stack_.back()->IsArray()) throw std::logic_error("");
	nodes_stack_.pop_back();
	return *this;
}

Node& Builder::Build() {
	if (root_ == nullptr || !nodes_stack_.empty()) {
		throw std::logic_error("");
	}
	return root_;
}

void Builder::AddItem(Node::Value value) {


	if (!nodes_stack_.empty()) {
		if (nodes_stack_.back()->IsArray()) {
			nodes_stack_.back()->AsArray().emplace_back(value);
			nodes_stack_.push_back(&(nodes_stack_.back()->AsArray().back()));
			return;
		}
		else if (nodes_stack_.back()->IsDict() && keys_.has_value()) {
			nodes_stack_.back()->AsDict().emplace(*keys_, value);
			nodes_stack_.push_back(&nodes_stack_.back()->AsDict().at(*keys_));
			keys_ = std::nullopt;
			return;
		}
	}

	throw std::logic_error("");
}

void Builder::AddRoot(Node::Value value) {
	root_.GetValue() = std::move(value);
    nodes_stack_.push_back(&root_);
}

ValueItemContext KeyItemContext::Value(json::Node::Value value) {
	build_.Value(move(value));
	return ValueItemContext(build_);
}

ValueItemContext KeyItemContext::StartDict() {
	return build_.StartDict();
}

ArrayItemContext KeyItemContext::StartArray() {
	return build_.StartArray();
}

KeyItemContext ValueItemContext::Key(std::string&& key) {
	return build_.Key(std::move(key));
}

Builder& ValueItemContext::EndDict() {
	return build_.EndDict();
}

Builder& ArrayItemContext::EndArray() {
	return build_.EndArray();
}

ArrayItemContext& ArrayItemContext::Value(json::Node::Value value) {
	build_.Value(move(value));
	return *this;
}
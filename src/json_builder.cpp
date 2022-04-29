#include "json_builder.h"

using namespace std::literals;

namespace json {
	DictKeyContext Builder::Key(std::string key) {
		if (node_stack_.back()->IsDict() && !key_) {
			key_ = std::move(key);
		}
		else {
			throw std::logic_error("The command \"Key\" was called in wrong place"s);
		}
		return *this;
	}

	Builder& Builder::Value(Node::Value value) {
		AddElement(value);
		return *this;
	}

	DictItemContext Builder::StartDict() {
		if (IsEmpty() || key_ || PrevIsArray()) {
			node_stack_.emplace_back(std::move(AddElement(Dict())));
			return DictItemContext(*this);
		}
		throw std::logic_error("The command \"StartDict\" was called in wrong place"s);
	}

	ArrayItemContext Builder::StartArray() {
		if (IsEmpty() || key_ || PrevIsArray()) {
			node_stack_.emplace_back(std::move(AddElement(Array())));
			return *this;
		}
		else {
			throw std::logic_error("The command \"StartArray\" was called in wrong place"s);
		}
	}

	Builder& Builder::EndDict() {
		if (PrevIsDict()) {
			node_stack_.pop_back();
		}
		else {
			throw std::logic_error("The command \"EndDict\" was called in wrong place"s);
		}
		return *this;
	}

	Builder& Builder::EndArray() {
		if (PrevIsArray()) {
			node_stack_.pop_back();
		}
		else {
			throw std::logic_error("The command \"EndArray\" was called in wrong place"s);
		}
		return *this;
	}

	json::Node Builder::Build() {
		if (node_stack_.size() == 1 && !node_stack_.back()->IsNull()) {
			return root_;
		}
		else {
			throw std::logic_error("The command \"Build\" was called in wrong place"s);
		}
	}

	bool Builder::IsEmpty() const {
		return node_stack_.back()->IsNull();
	}

	bool Builder::PrevIsArray() const {
		return (node_stack_.back()->IsArray());
	}

	bool Builder::PrevIsDict() const {
		return (node_stack_.back()->IsDict());
	}

	Node* Builder::AddElement(Node::Value value) {
		if (IsEmpty()) {
			node_stack_.back()->GetValue() = std::move(value);
			return node_stack_.back();
		}
		else if (PrevIsArray()) {
			Array& link_arr = std::get<Array>(node_stack_.back()->GetValue());
			return &link_arr.emplace_back(value);
		}
		else if (PrevIsDict() && key_) {
			Dict& link_dict = std::get<Dict>(node_stack_.back()->GetValue());
			auto ptr = link_dict.emplace(key_.value(), value);
			key_ = std::nullopt;
			return &ptr.first->second;
		}
		else {
			throw std::logic_error("The command \"Value\" was called in wrong place"s);
		}
	}

	DictKeyContext DictItemContext::Key(std::string key) {
		return builder_.Key(key);
	}
	Builder& DictItemContext::EndDict() {
		return builder_.EndDict();
	}
	DictItemContext DictKeyContext::Value(Node::Value value) {
		return DictItemContext(builder_.Value(value));
	}
	DictItemContext DictKeyContext::StartDict() {
		return builder_.StartDict();
	}
	ArrayItemContext DictKeyContext::StartArray() {
		return builder_.StartArray();
	}
	ArrayItemContext ArrayItemContext::Value(Node::Value value) {
		return ArrayItemContext(builder_.Value(value));
	}
	DictItemContext ArrayItemContext::StartDict() {
		return builder_.StartDict();
	}
	ArrayItemContext ArrayItemContext::StartArray() {
		return builder_.StartArray();
	}
	Builder& ArrayItemContext::EndArray() {
		return builder_.EndArray();
	}
}
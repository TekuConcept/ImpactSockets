/**
 * Created by TekuConcept on June 19, 2018
 */

#include "SocketPollTable.h"

using namespace Impact;


SocketPollTable::SocketPollTable() {}


SocketPollTable::SocketPollTable(PollInitializer handles) {
	for(auto& handle : handles) {
		push_back(handle);
	}
}


SocketPollTable::~SocketPollTable() {}


void SocketPollTable::resetEvents() {
	for(auto& descriptor : _descriptors_) {
		descriptor.revents = 0;
	}
}


bool SocketPollTable::empty() const {
	return _descriptors_.empty();
}


unsigned int SocketPollTable::size() const {
	return _descriptors_.size();
}


void SocketPollTable::reserve(unsigned int capacity) {
	_descriptors_.reserve(capacity);
}


void SocketPollTable::shrink_to_fit() {
	_descriptors_.shrink_to_fit();
}


void SocketPollTable::clear() {
	_descriptors_.clear();
}


void SocketPollTable::push_back(HandleEventPair pair) {
	struct pollfd sfd;
	sfd.fd = pair.first.descriptor;
	sfd.events = (short)pair.second;
	sfd.revents = 0;
	_descriptors_.push_back(sfd);
}


void SocketPollTable::erase(int position) {
	_descriptors_.erase(_descriptors_.begin() + position);
}


void SocketPollTable::pop_back() {
	_descriptors_.pop_back();
}


short& SocketPollTable::at(int idx) {
	return _descriptors_[idx].revents;
}


short& SocketPollTable::operator[] (int idx) {
	return _descriptors_[idx].revents;
}


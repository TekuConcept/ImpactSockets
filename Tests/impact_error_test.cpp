/**
 * Created by TekuConcept on July 16, 2018
 */

#include <iostream>
#include <cassert>
#include <sockets/impact_error.h>

// #define VERBOSE(x) std::cout << x << std::endl
#define VERBOSE(x)
#define TEST(x) std::cout << x << std::flush;

using namespace impact;

class StackObj {
public:
	StackObj() {}
	void helper_stack_C() { throw impact_error("Internal Message"); }
	void helper_stack_B() { try { helper_stack_C(); } catch (...) { throw; } }
	void helper_stack_A() { try { helper_stack_B(); } catch (...) { throw; } }
};


void test_message() {
	VERBOSE("\nTest Error Message");
	try {
		throw impact_error("Hello World");
		assert(false);
	}
	catch (impact_error e) {
		VERBOSE("Message: " << e.message());
		assert(true);
	}
	VERBOSE("Done!");
}


void test_trace() {
	VERBOSE("\nTest Stack Trace");
	try {
		StackObj obj;
		obj.helper_stack_A();
		assert(false);
	}
	catch (impact_error e) {
		VERBOSE(e.trace());
		assert(true);
	}
	VERBOSE("Done!");
}


void test_what() {
	VERBOSE("\nTest what() Message");
	try {
		StackObj obj;
		obj.helper_stack_A();
		assert(false);
	}
	catch (impact_error e) {
		VERBOSE(e.what());
		assert(true);
	}
	VERBOSE("Done!");
}


int main() {
	VERBOSE("- BEGIN NETWORKING TEST -");

	test_message();
	test_trace();
	test_what();
	TEST('.');

	VERBOSE("- END OF LINE -");
	return 0;
}

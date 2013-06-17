/*
Copyright (C) 2013 MoSync AB

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License,
version 2, as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
MA 02110-1301, USA.
*/

/*
* @file Test.h
* @brief Testing framework
*/

#include <conprint.h>
#include <matime.h>
#include <mavsprintf.h>
#include "Test.h"

namespace MATest
{
	using namespace MAUtil;

	/* ========== Class TestListener ========== */

	TestListener::TestListener() {}
	TestListener::~TestListener() {}
	void TestListener::beginTestSuite(const String& suiteName) {}
	void TestListener::endTestSuite() {}
	void TestListener::beginTestCase(const String& testCaseName) {}
	void TestListener::endTestCase() {}
	void TestListener::assertion(const String& assertionName, bool cond) {}
	void TestListener::expectation(const String& assertionName) {}

	/* ========== Class TestCase ========== */

	TestCase::TestCase(const String& name) :
		name(name)
	{
	}

	TestCase::~TestCase()
	{
	}

	void TestCase::open()
	{
	}

	void TestCase::close()
	{
	}

	bool TestCase::assert(const String& assertionName, bool cond)
	{
		suite->fireAssertion(assertionName, cond);
		return cond;
	}

	void TestCase::expect(const String& assertionName)
	{
		suite->fireExpectation(assertionName);
	}

	const String& TestCase::getName() const
	{
		return name;
	}

	void TestCase::setSuite(TestSuite *suite)
	{
		this->suite = suite;
	}

	TestSuite* TestCase::getSuite()
	{
		return this->suite;
	}

	void TestCase::runNextTestCase()
	{
		getSuite()->runNextCase();
	}

	/* ========== Class TestSuite ========== */

	TestSuite::TestSuite(const String& name) :
		mName(name),
		mCurrentTestCase(0),
		mRunNextCaseCalled(false),
		mIsRunningAsync(true) // Must be true to start tests.
	{
	}

	TestSuite::~TestSuite()
	{
	}

	void TestSuite::addTestCase(TestCase* testCase)
	{
		mTestCases.add(testCase);
		testCase->setSuite(this);
	}

// TODO: Why is this commented out?
#if 0
	void TestSuite::runTestCases() {
		for(int j = 0; j < mTestListeners.size(); j++) {
			mTestListeners[j]->beginTestSuite(mName);
		}
		for(int i = 0; i < mTestCases.size(); i++) {
			for(int j = 0; j < mTestListeners.size(); j++) {
				mTestListeners[j]->beginTestCase(mTestCases[i]->getName());
			}
			mTestCases[i]->open();
			mTestCases[i]->run();
			mTestCases[i]->close();
			for(int j = 0; j < mTestListeners.size(); j++) {
				mTestListeners[j]->endTestCase();
			}
		}
		for(int j = 0; j < mTestListeners.size(); j++) {
			mTestListeners[j]->endTestSuite();
		}
	}
#endif

	// Note: This method is called from TestCase::runNextTestCase().
	void TestSuite::runNextCase()
	{
		// This method has just been called.
		mRunNextCaseCalled = true;

		if (!mIsRunningAsync)
		{
			// If the current test case is not running
			// asynchronously, we just return. The caller
			// will proceed running in its while loop.
			return;
		}
		else
		{
			// If running asynchronously, this makes the
			// end of the test case, reset the flag and
			// continue into the while loop below.
			mIsRunningAsync = false;
		}

		// There must be test cases to run.
		if (mTestCases.size() > 0)
		{
			while (mCurrentTestCase < mTestCases.size())
			{
				// Is this the first test case?
				if (0 == mCurrentTestCase)
				{
					// Signal beginning of the test suite.
					fireBeginTestSuite(mName);
				}

				// If this is not the first test case then
				// signal the end of the previous test case.
				if (mCurrentTestCase > 0)
				{
					fireEndTestCase();
				}

				// Run current test case.
				TestCase* testCase = mTestCases[mCurrentTestCase];
				fireBeginTestCase(testCase->getName());
				testCase->open();

				// Set flags and start test case.
				mRunNextCaseCalled = false;
				mIsRunningAsync = false;
				testCase->start();
				if (!mRunNextCaseCalled)
				{
					// The start method did not call runNextTestCase,
					// this test now runs async. The while loop will
					// be entered next time runNextTestCase is called.
					mIsRunningAsync = true;
					return;
				}

				// Note: We do not call fireEndTestCase() here,
				// because it should not be called until the
				// current test case has called runNextTestCase().

				// Move to next test case.
				mCurrentTestCase ++;
			}
		}

		// If last test case has been run, reset the
		// test case index and end the suite.
		if (mCurrentTestCase >= mTestCases.size())
		{
			mCurrentTestCase = 0;
			mIsRunningAsync = true;
			fireEndTestSuite();
		}
	}

	const String& TestSuite::getName() const
	{
		return mName;
	}

	void TestSuite::addTestListener(TestListener* testListener)
	{
		mTestListeners.add(testListener);
	}

	void TestSuite::fireBeginTestSuite(const String& suiteName)
	{
		for (int j = 0; j < mTestListeners.size(); j++)
		{
			mTestListeners[j]->beginTestSuite(suiteName);
		}
	}

	void TestSuite::fireEndTestSuite()
	{
		for (int j = 0; j < mTestListeners.size(); j++)
		{
			mTestListeners[j]->endTestSuite();
		}
	}

	void TestSuite::fireBeginTestCase(const String& testCaseName)
	{
		for (int j = 0; j < mTestListeners.size(); j++)
		{
			mTestListeners[j]->beginTestCase(testCaseName);
		}
	}

	void TestSuite::fireEndTestCase()
	{
		for (int j = 0; j < mTestListeners.size(); j++)
		{
			mTestListeners[j]->endTestCase();
		}
	}

	void TestSuite::fireAssertion(const String& assertionName, bool cond)
	{
		for (int j = 0; j < mTestListeners.size(); j++)
		{
			mTestListeners[j]->assertion(assertionName, cond);
		}
	}

	void TestSuite::fireExpectation(const String& assertionName)
	{
		for (int j = 0; j < mTestListeners.size(); j++)
		{
			mTestListeners[j]->expectation(assertionName);
		}
	}
}
// namespace

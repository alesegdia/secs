#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace secs::tests {

	class BaseEngineTests : public testing::Test
	{
	protected:

	};

	TEST_F(BaseEngineTests, BaseTests_Test1)
	{
		EXPECT_THAT(true, testing::IsTrue());
	}

}


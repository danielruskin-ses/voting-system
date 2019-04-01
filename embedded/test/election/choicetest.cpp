/*
 * choicetest.cpp
 *
 *  Created on: Mar 31, 2019
 *      Author: jonathandavis
 */

#include <gtest/gtest.h>

#include "../../src/election/choice.h"

TEST(Choice, EqualValue)
{
	Choice choice("Cave Johnson");

	ASSERT_EQ(choice.getChoice(), "Cave Johnson");
}

TEST(Choice, UnequalValue)
{
	Choice choice("Cave Johnson");

	ASSERT_NE(choice.getChoice(), "cave johnson");
}

TEST(Choice, EqualOp)
{
	Choice choice1("apples");
	Choice choice2("apples");

	ASSERT_TRUE(choice1 == choice2);
}

TEST(Choice, EqualOpFalse)
{
	Choice choice1("apples");
	Choice choice2("oranges");

	ASSERT_FALSE(choice1 == choice2);
}

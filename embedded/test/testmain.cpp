/*
 * testmain.cpp
 *
 *  Created on: Mar 31, 2019
 *      Author: jonathandavis
 */

#include <gtest/gtest.h>

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

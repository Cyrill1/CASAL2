/**
 * @file AllValues.Test.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @version 1.0
 * @date 18/01/2013
 * @section LICENSE
 *
 * Copyright NIWA Science �2013 - www.niwa.co.nz
 *
 * $Date: 2008-03-04 16:33:32 +1300 (Tue, 04 Mar 2008) $
 */
#ifdef TESTMODE

// Headers
#include "AllValues.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <boost/lexical_cast.hpp>

#include "TestResources/MockClasses/Model.h"

// Namespaces
namespace niwa {

using ::testing::Return;

/**
 * Test the results of our KnifeEdge are correct
 */
TEST(Selectivities, AllValues) {
  MockModel model;
  EXPECT_CALL(model, min_age()).WillRepeatedly(Return(10));
  EXPECT_CALL(model, max_age()).WillRepeatedly(Return(20));
  EXPECT_CALL(model, age_spread()).WillRepeatedly(Return(11));

  niwa::selectivities::AllValues all_values(&model);

  vector<string> v;
  for (unsigned i = 0; i < 11; ++i)
    v.push_back(boost::lexical_cast<string>(i + 1));

  all_values.parameters().Add(PARAM_LABEL, "unit_test_all_values", __FILE__, __LINE__);
  all_values.parameters().Add(PARAM_V, v, __FILE__, __LINE__);
  all_values.parameters().Add(PARAM_TYPE, "not needed in test", __FILE__, __LINE__);
  all_values.Validate();
  all_values.Build();

  EXPECT_DOUBLE_EQ(0.0,  all_values.GetResult(9, nullptr)); // Below model->min_age()
  EXPECT_DOUBLE_EQ(1.0,  all_values.GetResult(10, nullptr)); // At model->min_age()
  EXPECT_DOUBLE_EQ(2.0,  all_values.GetResult(11, nullptr));
  EXPECT_DOUBLE_EQ(3.0,  all_values.GetResult(12, nullptr));
  EXPECT_DOUBLE_EQ(4.0,  all_values.GetResult(13, nullptr));
  EXPECT_DOUBLE_EQ(5.0, all_values.GetResult(14, nullptr));
  EXPECT_DOUBLE_EQ(6.0, all_values.GetResult(15, nullptr));
  EXPECT_DOUBLE_EQ(7.0, all_values.GetResult(16, nullptr));
  EXPECT_DOUBLE_EQ(8.0, all_values.GetResult(17, nullptr));
  EXPECT_DOUBLE_EQ(9.0, all_values.GetResult(18, nullptr));
  EXPECT_DOUBLE_EQ(10.0, all_values.GetResult(19, nullptr));
  EXPECT_DOUBLE_EQ(11.0,  all_values.GetResult(20, nullptr)); // At model->max_age()
  EXPECT_DOUBLE_EQ(0.0,  all_values.GetResult(21, nullptr)); // This is above model->max_age()
  EXPECT_DOUBLE_EQ(0.0,  all_values.GetResult(22, nullptr));
  EXPECT_DOUBLE_EQ(0.0,  all_values.GetResult(23, nullptr));
}

}

#endif /* ifdef TESTMODE */

/**
 * @file InverseLogistic.Test.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @version 1.0
 * @date 22/01/2013
 * @section LICENSE
 *
 * Copyright NIWA Science �2013 - www.niwa.co.nz
 *
 * $Date: 2008-03-04 16:33:32 +1300 (Tue, 04 Mar 2008) $
 */
#ifdef TESTMODE

// Headers
#include "Logistic.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <boost/lexical_cast.hpp>

#include "TestResources/MockClasses/Model.h"
#include "TimeSteps/Manager.h"
#include "AgeLengths/AgeLength.h"
#include "AgeLengths/Manager.h"

// Namespaces
namespace niwa {

using ::testing::Return;
/**
 * Class Definition
 */
class MockAgeLength : public niwa::AgeLength {
public:
  MOCK_CONST_METHOD0(distribution, string());
  MOCK_METHOD2(mean_length, Double(unsigned year, unsigned age));
  MOCK_METHOD2(mean_weight, Double(unsigned year, unsigned age));
  MOCK_METHOD3(cv, Double(unsigned year, unsigned age, unsigned time_step));
};



class MockTimeStepManager : public timesteps::Manager {
public:
  unsigned time_step_index_ = 0;
  unsigned current_time_step() const override final { return time_step_index_; }
};

/**
 * Test the results of our selectivity are correct
 */
TEST(Selectivities, Logistic) {
  MockModel model;
  EXPECT_CALL(model, min_age()).WillRepeatedly(Return(10));
  EXPECT_CALL(model, max_age()).WillRepeatedly(Return(20));

  niwa::selectivities::Logistic logistic(&model);

  logistic.parameters().Add(PARAM_LABEL, "unit_test_logistic", __FILE__, __LINE__);
  logistic.parameters().Add(PARAM_TYPE, "not needed in test", __FILE__, __LINE__);
  logistic.parameters().Add(PARAM_A50,   "2",  __FILE__, __LINE__);
  logistic.parameters().Add(PARAM_ATO95, "7",  __FILE__, __LINE__);
  logistic.Validate();
  logistic.Build();

  EXPECT_DOUBLE_EQ(0.0,                       logistic.GetResult(9, nullptr)); // Below model->min_age()
  EXPECT_DOUBLE_EQ(0.96659497164362229,       logistic.GetResult(10, nullptr)); // At model->min_age()
  EXPECT_DOUBLE_EQ(0.97781072943439207,       logistic.GetResult(11, nullptr));
  EXPECT_DOUBLE_EQ(0.98531798872758125,       logistic.GetResult(12, nullptr));
  EXPECT_DOUBLE_EQ(0.99031049840094476,       logistic.GetResult(13, nullptr));
  EXPECT_DOUBLE_EQ(0.99361634077929817,       logistic.GetResult(14, nullptr));
  EXPECT_DOUBLE_EQ(0.99579908776852011,       logistic.GetResult(15, nullptr));
  EXPECT_DOUBLE_EQ(0.99723756906077354,       logistic.GetResult(16, nullptr));
  EXPECT_DOUBLE_EQ(0.99818438198748194,       logistic.GetResult(17, nullptr));
  EXPECT_DOUBLE_EQ(0.99880706650531281,       logistic.GetResult(18, nullptr));
  EXPECT_DOUBLE_EQ(0.99921636273936254,       logistic.GetResult(19, nullptr));
  EXPECT_DOUBLE_EQ(0.99948530154281656,       logistic.GetResult(20, nullptr)); // At model->max_age()
  EXPECT_DOUBLE_EQ(0.0,                       logistic.GetResult(21, nullptr)); // This is above model->max_age()
  EXPECT_DOUBLE_EQ(0.0,                       logistic.GetResult(22, nullptr));
  EXPECT_DOUBLE_EQ(0.0,                       logistic.GetResult(23, nullptr));
}

/**
 * Test the results of our length based selectivity are correct
 */
TEST(Selectivities, Logistic_length_normal) {
/*
  MockModel model;
  MockAgeLength agelength1;
  MockTimeStepManager time_step_manager;
  time_step_manager.time_step_index_ = 0;

  EXPECT_CALL(model, min_age()).WillRepeatedly(Return(1));
  EXPECT_CALL(model, max_age()).WillRepeatedly(Return(10));
  EXPECT_CALL(model, current_year()).WillRepeatedly(Return(1999));

  niwa::selectivities::Logistic logistic(&model);
  logistic.parameters().Add(PARAM_LABEL, "unit_test_logistic", __FILE__, __LINE__);
  logistic.parameters().Add(PARAM_TYPE, "not needed in test", __FILE__, __LINE__);
  logistic.parameters().Add(PARAM_A50,   "2",  __FILE__, __LINE__);
  logistic.parameters().Add(PARAM_ATO95, "7",  __FILE__, __LINE__);
  logistic.parameters().Add(PARAM_LENGTH_BASED, "true",  __FILE__, __LINE__);
  logistic.Validate();
  logistic.Build();

  EXPECT_CALL(agelength1, distribution()).WillRepeatedly(Return("normal"));
  unsigned year,age,time_step;

  EXPECT_CALL(agelength1, cv(year,age,time_step)).WillRepeatedly(Return(0.3));
  // age 1
  EXPECT_CALL(agelength1, mean_length()).WillRepeatedly(Return(26.87972));
  EXPECT_DOUBLE_EQ(1,       logistic.GetResult(1, &agelength)); // At model->min_age()

  // age 4
  EXPECT_CALL(agelength1, mean_length()).WillOnce(Return(63.32838));
  EXPECT_DOUBLE_EQ(1,       logistic.GetResult(4, &agelength));
  // age 7
  EXPECT_CALL(agelength1, mean_length()).WillOnce(Return(78.14730));
  EXPECT_DOUBLE_EQ(1,       logistic.GetResult(7, &agelength));
  // age 9
  EXPECT_CALL(agelength1, mean_length()).WillOnce(Return(82.72808));
  EXPECT_DOUBLE_EQ(1,       logistic.GetResult(8, &agelength));
*/

}

}

#endif /* ifdef TESTMODE */

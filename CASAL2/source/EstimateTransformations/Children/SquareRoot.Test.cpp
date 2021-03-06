/**
 * @file SquareRoot.Test.cpp
 * @author Scott Rasmussen (scott.rasmussen@zaita.com)
 * @github https://github.com/Zaita
 * @date Jan 11, 2016
 * @section LICENSE
 *
 * Copyright NIWA Science �2016 - www.niwa.co.nz
 */
#ifdef TESTMODE

// headers
#include "ObjectiveFunction/ObjectiveFunction.h"
#include "Model/Model.h"
#include "TestResources/TestFixtures/InternalEmptyModel.h"
#include "TestResources/Models/TwoSex.h"
#include "TestResources/Models/TwoSexWithDLib.h"
#include "TestResources/Models/TwoSexWithDeSolver.h"

namespace niwa {
namespace estimatetransformations {
using niwa::testfixtures::InternalEmptyModel;

const string estimate_transformation_squareroot =
R"(
@estimate_transformation log_r0
type sqrt
estimate recruitment.r0
lower_bound 500
upper_bound 2500
)";

/**
 *
 */
TEST_F(InternalEmptyModel, EstimateTransformations_SquareRoot) {
  AddConfigurationLine(testresources::models::two_sex, "TestResources/Models/TwoSex.h", 28);
  AddConfigurationLine(estimate_transformation_squareroot, __FILE__, 22);
  LoadConfiguration();

  model_->Start(RunMode::kEstimation);

  ObjectiveFunction& obj_function = model_->objective_function();
  EXPECT_DOUBLE_EQ(1981.8766604103455, obj_function.score());
}

/**
 *
 */
const string estimate_transformation_squareroot_no_bounds =
R"(
@estimate_transformation log_r0
type sqrt
estimate recruitment.r0
)";

/**
 *
 */
TEST_F(InternalEmptyModel, EstimateTransformations_SquareRoot_NoBounds) {
  AddConfigurationLine(testresources::models::two_sex, "TestResources/Models/TwoSex.h", 28);
  AddConfigurationLine(estimate_transformation_squareroot_no_bounds, __FILE__, 47);
  LoadConfiguration();

  model_->Start(RunMode::kEstimation);

  ObjectiveFunction& obj_function = model_->objective_function();
  EXPECT_NEAR(1977.8957483899424, obj_function.score(), 1e-5);
}

/**
 *
 */
TEST_F(InternalEmptyModel, EstimateTransformations_SquareRoot_With_DLib_Minimiser) {
  AddConfigurationLine(testresources::models::two_sex_with_dlib, "TestResources/Models/TwoSexWithDLib.h", 28);
  AddConfigurationLine(estimate_transformation_squareroot, __FILE__, 22);
  LoadConfiguration();

  model_->Start(RunMode::kEstimation);

  ObjectiveFunction& obj_function = model_->objective_function();
  EXPECT_DOUBLE_EQ(1981.7778874187497, obj_function.score());
}

/**
 *
 */
TEST_F(InternalEmptyModel, EstimateTransformations_SquareRoot_With_DeSolver_Minimiser) {
  AddConfigurationLine(testresources::models::two_sex_with_de_solver, "TestResources/Models/TwoSexWithDeSolver.h", 28);
  AddConfigurationLine(estimate_transformation_squareroot, __FILE__, 22);
  LoadConfiguration();

  model_->Start(RunMode::kEstimation);

  ObjectiveFunction& obj_function = model_->objective_function();
  EXPECT_DOUBLE_EQ(2204.6380608863997, obj_function.score());
}

} /* namespace estimatetransformations */
} /* namespace niwa */
#endif

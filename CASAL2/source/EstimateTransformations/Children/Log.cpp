/**
 * @file Log.cpp
 * @author Scott Rasmussen (scott.rasmussen@zaita.com)
 * @github https://github.com/Zaita
 * @date Jan 8, 2016
 * @section LICENSE
 *
 * Copyright NIWA Science �2016 - www.niwa.co.nz
 *
 */

// headers
#include "Log.h"

#include "Model/Model.h"
#include "Model/Objects.h"
#include "Model/Managers.h"
#include "Estimates/Manager.h"
#include "Estimates/Estimate.h"

// namespaces
namespace niwa {
namespace estimatetransformations {

/**
 * Default constructor
 */
Log::Log(Model* model) : EstimateTransformation(model) {
  parameters_.Bind<string>(PARAM_ESTIMATE, &estimate_label_, "Estimate to transform", "");
}

/**
 */
void Log::DoValidate() {

}

/**
 *
 */
void Log::DoBuild() {
  LOG_TRACE();
  estimate_ = model_->managers().estimate()->GetEstimateByLabel(estimate_label_);
  if (estimate_ == nullptr) {
    LOG_ERROR_P(PARAM_ESTIMATE) << "Estimate " << estimate_label_ << " could not be found. Have you defined it?";
    return;
  }

  original_lower_bound_ = estimate_->lower_bound();
  original_upper_bound_ = estimate_->upper_bound();
  original_value_ = estimate_->value();
  if (!parameters_.Get(PARAM_LOWER_BOUND)->has_been_defined())
    lower_bound_ = log(original_lower_bound_);
  if (!parameters_.Get(PARAM_UPPER_BOUND)->has_been_defined())
    upper_bound_ = log(original_upper_bound_);
}

/**
 *
 */
void Log::Transform() {
  estimate_->set_lower_bound(lower_bound_);
  estimate_->set_upper_bound(upper_bound_);

  estimate_->set_value(log(estimate_->value()));
}

/**
 *
 */
void Log::Restore() {
  estimate_->set_lower_bound(original_lower_bound_);
  estimate_->set_upper_bound(original_upper_bound_);

  estimate_->set_value(exp(estimate_->value()));
}

Double Log::GetScore() {
//
//  jacobian_ = 1.0 / original_value_;
//  LOG_MEDIUM() << "jacobian: " << jacobian_;
  return 0.0;

}

/**
 * Get the target estimables so we can ensure each
 * object is not referencing multiple ones as this would
 * cause chain issues
 *
 * @return Set of estimable labels
 */
std::set<string> Log::GetTargetEstimates() {
  set<string> result;
  result.insert(estimate_label_);
  return result;
}
} /* namespace estimatetransformations */
} /* namespace niwa */

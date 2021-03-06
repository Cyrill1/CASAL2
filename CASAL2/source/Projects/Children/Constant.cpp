/**
 * @file Constant.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 28/05/2014
 * @section LICENSE
 *
 * Copyright NIWA Science �2014 - www.niwa.co.nz
 *
 */

// headers
#include "Constant.h"

// namespaces
namespace niwa {
namespace projects {

/**
 * Default constructor
 */
Constant::Constant(Model* model) : Project(model) {
  parameters_.Bind<Double>(PARAM_VALUES, &values_, "Values to assign to estimable", "");
}

/**
 * Validate
 */
void Constant::DoValidate() {
  if (values_.size() != 1 && values_.size() != years_.size()) {
    LOG_ERROR_P(PARAM_VALUES) << "length (" << values_.size() << ") must match the number of years provided (" << years_.size() << ")";
    return;
  }

  if (values_.size() == 1)
    values_.assign(years_.size(), values_[0]);
  for (unsigned i = 0; i < years_.size(); ++i) {
    year_values_[years_[i]] = values_[i];
  }
}

/**
 * Build
 */
void Constant::DoBuild() { }

/**
 * Reset
 */
void Constant::DoReset() { }

/**
 *
 */
void Constant::DoUpdate() {
  LOG_FINE() << "Setting Value to: " << year_values_[model_->current_year()];
  (this->*DoUpdateFunc_)(year_values_[model_->current_year()]);
}

} /* namespace projects */
} /* namespace niwa */

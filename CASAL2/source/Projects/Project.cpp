/**
 * @file Project.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 28/05/2014
 * @section LICENSE
 *
 * Copyright NIWA Science �2014 - www.niwa.co.nz
 *
 */

// headers
#include "Project.h"

#include "Model/Objects.h"
#include "Utilities/To.h"

// namespaces
namespace niwa {

/**
 * Default constructor
 */
Project::Project(Model* model) : model_(model) {
  parameters_.Bind<string>(PARAM_LABEL, &label_, "Label", "");
  parameters_.Bind<string>(PARAM_TYPE, &type_, "Type", "", "");
  parameters_.Bind<unsigned>(PARAM_YEARS, &years_, "Years to recalculate the values", "", true);
  parameters_.Bind<string>(PARAM_PARAMETER, &parameter_, "Parameter to project", "");

  original_value_ = 0;
}

/**
 *
 */
void Project::Validate() {
  parameters_.Populate();
  DoValidate();
}

/**
 *
 */
void Project::Build() {
  string type       = "";
  string label      = "";
  string index      = "";

  /**
   * Explode the parameter string sive o we can get the estimable
   * name (parameter) and the index
   */
  if (parameter_ == "") {
    parameters().Add(PARAM_PARAMETER, label_, parameters_.Get(PARAM_LABEL)->file_name(), parameters_.Get(PARAM_LABEL)->line_number());
    parameter_ = label_;
  }

  model_->objects().ExplodeString(parameter_, type, label, estimable_parameter_, index);
  if (type == "" || label == "" || estimable_parameter_ == "") {
    LOG_ERROR_P(PARAM_PARAMETER) << ": parameter " << parameter_
        << " is not in the correct format. Correct format is object_type[label].estimable(array index)";
  }

  string error = "";
  target_ = model_->objects().FindObject(parameter_, error);
  if (!target_)
    LOG_ERROR_P(PARAM_PARAMETER) << " " << parameter_ << " is not a valid estimable in the system. Error: " << error;


  estimable_type_ = target_->GetEstimableType(estimable_parameter_);
  switch(estimable_type_) {
    case Estimable::kInvalid:
      LOG_CODE_ERROR() << "Invalid estimable type: " << parameter_;
      break;
    case Estimable::kSingle:
      DoUpdateFunc_ = &Project::SetSingleValue;
      estimable_    = target_->GetEstimable(estimable_parameter_);
      original_value_ = *estimable_;
      break;
    case Estimable::kVector:
      DoUpdateFunc_ = &Project::SetVectorValue;
      estimable_vector_ = target_->GetEstimableVector(estimable_parameter_);
      break;
    case Estimable::kUnsignedMap:
      DoUpdateFunc_ = &Project::SetMapValue;
      estimable_map_ = target_->GetEstimableUMap(estimable_parameter_);
      break;
    default:
      LOG_ERROR() << "The estimable you have provided for use in a projection: " << parameter_ << " is not a type that is supported for projection modification";
      break;
  }
  DoBuild();
}

/**
 *
 */
void Project::Update(unsigned current_year) {
  LOG_TRACE();
  if (DoUpdateFunc_ == 0)
    LOG_CODE_ERROR() << "DoUpdateFunc_ == 0";

  if (years_.size() > 0 && std::find(years_.begin(), years_.end(), current_year) == years_.end())
    RestoreOriginalValue();
  else
    DoUpdate();
}

/**
 *
 */
void Project::RestoreOriginalValue() {
  LOG_TRACE();
  LOG_FINE() << "Setting original value to: " << original_value_;
  (this->*DoUpdateFunc_)(original_value_);
}

/**
 *
 */
void Project::SetSingleValue(Double value) {
  *estimable_ = value;
}

/**
 *
 */
void Project::SetVectorValue(Double value) {
  estimable_vector_->push_back(value);
}

/**
 *
 */
void Project::SetMapValue(Double value) {
  (*estimable_map_)[model_->current_year()] = value;
}

/**
 *  Store at projects values in a map to be referenced when executing projection method.
 */
void Project::StoreValue(unsigned current_year, unsigned start_year, unsigned final_year) {
  LOG_TRACE();
  switch (estimable_type_) {
  case Estimable::kVector:
    if (estimable_vector_->size() != (final_year - start_year + 1))
      LOG_ERROR() << "if estimate is of type kVector, there must be a value for each year from start_year to final_year";
    LOG_FINE() << " Value = " << (*estimable_vector_)[current_year - start_year];
    projected_parameters_[current_year] = (*estimable_vector_)[current_year - start_year];
    break;
  case Estimable::kSingle:
    LOG_FINE() << " Value = " << *estimable_;
    projected_parameters_[current_year] = *estimable_;
    break;

  case Estimable::kUnsignedMap:
    // Note of caution: If the parameter doesn't exist e.g catches in all years referenced a map will return a 0.0, by default
    LOG_FINE() << " Value = " << (*estimable_map_)[current_year];
    projected_parameters_[current_year] = (*estimable_map_)[current_year];
    break;

  default:
    LOG_ERROR() << "The estimable you have provided for use in a projection: " << estimable_parameter_
        << " is not a type that is supported for type LogNormal";
    break;
  }
}

} /* namespace niwa */




















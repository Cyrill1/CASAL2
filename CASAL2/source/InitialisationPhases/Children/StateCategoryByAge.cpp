/**
 * @file StateCategoryByAge.cpp
 * @author Scott Rasmussen (scott.rasmussen@zaita.com)
 * @github https://github.com/Zaita
 * @date 16/04/2015
 * @section LICENSE
 *
 * Copyright NIWA Science �2015 - www.niwa.co.nz
 *
 */

#include "StateCategoryByAge.h"

#include "Categories/Categories.h"
#include "Model/Managers.h"
#include "Model/Model.h"
#include "TimeSteps/Manager.h"

namespace niwa {
namespace initialisationphases {

StateCategoryByAge::StateCategoryByAge(Model* model)
  : InitialisationPhase(model),
    partition_(model) {

  n_table_ = new parameters::Table(PARAM_N);

  parameters_.Bind<string>(PARAM_CATEGORIES, &category_labels_, "List of categories to use", "");
  parameters_.Bind<unsigned>(PARAM_MIN_AGE, &min_age_, "Minimum age to use for this process", "");
  parameters_.Bind<unsigned>(PARAM_MAX_AGE, &max_age_, "Maximum age to use for this process", "");
  parameters_.BindTable(PARAM_N, n_table_, "Table of data", "", false, false);

  RegisterAsEstimable(&n_);
}

/**
 * Destructor
 */
StateCategoryByAge::~StateCategoryByAge() {
  delete n_table_;
}

/**
 * Validate the parameters passed in from the configuration file
 */
void StateCategoryByAge::DoValidate() {
  if (max_age_ < min_age_)
    LOG_ERROR_P(PARAM_MIN_AGE) << "(" << min_age_ << ") cannot be less than the max age(" << max_age_ << ")";

  column_count_ = (max_age_ - min_age_) + 2;

  /**
   * Validate our categories
   */
  for (string label : category_labels_) {
    if (!model_->categories()->IsValid(label))
      LOG_ERROR_P(PARAM_CATEGORIES) << " label " << label << " is not a valid category";
  }

  /**
   * Convert the string values to doubles and load them in to a table.
   */
  vector<vector<string>>& data = n_table_->data();
  unsigned row_number = 1;
  for (auto row : data) {
    if (row.size() != column_count_)
      LOG_ERROR_P(PARAM_N) << "the " << row_number << "the row has " << row.size() << " values but " << column_count_ << " values are expected";
    if (n_.find(row[0]) != n_.end())
      LOG_ERROR_P(PARAM_N) << "the category " << row[0] << " is defined more than once. You can only define a category once";
    if (std::find(category_labels_.begin(), category_labels_.end(), row[0]) == category_labels_.end())
      LOG_ERROR_P(PARAM_N) << "the category " << row[0] << " is not a valid category specified in the model";

    for (unsigned i = 1; i < row.size(); ++i) {
      Double temp = Double();
      if (!utilities::To<Double>(row[i], temp))
        LOG_ERROR_P(PARAM_N) << "value (" << row[i] << ") in row " << row_number << " is not a valid numeric";
      n_[row[0]].push_back(temp);
    }
    row_number++;
  }
}

/**
 * Build runtime relationships between this object and other objects.
 * Build any data objects that need to be built.
 */
void StateCategoryByAge::DoBuild() {
  partition_.Init(category_labels_);
}

/**
 * Execute this process
 */
void StateCategoryByAge::Execute() {
  for (auto iter : partition_) {
    unsigned i = 0;
    for (unsigned index = min_age_ - iter->min_age_; index <= max_age_ - iter->min_age_; ++index, ++i)  {
      if (index < 0)
        LOG_ERROR_P(PARAM_MIN_AGE) << " Cannot be less than category or model min_age which is " << iter->min_age_;
      LOG_MEDIUM() << ": the partition changes from " <<  iter->data_[index];
      iter->data_[index] = n_[iter->name_][i];
      LOG_MEDIUM() << " to = " << iter->data_[index];
    }
  }
}

} /* namespace initialisationphases */
} /* namespace niwa */

/**
 * @file SurvivalConstantRate.cpp
 * @author  You're Name e.g. Craig Marsh
 * @institute NIWA 
 * @version 1.0
 * @date date of creation e.g. 17/07/16
 * @licence 
 *
 */

// Headers
#include "SurvivalConstantRate.h"

#include <numeric>

#include "Categories/Categories.h"
#include "Selectivities/Manager.h"
#include "Selectivities/Selectivity.h"
#include "TimeSteps/Manager.h"

// Namespaces
namespace niwa {
namespace processes {

/**
 * Default Constructor
 */
SurvivalConstantRate::SurvivalConstantRate(Model* model)
  : Process(model),
    partition_(model) {
  LOG_TRACE();
  process_type_ = ProcessType::kMortality;
  partition_structure_ = PartitionStructure::kAge;

  parameters_.Bind<string>(PARAM_CATEGORIES, &category_labels_, "List of categories", "");
  parameters_.Bind<Double>(PARAM_S, &s_input_, "Mortality rates", "");
  parameters_.Bind<Double>(PARAM_TIME_STEP_RATIO, &ratios_, "Time step ratios for S", "", true);
  parameters_.Bind<string>(PARAM_SELECTIVITIES, &selectivity_names_, "Selectivities", "");

  RegisterAsEstimable(PARAM_S, &s_);
}

/**
 * Validate our Survival Constant Rate process
 *
 * - Validate the required parameters
 * - Assign the label from the parameters
 * - Assign and validate remaining parameters
 * - Duplicate 's' and 'selectivities' if only 1 vale specified
 * - Check s is between 0.0 and 1.0
 * - Check the categories are real
 */
void SurvivalConstantRate::DoValidate() {
  category_labels_ = model_->categories()->ExpandLabels(category_labels_, parameters_.Get(PARAM_CATEGORIES));

  if (s_input_.size() == 1)
    s_input_.assign(category_labels_.size(), s_input_[0]);
  if (selectivity_names_.size() == 1)
    selectivity_names_.assign(category_labels_.size(), selectivity_names_[0]);

  if (s_input_.size() != category_labels_.size()) {
    LOG_ERROR_P(PARAM_S)
        << ": Number of Ms provided is not the same as the number of categories provided. Expected: "
        << category_labels_.size()<< " but got " << s_input_.size();
  }

  if (selectivity_names_.size() != category_labels_.size()) {
    LOG_ERROR_P(PARAM_SELECTIVITIES)
        << ": Number of selectivities provided is not the same as the number of categories provided. Expected: "
        << category_labels_.size()<< " but got " << selectivity_names_.size();
  }

  // Validate our Ms are between 1.0 and 0.0
  for (Double s : s_input_) {
    if (s < 0.0 || s > 1.0)
      LOG_ERROR_P(PARAM_S) << ": m value " << AS_DOUBLE(s) << " must be between 0.0 and 1.0 (inclusive)";
  }

  for (unsigned i = 0; i < s_input_.size(); ++i)
    m_[category_labels_[i]] = s_input_[i];

  // Check categories are real
  for (const string& label : category_labels_) {
    if (!model_->categories()->IsValid(label))
      LOG_ERROR_P(PARAM_CATEGORIES) << ": category " << label << " does not exist. Have you defined it?";
  }
}

/**
 * Build any runtime relationships
 * - Build the partition accessor
 * - Build our list of selectivities
 * - Build our ratios for the number of time steps
 */
void SurvivalConstantRate::DoBuild() {
  partition_.Init(category_labels_);

  for (string label : selectivity_names_) {
    Selectivity* selectivity = model_->managers().selectivity()->GetSelectivity(label);
    if (!selectivity)
      LOG_ERROR_P(PARAM_SELECTIVITIES) << ": selectivity " << label << " does not exist. Have you defined it?";

    selectivities_.push_back(selectivity);
  }

  /**
   * Organise our time step ratios. Each time step can
   * apply a different ratio of S so here we want to verify
   * we have enough and re-scale them to 1.0
   */
  vector<TimeStep*> time_steps = model_->managers().time_step()->ordered_time_steps();
  LOG_FINEST() << "time_steps.size(): " << time_steps.size();
  vector<unsigned> active_time_steps;
  for (unsigned i = 0; i < time_steps.size(); ++i) {
    if (time_steps[i]->HasProcess(label_))
      active_time_steps.push_back(i);
  }

  if (ratios_.size() == 0) {
    for (unsigned i : active_time_steps)
      time_step_ratios_[i] = 1.0;
  } else {
    if (ratios_.size() != active_time_steps.size())
      LOG_ERROR_P(PARAM_TIME_STEP_RATIO) << " length (" << ratios_.size()
          << ") does not match the number of time steps this process has been assigned to (" << active_time_steps.size() << ")";

    for (Double value : ratios_) {
      if (value <= 0.0 || value > 1.0)
        LOG_ERROR_P(PARAM_TIME_STEP_RATIO) << " value (" << value << ") must be between 0.0 (exclusive) and 1.0 (inclusive)";
    }

    for (unsigned i = 0; i < ratios_.size(); ++i)
      time_step_ratios_[active_time_steps[i]] = ratios_[i];
  }
}

/**
 * Execute the process
 */
void SurvivalConstantRate::DoExecute() {
  LOG_FINEST() << "year: " << model_->current_year();

  // get the ratio to apply first
  unsigned time_step = model_->managers().time_step()->current_time_step();

  LOG_FINEST() << "Ratios.size() " << time_step_ratios_.size() << " : time_step: " << time_step << "; ratio: " << time_step_ratios_[time_step];
  Double ratio = time_step_ratios_[time_step];

  StoreForReport("year", model_->current_year());

  unsigned i = 0;
  for (auto category : partition_) {
    Double s = s_[category->name_];

    unsigned j = 0;
    LOG_FINEST() << "category " << category->name_ << "; min_age: " << category->min_age_ << "; ratio: " << ratio;
    StoreForReport(category->name_ + " ratio", ratio);
    for (Double& data : category->data_) {
      data -= data * (1-exp(-selectivities_[i]->GetResult(category->min_age_ + j, category->age_length_)  * (s * ratio)));
      ++j;
    }

    ++i;
  }
}

/**
 * Reset the Survival Process
 */
void SurvivalConstantRate::DoReset() {
  mortality_rates_.clear();
}

} /* namespace processes */
} /* namespace niwa */
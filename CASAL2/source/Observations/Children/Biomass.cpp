/**
 * @file Biomass.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 7/01/2014
 * @section LICENSE
 *
 * Copyright NIWA Science �2013 - www.niwa.co.nz
 *
 */

// headers
#include "Biomass.h"

#include "Catchabilities/Manager.h"
#include "Selectivities/Manager.h"
#include "Utilities/Map.h"
#include "Utilities/To.h"

// namespaces
namespace niwa {
namespace observations {

namespace utils = niwa::utilities;

/**
 * Default constructor
 */
Biomass::Biomass(Model* model) : Observation(model) {
  parameters_.Bind<string>(PARAM_CATCHABILITY, &catchability_label_, "Catchability of Biomass", "");
  parameters_.Bind<string>(PARAM_TIME_STEP, &time_step_label_, "Time step to execute in", "");
  parameters_.Bind<string>(PARAM_OBS, &obs_, "Observation values", "");
  parameters_.Bind<unsigned>(PARAM_YEARS, &years_, "Years to execute in", "");
  parameters_.Bind<Double>(PARAM_ERROR_VALUE, &error_values_, "The error values to use against the observation values", "");
  parameters_.Bind<Double>(PARAM_DELTA, &delta_, "Delta value for error values", "", Double(1e-10));
  parameters_.Bind<string>(PARAM_SELECTIVITIES, &selectivity_labels_, "Selectivity labels to use", "", true);
  parameters_.Bind<Double>(PARAM_PROCESS_ERROR, &process_error_value_, "Process error", "", Double(0.0));

}

/**
 *
 */
void Biomass::DoValidate() {
  LOG_TRACE();

  if (category_labels_.size() != selectivity_labels_.size() && expected_selectivity_count_ != selectivity_labels_.size())
    LOG_ERROR_P(PARAM_SELECTIVITIES) << ": Number of selectivities provided (" << selectivity_labels_.size()
        << ") is not valid. You can specify either the number of category collections (" << category_labels_.size() << ") or "
        << "the number of total categories (" << expected_selectivity_count_ << ")";

  // Delta
  if (delta_ < 0.0)
    LOG_ERROR_P(PARAM_DELTA) << ": delta (" << AS_DOUBLE(delta_) << ") cannot be less than 0.0";
  if (process_error_value_ < 0.0)
    LOG_ERROR_P(PARAM_PROCESS_ERROR) << ": process_error (" << AS_DOUBLE(process_error_value_) << ") cannot be less than 0.0";

  // Obs
  vector<string> obs  = obs_;
  if (obs.size() != category_labels_.size() * years_.size())
    LOG_ERROR_P(PARAM_OBS) << ": obs values length (" << obs.size() << ") must match the number of category collections provided ("
        << category_labels_.size() << ") * years (" << years_.size() << ")";


  // Error Value
  if (error_values_.size() == 1 && obs.size() > 1)
    error_values_.assign(obs.size(), error_values_[0]);
  if (error_values_.size() != obs.size())
    LOG_ERROR_P(PARAM_ERROR_VALUE) << ": error_value length (" << error_values_.size()
        << ") must be same length as obs (" << obs.size() << ")";

  error_values_by_year_ = utils::Map::create(years_, error_values_);

  Double value = 0.0;
  for (unsigned i = 0; i < years_.size(); ++i) {
    for (unsigned j = 0; j < category_labels_.size(); ++j) {
      unsigned index = (i * category_labels_.size()) + j;

      if (!utils::To<Double>(obs[index], value))
            LOG_ERROR_P(PARAM_OBS) << ": obs value " << obs[index] << " cannot be converted to a double";
          if (value <= 0.0)
            LOG_ERROR_P(PARAM_OBS) << ": obs value " << value << " cannot be less than or equal to 0.0";

          proportions_by_year_[years_[i]].push_back(value);
    }
  }

  /**
   * Verify that the likelihood is from the acceptable ones.
   */
  if (likelihood_type_ != PARAM_NORMAL && likelihood_type_ != PARAM_LOGNORMAL && likelihood_type_ != PARAM_PSEUDO)
    LOG_ERROR_P(PARAM_LIKELIHOOD) << ": likelihood " << likelihood_type_ << " is not supported by the biomass observation";
}

/**
 *
 */
void Biomass::DoBuild() {
  LOG_TRACE();

  catchability_ = model_->managers().catchability()->GetCatchability(catchability_label_);
  if (!catchability_)
    LOG_ERROR_P(PARAM_CATCHABILITY) << ": catchability " << catchability_label_ << " could not be found. Have you defined it?";

  partition_ = CombinedCategoriesPtr(new niwa::partition::accessors::CombinedCategories(model_, category_labels_));
  cached_partition_ = CachedCombinedCategoriesPtr(new niwa::partition::accessors::cached::CombinedCategories(model_, category_labels_));

  // Build Selectivity pointers
  for(string label : selectivity_labels_) {
    Selectivity* selectivity = model_->managers().selectivity()->GetSelectivity(label);
    if (!selectivity)
      LOG_ERROR_P(PARAM_SELECTIVITIES) << ": Selectivity " << label << " does not exist. Have you defined it?";
    selectivities_.push_back(selectivity);
  }
  if (selectivities_.size() == 1 && category_labels_.size() != 1)
    selectivities_.assign(category_labels_.size(), selectivities_[0]);


  if (partition_->category_count() != selectivities_.size())
    LOG_ERROR_P(PARAM_SELECTIVITIES) << ": number of selectivities provided (" << selectivities_.size() << ") does not match the number "
        "of categories provided (" << partition_->category_count() << ")";


}

/**
 *
 */
void Biomass::PreExecute() {
  cached_partition_->BuildCache();
}

/**
 *
 */
void Biomass::Execute() {
  LOG_FINEST() << "Entering observation " << label_;

  Double expected_total = 0.0; // value in the model
  vector<string> keys;
  vector<Double> expecteds;
  vector<Double> observeds;
  vector<Double> error_values;
  vector<Double> process_errors;
  vector<Double> scores;

  Double selectivity_result = 0.0;
  Double start_value = 0.0;
  Double end_value = 0.0;
  Double final_value = 0.0;
  unsigned age = 0;
  Double error_value = 0.0;

  unsigned current_year = model_->current_year();

  // Loop through the obs
  auto cached_partition_iter = cached_partition_->Begin();
  auto partition_iter = partition_->Begin(); // auto = map<map<string, vector<partition::category&> > >

  if (cached_partition_->Size() != proportions_by_year_[current_year].size())
    LOG_CODE_ERROR() << "cached_partition_->Size() != proportions_.size()";
  if (partition_->Size() != proportions_by_year_[current_year].size())
    LOG_CODE_ERROR() << "partition_->Size() != proportions_.size()";

  for (unsigned proportions_index = 0; proportions_index < proportions_by_year_[current_year].size(); ++proportions_index, ++partition_iter, ++cached_partition_iter) {
    expected_total = 0.0;

    auto category_iter = partition_iter->begin();
    auto cached_category_iter = cached_partition_iter->begin();
    for (unsigned category_offset = 0; category_iter != partition_iter->end(); ++category_offset, ++cached_category_iter, ++category_iter) {
      (*category_iter)->UpdateMeanWeightData();

      for (unsigned data_offset = 0; data_offset < (*category_iter)->data_.size(); ++data_offset) {
        age = (*category_iter)->min_age_ + data_offset;

        selectivity_result = selectivities_[category_offset]->GetResult(age, (*category_iter)->age_length_);
        start_value = (*cached_category_iter).data_[data_offset];
        end_value = (*category_iter)->data_[data_offset];
        final_value = 0.0;

        if (mean_proportion_method_)
          final_value = start_value + ((end_value - start_value) * proportion_of_time_);
        else {
          // re-write of std::abs(start_value - end_value) * temp_step_proportion for ADMB
          Double temp = start_value - end_value;
          temp = temp < 0 ? temp : temp * -1.0;
          final_value = temp * proportion_of_time_;
        }
        expected_total += selectivity_result * final_value * (*category_iter)->mean_weight_per_[age];
      }
    }

    /**
     * expected_total is the number of fish the model has for the category across
     */
    expected_total *= catchability_->q();
    error_value = error_values_by_year_[current_year];

    // Store the values
    keys.push_back(category_labels_[proportions_index]);
    expecteds.push_back(expected_total);
    observeds.push_back(proportions_by_year_[current_year][proportions_index]);
    error_values.push_back(error_value);
    process_errors.push_back(process_error_value_);
  }

  for (unsigned index = 0; index < observeds.size(); ++index)
    SaveComparison(keys[index], expecteds[index], observeds[index], process_errors[index], error_values[index], delta_, 0.0);
}

/**
 *
 */
void Biomass::CalculateScore() {
  /**
   * Simulate or generate results
   * During simulation mode we'll simulate results for this observation
   */
  if (model_->run_mode() == RunMode::kSimulation) {
    likelihood_->SimulateObserved(comparisons_);
  } else {
    /**
     * Convert the expected_values in to a proportion
     */
    for (unsigned year : years_) {
      scores_[year] = likelihood_->GetInitialScore(comparisons_, year);
      likelihood_->GetScores(comparisons_);
      for (obs::Comparison comparison : comparisons_[year]) {
        scores_[year] += comparison.score_;
      }
    }
  }
}


} /* namespace observations */
} /* namespace niwa */

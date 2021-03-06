/**
 * @file TagByLength.cpp
 * @author Scott Rasmussen (scott.rasmussen@zaita.com)
 * @github https://github.com/Zaita
 * @date 18/06/2015
 * @section LICENSE
 *
 * Copyright NIWA Science �2015 - www.niwa.co.nz
 *
 */

// headers
#include "TagByLength.h"

#include "Categories/Categories.h"
#include "Selectivities/Manager.h"
#include "Penalties/Manager.h"
#include "Utilities/DoubleCompare.h"

// namespaces
namespace niwa {
namespace processes {

/**
 *
 */
TagByLength::TagByLength(Model* model)
  : Process(model),
    to_partition_(model),
    from_partition_(model) {
  process_type_ = ProcessType::kTransition;
  partition_structure_ = PartitionStructure::kAge;

  numbers_table_ = new parameters::Table(PARAM_NUMBERS);
  proportions_table_ = new parameters::Table(PARAM_PROPORTIONS);

  parameters_.Bind<string>(PARAM_FROM, &from_category_labels_, "Categories to transition from", "");
  parameters_.Bind<string>(PARAM_TO, &to_category_labels_, "Categories to transition to", "");
  parameters_.Bind<bool>(PARAM_PLUS_GROUP, &plus_group_, "Use plus group for last length bin", "", false);
  parameters_.Bind<Double>(PARAM_MAX_LENGTH, &max_length_, "The upper length when there is no plus group", "",Double(0));
  parameters_.Bind<string>(PARAM_PENALTY, &penalty_label_, "Penalty label", "", "");
  parameters_.Bind<Double>(PARAM_U_MAX, &u_max_, "U Max", "", 0.99);
  parameters_.Bind<unsigned>(PARAM_YEARS, &years_, "Years to execute the transition in", "");
  parameters_.Bind<Double>(PARAM_INITIAL_MORTALITY, &initial_mortality_, "", "", Double(0));
  parameters_.Bind<string>(PARAM_INITIAL_MORTALITY_SELECTIVITY, &initial_mortality_selectivity_label_, "", "", "");
  parameters_.Bind<string>(PARAM_SELECTIVITIES, &selectivity_labels_, "", "");
  parameters_.Bind<Double>(PARAM_N, &n_, "", "", true);
  parameters_.BindTable(PARAM_NUMBERS, numbers_table_, "Table of N data", "", true, true);
  parameters_.BindTable(PARAM_PROPORTIONS, proportions_table_, "Table of proportions to move", "" , true, true);
}

/**
 * Destructor
 */
TagByLength::~TagByLength() {
  delete numbers_table_;
  delete proportions_table_;
}

/**
 * Validate the parameters from the configuration file
 */
void TagByLength::DoValidate() {
  from_category_labels_ = model_->categories()->ExpandLabels(from_category_labels_, parameters_.Get(PARAM_FROM));
  to_category_labels_ = model_->categories()->ExpandLabels(to_category_labels_, parameters_.Get(PARAM_TO));

  if (from_category_labels_.size() != to_category_labels_.size()) {
    LOG_ERROR_P(PARAM_TO) << " number of values supplied (" << to_category_labels_.size()
        << ") does not match the number of from categories provided (" << from_category_labels_.size() << ")";
  }
  if (u_max_ <= 0.0 || u_max_ > 1.0)
    LOG_ERROR_P(PARAM_U_MAX) << " (" << u_max_ << ") must be greater than 0.0 and less than 1.0";
  if (!plus_group_ && max_length_ == 0.0)
    LOG_ERROR_P(PARAM_MAX_LENGTH) << "Must supply parameter " << PARAM_MAX_LENGTH << " when plus group is false";
  if (plus_group_ && max_length_ != 0.0)
    LOG_WARNING() << "You have specified parameter " << PARAM_MAX_LENGTH << " and set plus group = true. " << PARAM_MAX_LENGTH
          << " will be ignored";

  /**
   * Get our first year
   */
  first_year_ = years_[0];
  std::for_each(years_.begin(), years_.end(), [this](unsigned year){ first_year_ = year < first_year_ ? year : first_year_; });

  /**
   * Build our tables
   */
  if (numbers_table_->row_count() == 0 && proportions_table_->row_count() == 0)
    LOG_ERROR() << location() << " must have either a " << PARAM_NUMBERS << " or " << PARAM_PROPORTIONS << " table defined with appropriate data";
  if (numbers_table_->row_count() != 0 && proportions_table_->row_count() != 0)
    LOG_ERROR() << location() << " cannot have both a " << PARAM_NUMBERS << " and " << PARAM_PROPORTIONS << " table defined. Please only use 1.";
  if (proportions_table_->row_count() != 0 && !parameters_.Get(PARAM_N)->has_been_defined())
    LOG_ERROR() << location() << " cannot have a " << PARAM_PROPORTIONS << " table without defining " << PARAM_N;

  /**
   * Load our N data in to the map
   */
  if (numbers_table_->row_count() != 0) {
    vector<string> columns = numbers_table_->columns();
    if (columns[0] != PARAM_YEAR)
      LOG_ERROR_P(PARAM_NUMBERS) << " first column label (" << columns[0] << ") provided must be 'year'";

    unsigned number_bins = columns.size();

    for (unsigned i = 1; i < number_bins; ++i) {
      Double length = 0;
      if (!utilities::To<Double>(columns[i], length))
        LOG_ERROR() << "Could not convert value " << columns[i] << " to Double";
      length_bins_.push_back(length);
    }
    if (!plus_group_)
      length_bins_.push_back(max_length_);


    // load our table data in to our map
    vector<vector<string>> data = numbers_table_->data();
    unsigned year = 0;
    Double n_value = 0.0;
    for (auto iter : data) {
      if (!utilities::To<unsigned>(iter[0], year))
        LOG_ERROR_P(PARAM_NUMBERS) << " value (" << iter[0] << ") is not a valid unsigned value that could be converted to a model year";
      for (unsigned i = 1; i < iter.size(); ++i) {
        if (!utilities::To<Double>(iter[i], n_value))
          LOG_ERROR_P(PARAM_NUMBERS) << " value (" << iter[i] << ") could not be converted to a double. Please ensure it's a numeric value";
        if (numbers_[year].size() == 0)
          numbers_[year].resize(number_bins, 0.0);
        numbers_[year][i - 1] = n_value;
      }
    }

    for (auto iter : numbers_) {
      if (std::find(years_.begin(), years_.end(), iter.first) == years_.end())
        LOG_ERROR_P(PARAM_NUMBERS) << " table contains year " << iter.first << " that is not a valid year defined in this process";
    }

  } else if (proportions_table_->row_count() != 0) {
    /**
     * Load data from proportions table using n parameter
     */
    vector<string> columns = proportions_table_->columns();
    if (columns[0] != PARAM_YEAR)
      LOG_ERROR_P(PARAM_PROPORTIONS) << " first column label (" << columns[0] << ") provided must be 'year'";
    unsigned number_bins = columns.size();

    for (unsigned i = 1; i < number_bins; ++i) {
      Double length = 0;
      if (!utilities::To<Double>(columns[i], length))
        LOG_ERROR() << "Could not convert value " << columns[i] << " to Double";
      length_bins_.push_back(length);
    }
    if (!plus_group_)
      length_bins_.push_back(max_length_);

    // build a map of n data by year
    if (n_.size() == 1)
      n_.assign(years_.size(), n_[0]);
    else if (n_.size() != years_.size())
      LOG_ERROR_P(PARAM_N) << " values provied (" << n_.size() << ") does not match the number of years (" << years_.size() << ")";
    map<unsigned, Double> n_by_year = utilities::Map::create(years_, n_);

    // load our table data in to our map
    vector<vector<string>> data = proportions_table_->data();
    unsigned year = 0;
    Double proportion = 0.0;
    for (auto iter : data) {
      if (!utilities::To<unsigned>(iter[0], year))
        LOG_ERROR_P(PARAM_PROPORTIONS) << " value (" << iter[0] << ") is not a valid unsigned value that could be converted to a model year";
      Double total_proportion = 0.0;
      for (unsigned i = 1; i < iter.size(); ++i) {
        if (!utilities::To<Double>(iter[i], proportion))
          LOG_ERROR_P(PARAM_PROPORTIONS) << " value (" << iter[i] << ") could not be converted to a double. Please ensure it's a numeric value";
        if (numbers_[year].size() == 0)
          numbers_[year].resize(number_bins, 0.0);
        numbers_[year][i - 1] = n_by_year[year] * proportion;
        total_proportion += proportion;
      }
      if (fabs(1.0 - total_proportion) > 0.00001)
        LOG_ERROR_P(PARAM_PROPORTIONS) << " total (" << total_proportion << ") is not 1.0 for year " << year;
    }

    for (auto iter : numbers_) {
      if (std::find(years_.begin(), years_.end(), iter.first) == years_.end())
        LOG_ERROR_P(PARAM_PROPORTIONS) << " table contains year " << iter.first << " that is not a valid year defined in this process";
    }
  }

  // Check value for initial mortality
  if ((initial_mortality_ < 0) | (initial_mortality_ > 1.0))
    LOG_ERROR_P(PARAM_INITIAL_MORTALITY) << ": must be between 0.0 (inclusive) amd less than 1.0 (inclusive)";

}

/**
 * Build relationships between this object and others
 */
void TagByLength::DoBuild() {
  from_partition_.Init(from_category_labels_);
  to_partition_.Init(to_category_labels_);

  if (penalty_label_ != "")
    penalty_ = model_->managers().penalty()->GetPenalty(penalty_label_);
  else
    LOG_WARNING() << location() << " no penalty has been specified. Exploitation above u_max will not affect the objective function";

  selectivities::Manager& selectivity_manager = *model_->managers().selectivity();
  for (unsigned i = 0; i < selectivity_labels_.size(); ++i) {
    Selectivity* selectivity = selectivity_manager.GetSelectivity(selectivity_labels_[i]);
    if (!selectivity)
      LOG_ERROR() << "Selectivity: " << selectivity_labels_[i] << " not found";
    selectivities_[from_category_labels_[i]] = selectivity;
  }
  if (initial_mortality_selectivity_label_ != "")
    initial_mortality_selectivity_ = selectivity_manager.GetSelectivity(initial_mortality_selectivity_label_);
}

/**
 * Execute this process
 */
void TagByLength::DoExecute() {
  unsigned current_year = model_->current_year();
  if (std::find(years_.begin(), years_.end(), current_year) == years_.end())
    return;

  auto from_iter = from_partition_.begin();
  auto to_iter   = to_partition_.begin();
  /**
   * Do the transition with mortality on the fish we're moving
   */
  LOG_FINEST() << "numbers__.size(): " << numbers_.size();
  LOG_FINEST() << "numbers__[current_year].size(): " << numbers_[current_year].size();
  for (unsigned i = 0; i < numbers_[current_year].size(); ++i)
    LOG_FINEST() << "numbers__[current_year][" << i << "]: " << numbers_[current_year][i];

  Double total_stock_with_selectivities = 0.0;
  unsigned number_bins = length_bins_.size();
  if (!plus_group_)
    number_bins -= 1;

  LOG_FINE() << "number of length bins: " << number_bins << " in year " << current_year;

  // iterate over from_categories to update length data and age length matrix instead of doing in a length loop
  from_iter = from_partition_.begin();
  for (; from_iter != from_partition_.end(); from_iter++) {
    (*from_iter)->UpdateMeanLengthData();
    //  build numbers at age length
    (*from_iter)->UpdateAgeLengthData(length_bins_, plus_group_, selectivities_[(*from_iter)->name_]);
    //  total numbers at length
    (*from_iter)->CollapseAgeLengthDataToLength();
  }

  // Calculate the exploitation rate by length bin
  for (unsigned i = 0; i <  length_bins_.size(); ++i) {
    /**
     * Calculate the vulnerable abundance to the tagging event
     */
    from_iter = from_partition_.begin();
    to_iter   = to_partition_.begin();

    LOG_FINEST() << "selectivity.size(): " << selectivities_.size();
    for (auto iter : selectivities_)
      LOG_FINE() << "selectivity: " << iter.first;

    total_stock_with_selectivities = 0.0;
    for (; from_iter != from_partition_.end(); from_iter++, to_iter++) {

      total_stock_with_selectivities += (*from_iter)->length_data_[i];

      LOG_FINEST() << "name: " << (*from_iter)->name_ << " in bin with lower length value = " << length_bins_[i];
      LOG_FINEST() << "Numbers at length: " << (*from_iter)->length_data_[i];
      LOG_FINEST() << "**";
    }
    LOG_FINEST() << "total_stock_with_selectivities: " << total_stock_with_selectivities << " at length " << length_bins_[i];

    /**
     * Migrate the exploited amount
     */
    from_iter = from_partition_.begin();
    to_iter   = to_partition_.begin();
    for (; from_iter != from_partition_.end(); from_iter++, to_iter++) {
      LOG_FINE() << "--";
      LOG_FINE() << "Working with categories: from " << (*from_iter)->name_ << "; to " << (*to_iter)->name_;
      string category_label = (*from_iter)->name_;

      if (numbers_[current_year][i] == 0)
        continue;

      Double current = numbers_[current_year][i] * ((*from_iter)->length_data_[i] / total_stock_with_selectivities);

      Double exploitation = current / utilities::doublecompare::ZeroFun((*from_iter)->length_data_[i]);
      if (exploitation > u_max_) {
        LOG_FINE() << "Exploitation(" << exploitation << ") triggered u_max(" << u_max_ << ") with current(" << current << ")";

        exploitation = u_max_;
        current = (*from_iter)->length_data_[i] *  u_max_;
        LOG_FINE() << "tagging amount overridden with " << current << " = " << (*from_iter)->length_data_[i] << " * " << u_max_;

        if (penalty_)
          penalty_->Trigger(label_, numbers_[current_year][i], (*from_iter)->length_data_[i] * u_max_);
      }

      LOG_FINE() << "total_stock_with_selectivities: " << total_stock_with_selectivities;
      LOG_FINE() << "numbers/n: " << numbers_[current_year][i];
      LOG_FINE() << (*from_iter)->name_ << " population at length " << length_bins_[i] << ": " << (*from_iter)->length_data_[i];
      if (exploitation > u_max_) {
        LOG_FINE() << "exploitation: " << u_max_ << " (u_max)";
        LOG_FINE() << "tagging amount: " << current << " = " << (*from_iter)->length_data_[i] << " * " << u_max_;
      } else {
        LOG_FINE() << "exploitation: " << exploitation << "; calculated as " << current << " / ("
                        << (*from_iter)->length_data_[i] << ")";
        LOG_FINE() << "tagging amount: " << current << " = " << numbers_[current_year][i] << " * "
            << (*from_iter)->length_data_[i] << " * " << " / " << total_stock_with_selectivities;
      }
      vector<Double> numbers_at_age((*from_iter)->data_.size(), 0.0);

      for (unsigned j = 0; j < (*from_iter)->data_.size(); ++j) {
        numbers_at_age[j] += (*from_iter)->age_length_matrix_[j][i] * exploitation;
        (*from_iter)->data_[j] -= numbers_at_age[j];
        (*to_iter)->data_[j] += numbers_at_age[j];
        LOG_FINEST() << "Numbers from age = " <<  (*from_iter)->min_age_ + j << " in length bin " << length_bins_[i] << " = "
            << numbers_at_age[j];
        /**
         * Apply the Initial mortality and tag loss after the tagging event
         */
        // Currently multiplying a proportion by a selectivity
        if((initial_mortality_selectivity_label_ != "") & (initial_mortality_ > 0.0))
          (*to_iter)->data_[j] -= (*to_iter)->data_[j] * initial_mortality_ * initial_mortality_selectivity_->GetResult((*to_iter)->min_age_ + j, (*to_iter)->age_length_);
        else if((initial_mortality_selectivity_label_ == "") & (initial_mortality_ > 0.0))
          (*to_iter)->data_[j] -= (*to_iter)->data_[j] * initial_mortality_;
      }
    }
  }  // for (unsigned i = 0; i <  length_bins_.size(); ++i)


  for (unsigned year : years_) {
    if (numbers_.find(year) == numbers_.end())
      LOG_ERROR_P(PARAM_YEARS) << " value (" << year << ") does not have a corresponding entry in the numbers or proportions table";
  }
}

} /* namespace processes */
} /* namespace niwa */

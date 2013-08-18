/**
 * @file Biomass.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 18/07/2013
 * @section LICENSE
 *
 * Copyright NIWA Science �2013 - www.niwa.co.nz
 *
 */

// headers
#include "Biomass.h"

// namespaces
namespace isam {
namespace derivedquantities {

/**
 * default constructor
 */
Biomass::Biomass() {
}

/**
 *
 */
void Biomass::Calculate() {
  Double value = 0.0;

  auto iterator = partition_->Begin();
  // iterate over each category
  for (unsigned i = 0; i < partition_->Size() && iterator != partition_->End(); ++i, ++iterator) {
    for (unsigned j = 0; j < (*iterator)->data_.size(); ++j) {
      value += (*iterator)->data_[j] * selectivities_[i]->GetResult((*iterator)->min_age_ + j) * Partition::Instance().mean_weight(0, "");
    }
  }

  values_[model_->current_year()] = value;

}

/**
 *
 */
void Biomass::Calculate(unsigned initialisation_phase) {
  Double value = 0.0;

  auto iterator = partition_->Begin();
  // iterate over each category
  for (unsigned i = 0; i < partition_->Size() && iterator != partition_->End(); ++i, ++iterator) {
    for (unsigned j = 0; j < (*iterator)->data_.size(); ++j) {
      value += (*iterator)->data_[j] * selectivities_[i]->GetResult((*iterator)->min_age_ + j) * Partition::Instance().mean_weight(0, "");
    }
  }

  initialisation_values_[initialisation_phase].push_back(value);
}

} /* namespace derivedquantities */
} /* namespace isam */
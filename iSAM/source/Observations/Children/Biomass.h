/**
 * @file Biomass.h
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 7/01/2014
 * @section LICENSE
 *
 * Copyright NIWA Science �2013 - www.niwa.co.nz
 *
 * @section DESCRIPTION
 *
 * << Add Description >>
 */
#ifndef OBSERVATIONS_BIOMASS_H_
#define OBSERVATIONS_BIOMASS_H_

// headers
#include "Observations/Observation.h"

#include "Catchabilities/Catchability.h"
#include "Partition/Accessors/CombinedCategories.h"
#include "Partition/Accessors/Cached/CombinedCategories.h"

// namespaces
namespace isam {
namespace observations {

using partition::accessors::CombinedCategoriesPtr;
using partition::accessors::cached::CachedCombinedCategoriesPtr;

/**
 * class definition
 */
class Biomass : public isam::Observation {
public:
  // methods
  Biomass();
  virtual                     ~Biomass() = default;
  void                        DoValidate() override final;
  void                        DoBuild() override final;
  void                        DoReset() override final { };
  void                        PreExecute() override final;
  void                        Execute() override final;
  void                        PostExecute() override final { };

private:
  // members
  // Members
  map<unsigned, vector<Double> >  proportions_by_year_;
  map<unsigned, Double>           error_values_by_year_;
  vector<Double>                  error_values_;
  string                          catchability_label_;
  CatchabilityPtr                 catchability_;
  Double                          delta_;
  Double                          process_error_;
  CachedCombinedCategoriesPtr     cached_partition_;
  CombinedCategoriesPtr           partition_;
  vector<string>                  obs_;
};

} /* namespace observations */
} /* namespace isam */
#endif /* OBSERVATIONS_BIOMASS_H_ */

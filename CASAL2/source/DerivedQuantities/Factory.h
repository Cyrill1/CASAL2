/**
 * @file Factory.h
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 6/06/2013
 * @section LICENSE
 *
 * Copyright NIWA Science �2013 - www.niwa.co.nz
 *
 * @section DESCRIPTION
 *
 * Standard factory class for the derived quantity
 */
#ifndef DERIVEDQUANTITIES_FACTORY_H_
#define DERIVEDQUANTITIES_FACTORY_H_

// namespaces
#include "DerivedQuantities/DerivedQuantity.h"

// namespaces
namespace niwa {
class Model;

namespace derivedquantities {

// classes
class Factory {
public:
  // methods
  static DerivedQuantity*     Create(Model* model, const string& object_type, const string& sub_type);

private:
  // methods
  Factory() = delete;
  virtual ~Factory() = delete;
};

} /* namespace derivedquantities */
} /* namespace niwa */
#endif /* FACTORY_H_ */

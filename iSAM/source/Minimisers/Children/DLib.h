/**
 * @file DLib.h
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 14/03/2014
 * @section LICENSE
 *
 * Copyright NIWA Science �2014 - www.niwa.co.nz
 *
 * @section DESCRIPTION
 *
 * << Add Description >>
 */
#ifndef USE_AUTODIFF
#ifndef MINIMISERS_DLIB_H_
#define MINIMISERS_DLIB_H_

// headers
#include "Minimisers/Minimiser.h"

// namespaces
namespace isam {
namespace minimisers {

/**
 * Class definition
 */
class DLib : public isam::Minimiser {
public:
  // methods
  DLib();
  virtual                     ~DLib() = default;
  void                        DoValidate() override final { };
  void                        DoBuild() override final { };
  void                        DoReset() override final { };
  void                        Execute() override final;
};

} /* namespace minimisers */
} /* namespace isam */
#endif /* MINIMISERS_DLIB_H_ */
#endif /* NOT USE_AUTODIFF */
/**
 * @file Factory.h
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @version 1.0
 * @date 15/02/2013
 * @section LICENSE
 *
 * Copyright NIWA Science �2013 - www.niwa.co.nz
 *
 * $Date: 2008-03-04 16:33:32 +1300 (Tue, 04 Mar 2008) $
 */
#ifndef PENALTIES_FACTORY_H_
#define PENALTIES_FACTORY_H_

// Headers
#include "BaseClasses/Factory.h"
#include "Penalties/Manager.h"
#include "Penalties/Penalty.h"

// Namespaces
namespace isam {
namespace penalties {

/**
 *
 */
class Factory : public isam::base::Factory<isam::Penalty, isam::penalties::Manager> {};

} /* namespace penalties */
} /* namespace isam */
#endif /* FACTORY_H_ */
/**
 * @file Manager.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 28/05/2014
 * @section LICENSE
 *
 * Copyright NIWA Science �2014 - www.niwa.co.nz
 *
 */

// headers
#include "Manager.h"

#include "Model/Model.h"
#include "Model/Managers.h"
#include "Processes/Manager.h"
#include "Model/Objects.h"

// namespaces
namespace niwa {
namespace projects {

/**
 * Default constructor
 */
Manager::Manager() {
}

/**
 * Destructor
 */
Manager::~Manager() noexcept(true) {
}

/**
 * Build
 */
void Manager::Build() {
  LOG_CODE_ERROR() << "This method is not supported";
}

/**
 * Build
 */
void Manager::Build(Model* model) {
  LOG_TRACE();
  if (model->run_mode() == RunMode::kProjection) {
    bool ycs_values_exist = false;
    for (auto project : objects_) {
      LOG_FINE() << "Building Project: " << project->label();
      project->Build();
      if (project->estimable_parameter() == PARAM_YCS_VALUES)
        ycs_values_exist = true;
    }

    if (!ycs_values_exist) {
      for (auto process : model->managers().process()->objects()) {
        if (process->type() == PARAM_RECRUITMENT_BEVERTON_HOLT)
          LOG_ERROR() << process->location() << " process " << process->label() << " does not contain a @project for ycs_values, but you are running in projection mode";
      }
    }
  }
}

/**
 *
 */
void Manager::Update(unsigned current_year) {
  LOG_TRACE();
  for (auto project : objects_)
    project->Update(current_year);
}

/**
 *  @param current_year teh current year this method is calles
 *
 *  This function will store all parameter values that we may want to overwrite at in projection years
 */
void Manager::StoreValues(unsigned current_year, unsigned start_year, unsigned final_year) {
  LOG_TRACE();
  // iterate over all @project blocks
  for (auto project : objects_) {
    LOG_FINE() << "@Project: " << project->label();
    project->StoreValue(current_year, start_year, final_year);
  }
}

} /* namespace projects */
} /* namespace niwa */

/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/nova-physics

*/

#ifndef NOVAPHYSICS_SPACE_SETTINGS_H
#define NOVAPHYSICS_SPACE_SETTINGS_H

#include "novaphysics/internal.h"
#include "novaphysics/constraints/contact_constraint.h"


/**
 * @file space_settings.h
 * 
 * @brief Physics space simulation settings.
 */


typedef struct {
    nv_float baumgarte; /**< Baumgarte stabilization factor is used to correct constraint erros in the iterative solver. */

    nv_float penetration_slop; /**< Amount of penetration error allowed in position correction. */

    nvContactPositionCorrection contact_position_correction; /**< Position correction algorithm to use for collisions. */

    nv_uint32 velocity_iterations; /**< Solver iteration count for velocity constraints,
                                        for a game 5-10 should be sufficient. */

    nv_uint32 position_iterations; /**< NGS iteration count for contact position correction only,
                                        for a game 2-6 should be sufficient. */

    nv_uint32 substeps; /**< Substep count defines how many substeps the current simulation step is going to get
                             divided into. This effectively increases the accuracy of the simulation but
                             also impacts the performance greatly because the whole simulation is processed 
                             and collisions are recalculated by given amounts of times internally. In a game,
                             you wouldn't need this much detail. Best to leave it at 1. */

    nv_float linear_damping;

    nv_float angular_damping;

    nv_bool warmstarting; /**< Whether to allow warmstarting constraints. */

    nv_bool sleeping; /**< Whether to allow sleeping. */

    nvCoefficientMix restitution_mix; /**< Mixing function used for restitution. */

    nvCoefficientMix friction_mix; /**< Mixing function used for friction. */

} nvSpaceSettings;


#endif
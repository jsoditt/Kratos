//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main author:     Alex Jarauta
//  Co-author  :     Elaf Mahrous




#if !defined(KRATOS_ULF_APPLICATION_VARIABLES_H_INCLUDED )
#define  KRATOS_ULF_APPLICATION_VARIABLES_H_INCLUDED

// System includes

// External includes

// Project includes
#include "includes/define.h"
#include "includes/define_python.h"

#include <pybind11/pybind11.h>

#include "includes/kratos_application.h"
#include "includes/variables.h"
#include "includes/dem_variables.h"
#include "includes/cfd_variables.h"
#include "includes/legacy_structural_app_vars.h"

namespace Kratos
{

     KRATOS_DEFINE_3D_VARIABLE_WITH_COMPONENTS(PRESSURE_FORCE)
     KRATOS_DEFINE_3D_VARIABLE_WITH_COMPONENTS(DISP_FRAC)
     KRATOS_DEFINE_3D_VARIABLE_WITH_COMPONENTS(VAUX)
     KRATOS_DEFINE_APPLICATION_VARIABLE(ULF_APPLICATION, int, DISABLE)
     KRATOS_DEFINE_APPLICATION_VARIABLE(ULF_APPLICATION, double, TAUONE)
     KRATOS_DEFINE_APPLICATION_VARIABLE(ULF_APPLICATION, double, TAUTWO)
     KRATOS_DEFINE_APPLICATION_VARIABLE(ULF_APPLICATION, double, NODAL_LENGTH)
     KRATOS_DEFINE_APPLICATION_VARIABLE(ULF_APPLICATION, double, MEAN_CURVATURE_2D)
     KRATOS_DEFINE_APPLICATION_VARIABLE(ULF_APPLICATION, double, TRIPLE_POINT)
     KRATOS_DEFINE_APPLICATION_VARIABLE(ULF_APPLICATION, double, CONTACT_ANGLE)
     KRATOS_DEFINE_APPLICATION_VARIABLE(ULF_APPLICATION, double, CONTACT_ANGLE_STATIC )
     KRATOS_DEFINE_APPLICATION_VARIABLE(ULF_APPLICATION, double, SURFACE_TENSION_COEF)
     KRATOS_DEFINE_APPLICATION_VARIABLE(ULF_APPLICATION, double, MEAN_CURVATURE_3D)
     KRATOS_DEFINE_APPLICATION_VARIABLE(ULF_APPLICATION, double, GAUSSIAN_CURVATURE)
     KRATOS_DEFINE_APPLICATION_VARIABLE(ULF_APPLICATION, double, PRINCIPAL_CURVATURE_1)
     KRATOS_DEFINE_APPLICATION_VARIABLE(ULF_APPLICATION, double, PRINCIPAL_CURVATURE_2)
     KRATOS_DEFINE_APPLICATION_VARIABLE(ULF_APPLICATION, double, SUBSCALE_PRESSURE)
     KRATOS_DEFINE_APPLICATION_VARIABLE(ULF_APPLICATION, double, INITIAL_MESH_SIZE)
//      KRATOS_DEFINE_APPLICATION_VARIABLE(ULF_APPLICATION, double, INITIAL_NODAL_TOTAL_AREA)
     KRATOS_DEFINE_APPLICATION_VARIABLE(ULF_APPLICATION, double, DISSIPATIVE_FORCE_COEFF_JM)
     KRATOS_DEFINE_APPLICATION_VARIABLE(ULF_APPLICATION, double, DISSIPATIVE_FORCE_COEFF_BM)
     KRATOS_DEFINE_APPLICATION_VARIABLE(ULF_APPLICATION, double, DISSIPATIVE_FORCE_COEFF_SM)
//      KRATOS_DEFINE_APPLICATION_VARIABLE(ULF_APPLICATION, double, SOLID_LIQIUD_SURFTENS_COEFF)
//      KRATOS_DEFINE_APPLICATION_VARIABLE(ULF_APPLICATION, double, SOLID_AIR_SURFTENS_COEFF)
     KRATOS_DEFINE_3D_VARIABLE_WITH_COMPONENTS(RHS_VECTOR)
     KRATOS_DEFINE_3D_VARIABLE_WITH_COMPONENTS(AUX_VECTOR)
     //KRATOS_DEFINE_3D_VARIABLE_WITH_COMPONENTS(AUX_VEL)
     //KRATOS_DEFINE_3D_VARIABLE_WITH_COMPONENTS(CONVECTION_VELOCITY)
     //KRATOS_DEFINE_3D_VARIABLE_WITH_COMPONENTS(AUX_VEL1)
     KRATOS_DEFINE_3D_APPLICATION_VARIABLE_WITH_COMPONENTS(ULF_APPLICATION, VISCOUS_STRESSX)
     KRATOS_DEFINE_3D_APPLICATION_VARIABLE_WITH_COMPONENTS(ULF_APPLICATION, VISCOUS_STRESSY)
     KRATOS_DEFINE_3D_APPLICATION_VARIABLE_WITH_COMPONENTS(ULF_APPLICATION, VISCOUS_STRESSZ) 
     KRATOS_DEFINE_3D_APPLICATION_VARIABLE_WITH_COMPONENTS(ULF_APPLICATION, PRINCIPAL_DIRECTION_1) 
     KRATOS_DEFINE_3D_APPLICATION_VARIABLE_WITH_COMPONENTS(ULF_APPLICATION, PRINCIPAL_DIRECTION_2)
     KRATOS_DEFINE_3D_APPLICATION_VARIABLE_WITH_COMPONENTS(ULF_APPLICATION, NORMAL_GEOMETRIC)
     KRATOS_DEFINE_3D_APPLICATION_VARIABLE_WITH_COMPONENTS(ULF_APPLICATION, ADHESION_FORCE)
     KRATOS_DEFINE_3D_APPLICATION_VARIABLE_WITH_COMPONENTS(ULF_APPLICATION, NORMAL_EQUILIBRIUM)
     KRATOS_DEFINE_3D_APPLICATION_VARIABLE_WITH_COMPONENTS(ULF_APPLICATION, NORMAL_CONTACT_LINE_EQUILIBRIUM)
     KRATOS_DEFINE_3D_APPLICATION_VARIABLE_WITH_COMPONENTS(ULF_APPLICATION, NORMAL_TRIPLE_POINT)
     KRATOS_DEFINE_3D_APPLICATION_VARIABLE_WITH_COMPONENTS(ULF_APPLICATION, NORMAL_CONTACT_LINE)
     KRATOS_DEFINE_3D_APPLICATION_VARIABLE_WITH_COMPONENTS(ULF_APPLICATION, SOLID_FRACTION_GRADIENT_PROJECTED)
     KRATOS_DEFINE_3D_APPLICATION_VARIABLE_WITH_COMPONENTS(ULF_APPLICATION, SUBSCALE_VELOCITY)

    }

#endif	

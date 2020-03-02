//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ \.
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics FemDem Application
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Alejandro Cornejo Velazquez
//

#if !defined(KRATOS_FEMDEM_COUPLING_UTILITIES)
#define KRATOS_FEMDEM_COUPLING_UTILITIES

// System includes

// External includes

// Project includes
#include "fem_to_dem_application_variables.h"

namespace Kratos
{
///@name Kratos Globals
///@{

///@}
///@name Type Definitions
///@{

/// The size type definition
typedef std::size_t SizeType;

///@}
///@name  Enum's
///@{

///@}
///@name  Functions
///@{

///@}
///@name Kratos Classes
///@{

/**
 * @class FEMDEMCouplingUtilities
 * @ingroup FemToDemApplication
 * @brief This class includes several utilities necessaries for the coupling between the FEM and the DEM
 * @details The methods are static, so it can be called without constructing the class
 * @tparam TDim The dimension of the problem
 * @author Alejandro Cornejo
 */
template<SizeType TDim = 3>
class FEMDEMCouplingUtilities
{
  public:
    ///@name Type definitions
    ///@{

    /// The index type definition
    typedef std::size_t IndexType;

    ///@}
    ///@name  Enum's
    ///@{

    ///@}
    ///@name Life Cycle
    ///@{

    ///@}
    ///@name Operations
    ///@{




}; // class FEMDEMCouplingUtilities
} // namespace Kratos
#endif /* KRATOS_FEMDEM_COUPLING_UTILITIES defined */
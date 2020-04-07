// KRATOS  ___|  |                   |                   |
//       \___ \  __|  __| |   |  __| __| |   |  __| _` | |
//             | |   |    |   | (    |   |   | |   (   | |
//       _____/ \__|_|   \__,_|\___|\__|\__,_|_|  \__,_|_| MECHANICS
//
//  License:		 BSD License
//					 license: structural_mechanics_application/license.txt
//
//  Main authors:    Vicente Mataix Ferrandiz
//

// System includes

// External includes

// Project includes
#include "includes/define.h"
#include "includes/checks.h"
#include "includes/kratos_flags.h"
#include "custom_elements/nodal_concentrated_element.h"
#include "custom_utilities/structural_mechanics_element_utilities.h"
#include "structural_mechanics_application_variables.h"

namespace Kratos
{
/**
 * Flags related to the element computation
 */
KRATOS_CREATE_LOCAL_FLAG( NodalConcentratedElement, COMPUTE_DISPLACEMENT_STIFFNESS,   0 );
KRATOS_CREATE_LOCAL_FLAG( NodalConcentratedElement, COMPUTE_NODAL_MASS,               1 );
KRATOS_CREATE_LOCAL_FLAG( NodalConcentratedElement, COMPUTE_ROTATIONAL_STIFFNESS,     2 );
KRATOS_CREATE_LOCAL_FLAG( NodalConcentratedElement, COMPUTE_NODAL_INERTIA,            3 );
KRATOS_CREATE_LOCAL_FLAG( NodalConcentratedElement, COMPUTE_DAMPING_RATIO,            4 );
KRATOS_CREATE_LOCAL_FLAG( NodalConcentratedElement, COMPUTE_ROTATIONAL_DAMPING_RATIO, 5 );
KRATOS_CREATE_LOCAL_FLAG( NodalConcentratedElement, COMPUTE_RAYLEIGH_DAMPING,         6 );

//***********************DEFAULT CONSTRUCTOR******************************************
/***********************************************************************************/

NodalConcentratedElement::NodalConcentratedElement(
    IndexType NewId,
    GeometryType::Pointer pGeometry
    )
    : Element( NewId, pGeometry )
{
    // If we compute the Rayleigh damping or we use the damping ratio instead
    mELementalFlags.Set(NodalConcentratedElement::COMPUTE_RAYLEIGH_DAMPING, false);
}

//******************************CONSTRUCTOR*******************************************
/***********************************************************************************/

NodalConcentratedElement::NodalConcentratedElement(
    IndexType NewId,
    GeometryType::Pointer pGeometry,
    PropertiesType::Pointer pProperties
    )
    : Element( NewId, pGeometry, pProperties )
{
    if (pProperties->Has(CONSIDER_RAYLEIGH_DAMPING)) {
        mELementalFlags.Set(NodalConcentratedElement::COMPUTE_RAYLEIGH_DAMPING, pProperties->GetValue(CONSIDER_RAYLEIGH_DAMPING));
    } else {
        mELementalFlags.Set(NodalConcentratedElement::COMPUTE_RAYLEIGH_DAMPING, false);
    }
}

//******************************COPY CONSTRUCTOR**************************************
/***********************************************************************************/

NodalConcentratedElement::NodalConcentratedElement( NodalConcentratedElement const& rOther)
    :Element(rOther)
    ,mELementalFlags(rOther.mELementalFlags)
{

}

//*******************************ASSIGMENT OPERATOR***********************************
/***********************************************************************************/

NodalConcentratedElement&  NodalConcentratedElement::operator=(NodalConcentratedElement const& rOther)
{
    //ALL MEMBER VARIABLES THAT MUST BE KEPT IN AN "=" OPERATION NEEDS TO BE COPIED HERE

    Element::operator=(rOther);
    mELementalFlags = rOther.mELementalFlags;

    return *this;
}

//*********************************OPERATIONS*****************************************
/***********************************************************************************/

Element::Pointer NodalConcentratedElement::Create(
    IndexType NewId,
    NodesArrayType const& rThisNodes,
    PropertiesType::Pointer pProperties
    ) const
{
    //NEEDED TO CREATE AN ELEMENT
    return Kratos::make_intrusive<NodalConcentratedElement>( NewId, GetGeometry().Create( rThisNodes ), pProperties );
}

/***********************************************************************************/
/***********************************************************************************/

Element::Pointer NodalConcentratedElement::Create(
    IndexType NewId,
    GeometryType::Pointer pGeom,
    PropertiesType::Pointer pProperties
    ) const
{
    //NEEDED TO CREATE AN ELEMENT
    return Kratos::make_intrusive<NodalConcentratedElement>( NewId, pGeom, pProperties);
}

//************************************CLONE*******************************************
/***********************************************************************************/

Element::Pointer NodalConcentratedElement::Clone(
    IndexType NewId,
    NodesArrayType const& rThisNodes
    ) const
{
    //YOU CREATE A NEW ELEMENT CLONING THEIR VARIABLES
    //ALL MEMBER VARIABLES THAT MUST BE CLONED HAVE TO BE DEFINED HERE

    Element::Pointer p_new_elem = Kratos::make_intrusive<NodalConcentratedElement>(NewId, GetGeometry().Create(rThisNodes), pGetProperties());
    p_new_elem->SetData(this->GetData());
    p_new_elem->Set(Flags(*this));
    return p_new_elem;
}

//*******************************DESTRUCTOR*******************************************
/***********************************************************************************/

NodalConcentratedElement::~NodalConcentratedElement()
{
}

//************* GETTING METHODS
/***********************************************************************************/
/***********************************************************************************/

void NodalConcentratedElement::GetDofList(
    DofsVectorType& rElementalDofList,
    const ProcessInfo& rCurrentProcessInfo
    ) const
{
    //NEEDED TO DEFINE THE DOFS OF THE ELEMENT
    const SizeType dimension = GetGeometry().WorkingSpaceDimension();

    // Resizing as needed
    const SizeType system_size = this->ComputeSizeOfSystem();

    if ( rElementalDofList.size() != system_size )
        rElementalDofList.resize( system_size );

    // Auxiliar index
    IndexType aux_index = 0;

    // The displacement terms
    if( mELementalFlags.Is(NodalConcentratedElement::COMPUTE_DISPLACEMENT_STIFFNESS) ||
        mELementalFlags.Is(NodalConcentratedElement::COMPUTE_NODAL_MASS)) {

        rElementalDofList[0] = GetGeometry()[0].pGetDof( DISPLACEMENT_X );
        rElementalDofList[1] = GetGeometry()[0].pGetDof( DISPLACEMENT_Y );
        if( dimension == 3)
            rElementalDofList[2] = GetGeometry()[0].pGetDof( DISPLACEMENT_Z );

        aux_index += dimension;
    }

    // The rotational terms
    if( mELementalFlags.Is(NodalConcentratedElement::COMPUTE_ROTATIONAL_STIFFNESS) ||
        mELementalFlags.Is(NodalConcentratedElement::COMPUTE_NODAL_INERTIA)) {

        rElementalDofList[aux_index + 0] = GetGeometry()[0].pGetDof( ROTATION_X );
        rElementalDofList[aux_index + 1] = GetGeometry()[0].pGetDof( ROTATION_Y );
        if( dimension == 3)
            rElementalDofList[aux_index + 2] = GetGeometry()[0].pGetDof( ROTATION_Z );
    }
}

/***********************************************************************************/
/***********************************************************************************/

void NodalConcentratedElement::EquationIdVector(
    EquationIdVectorType& rResult,
    const ProcessInfo& rCurrentProcessInfo
    ) const
{
    //NEEDED TO DEFINE GLOBAL IDS FOR THE CORRECT ASSEMBLY
    const SizeType dimension = GetGeometry().WorkingSpaceDimension();

    // Resizing as needed
    const SizeType system_size = this->ComputeSizeOfSystem();

    if ( rResult.size() != system_size )
        rResult.resize( system_size, false );

    // Auxiliar index
    IndexType aux_index = 0;

    // The displacement terms
    if( mELementalFlags.Is(NodalConcentratedElement::COMPUTE_DISPLACEMENT_STIFFNESS) ||
        mELementalFlags.Is(NodalConcentratedElement::COMPUTE_NODAL_MASS)) {

        rResult[0] = GetGeometry()[0].GetDof( DISPLACEMENT_X ).EquationId();
        rResult[1] = GetGeometry()[0].GetDof( DISPLACEMENT_Y ).EquationId();
        if( dimension == 3)
            rResult[2] = GetGeometry()[0].GetDof( DISPLACEMENT_Z ).EquationId();

        aux_index += dimension;
    }

    // The rotational terms
    if( mELementalFlags.Is(NodalConcentratedElement::COMPUTE_ROTATIONAL_STIFFNESS) ||
        mELementalFlags.Is(NodalConcentratedElement::COMPUTE_NODAL_INERTIA)) {

        rResult[aux_index + 0] = GetGeometry()[0].GetDof( ROTATION_X ).EquationId();
        rResult[aux_index + 1] = GetGeometry()[0].GetDof( ROTATION_Y ).EquationId();
        if( dimension == 3)
            rResult[aux_index + 2] = GetGeometry()[0].GetDof( ROTATION_Z ).EquationId();
    }
}

//*********************************DISPLACEMENT***************************************
/***********************************************************************************/

void NodalConcentratedElement::GetValuesVector( Vector& rValues, int Step )
{
    //GIVES THE VECTOR WITH THE DOFS VARIABLES OF THE ELEMENT (i.e. ELEMENT DISPLACEMENTS)
    const SizeType dimension = GetGeometry().WorkingSpaceDimension();

    // Resizing as needed
    const SizeType system_size = this->ComputeSizeOfSystem();

    if ( rValues.size() != system_size )
        rValues.resize( system_size, false );

    // Auxiliar index
    IndexType aux_index = 0;

    // The displacement terms
    if( mELementalFlags.Is(NodalConcentratedElement::COMPUTE_DISPLACEMENT_STIFFNESS) ||
        mELementalFlags.Is(NodalConcentratedElement::COMPUTE_NODAL_MASS)) {

        rValues[0] = GetGeometry()[0].GetSolutionStepValue( DISPLACEMENT_X, Step );
        rValues[1] = GetGeometry()[0].GetSolutionStepValue( DISPLACEMENT_Y, Step );

        if ( dimension == 3 )
            rValues[2] = GetGeometry()[0].GetSolutionStepValue( DISPLACEMENT_Z, Step );

        aux_index += dimension;
    }

    // The rotational terms
    if( mELementalFlags.Is(NodalConcentratedElement::COMPUTE_ROTATIONAL_STIFFNESS) ||
        mELementalFlags.Is(NodalConcentratedElement::COMPUTE_NODAL_INERTIA)) {

        rValues[aux_index + 0] = GetGeometry()[0].GetSolutionStepValue( ROTATION_X, Step );
        rValues[aux_index + 1] = GetGeometry()[0].GetSolutionStepValue( ROTATION_Y, Step );

        if ( dimension == 3 )
            rValues[aux_index + 2] = GetGeometry()[0].GetSolutionStepValue( ROTATION_Z, Step );
    }
}


//************************************VELOCITY****************************************
/***********************************************************************************/

void NodalConcentratedElement::GetFirstDerivativesVector( Vector& rValues, int Step )
{
    //GIVES THE VECTOR WITH THE TIME DERIVATIVE OF THE DOFS VARIABLES OF THE ELEMENT (i.e. ELEMENT VELOCITIES)
    const SizeType dimension = GetGeometry().WorkingSpaceDimension();

    // Resizing as needed
    const SizeType system_size = this->ComputeSizeOfSystem();

    if ( rValues.size() != system_size )
        rValues.resize( system_size, false );

    // Auxiliar index
    IndexType aux_index = 0;

    // The displacement terms
    if( mELementalFlags.Is(NodalConcentratedElement::COMPUTE_DISPLACEMENT_STIFFNESS) ||
        mELementalFlags.Is(NodalConcentratedElement::COMPUTE_NODAL_MASS)) {

        rValues[0] = GetGeometry()[0].GetSolutionStepValue( VELOCITY_X, Step );
        rValues[1] = GetGeometry()[0].GetSolutionStepValue( VELOCITY_Y, Step );

        if ( dimension == 3 )
            rValues[2] = GetGeometry()[0].GetSolutionStepValue( VELOCITY_Z, Step );

        aux_index += dimension;
    }

    // The rotational terms
    if( mELementalFlags.Is(NodalConcentratedElement::COMPUTE_ROTATIONAL_STIFFNESS) ||
        mELementalFlags.Is(NodalConcentratedElement::COMPUTE_NODAL_INERTIA)) {

        rValues[aux_index + 0] = GetGeometry()[0].GetSolutionStepValue( ANGULAR_VELOCITY_X, Step );
        rValues[aux_index + 1] = GetGeometry()[0].GetSolutionStepValue( ANGULAR_VELOCITY_Y, Step );

        if ( dimension == 3 )
            rValues[aux_index + 2] = GetGeometry()[0].GetSolutionStepValue( ANGULAR_VELOCITY_Z, Step );
    }
}

//*********************************ACCELERATION***************************************
/***********************************************************************************/

void NodalConcentratedElement::GetSecondDerivativesVector( Vector& rValues, int Step )
{
    //GIVES THE VECTOR WITH THE TIME SECOND DERIVATIVE OF THE DOFS VARIABLES OF THE ELEMENT (i.e. ELEMENT ACCELERATIONS)
    const SizeType dimension = GetGeometry().WorkingSpaceDimension();

    // Resizing as needed
    const SizeType system_size = this->ComputeSizeOfSystem();

    if ( rValues.size() != system_size )
        rValues.resize( system_size, false );

    // Auxiliar index
    IndexType aux_index = 0;

    // The displacement terms
    if( mELementalFlags.Is(NodalConcentratedElement::COMPUTE_DISPLACEMENT_STIFFNESS) ||
        mELementalFlags.Is(NodalConcentratedElement::COMPUTE_NODAL_MASS)) {

        rValues[0] = GetGeometry()[0].GetSolutionStepValue( ACCELERATION_X, Step );
        rValues[1] = GetGeometry()[0].GetSolutionStepValue( ACCELERATION_Y, Step );

        if ( dimension == 3 )
            rValues[2] = GetGeometry()[0].GetSolutionStepValue( ACCELERATION_Z, Step );

        aux_index += dimension;
    }

    // The rotational terms
    if( mELementalFlags.Is(NodalConcentratedElement::COMPUTE_ROTATIONAL_STIFFNESS) ||
        mELementalFlags.Is(NodalConcentratedElement::COMPUTE_NODAL_INERTIA)) {

        rValues[aux_index + 0] = GetGeometry()[0].GetSolutionStepValue( ANGULAR_ACCELERATION_X, Step );
        rValues[aux_index + 1] = GetGeometry()[0].GetSolutionStepValue( ANGULAR_ACCELERATION_Y, Step );

        if ( dimension == 3 )
            rValues[aux_index + 2] = GetGeometry()[0].GetSolutionStepValue( ANGULAR_ACCELERATION_Z, Step );
    }
}

//************* STARTING - ENDING  METHODS
/***********************************************************************************/
/***********************************************************************************/

void NodalConcentratedElement::Initialize(const ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY;

    // Defining auxiliar zero array
    const array_1d<double, 3> zero_array = ZeroVector(3);

    // We get the reference
    const auto& rconst_this = *this;

    if (HasProperties()) {
        // We check the nodal stiffness
        if (rconst_this.Has(NODAL_DISPLACEMENT_STIFFNESS) || GetProperties().Has(NODAL_DISPLACEMENT_STIFFNESS)) {
            KRATOS_WARNING_IF("NodalConcentratedElement", rconst_this.Has(NODAL_DISPLACEMENT_STIFFNESS) && GetProperties().Has(NODAL_DISPLACEMENT_STIFFNESS)) << "NODAL_DISPLACEMENT_STIFFNESS is defined both in properties and elemental data. Properties are considered by DEFAULT" << std::endl;
            mELementalFlags.Set(NodalConcentratedElement::COMPUTE_DISPLACEMENT_STIFFNESS, true);
        } else
            mELementalFlags.Set(NodalConcentratedElement::COMPUTE_DISPLACEMENT_STIFFNESS, false);

        // We check the nodal rotational stiffness
        if (GetGeometry()[0].SolutionStepsDataHas(ROTATION_X) &&
            (rconst_this.Has(NODAL_ROTATIONAL_STIFFNESS) || GetProperties().Has(NODAL_ROTATIONAL_STIFFNESS))) {
            KRATOS_WARNING_IF("NodalConcentratedElement", rconst_this.Has(NODAL_ROTATIONAL_STIFFNESS) && GetProperties().Has(NODAL_ROTATIONAL_STIFFNESS)) << "NODAL_ROTATIONAL_STIFFNESS is defined both in properties and elemental data. Properties are considered by DEFAULT" << std::endl;
            mELementalFlags.Set(NodalConcentratedElement::COMPUTE_ROTATIONAL_STIFFNESS, true);
        } else
            mELementalFlags.Set(NodalConcentratedElement::COMPUTE_ROTATIONAL_STIFFNESS, false);

        // We check the nodal mass
        if (rconst_this.Has(NODAL_MASS) || GetProperties().Has(NODAL_MASS)) {
            KRATOS_WARNING_IF("NodalConcentratedElement", rconst_this.Has(NODAL_MASS) && GetProperties().Has(NODAL_MASS)) << "NODAL_MASS is defined both in properties and elemental data. Properties are considered by DEFAULT" << std::endl;
            mELementalFlags.Set(NodalConcentratedElement::COMPUTE_NODAL_MASS, true);
        } else
            mELementalFlags.Set(NodalConcentratedElement::COMPUTE_NODAL_MASS, false);

        // We check the nodal inertia
        if (GetGeometry()[0].SolutionStepsDataHas(ROTATION_X) &&
            (rconst_this.Has(NODAL_INERTIA) || GetProperties().Has(NODAL_INERTIA))) {
            KRATOS_WARNING_IF("NodalConcentratedElement", rconst_this.Has(NODAL_INERTIA) && GetProperties().Has(NODAL_INERTIA)) << "NODAL_INERTIA is defined both in properties and elemental data. Properties are considered by DEFAULT" << std::endl;
            mELementalFlags.Set(NodalConcentratedElement::COMPUTE_NODAL_INERTIA, true);
        } else
            mELementalFlags.Set(NodalConcentratedElement::COMPUTE_NODAL_INERTIA, false);

        // We check the damping ratio
        if (rconst_this.Has(NODAL_DAMPING_RATIO) || GetProperties().Has(NODAL_DAMPING_RATIO)) {
            KRATOS_WARNING_IF("NodalConcentratedElement", rconst_this.Has(NODAL_DAMPING_RATIO) && GetProperties().Has(NODAL_DAMPING_RATIO)) << "NODAL_DAMPING_RATIO is defined both in properties and elemental data. Properties are considered by DEFAULT" << std::endl;
            mELementalFlags.Set(NodalConcentratedElement::COMPUTE_DAMPING_RATIO, true);
        } else
            mELementalFlags.Set(NodalConcentratedElement::COMPUTE_DAMPING_RATIO, false);

        // We check the rotational damping ratio
        if (GetGeometry()[0].SolutionStepsDataHas(ROTATION_X) &&
            (rconst_this.Has(NODAL_ROTATIONAL_DAMPING_RATIO) || GetProperties().Has(NODAL_ROTATIONAL_DAMPING_RATIO))) {
            KRATOS_WARNING_IF("NodalConcentratedElement", rconst_this.Has(NODAL_ROTATIONAL_DAMPING_RATIO) && GetProperties().Has(NODAL_ROTATIONAL_DAMPING_RATIO)) << "NODAL_ROTATIONAL_DAMPING_RATIO is defined both in properties and elemental data. Properties are considered by DEFAULT" << std::endl;
            mELementalFlags.Set(NodalConcentratedElement::COMPUTE_ROTATIONAL_DAMPING_RATIO, true);
        } else
            mELementalFlags.Set(NodalConcentratedElement::COMPUTE_ROTATIONAL_DAMPING_RATIO, false);
    } else {
        // We check the nodal stiffness
        if (rconst_this.Has(NODAL_DISPLACEMENT_STIFFNESS))
            mELementalFlags.Set(NodalConcentratedElement::COMPUTE_DISPLACEMENT_STIFFNESS, true);
        else
            mELementalFlags.Set(NodalConcentratedElement::COMPUTE_DISPLACEMENT_STIFFNESS, false);

        // We check the nodal rotational stiffness
        if (GetGeometry()[0].SolutionStepsDataHas(ROTATION_X) &&
            rconst_this.Has(NODAL_ROTATIONAL_STIFFNESS))
            mELementalFlags.Set(NodalConcentratedElement::COMPUTE_ROTATIONAL_STIFFNESS, true);
        else
            mELementalFlags.Set(NodalConcentratedElement::COMPUTE_ROTATIONAL_STIFFNESS, false);

        // We check the nodal mass
        if (rconst_this.Has(NODAL_MASS))
            mELementalFlags.Set(NodalConcentratedElement::COMPUTE_NODAL_MASS, true);
        else
            mELementalFlags.Set(NodalConcentratedElement::COMPUTE_NODAL_MASS, false);

        // We check the nodal inertia
        if (GetGeometry()[0].SolutionStepsDataHas(ROTATION_X) &&
            rconst_this.Has(NODAL_INERTIA))
            mELementalFlags.Set(NodalConcentratedElement::COMPUTE_NODAL_INERTIA, true);
        else
            mELementalFlags.Set(NodalConcentratedElement::COMPUTE_NODAL_INERTIA, false);

        // We check the damping ratio
        if (rconst_this.Has(NODAL_DAMPING_RATIO))
            mELementalFlags.Set(NodalConcentratedElement::COMPUTE_DAMPING_RATIO, true);
        else
            mELementalFlags.Set(NodalConcentratedElement::COMPUTE_DAMPING_RATIO, false);

        // We check the rotational damping ratio
        if (GetGeometry()[0].SolutionStepsDataHas(ROTATION_X) &&
            rconst_this.Has(NODAL_ROTATIONAL_DAMPING_RATIO))
            mELementalFlags.Set(NodalConcentratedElement::COMPUTE_ROTATIONAL_DAMPING_RATIO, true);
        else
            mELementalFlags.Set(NodalConcentratedElement::COMPUTE_ROTATIONAL_DAMPING_RATIO, false);
    }

    KRATOS_CATCH( "" );
}

/***********************************************************************************/
/***********************************************************************************/

void NodalConcentratedElement::InitializeSolutionStep( const ProcessInfo& rCurrentProcessInfo )
{
    KRATOS_TRY;

    BaseType::InitializeSolutionStep(rCurrentProcessInfo);

    KRATOS_CATCH( "" );
}

/***********************************************************************************/
/***********************************************************************************/

void NodalConcentratedElement::InitializeNonLinearIteration( const ProcessInfo& rCurrentProcessInfo )
{
    KRATOS_TRY;

    BaseType::InitializeNonLinearIteration(rCurrentProcessInfo);

    KRATOS_CATCH( "" );
}

/***********************************************************************************/
/***********************************************************************************/

void NodalConcentratedElement::FinalizeNonLinearIteration( const ProcessInfo& rCurrentProcessInfo )
{
    KRATOS_TRY;

    BaseType::FinalizeNonLinearIteration(rCurrentProcessInfo);

    KRATOS_CATCH( "" );
}

/***********************************************************************************/
/***********************************************************************************/

void NodalConcentratedElement::FinalizeSolutionStep( const ProcessInfo& rCurrentProcessInfo )
{
    KRATOS_TRY;

    BaseType::FinalizeSolutionStep(rCurrentProcessInfo);

    KRATOS_CATCH( "" );
}

//************* COMPUTING  METHODS
/***********************************************************************************/
/***********************************************************************************/

void NodalConcentratedElement::CalculateLocalSystem(
    MatrixType& rLeftHandSideMatrix,
    VectorType& rRightHandSideVector,
    const ProcessInfo& rCurrentProcessInfo
    )
{
    KRATOS_TRY;

    /* Calculate elemental system */

    // Compute RHS (RHS = rRightHandSideVector = Fext - Fint)
    this->CalculateRightHandSide(rRightHandSideVector, rCurrentProcessInfo);

    // Compute LHS
    this->CalculateLeftHandSide(rLeftHandSideMatrix, rCurrentProcessInfo);

    KRATOS_CATCH( "" );
}

/***********************************************************************************/
/***********************************************************************************/

void NodalConcentratedElement::CalculateRightHandSide(
    VectorType& rRightHandSideVector,
    const ProcessInfo& rCurrentProcessInfo
    )
{
    const SizeType dimension = GetGeometry().WorkingSpaceDimension();

    // Resizing as needed the RHS
    const SizeType system_size = this->ComputeSizeOfSystem();

    if ( rRightHandSideVector.size() != system_size )
        rRightHandSideVector.resize( system_size, false );

    rRightHandSideVector = ZeroVector( system_size ); //resetting RHS

    // Auxiliar value
    const array_1d<double, 3> zero_array = ZeroVector(3);

    // We get the reference
    const auto& rconst_this = *this;

    // Volume acceleration
    if (mELementalFlags.Is(NodalConcentratedElement::COMPUTE_NODAL_MASS) && GetGeometry()[0].SolutionStepsDataHas(VOLUME_ACCELERATION)) {
        const array_1d<double, 3 >& volume_acceleration = GetGeometry()[0].FastGetSolutionStepValue(VOLUME_ACCELERATION);

        // Compute and add external forces
        const double nodal_mass = HasProperties() ? (GetProperties().Has(NODAL_MASS) ? GetProperties().GetValue(NODAL_MASS) : rconst_this.GetValue(NODAL_MASS)) : rconst_this.GetValue(NODAL_MASS);

        for ( IndexType j = 0; j < dimension; ++j )
            rRightHandSideVector[j]  += volume_acceleration[j] * nodal_mass;
    }

    // Auxiliar index
    IndexType aux_index = 0;

    // The displacement terms
    if( mELementalFlags.Is(NodalConcentratedElement::COMPUTE_DISPLACEMENT_STIFFNESS) ||
        mELementalFlags.Is(NodalConcentratedElement::COMPUTE_NODAL_MASS)) {

        // Compute and add internal forces
        const array_1d<double, 3 >& current_displacement = GetGeometry()[0].FastGetSolutionStepValue(DISPLACEMENT);
        const array_1d<double, 3 >& initial_displacement = this->Has(NODAL_INITIAL_DISPLACEMENT) ? this->GetValue(NODAL_INITIAL_DISPLACEMENT) : zero_array;
        const array_1d<double, 3 >& nodal_stiffness = HasProperties() ? (GetProperties().Has(NODAL_DISPLACEMENT_STIFFNESS) ? GetProperties().GetValue(NODAL_DISPLACEMENT_STIFFNESS) : rconst_this.GetValue(NODAL_DISPLACEMENT_STIFFNESS)) : rconst_this.GetValue(NODAL_DISPLACEMENT_STIFFNESS);

        for ( IndexType j = 0; j < dimension; ++j )
            rRightHandSideVector[j]  -= nodal_stiffness[j] * (current_displacement[j] - initial_displacement[j]);

        aux_index += dimension;
    }

    // The rotational terms
    if( mELementalFlags.Is(NodalConcentratedElement::COMPUTE_ROTATIONAL_STIFFNESS) ||
        mELementalFlags.Is(NodalConcentratedElement::COMPUTE_NODAL_INERTIA)) {

        // Compute and add internal forces
        const array_1d<double, 3 >& current_rotation = GetGeometry()[0].FastGetSolutionStepValue(ROTATION);
        const array_1d<double, 3 >& initial_rotation = this->Has(NODAL_INITIAL_ROTATION) ? this->GetValue(NODAL_INITIAL_ROTATION) : zero_array;
        const array_1d<double, 3 >& nodal_rotational_stiffness = HasProperties() ? (GetProperties().Has(NODAL_ROTATIONAL_STIFFNESS) ? GetProperties().GetValue(NODAL_ROTATIONAL_STIFFNESS) : rconst_this.GetValue(NODAL_ROTATIONAL_STIFFNESS)) : rconst_this.GetValue(NODAL_ROTATIONAL_STIFFNESS);

        for ( IndexType j = 0; j < dimension; ++j )
            rRightHandSideVector[aux_index + j]  -= nodal_rotational_stiffness[j] * (current_rotation[j] - initial_rotation[j]);
    }
}

/***********************************************************************************/
/***********************************************************************************/

void NodalConcentratedElement::CalculateLeftHandSide(
    MatrixType& rLeftHandSideMatrix,
    const ProcessInfo& rCurrentProcessInfo
    )
{
    const SizeType dimension = GetGeometry().WorkingSpaceDimension();

    // Resizing as needed the LHS
    const SizeType system_size = this->ComputeSizeOfSystem();

    if ( rLeftHandSideMatrix.size1() != system_size )
        rLeftHandSideMatrix.resize( system_size, system_size, false );

    noalias( rLeftHandSideMatrix ) = ZeroMatrix( system_size, system_size ); //resetting LHS

    // We get the reference
    const auto& rconst_this = *this;

    // Auxiliar index
    IndexType aux_index = 0;

    // The displacement terms
    if( mELementalFlags.Is(NodalConcentratedElement::COMPUTE_DISPLACEMENT_STIFFNESS) ||
        mELementalFlags.Is(NodalConcentratedElement::COMPUTE_NODAL_MASS)) {

        const array_1d<double, 3 >& nodal_stiffness = HasProperties() ? (GetProperties().Has(NODAL_DISPLACEMENT_STIFFNESS) ? GetProperties().GetValue(NODAL_DISPLACEMENT_STIFFNESS) : rconst_this.GetValue(NODAL_DISPLACEMENT_STIFFNESS)) : rconst_this.GetValue(NODAL_DISPLACEMENT_STIFFNESS);

        for ( IndexType j = 0; j < dimension; ++j )
            rLeftHandSideMatrix( j, j ) += nodal_stiffness[j];

        aux_index += dimension;
    }

    // The rotational terms
    if( mELementalFlags.Is(NodalConcentratedElement::COMPUTE_ROTATIONAL_STIFFNESS) ||
        mELementalFlags.Is(NodalConcentratedElement::COMPUTE_NODAL_INERTIA)) {

        const array_1d<double, 3 >& nodal_rotational_stiffness = HasProperties() ? (GetProperties().Has(NODAL_ROTATIONAL_STIFFNESS) ? GetProperties().GetValue(NODAL_ROTATIONAL_STIFFNESS) : rconst_this.GetValue(NODAL_ROTATIONAL_STIFFNESS)) : rconst_this.GetValue(NODAL_ROTATIONAL_STIFFNESS);

        for ( IndexType j = 0; j < dimension; ++j )
            rLeftHandSideMatrix( aux_index + j, aux_index + j ) += nodal_rotational_stiffness[j];
    }
}

/***********************************************************************************/
/***********************************************************************************/

void NodalConcentratedElement::AddExplicitContribution(
    const VectorType& rRHSVector,
    const Variable<VectorType>& rRHSVariable,
    Variable<double>& rDestinationVariable,
    const ProcessInfo& rCurrentProcessInfo
    )
{
    KRATOS_TRY;

    auto& r_geom = this->GetGeometry();

    // We get the reference
    const auto& rconst_this = *this;

    // The displacement terms
    // Computing the nodal mass
    if (rDestinationVariable == NODAL_MASS ) {
        if( mELementalFlags.Is(NodalConcentratedElement::COMPUTE_DISPLACEMENT_STIFFNESS) ||
            mELementalFlags.Is(NodalConcentratedElement::COMPUTE_NODAL_MASS)) {

            const double nodal_mass = HasProperties() ? (GetProperties().Has(NODAL_MASS) ? GetProperties().GetValue(NODAL_MASS) : rconst_this.GetValue(NODAL_MASS)) : rconst_this.GetValue(NODAL_MASS);

            #pragma omp atomic
            r_geom[0].GetValue(NODAL_MASS) += nodal_mass;

        }
    }

    // The rotational terms
    // Computing the nodal inertia
    if (rDestinationVariable == NODAL_INERTIA ) {
        if( mELementalFlags.Is(NodalConcentratedElement::COMPUTE_ROTATIONAL_STIFFNESS) ||
            mELementalFlags.Is(NodalConcentratedElement::COMPUTE_NODAL_INERTIA)) {

            const array_1d<double, 3>& const_nodal_inertia =  HasProperties() ? (GetProperties().Has(NODAL_INERTIA) ? GetProperties().GetValue(NODAL_INERTIA) : rconst_this.GetValue(NODAL_INERTIA)) : rconst_this.GetValue(NODAL_INERTIA);

            array_1d<double, 3>& r_nodal_inertia = r_geom[0].GetValue(NODAL_INERTIA);

            for (IndexType i_dim = 0; i_dim < 3; ++i_dim) {
                #pragma omp atomic
                r_nodal_inertia[i_dim] += const_nodal_inertia[i_dim];
            }
        }
    }

    KRATOS_CATCH("")
}

/***********************************************************************************/
/***********************************************************************************/

void NodalConcentratedElement::AddExplicitContribution(
    const VectorType& rRHSVector,
    const Variable<VectorType>& rRHSVariable,
    Variable<array_1d<double, 3>>& rDestinationVariable,
    const ProcessInfo& rCurrentProcessInfo
    )
{
    KRATOS_TRY;

    auto& r_geom = this->GetGeometry();
    const SizeType dimension = r_geom.WorkingSpaceDimension();
    const SizeType system_size = this->ComputeSizeOfSystem();

    Vector damping_residual_contribution = ZeroVector(system_size);

    // Calculate damping contribution to residual -->
    if (mELementalFlags.Is(NodalConcentratedElement::COMPUTE_DAMPING_RATIO) || mELementalFlags.Is(NodalConcentratedElement::COMPUTE_ROTATIONAL_DAMPING_RATIO)) {
        Vector current_nodal_velocities = ZeroVector(system_size);
        this->GetFirstDerivativesVector(current_nodal_velocities);

        Matrix damping_matrix = ZeroMatrix(system_size, system_size);
        this->CalculateDampingMatrix(damping_matrix, rCurrentProcessInfo);

        // Current residual contribution due to damping
        noalias(damping_residual_contribution) = prod(damping_matrix, current_nodal_velocities);
    }

    // Auxiliar index
    IndexType aux_index = 0;

    // Computing the force residual
    if (rRHSVariable == RESIDUAL_VECTOR && rDestinationVariable == FORCE_RESIDUAL) {
        if( mELementalFlags.Is(NodalConcentratedElement::COMPUTE_DISPLACEMENT_STIFFNESS) ||
            mELementalFlags.Is(NodalConcentratedElement::COMPUTE_NODAL_MASS)) {

            array_1d<double, 3>& r_force_residual = r_geom[0].FastGetSolutionStepValue(FORCE_RESIDUAL);

            for (IndexType j = 0; j < dimension; ++j) {
                #pragma omp atomic
                r_force_residual[j] += rRHSVector[j] - damping_residual_contribution[j];
            }

            aux_index += dimension;
        }
    }

    if (rRHSVariable == RESIDUAL_VECTOR && rDestinationVariable == MOMENT_RESIDUAL) {
        if( mELementalFlags.Is(NodalConcentratedElement::COMPUTE_ROTATIONAL_STIFFNESS) ||
            mELementalFlags.Is(NodalConcentratedElement::COMPUTE_NODAL_INERTIA)) {

            array_1d<double, 3>& r_moment_residual = r_geom[0].FastGetSolutionStepValue(MOMENT_RESIDUAL);

            for (IndexType j = 0; j < dimension; ++j) {
                #pragma omp atomic
                r_moment_residual[j] += rRHSVector[aux_index + j] - damping_residual_contribution[aux_index + j];
            }
        }
    }

    KRATOS_CATCH("")
}

//*************************COMPUTE DELTA POSITION*************************************
/***********************************************************************************/

void NodalConcentratedElement::CalculateMassMatrix(
    MatrixType& rMassMatrix,
    const ProcessInfo& rCurrentProcessInfo
    )
{
    KRATOS_TRY

    // Lumped (by definition)
    const SizeType dimension = GetGeometry().WorkingSpaceDimension();
    const SizeType system_size = this->ComputeSizeOfSystem();

    if ( rMassMatrix.size1() != system_size )
        rMassMatrix.resize( system_size, system_size, false );

    rMassMatrix = ZeroMatrix( system_size, system_size );

    // We get the reference
    const auto& rconst_this = *this;

    // Auxiliar index
    IndexType aux_index = 0;

    // The displacement terms
    if( mELementalFlags.Is(NodalConcentratedElement::COMPUTE_DISPLACEMENT_STIFFNESS) ||
        mELementalFlags.Is(NodalConcentratedElement::COMPUTE_NODAL_MASS)) {

        const double nodal_mass = HasProperties() ? (GetProperties().Has(NODAL_MASS) ? GetProperties().GetValue(NODAL_MASS) : rconst_this.GetValue(NODAL_MASS)) : rconst_this.GetValue(NODAL_MASS);

        for ( IndexType j = 0; j < dimension; ++j )
            rMassMatrix( j, j ) += nodal_mass;

        aux_index += dimension;
    }

    // The rotational terms
    if( mELementalFlags.Is(NodalConcentratedElement::COMPUTE_ROTATIONAL_STIFFNESS) ||
        mELementalFlags.Is(NodalConcentratedElement::COMPUTE_NODAL_INERTIA)) {

        const array_1d<double, 3>& r_nodal_inertia =  HasProperties() ? (GetProperties().Has(NODAL_INERTIA) ? GetProperties().GetValue(NODAL_INERTIA) : rconst_this.GetValue(NODAL_INERTIA)) : rconst_this.GetValue(NODAL_INERTIA);

        for ( IndexType j = 0; j < dimension; ++j )
            rMassMatrix( aux_index + j, aux_index + j ) += r_nodal_inertia[j];
    }

    KRATOS_CATCH( "" );
}

/***********************************************************************************/
/***********************************************************************************/

void NodalConcentratedElement::CalculateDampingMatrix(
    MatrixType& rDampingMatrix,
    const ProcessInfo& rCurrentProcessInfo
    )
{
    KRATOS_TRY;

    // The domain size
    const SizeType dimension = GetGeometry().WorkingSpaceDimension();

    // Resizing as needed the LHS
    const SizeType system_size = this->ComputeSizeOfSystem();

    // 0.-Initialize the DampingMatrix:
    rDampingMatrix = ZeroMatrix( system_size, system_size );

    //Check, if Rayleigh damping is available; use nodal damping, if not
    if( mELementalFlags.Is(NodalConcentratedElement::COMPUTE_RAYLEIGH_DAMPING) ) {
        StructuralMechanicsElementUtilities::CalculateRayleighDampingMatrix(*this, rDampingMatrix, const_cast<ProcessInfo&>(rCurrentProcessInfo), system_size);
    } else {
        // We get the reference
        const auto& rconst_this = *this;

        IndexType aux_index = 0;
        if( mELementalFlags.Is(NodalConcentratedElement::COMPUTE_DAMPING_RATIO) ) {
            const array_1d<double, 3 >& nodal_damping_ratio = HasProperties() ? (GetProperties().Has(NODAL_DAMPING_RATIO) ? GetProperties().GetValue(NODAL_DAMPING_RATIO) : rconst_this.GetValue(NODAL_DAMPING_RATIO)) : rconst_this.GetValue(NODAL_DAMPING_RATIO);
            for ( IndexType j = 0; j < dimension; ++j )
                rDampingMatrix(j, j) += nodal_damping_ratio[j];

            aux_index += dimension;
        }
        if( mELementalFlags.Is(NodalConcentratedElement::COMPUTE_ROTATIONAL_DAMPING_RATIO) ) {
            const array_1d<double, 3 >& nodal_rotational_damping_ratio = HasProperties() ? (GetProperties().Has(NODAL_ROTATIONAL_DAMPING_RATIO) ? GetProperties().GetValue(NODAL_ROTATIONAL_DAMPING_RATIO) : rconst_this.GetValue(NODAL_ROTATIONAL_DAMPING_RATIO)) : rconst_this.GetValue(NODAL_ROTATIONAL_DAMPING_RATIO);
            for ( IndexType j = 0; j < dimension; ++j )
                rDampingMatrix(aux_index + j, aux_index+ j) += nodal_rotational_damping_ratio[j];
        }
    }

    KRATOS_CATCH( "" );
}

/***********************************************************************************/
/***********************************************************************************/

void NodalConcentratedElement::CalculateLocalSystem(
    MatrixType& rLeftHandSideMatrix,
    VectorType& rRightHandSideVector,
    ProcessInfo& rCurrentProcessInfo
    )
{
    KRATOS_TRY;

    const ProcessInfo& r_const_current_process_info = rCurrentProcessInfo;
    this->CalculateLocalSystem(rLeftHandSideMatrix, rRightHandSideVector, r_const_current_process_info);

    KRATOS_CATCH( "" );
}

/***********************************************************************************/
/***********************************************************************************/

void NodalConcentratedElement::CalculateRightHandSide(
    VectorType& rRightHandSideVector,
    ProcessInfo& rCurrentProcessInfo
    )
{
    const ProcessInfo& r_const_current_process_info = rCurrentProcessInfo;
    this->CalculateRightHandSide(rRightHandSideVector, r_const_current_process_info);
}

/***********************************************************************************/
/***********************************************************************************/

void NodalConcentratedElement::CalculateLeftHandSide(
    MatrixType& rLeftHandSideMatrix,
    ProcessInfo& rCurrentProcessInfo
    )
{
    const ProcessInfo& r_const_current_process_info = rCurrentProcessInfo;
    this->CalculateLeftHandSide(rLeftHandSideMatrix, r_const_current_process_info);
}

//*************************COMPUTE DELTA POSITION*************************************
/***********************************************************************************/

void NodalConcentratedElement::CalculateMassMatrix(
    MatrixType& rMassMatrix,
    ProcessInfo& rCurrentProcessInfo
    )
{
    const ProcessInfo& r_const_current_process_info = rCurrentProcessInfo;
    this->CalculateMassMatrix(rMassMatrix, r_const_current_process_info);
}

/***********************************************************************************/
/***********************************************************************************/

void NodalConcentratedElement::CalculateDampingMatrix(
    MatrixType& rDampingMatrix,
    ProcessInfo& rCurrentProcessInfo
    )
{
    const ProcessInfo& r_const_current_process_info = rCurrentProcessInfo;
    this->CalculateDampingMatrix(rDampingMatrix, r_const_current_process_info);
}

/***********************************************************************************/
/***********************************************************************************/

std::size_t NodalConcentratedElement::ComputeSizeOfSystem() const
{
    SizeType system_size = 0;

    const SizeType dimension = GetGeometry().WorkingSpaceDimension();

    // The displacement terms
    if( mELementalFlags.Is(NodalConcentratedElement::COMPUTE_DISPLACEMENT_STIFFNESS) ||
        mELementalFlags.Is(NodalConcentratedElement::COMPUTE_NODAL_MASS)) {
        system_size += dimension;
    }

    // The rotational terms
    if( mELementalFlags.Is(NodalConcentratedElement::COMPUTE_ROTATIONAL_STIFFNESS) ||
        mELementalFlags.Is(NodalConcentratedElement::COMPUTE_NODAL_INERTIA)) {
        system_size += dimension;
    }

    return system_size;
}

/***********************************************************************************/
/***********************************************************************************/

int NodalConcentratedElement::Check( const ProcessInfo& rCurrentProcessInfo )
{
    KRATOS_TRY

    // Check that all required variables have been registered

    // The displacement terms
    if( mELementalFlags.Is(NodalConcentratedElement::COMPUTE_DISPLACEMENT_STIFFNESS) ||
        mELementalFlags.Is(NodalConcentratedElement::COMPUTE_NODAL_MASS)) {
        KRATOS_CHECK_VARIABLE_KEY(DISPLACEMENT)
        KRATOS_CHECK_VARIABLE_KEY(VELOCITY)
        KRATOS_CHECK_VARIABLE_KEY(ACCELERATION)
        KRATOS_CHECK_VARIABLE_KEY(NODAL_MASS)
        KRATOS_CHECK_VARIABLE_KEY(NODAL_DISPLACEMENT_STIFFNESS)
    }

        // The rotational terms
    if( mELementalFlags.Is(NodalConcentratedElement::COMPUTE_ROTATIONAL_STIFFNESS) ||
        mELementalFlags.Is(NodalConcentratedElement::COMPUTE_NODAL_INERTIA)) {
        KRATOS_CHECK_VARIABLE_KEY(ROTATION)
        KRATOS_CHECK_VARIABLE_KEY(ANGULAR_VELOCITY)
        KRATOS_CHECK_VARIABLE_KEY(ANGULAR_ACCELERATION)
        KRATOS_CHECK_VARIABLE_KEY(NODAL_INERTIA)
        KRATOS_CHECK_VARIABLE_KEY(NODAL_ROTATIONAL_STIFFNESS)
    }

    KRATOS_CHECK_VARIABLE_KEY(NODAL_DAMPING_RATIO)
    KRATOS_CHECK_VARIABLE_KEY(NODAL_ROTATIONAL_DAMPING_RATIO)
    KRATOS_CHECK_VARIABLE_KEY(VOLUME_ACCELERATION)

    // Check that the element's nodes contain all required SolutionStepData and Degrees of freedom
    NodeType& rnode = this->GetGeometry()[0];

    // The displacement terms
    if( mELementalFlags.Is(NodalConcentratedElement::COMPUTE_DISPLACEMENT_STIFFNESS) ||
        mELementalFlags.Is(NodalConcentratedElement::COMPUTE_NODAL_MASS)) {
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(DISPLACEMENT,rnode)
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(VOLUME_ACCELERATION,rnode)

        KRATOS_CHECK_DOF_IN_NODE(DISPLACEMENT_X,rnode)
        KRATOS_CHECK_DOF_IN_NODE(DISPLACEMENT_Y,rnode)
        KRATOS_CHECK_DOF_IN_NODE(DISPLACEMENT_Z,rnode)
    }

    // The rotational terms
    if( mELementalFlags.Is(NodalConcentratedElement::COMPUTE_ROTATIONAL_STIFFNESS) ||
        mELementalFlags.Is(NodalConcentratedElement::COMPUTE_NODAL_INERTIA)) {
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(ROTATION,rnode)

        KRATOS_CHECK_DOF_IN_NODE(ROTATION_X,rnode)
        KRATOS_CHECK_DOF_IN_NODE(ROTATION_Y,rnode)
        KRATOS_CHECK_DOF_IN_NODE(ROTATION_Z,rnode)
    }

    return 0;

    KRATOS_CATCH( "Problem in the Check in the NodalConcentratedElement" )
}


/***********************************************************************************/
/***********************************************************************************/

void NodalConcentratedElement::save( Serializer& rSerializer ) const
{
    KRATOS_SERIALIZE_SAVE_BASE_CLASS( rSerializer, Element )
    rSerializer.save("ELementalFlags",mELementalFlags);
}

void NodalConcentratedElement::load( Serializer& rSerializer )
{
    KRATOS_SERIALIZE_LOAD_BASE_CLASS( rSerializer, Element )
    rSerializer.load("ELementalFlags",mELementalFlags);
}

} // Namespace Kratos

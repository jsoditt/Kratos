//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:         BSD License
//                   Kratos default license: kratos/license.txt
//
//  Main authors:    Tobias Teschemacher
//

#if !defined(KRATOS_SUPPORT_PENALTY_CONDITION_H_INCLUDED )
#define  KRATOS_SUPPORT_PENALTY_CONDITION_H_INCLUDED


// System includes
#include "includes/define.h"
#include "includes/condition.h"

// External includes

// Project includes
#include "iga_application_variables.h"
#include "custom_utilities/iga_flags.h"

namespace Kratos
{
    /// Condition for basic load types
    /*
    * @brief This condition use the geometrical information, which
    *        is provided from the respective geometry. The respective
    *        loads are set to the data container.
    */
    class SupportPenaltyCondition
        : public Condition
    {
    public:
        ///@name Type Definitions
        ///@{

        /// Counted pointer definition of SupportPenaltyCondition
        KRATOS_CLASS_INTRUSIVE_POINTER_DEFINITION(SupportPenaltyCondition);

        /// Size types
        typedef std::size_t SizeType;
        typedef std::size_t IndexType;

        ///@}
        ///@name Life Cycle
        ///@{

        /// Constructor with Id and geometry
        SupportPenaltyCondition(
            IndexType NewId,
            GeometryType::Pointer pGeometry)
            : Condition(NewId, pGeometry)
        {};

        /// Constructor with Id, geometry and property
        SupportPenaltyCondition(
            IndexType NewId,
            GeometryType::Pointer pGeometry,
            PropertiesType::Pointer pProperties)
            : Condition(NewId, pGeometry, pProperties)
        {};

        /// Default constructor
        SupportPenaltyCondition() : Condition()
        {};

        /// Destructor
        virtual ~SupportPenaltyCondition() override
        {};

        ///@}
        ///@name Life Cycle
        ///@{

        /// Create with Id, pointer to geometry and pointer to property
        Condition::Pointer Create(
            IndexType NewId,
            GeometryType::Pointer pGeom,
            PropertiesType::Pointer pProperties
        ) const override
        {
            return Kratos::make_intrusive<SupportPenaltyCondition>(
                NewId, pGeom, pProperties);
        };

        /// Create with Id, pointer to geometry and pointer to property
        Condition::Pointer Create(
            IndexType NewId,
            NodesArrayType const& ThisNodes,
            PropertiesType::Pointer pProperties
        ) const override
        {
            return Kratos::make_intrusive< SupportPenaltyCondition >(
                NewId, GetGeometry().Create(ThisNodes), pProperties);
        };

        ///@}
        ///@name Operations
        ///@{

        /**
        * @brief This is called during the assembling process in order
        *        to calculate the condition right hand side matrix
        * @param rLeftHandSideMatrix the condition right hand side matrix
        * @param rCurrentProcessInfo the current process info
        */
        void CalculateRightHandSide(
            VectorType& rRightHandSideVector,
            ProcessInfo& rCurrentProcessInfo) override
        {
            MatrixType left_hand_side_matrix = Matrix(0, 0);

            CalculateAll(left_hand_side_matrix, rRightHandSideVector,
                rCurrentProcessInfo, false, true);
        }

        /**
        * @brief This is called during the assembling process in order
        *        to calculate the condition left hand side matrix
        * @param rLeftHandSideMatrix the condition left hand side matrix
        * @param rCurrentProcessInfo the current process info
        */
        void CalculateLeftHandSide(
            MatrixType& rLeftHandSideMatrix,
            ProcessInfo& rCurrentProcessInfo) override
        {
            VectorType right_hand_side_vector = Vector(0);

            CalculateAll(rLeftHandSideMatrix, right_hand_side_vector,
                rCurrentProcessInfo, true, false);
        }

        /**
         * @brief This function provides a more general interface to the element.
         * @details It is designed so that rLHSvariables and rRHSvariables are
         *          passed to the element thus telling what is the desired output
         * @param rLeftHandSideMatrix container with the output Left Hand Side matrix
         * @param rRightHandSideVector container for the desired RHS output
         * @param rCurrentProcessInfo the current process info instance
         */
        void CalculateLocalSystem(
            MatrixType& rLeftHandSideMatrix,
            VectorType& rRightHandSideVector,
            ProcessInfo& rCurrentProcessInfo) override
        {
            CalculateAll(rLeftHandSideMatrix, rRightHandSideVector,
                rCurrentProcessInfo, true, true);
        }

        /**
        * @brief Sets on rResult the ID's of the element degrees of freedom
        * @param rResult The vector containing the equation id
        * @param rCurrentProcessInfo The current process info instance
        */
        void EquationIdVector(
            EquationIdVectorType& rResult,
            ProcessInfo& rCurrentProcessInfo
        ) override;

        /**
        * @brief Sets on rConditionDofList the degrees of freedom of the considered element geometry
        * @param rElementalDofList The vector containing the dof of the element
        * @param rCurrentProcessInfo The current process info instance
        */
        void GetDofList(
            DofsVectorType& rElementalDofList,
            ProcessInfo& rCurrentProcessInfo
        ) override;

        /**
        * This functions calculates both the RHS and the LHS
        * @param rLeftHandSideMatrix: The LHS
        * @param rRightHandSideVector: The RHS
        * @param rCurrentProcessInfo: The current process info instance
        * @param CalculateStiffnessMatrixFlag: The flag to set if compute the LHS
        * @param CalculateResidualVectorFlag: The flag to set if compute the RHS
        */
        void CalculateAll(
            MatrixType& rLeftHandSideMatrix,
            VectorType& rRightHandSideVector,
            ProcessInfo& rCurrentProcessInfo,
            const bool CalculateStiffnessMatrixFlag,
            const bool CalculateResidualVectorFlag
        );

        void DeterminantOfJacobianInitial(
            const GeometryType& rGeometry,
            Vector& rDeterminantOfJacobian);

        ///@}
        ///@name Check
        ///@{

        /**
        * This function provides the place to perform checks on the completeness of the input.
        * It is designed to be called only once (or anyway, not often) typically at the beginning
        * of the calculations, so to verify that nothing is missing from the input
        * or that no common error is found.
        * @param rCurrentProcessInfo
        */
        int Check(const ProcessInfo& rCurrentProcessInfo) override;

        ///@}
        ///@name Input and output
        ///@{

        /// Turn back information as a string.
        std::string Info() const override
        {
            std::stringstream buffer;
            buffer << "\"SupportPenaltyCondition\" #" << Id();
            return buffer.str();
        }

        /// Print information about this object.
        void PrintInfo(std::ostream& rOStream) const override
        {
            rOStream << "\"SupportPenaltyCondition\" #" << Id();
        }

        /// Print object's data.
        void PrintData(std::ostream& rOStream) const override
        {
            pGetGeometry()->PrintData(rOStream);
        }

        ///@}

    private:
        ///@name Serialization
        ///@{

        friend class Serializer;

        virtual void save(Serializer& rSerializer) const override
        {
            KRATOS_SERIALIZE_SAVE_BASE_CLASS(rSerializer, Condition);
        }

        virtual void load(Serializer& rSerializer) override
        {
            KRATOS_SERIALIZE_LOAD_BASE_CLASS(rSerializer, Condition);
        }

        ///@}

    }; // Class SupportPenaltyCondition

}  // namespace Kratos.

#endif // KRATOS_SUPPORT_PENALTY_CONDITION_H_INCLUDED  defined
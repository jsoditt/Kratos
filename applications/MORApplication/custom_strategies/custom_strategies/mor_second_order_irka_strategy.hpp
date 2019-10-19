//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ \.
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Riccardo Rossi
//

#if !defined(MOR_SECOND_ORDER_IRKA_STRATEGY)
#define MOR_SECOND_ORDER_IRKA_STRATEGY

// System includes

// External includes

// Project includes
#include "includes/define.h"
// #include "solving_strategies/strategies/solving_strategy.h"
// #include "solving_strategies/convergencecriterias/convergence_criteria.h"
#include "utilities/builtin_timer.h"
#include "utilities/qr_utility.h"
#include "custom_strategies/custom_strategies/mor_offline_second_order_strategy.hpp"

//default builder and solver
#include "custom_strategies/custom_builder_and_solvers/system_matrix_builder_and_solver.hpp"

namespace Kratos
{
    using complex = std::complex<double>;

///@name Kratos Globals
///@{

///@}
///@name Type Definitions
///@{

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
 * @class MorSecondOrderIRKAStrategy
 * @ingroup KratosCore
 * @brief This is the Iterative Rational Krylov Algorithm
 * @details This strategy builds the reduced K and M matrices and outputs them
 * @author Matthias Ebert, based on code of Quirin Aumann
 */
template <class TSparseSpace,
          class TDenseSpace,  // = DenseSpace<double>,
          class TLinearSolver //= LinearSolver<TSparseSpace,TDenseSpace>
          >
class MorSecondOrderIRKAStrategy
    // : public SolvingStrategy<TSparseSpace, TDenseSpace, TLinearSolver>
    : public MorOfflineSecondOrderStrategy< TSparseSpace, TDenseSpace, TLinearSolver >
{
  public:
    ///@name Type Definitions
    ///@{
    // Counted pointer of ClassName
    KRATOS_CLASS_POINTER_DEFINITION(MorSecondOrderIRKAStrategy);

    typedef TUblasSparseSpace<complex> ComplexSparseSpaceType;
    typedef TUblasDenseSpace<complex> ComplexDenseSpaceType;

    // typedef SolvingStrategy<TSparseSpace, TDenseSpace, TLinearSolver> BaseType;
    typedef MorOfflineSecondOrderStrategy<TSparseSpace, TDenseSpace, TLinearSolver> BaseType;

    typedef SystemMatrixBuilderAndSolver< TSparseSpace, TDenseSpace, TLinearSolver > TBuilderAndSolverType;

    typedef typename BaseType::TDataType TDataType;

    typedef TSparseSpace SparseSpaceType;

    typedef TDenseSpace DenseSpaceType;

    typedef typename TDenseSpace::VectorType TDenseVectorType;

    typedef typename TDenseSpace::MatrixType TDenseMatrixType;

    typedef typename TDenseSpace::MatrixPointerType TDenseMatrixPointerType;

    typedef typename BaseType::TSchemeType TSchemeType;

    //typedef typename BaseType::DofSetType DofSetType;

    typedef typename BaseType::DofsArrayType DofsArrayType;

    typedef typename BaseType::TSystemMatrixType TSystemMatrixType;

    typedef typename BaseType::TSystemVectorType TSystemVectorType;

    typedef typename BaseType::LocalSystemVectorType LocalSystemVectorType;

    typedef typename BaseType::LocalSystemMatrixType LocalSystemMatrixType;

    typedef typename BaseType::TSystemMatrixPointerType TSystemMatrixPointerType;

    typedef typename BaseType::TSystemVectorPointerType TSystemVectorPointerType;


    // don't know if this is necessary
    typedef typename ComplexSparseSpaceType::MatrixType TSolutionMatrixType;

    typedef typename ComplexSparseSpaceType::MatrixPointerType TSolutionMatrixPointerType;

    typedef typename ComplexSparseSpaceType::VectorType TSolutionVectorType;

    typedef typename ComplexSparseSpaceType::VectorPointerType TSolutionVectorPointerType;

    typedef ComplexSparseSpaceType TSolutionSpace;




    typedef LinearSolver<ComplexSparseSpaceType, ComplexDenseSpaceType> ComplexLinearSolverType;



    ///@}
    ///@name Life Cycle

    ///@{

    /**
     * Default constructor
     * @param rModelPart The model part of the problem
     * @param pScheme The integration schemed
     * @param MoveMeshFlag The flag that allows to move the mesh
     */
    MorSecondOrderIRKAStrategy(
        ModelPart& rModelPart,
        typename TSchemeType::Pointer pScheme,
        typename TLinearSolver::Pointer pNewLinearSolver,
        typename ComplexLinearSolverType::Pointer pNewComplexLinearSolver,
        //typename TLinearSolver::Pointer pNewLinearEigenSolver,
        vector< double > samplingPoints_real,
        vector< double > samplingPoints_imag,
        bool MoveMeshFlag = false)
        : BaseType(rModelPart, pScheme, pNewLinearSolver, MoveMeshFlag)
    {
        KRATOS_TRY;

        // Saving the scheme
        this->SetScheme(pScheme);


         //mpLinearSolver = typename TLinearSolver::Pointer(new TLinearSolver);


        // Setting up the default builder and solver
        this->SetBuilderAndSolver(typename TBuilderAndSolverType::Pointer(
            new TBuilderAndSolverType(pNewLinearSolver)));
            //new TBuilderAndSolverType(mpLinearSolver)));
            //new TBuilderAndSolverType(pNewComplexLinearSolver)));

        // Saving the linear solver        
        this->SetLinearSolver(pNewLinearSolver);
        //this->SetLinearSolver(pNewComplexLinearSolver);

        // Set flags to start correctly the calculations
        mSolutionStepIsInitialized = false;
        mInitializeWasPerformed = false;

        // Tells to the builder and solver if the reactions have to be calculated or not
        this->GetBuilderAndSolver()->SetCalculateReactionsFlag(false);

        // Tells to the Builder And Solver if the system matrix and vectors need to
        // be reshaped at each step or not
        this->GetBuilderAndSolver()->SetReshapeMatrixFlag(false);

        // Set EchoLevel to the default value (only time is displayed)
        this->SetEchoLevel(1);

        // By default the matrices are rebuilt at each iteration
        this->SetRebuildLevel(0);

        // Set members
        //TODO: let pybind11 do the conversion
        mSamplingPoints.resize(samplingPoints_real.size());
        for(size_t i=0; i<samplingPoints_real.size(); i++)
        {
            mSamplingPoints(i) = complex( samplingPoints_real(i), samplingPoints_imag(i) );
        }
        //mSamplingPoints = samplingPoints;

        mpComplexLinearSolver = pNewComplexLinearSolver;


        //mpNewLinearEigenSolver = pNewLinearEigenSolver;


        mpM = ComplexSparseSpaceType::CreateEmptyMatrixPointer();
        mpK = ComplexSparseSpaceType::CreateEmptyMatrixPointer();
        mpD = ComplexSparseSpaceType::CreateEmptyMatrixPointer();
        mpb = ComplexSparseSpaceType::CreateEmptyVectorPointer();






        KRATOS_CATCH("");
    }

    /**
     * @brief Destructor.
     * @details In trilinos third party library, the linear solver's preconditioner should be freed before the system matrix. We control the deallocation order with Clear().
     */
    ~MorSecondOrderIRKAStrategy() override
    {
        //this->Clear();


        // TSolutionSpace::Clear(mpM);
        // TSolutionSpace::Clear(mpK);
        // TSolutionSpace::Clear(mpD);
        // TSolutionSpace::Clear(mpb);

        if (mpM != nullptr)
            TSolutionSpace::Clear(mpM);
        if (mpK != nullptr)
            TSolutionSpace::Clear(mpK);
        if (mpD != nullptr)
            TSolutionSpace::Clear(mpD);
        if (mpb != nullptr)
            TSolutionSpace::Clear(mpb);

        
        this->Clear();


    }

    

    //*********************************************************************************
    /**OPERATIONS ACCESSIBLE FROM THE INPUT: **/

    /**
     * @brief Solves the current step. This function returns true if a solution has been found, false otherwise.
     */
    bool SolveSolutionStep() override
    {
        KRATOS_TRY;
        std::cout << "hello! this is where the second order IRKA MOR magic happens" << std::endl;
        typename TSchemeType::Pointer p_scheme = this->GetScheme();
        typename TBuilderAndSolverType::Pointer p_builder_and_solver = this->GetBuilderAndSolver();
        //TODO: clear new insance at the end
        // is there any shorter way do define this?
        //typename TBuilderAndSolverType::Pointer p_eigensolver = typename TBuilderAndSolverType::Pointer(new TBuilderAndSolverType(mpNewLinearEigenSolver));
        
        TSystemMatrixType& r_K_tmp = this->GetSystemMatrix();
        TSystemMatrixType& r_M_tmp = this->GetMassMatrix();
        TSystemMatrixType& r_D_tmp = this->GetDampingMatrix();
        TSystemVectorType& r_b_tmp = this->GetSystemVector();  // originally RHS, check if results are bad

        TSystemVectorType tmp(r_K_tmp.size1(), 0.0);

        //define references for the complex matrices
        TSolutionMatrixType& r_K = *mpK;
        TSolutionMatrixType& r_M = *mpM;
        TSolutionMatrixType& r_D = *mpD;
        TSolutionVectorType& r_b = *mpb;


        // build the matrices based on the used model
        p_builder_and_solver->BuildRHS(p_scheme, BaseType::GetModelPart(), r_b_tmp);

        p_builder_and_solver->BuildStiffnessMatrix(p_scheme, BaseType::GetModelPart(), r_K_tmp, tmp);
        p_builder_and_solver->ApplyDirichletConditions(p_scheme, BaseType::GetModelPart(), r_K_tmp, tmp, r_b_tmp);  

        p_builder_and_solver->BuildMassMatrix(p_scheme, BaseType::GetModelPart(), r_M_tmp, tmp);
        p_builder_and_solver->ApplyDirichletConditionsForMassMatrix(p_scheme, BaseType::GetModelPart(), r_M_tmp);

        p_builder_and_solver->BuildDampingMatrix(p_scheme, BaseType::GetModelPart(), r_D_tmp, tmp);
        p_builder_and_solver->ApplyDirichletConditionsForDampingMatrix(p_scheme, BaseType::GetModelPart(), r_D_tmp);


        // transform the real matrices to complex types
        r_K = TSolutionMatrixType(r_K_tmp);
        r_M = TSolutionMatrixType(r_M_tmp);
        r_D = TSolutionMatrixType(r_D_tmp);
        r_b = TSolutionVectorType(r_b_tmp);

        //KRATOS_WATCH(r_K);


        // DEBUG: print matrix information 
        //this->EchoInfo(0);


        const unsigned int system_size = p_builder_and_solver->GetEquationSystemSize(); // n
        //sampling points
        KRATOS_WATCH(mSamplingPoints)
        const std::size_t n_sampling_points = mSamplingPoints.size(); // number of sampling points
        const std::size_t reduced_system_size = n_sampling_points; // r

        KRATOS_WATCH(n_sampling_points)

        // //DBUG sparse MKD matrices
        // std::cout<<"K:"<<std::endl;
        // for(size_t i=0; i<system_size; i++){
        //     for(size_t j=0; j<system_size; j++){
        //         if(abs(r_K(i,j))>1e-10)
        //         std::cout<<"("<<i+1<<","<<j+1<<")   "<<r_K(i,j)<<std::endl;
        //     }
        // }


        //TODO: allow for complex sampling points, cplxpair
        // store the sampling points for error calculations in the convergence loop for convergence check
        auto samplingPoints_old = mSamplingPoints;


        


        
        // // initialize V (=W, due to symmetry in FEM applications)
        // auto  Vr_ptr = SparseSpaceType::CreateEmptyMatrixPointer();
        // auto& r_Vr   = *Vr_ptr;
        // SparseSpaceType::Resize(r_Vr, system_size, reduced_system_size); // n x r
        // //DenseSpaceType::Set(r_Vr, 0.0); // only works with vectors, matrices are automatically set to zero        
        

        // // initialize helper variables for V
        // auto  tmp_Vn_ptr = SparseSpaceType::CreateEmptyMatrixPointer();
        // auto& r_tmp_Vn   = *tmp_Vn_ptr;
        // SparseSpaceType::Resize(r_tmp_Vn, system_size, system_size); // n x n
        
        // auto  tmp_Vr_col_ptr = DenseSpaceType::CreateEmptyVectorPointer();
        // auto& r_tmp_Vr_col   = *tmp_Vr_col_ptr;
        // DenseSpaceType::Resize(r_tmp_Vr_col, system_size); // n x 1
        // DenseSpaceType::Set(r_tmp_Vr_col, 0.0); // set vector to zero

        // //KRATOS_WATCH(r_tmp_Vr_col)



        // complex variant
        // initialize V (=W, due to symmetry in FEM applications)
        auto  Vr_dense_ptr = DenseSpaceType::CreateEmptyMatrixPointer();
        auto& r_Vr_dense   = *Vr_dense_ptr;
        DenseSpaceType::Resize(r_Vr_dense, system_size, reduced_system_size); // n x r
        //TDenseSpace::SetToZero(r_Vr);

        auto& r_Vr_sparse = this->GetBasis();
        SparseSpaceType::Resize(r_Vr_sparse, system_size, reduced_system_size); // n x r
        TSparseSpace::SetToZero(r_Vr_sparse);

        // initialize helper variables for V
        auto  tmp_Vn_ptr = ComplexSparseSpaceType::CreateEmptyMatrixPointer();
        auto& r_tmp_Vn   = *tmp_Vn_ptr;
        ComplexSparseSpaceType::Resize(r_tmp_Vn, system_size, system_size); // n x n
        TUblasSparseSpace<complex>::SetToZero(r_tmp_Vn);
        
        auto  tmp_Vr_col_ptr = ComplexDenseSpaceType::CreateEmptyVectorPointer();
        auto& r_tmp_Vr_col   = *tmp_Vr_col_ptr;
        ComplexDenseSpaceType::Resize(r_tmp_Vr_col, system_size); // n x 1
        ComplexDenseSpaceType::Set(r_tmp_Vr_col, 0.0); // set vector to zero


        //mpComplexLinearSolver->Initialize( r_tmp_Vn, r_tmp_Vr_col, r_b);


        for(size_t i=0; i < n_sampling_points/2; ++i)
        {
            KRATOS_WATCH(mSamplingPoints(2*i))
            r_tmp_Vn = std::pow( mSamplingPoints(2*i), 2.0 ) * r_M + mSamplingPoints(2*i) * r_D + r_K;
            //KRATOS_WATCH(r_tmp_Vn) //ok
            mpComplexLinearSolver->Initialize( r_tmp_Vn, r_tmp_Vr_col, r_b);
            mpComplexLinearSolver->Solve( r_tmp_Vn, r_tmp_Vr_col, r_b); // Ax = b, solve for x
            //KRATOS_WATCH(r_tmp_Vr_col) // garbage values

            column(r_Vr_dense, 2*i) = real(r_tmp_Vr_col) ;
            column(r_Vr_dense, 2*i+1) = imag(r_tmp_Vr_col) ;
     

            //KRATOS_WATCH(column(r_Vr, 2*i)); 
            //KRATOS_WATCH(column(r_Vr, 2*i+1));
        }


        //orthogonalize
        // TODO: if time left, replace by some orthogonlization method
        mQR_decomposition.compute( system_size, reduced_system_size, &(r_Vr_dense)(0,0) );
        mQR_decomposition.compute_q();

        for(size_t i=0; i < system_size; ++i)
        {
            for(size_t j=0; j < reduced_system_size; ++j)
            {
                r_Vr_sparse(i,j) = mQR_decomposition.Q(i,j);

            }
        }



        auto& r_b_reduced = this->GetRHSr();
        auto& r_K_reduced = this->GetKr();
        auto& r_M_reduced = this->GetMr();
        auto& r_D_reduced = this->GetDr();


        TSystemMatrixType T;

        // projections
        T = prod( trans(r_Vr_sparse), r_M_tmp );
        r_M_reduced = prod( T, r_Vr_sparse );

        T = prod( trans(r_Vr_sparse), r_D_tmp );
        r_D_reduced = prod( T, r_Vr_sparse );

        T = prod( trans(r_Vr_sparse), r_K_tmp );
        r_K_reduced = prod( T, r_Vr_sparse );

        r_b_reduced = prod( trans(r_Vr_sparse), r_b_tmp);

        KRATOS_WATCH(r_M_reduced)
  

/*


        //mit dem hier, angelehnt an den Matlab-Code, klappt es nicht...

        for(size_t i=0; i < n_sampling_points/2; ++i)
        {
            KRATOS_WATCH(mSamplingPoints(2*i)) // ok
            r_tmp_Vn = std::pow( mSamplingPoints(2*i), 2.0 ) * r_M + mSamplingPoints(2*i) * r_D + r_K;
            //KRATOS_WATCH(r_tmp_Vn) // ok
            this->GetLinearSolver()->Solve( r_tmp_Vn, r_tmp_Vr_col, r_b); // Ax = b, solve for x
            KRATOS_WATCH(r_tmp_Vr_col);  // ok, but sometimes needs two runs to work
            //auto aux_col_real = real(r_tmp_Vr_col);
            //auto aux_col_imag = imag(r_tmp_Vr_col);
            column(r_Vr, 2*i) = real(r_tmp_Vr_col) ;
            column(r_Vr, 2*i+1) = imag(r_tmp_Vr_col) ;
            //column(r_Vr, 2*i) = aux_col_real;
            //column(r_Vr, 2*i+1) = aux_col_imag;

            KRATOS_WATCH(column(r_Vr, 2*i));  // not ok, all zero
            KRATOS_WATCH(column(r_Vr, 2*i+1)); // not ok, all zero
        }


        // complex tests to better understand whats the problem with real and imag
        // in minimal examples it works as expected...
        
        // //vector<complex> test_vec (2);// = {(1.5, 2.3), (2.7, 5.4)};
        // //test_vec(0) = complex (1.5, 2.3);
        // //test_vec(1) = complex (2.7, 5.4);
        // vector<double> test_vec(2);
        // test_vec(0) = 1.5;
        // test_vec(1) = 2.7;
        // KRATOS_WATCH(test_vec);
        // //vector<double> test_real = real(test_vec);
        // //vector<double> test_imag = imag(test_vec);
        // KRATOS_WATCH(real(test_vec));
        // KRATOS_WATCH(imag(test_vec));


        // TODO: very big entries in r_Vr, check if results are wrong
        // works for n=50 (306), but not for n=5 (36)
        //KRATOS_WATCH(r_Vr)


        // for(size_t i=0; i < n_sampling_points; ++i)
        // {
        //     r_tmp_Vn = std::pow( mSamplingPoints(i), 2.0 ) * r_M + mSamplingPoints(i) * r_D + r_K;
        //     p_builder_and_solver->GetLinearSystemSolver()->Solve( r_tmp_Vn, r_tmp_Vr_col, r_b);
        //     column(r_Vr, i) = r_tmp_Vr_col;
        // }




        //orthogonalize
        // TODO: if time left, replace by some orthogonlization method
        mQR_decomposition.compute( system_size, reduced_system_size, &(r_Vr)(0,0) );
        mQR_decomposition.compute_q();

        for(size_t i=0; i < system_size; ++i)
        {
            for(size_t j=0; j < reduced_system_size; ++j)
            {
                r_Vr(i,j) = mQR_decomposition.Q(i,j);

            }
        }


        // here only 0 entries...
        //KRATOS_WATCH(r_Vr)

    


        // initialize reduced matrices and vectors
        auto& r_b_reduced = this->GetRHSr();
        auto& r_K_reduced = this->GetKr();
        auto& r_M_reduced = this->GetMr();
        auto& r_D_reduced = this->GetDr();

        // helper for auxiliary products
        TSystemMatrixType T;

        // initialize linearization matrices
        auto L1_ptr = SparseSpaceType::CreateEmptyMatrixPointer();
        auto& r_L1 = *L1_ptr;
        SparseSpaceType::Resize(r_L1, 2*reduced_system_size, 2*reduced_system_size); // 2r x 2r

        auto L2_ptr = SparseSpaceType::CreateEmptyMatrixPointer();
        auto& r_L2 = *L2_ptr;
        SparseSpaceType::Resize(r_L2, 2*reduced_system_size, 2*reduced_system_size); // 2r x 2r

        // identity matrix of size r x r
        identity_matrix<double> id_r (reduced_system_size); 

        // initialize output for Eigensolver
        TDenseVectorType Eigenvalues;
        TDenseMatrixType Eigenvectors;




        int iter = 4;
        int err = 1;
        while(iter < 5 && err > 1e-4)
        {
            // projections
            T = prod( trans(r_Vr), r_M );
            r_M_reduced = prod( T, r_Vr );

            T = prod( trans(r_Vr), r_D );
            r_D_reduced = prod( T, r_Vr );

            T = prod( trans(r_Vr), r_K );
            r_K_reduced = prod( T, r_Vr );

            KRATOS_WATCH(r_M_reduced)  //hm, ok...
            KRATOS_WATCH(r_D_reduced)  // rest not so ok
            KRATOS_WATCH(r_K_reduced)

            subrange(r_L1, 0, reduced_system_size, 0, reduced_system_size) = r_D_reduced;
            subrange(r_L1, 0, reduced_system_size, reduced_system_size, 2*reduced_system_size) = r_K_reduced;
            subrange(r_L1, reduced_system_size, 2*reduced_system_size, 0, reduced_system_size) = -1.0*id_r;

            subrange(r_L2, 0, reduced_system_size, 0, reduced_system_size) = r_M_reduced;
            subrange(r_L2, reduced_system_size, 2*reduced_system_size, reduced_system_size, 2*reduced_system_size) = id_r;


            p_eigensolver->GetLinearSystemSolver()->Solve(
                r_L1,
                r_L2,
                Eigenvalues,
                Eigenvectors
            );

            KRATOS_WATCH(Eigenvalues)

            this->test_fun();

            // test sorting
            // does not work, maybe build own one if no boost/ublas function available

            // // vector<double> ev_real;
            // // vector<double> ev_imag;
            // // ev_real.resize(reduced_system_size*2);
            // // ev_imag.resize(reduced_system_size*2);

            // // std::vector<std::pair<double,double>> test_pair( std::piecewise_construct, std::forward_as_tuple(ev_real), std::forward_as_tuple(ev_imag));

            // // for(int i = 0; i < 2*reduced_system_size; i++)
            // // {
            // //     ev_real(i) = Eigenvalues(2*i);
            // //     ev_imag(i) = Eigenvalues(2*i+1);
            // // }

            // // // KRATOS_WATCH(ev_real)
            // // // KRATOS_WATCH(ev_imag)
            
            // // // does not work, invalid use; maybe because compareFunc is protected
            // // std::sort(test_pair.begin(), test_pair.end(), this->compareFunc);

            // // // KRATOS_WATCH(ev_real)
            // // // KRATOS_WATCH(ev_imag)



            // here we need the complex solver


            




            iter++;


        }

        r_b_reduced = prod( trans(r_Vr), r_b);
        
        
        KRATOS_WATCH(r_b_reduced)
        //KRATOS_WATCH(r_mass_matrix_reduced)
*/


        // TSolutionSpace::Clear(mpM);
        // TSolutionSpace::Clear(mpK);
        // TSolutionSpace::Clear(mpD);
        // TSolutionSpace::Clear(mpb);





        std::cout << "MOR offline solve finished" << std::endl;
        
		return true;

        KRATOS_CATCH("");
    }

    ///@}
    ///@name Operators
    ///@{

    ///@}
    ///@name Operations
    ///@{

    ///@}
    ///@name Access
    ///@{

    ///@}
    ///@name Input and output
    ///@{

    /// Turn back information as a string.
    virtual std::string Info() const override
    {
        return "MorSecondOrderIRKAStrategy";
    }

    ///@}
    ///@name Inquiry
    ///@{

    ///@}
    ///@name Friends
    ///@{

    ///@}

  private:
    ///@name Protected static Member Variables
    ///@{

    ///@}
    ///@name Protected member Variables
    ///@{

    ///@}
    ///@name Protected Operators
    ///@{

    ///@}
    ///@name Protected Operations
    ///@{

    ///@}
    ///@name Protected  Access
    ///@{

    ///@}
    ///@name Protected Inquiry
    ///@{

    ///@}
    ///@name Protected LifeCycle
    ///@{

    ///@}

  protected:
    ///@name Static Member Variables
    ///@{

    ///@}
    ///@name Member Variables
    ///@{

    vector< complex > mSamplingPoints;
    QR<double, row_major> mQR_decomposition;
    //typename TLinearSolver::Pointer mpNewLinearEigenSolver;
    ComplexLinearSolverType::Pointer mpComplexLinearSolver;

    //typename TLinearSolver::Pointer mpLinearSolver;


    TSolutionMatrixPointerType mpM; // mass matrix
    TSolutionMatrixPointerType mpK; // stiffness matrix
    TSolutionMatrixPointerType mpD; // damping matrix
    TSolutionVectorPointerType mpb; // RHS vector






    /**
     * @brief Flag telling if it is needed to reform the DofSet at each
    solution step or if it is possible to form it just once
    * @details Default = false
        - true  : Reforme at each time step
        - false : Form just one (more efficient)
     */
    bool mReformDofSetAtEachStep;

    bool mSolutionStepIsInitialized; /// Flag to set as initialized the solution step

    bool mInitializeWasPerformed; /// Flag to set as initialized the strategy

    ///@}
    ///@name Private Operators
    ///@{

    ///@}
    ///@name Private Operations
    ///@{

    ///@}
    ///@name Private  Access
    ///@{

    ///@}
    ///@name Private Inquiry
    ///@{

    ///@}
    ///@name Un accessible methods

        void test_fun(){
            std::cout<<"test worked \n";
        }

        //TODO: adapt this example of stackoverflow
        bool compareFunc(std::pair<double,double> &a, std::pair<double,double> &b){
            return a.second > b.second;
        }



    ///@{

    /**
     * Copy constructor.
     */

    MorSecondOrderIRKAStrategy(const MorSecondOrderIRKAStrategy &Other){};

    ///@}

}; /* Class MorSecondOrderIRKAStrategy */

///@}

///@name Type Definitions
///@{

///@}

} /* namespace Kratos. */

#endif /* MOR_SECOND_ORDER_IRKA_STRATEGY  defined */

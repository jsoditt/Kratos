//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//
//  Main authors:    Ruben Zorrilla
//
//					 Kratos default license: kratos/license.txt
//

// System includes

// External includes

// Application includes
#include "move_mesh_utilities.h"
#include "explicit_mesh_moving_utilities.h"

// Project includes
#include "includes/mesh_moving_variables.h"
#include "spatial_containers/spatial_containers.h"
#include "utilities/variable_utils.h"
#include "utilities/binbased_fast_point_locator.h"
#include "utilities/spatial_containers_configure.h"
#include "custom_utilities/mesh_velocity_calculation.h"

namespace Kratos
{
    /* Public functions *******************************************************/

    ExplicitMeshMovingUtilities::ExplicitMeshMovingUtilities(
        ModelPart &rVirtualModelPart,
        ModelPart &rStructureModelPart,
        const double SearchRadius) :
        mSearchRadius(SearchRadius),
        mrVirtualModelPart(rVirtualModelPart),
        mrStructureModelPart(rStructureModelPart),
        mpOriginModelPart(nullptr)
    {
        if (mrStructureModelPart.GetBufferSize() < 2) {
            (mrStructureModelPart.GetRootModelPart()).SetBufferSize(2);
            KRATOS_WARNING("ExplicitMeshMovingUtilities") << "Structure model part buffer size is 1. Setting buffer size to 2." << std::endl;
        }
    }

    void ExplicitMeshMovingUtilities::FillVirtualModelPart(ModelPart& rOriginModelPart)
    {
        // Check that the origin model part has nodes and elements to be copied
        KRATOS_ERROR_IF(rOriginModelPart.NumberOfNodes() == 0) << "Origin model part has no nodes.";
        KRATOS_ERROR_IF(rOriginModelPart.NumberOfElements() == 0) << "Origin model part has no elements.";

        // Save the selected origin model part
        mpOriginModelPart = &rOriginModelPart;

        // Set the buffer size in the virtual model part
        mrVirtualModelPart.SetBufferSize(rOriginModelPart.GetBufferSize());

        // Copy the origin model part nodes
        auto &r_nodes_array = rOriginModelPart.NodesArray();
        for(auto &it_node : r_nodes_array){
            // Create a copy of the origin model part node and add DOFs
            auto p_node = mrVirtualModelPart.CreateNewNode(it_node->Id(),*it_node, 0);
            p_node->pAddDof(MESH_DISPLACEMENT_X);
            p_node->pAddDof(MESH_DISPLACEMENT_Y);
            p_node->pAddDof(MESH_DISPLACEMENT_Z);
        }

        // Copy the origin model part elements
        auto &r_elems = rOriginModelPart.Elements();
        for(auto &elem : r_elems){
            // Set the array of virtual nodes to create the element from the original ids.
            PointsArrayType nodes_array;
            auto &r_orig_geom = elem.GetGeometry();
            for (unsigned int i = 0; i < r_orig_geom.PointsNumber(); ++i){
                nodes_array.push_back(mrVirtualModelPart.pGetNode(r_orig_geom[i].Id()));
            }

            // Create the same element but using the virtual model part nodes
            auto p_elem = elem.Create(elem.Id(), nodes_array, elem.pGetProperties());
            mrVirtualModelPart.AddElement(p_elem);
        }

        // Check that the nodes and elements have been correctly copied
        KRATOS_ERROR_IF(rOriginModelPart.NumberOfNodes() != mrVirtualModelPart.NumberOfNodes())
            << "Origin and virtual model part have different number of nodes.";
        KRATOS_ERROR_IF(rOriginModelPart.NumberOfElements() != mrVirtualModelPart.NumberOfElements())
            << "Origin and virtual model part have different number of elements.";
    }

    void ExplicitMeshMovingUtilities::ComputeExplicitMeshMovement(const double DeltaTime)
    {
        VectorResultNodesContainerType search_results;
        DistanceVectorContainerType search_distance_results;

        // KRATOS_WATCH("About to create extra nodes...")
        // KRATOS_WATCH(mrStructureModelPart.NumberOfNodes())
        // CreateExtraStructureNodes();
        // KRATOS_WATCH(mrStructureModelPart.NumberOfNodes())
        SearchStructureNodes(search_results, search_distance_results);
        ComputeMeshDisplacement(search_results, search_distance_results);
        // KRATOS_WATCH("ComputeMeshDisplacement done!")
        // RemoveExtraStructureNodes();
        // KRATOS_WATCH("RemoveExtraStructureNodes done!")
        TimeDiscretization::BDF1 time_disc_BDF1;
        mrVirtualModelPart.GetProcessInfo()[DELTA_TIME] = DeltaTime;
        MeshVelocityCalculation::CalculateMeshVelocities(mrVirtualModelPart, time_disc_BDF1);
        MoveMeshUtilities::MoveMesh(mrVirtualModelPart.Nodes());

        // Check that the moved virtual mesh has no negative Jacobian elements
        for (auto it_elem : mrVirtualModelPart.ElementsArray())
            KRATOS_ERROR_IF((it_elem->GetGeometry()).Area() < 0.0) << "Element " << it_elem->Id() << " in virtual model part has negative jacobian." << std::endl;
    }

    void ExplicitMeshMovingUtilities::UndoMeshMovement()
    {
        auto &r_nodes = mrVirtualModelPart.Nodes();
        VariableUtils().UpdateCurrentToInitialConfiguration(r_nodes);
    }

    template <unsigned int TDim>
    void ExplicitMeshMovingUtilities::ProjectVirtualValues(
        ModelPart& rOriginModelPart,
        unsigned int BufferSize){

        // Check that the virtual model part has elements
        KRATOS_ERROR_IF(mrVirtualModelPart.NumberOfElements() == 0) << "Virtual model part has no elements.";

        // Set the binbased fast point locator utility
        BinBasedFastPointLocator<TDim> bin_based_point_locator(mrVirtualModelPart);
        bin_based_point_locator.UpdateSearchDatabase();

        // Search the origin model part nodes in the virtual mesh elements and
        // interpolate the values in the virtual element to the origin model part node
        auto &r_nodes_array = rOriginModelPart.NodesArray();
        for(auto it_node : r_nodes_array){
            // Find the origin model part node in the virtual mesh
            Vector aux_N;
            Element::Pointer p_elem;
            const bool is_found = bin_based_point_locator.FindPointOnMeshSimplified(it_node->Coordinates(), aux_N, p_elem);

            // Check if the node is found
            if (is_found){
                // Initialize historical data
                // The current step values are also set as a prediction
                auto &r_mesh_vel = it_node->GetSolutionStepValue(MESH_VELOCITY);
                r_mesh_vel = ZeroVector(3);
                if (!it_node->IsFixed(PRESSURE)) {
                    for (unsigned int i_step = 1; i_step < BufferSize; ++i_step){
                        it_node->GetSolutionStepValue(PRESSURE, i_step) = 0.0;
                    }
                }
                if (!it_node->IsFixed(VELOCITY_X)) {
                    for (unsigned int i_step = 1; i_step < BufferSize; ++i_step){
                        it_node->GetSolutionStepValue(VELOCITY_X, i_step) = 0.0;
                    }
                }
                if (!it_node->IsFixed(VELOCITY_Y)) {
                    for (unsigned int i_step = 1; i_step < BufferSize; ++i_step){
                        it_node->GetSolutionStepValue(VELOCITY_Y, i_step) = 0.0;
                    }
                }
                if (!it_node->IsFixed(VELOCITY_Z)) {
                    for (unsigned int i_step = 1; i_step < BufferSize; ++i_step){
                        it_node->GetSolutionStepValue(VELOCITY_Z, i_step) = 0.0;
                    }
                }

                // Interpolate the origin model part nodal values
                auto &r_geom = p_elem->GetGeometry();
                for (std::size_t i_virt_node = 0; i_virt_node < r_geom.PointsNumber(); ++i_virt_node){
                    r_mesh_vel += aux_N(i_virt_node) * r_geom[i_virt_node].GetSolutionStepValue(MESH_VELOCITY);
                    if (!it_node->IsFixed(PRESSURE)) {
                        for (unsigned int i_step = 1; i_step < BufferSize; ++i_step){
                            it_node->GetSolutionStepValue(PRESSURE, i_step) += aux_N(i_virt_node) * r_geom[i_virt_node].GetSolutionStepValue(PRESSURE, i_step);
                        }
                    }
                    if (!it_node->IsFixed(VELOCITY_X)) {
                        for (unsigned int i_step = 1; i_step < BufferSize; ++i_step){
                            it_node->GetSolutionStepValue(VELOCITY_X, i_step) += aux_N(i_virt_node) * r_geom[i_virt_node].GetSolutionStepValue(VELOCITY_X, i_step);
                        }
                    }
                    if (!it_node->IsFixed(VELOCITY_Y)) {
                        for (unsigned int i_step = 1; i_step < BufferSize; ++i_step){
                            it_node->GetSolutionStepValue(VELOCITY_Y, i_step) += aux_N(i_virt_node) * r_geom[i_virt_node].GetSolutionStepValue(VELOCITY_Y, i_step);
                        }
                    }
                    if (!it_node->IsFixed(VELOCITY_Z)) {
                        for (unsigned int i_step = 1; i_step < BufferSize; ++i_step){
                            it_node->GetSolutionStepValue(VELOCITY_Z, i_step) += aux_N(i_virt_node) * r_geom[i_virt_node].GetSolutionStepValue(VELOCITY_Z, i_step);
                        }
                    }
                }
            } else {
                KRATOS_WARNING("ExplicitMeshMovingUtility") << "Origin model part node " << it_node->Id() << " has not been found in any virtual model part element." << std::endl;
            }
        }
    }

    /* Private functions *******************************************************/

    void ExplicitMeshMovingUtilities::CreateExtraStructureNodes(const GeometryData::IntegrationMethod &rIntegrationMethod)
    {
        unsigned int new_node_id = 0;
        for (int i_node = 0; i_node < mrStructureModelPart.GetRootModelPart().NumberOfNodes(); ++i_node) {
            auto it_node = mrStructureModelPart.GetRootModelPart().NodesBegin() + i_node;
            if (it_node->Id() > new_node_id) {
                new_node_id = it_node->Id();
            }
        }
        new_node_id++;
        KRATOS_WATCH(new_node_id)

        // Loop all the structure conditions
        for (int i_cond = 0; i_cond < mrStructureModelPart.NumberOfConditions(); ++i_cond) {
            const auto it_cond = mrStructureModelPart.ConditionsBegin() + i_cond;
            const auto &r_geom = it_cond->GetGeometry();
            const auto gauss_pts = r_geom.IntegrationPoints(rIntegrationMethod);

            // Get the current condition nodal values
            const unsigned int n_nodes = r_geom.PointsNumber();
            std::vector<array_1d<double,3>> u_0(n_nodes); // Current step displacement
            std::vector<array_1d<double,3>> u_1(n_nodes); // Previous step displacement
            for (unsigned int i_node = 0; i_node < n_nodes; ++i_node) {
                u_0[i_node] = r_geom[i_node].FastGetSolutionStepValue(DISPLACEMENT, 0);
                u_1[i_node] = r_geom[i_node].FastGetSolutionStepValue(DISPLACEMENT, 1);
            }

            // For each condition create the auxiliar nodes in the Gauss pts. position
            for (unsigned int i_gauss = 0; i_gauss < gauss_pts.size(); ++i_gauss) {
                // Create an auxiliary node in the Gauss pt. position
                array_1d<double, 3> aux_coords;
                r_geom.GlobalCoordinates(aux_coords, gauss_pts[i_gauss].Coordinates());
                // TODO: REIMPLEMENT THE ID COUNTER FOR THE PARALLEL LOOP
                auto p_new_node = mrStructureModelPart.CreateNewNode(new_node_id, aux_coords[0], aux_coords[1], aux_coords[2]);
                new_node_id++;

                // Flag it to be removed afterwards
                p_new_node->Set(TO_ERASE, true);

                // Interpolate the displacements in the auxiliary node
                Vector N_values;
                r_geom.ShapeFunctionsValues(N_values, aux_coords);
                auto &r_new_node_u_0 = p_new_node->FastGetSolutionStepValue(DISPLACEMENT, 0);
                auto &r_new_node_u_1 = p_new_node->FastGetSolutionStepValue(DISPLACEMENT, 1);
                r_new_node_u_0 = ZeroVector(3);
                r_new_node_u_1 = ZeroVector(3);
                for (unsigned int i_node = 0; i_node < n_nodes; ++i_node) {
                    r_new_node_u_0 += N_values[i_node] * u_0[i_node];
                    r_new_node_u_1 += N_values[i_node] * u_1[i_node];
                }
            }
        }
    }

    inline void ExplicitMeshMovingUtilities::RemoveExtraStructureNodes()
    {
        // Remove the auxiliary structure nodes after computing the MESH_DISPLACEMENT
        mrStructureModelPart.RemoveNodesFromAllLevels(TO_ERASE);
    }

    void ExplicitMeshMovingUtilities::SearchStructureNodes(
        VectorResultNodesContainerType &rSearchResults,
        DistanceVectorContainerType &rSearchDistanceResults) {

        auto &r_virt_nodes = mrVirtualModelPart.NodesArray();
        const unsigned int n_virt_nodes = r_virt_nodes.size();
        auto &r_structure_nodes = mrStructureModelPart.NodesArray();
        const unsigned int max_number_of_nodes = r_structure_nodes.size();

        rSearchResults.resize(n_virt_nodes);
        rSearchDistanceResults.resize(n_virt_nodes);

        NodeBinsType bins(r_structure_nodes.begin(), r_structure_nodes.end());

        #pragma omp parallel for
        for(int i_fl = 0; i_fl < static_cast<int>(n_virt_nodes); ++i_fl){
            // Initialize the search results variables
            std::size_t n_results = 0;
            ResultNodesContainerType i_node_results(max_number_of_nodes);
            DistanceVectorType i_node_distance_results(max_number_of_nodes);
            auto it_results = i_node_results.begin();
            auto it_results_distances = i_node_distance_results.begin();

            // Perform the structure nodes search
            auto it_virt_node = r_virt_nodes.begin() + i_fl;
            n_results = bins.SearchObjectsInRadiusExclusive(*it_virt_node, mSearchRadius, it_results, it_results_distances, max_number_of_nodes);

            // Resize and save the current virtual node results
            i_node_results.resize(n_results);
            i_node_distance_results.resize(n_results);

            rSearchResults[i_fl] = i_node_results;
            rSearchDistanceResults[i_fl] = i_node_distance_results;
        }
    }

    void ExplicitMeshMovingUtilities::ComputeMeshDisplacement(
        const VectorResultNodesContainerType &rSearchResults,
        const DistanceVectorContainerType &rSearchDistanceResults)
    {
        #pragma omp parallel for
        for(int i_fl = 0; i_fl < static_cast<int>(mrVirtualModelPart.NumberOfNodes()); ++i_fl){
            // Get auxiliar current fluid node info.
            auto it_node = mrVirtualModelPart.NodesBegin() + i_fl;
            const auto i_fl_str_nodes = rSearchResults[i_fl];
            const auto i_fl_str_dists = rSearchDistanceResults[i_fl];
            const std::size_t n_str_nodes = i_fl_str_nodes.size();

            // Check origin model part mesh displacementfixity
            const auto it_orig_node = mpOriginModelPart->NodesBegin() + i_fl;
            if (it_orig_node->IsFixed(MESH_DISPLACEMENT_X)) {
                it_node->Fix(MESH_DISPLACEMENT_X);
            }
            if (it_orig_node->IsFixed(MESH_DISPLACEMENT_Y)) {
                it_node->Fix(MESH_DISPLACEMENT_Y);
            }
            if (it_orig_node->IsFixed(MESH_DISPLACEMENT_Z)) {
                it_node->Fix(MESH_DISPLACEMENT_Z);
            }

            // Initialize the current virtual model part node MESH_DISPLACEMENT
            auto &r_mesh_disp = it_node->FastGetSolutionStepValue(MESH_DISPLACEMENT);
            r_mesh_disp = ZeroVector(3);

            // Check if any structure node is found
            if (n_str_nodes != 0){
                // Compute the closest structure node MESH_DISPLACEMENT
                double min_distance = 1.01;
                Node<3>::Pointer str_node_ptr = nullptr;
                for(unsigned int i_str = 0; i_str < n_str_nodes; ++i_str) {
                    // Compute the structure point weight according to the kernel function
                    const double normalised_distance = std::sqrt(i_fl_str_dists[i_str]) / mSearchRadius;

                    // Check if the point is closer than the current one
                    if (normalised_distance < min_distance) {
                        min_distance = normalised_distance;
                        str_node_ptr = i_fl_str_nodes[i_str];
                    }
                }

                // Current step structure pt. DISPLACEMENT values
                if (str_node_ptr) {
                    const double weight = this->ComputeKernelValue(min_distance);
                    const auto &r_str_disp_0 = str_node_ptr->FastGetSolutionStepValue(DISPLACEMENT,0);
                    const auto &r_str_disp_1 = str_node_ptr->FastGetSolutionStepValue(DISPLACEMENT,1);
                    const auto str_disp = weight * (r_str_disp_0 - r_str_disp_1);
                    if (!it_node->IsFixed(MESH_DISPLACEMENT_X)) {
                        r_mesh_disp[0] = str_disp[0];
                    }
                    if (!it_node->IsFixed(MESH_DISPLACEMENT_Y)) {
                        r_mesh_disp[1] = str_disp[1];
                    }
                    if (!it_node->IsFixed(MESH_DISPLACEMENT_Z)) {
                        r_mesh_disp[2] = str_disp[2];
                    }
                }
            }
        }
    }

    inline double ExplicitMeshMovingUtilities::ComputeKernelValue(const double NormalisedDistance){
        // Epanechnikov (parabolic) kernel function
        // return (std::abs(NormalisedDistance) < 1.0) ? std::abs((3.0/4.0)*(1.0-std::pow(NormalisedDistance,2))) : 0.0;
        // Triangle kernel function
        return (std::abs(NormalisedDistance) < 1.0) ? 1.0 - std::abs(NormalisedDistance) : 0.0;
    }

    /* External functions *****************************************************/

    /// output stream function
    inline std::ostream& operator << (
        std::ostream& rOStream,
        const ExplicitMeshMovingUtilities& rThis) {

        rThis.PrintData(rOStream);
        return rOStream;
    }

    template void ExplicitMeshMovingUtilities::ProjectVirtualValues<2>(ModelPart &rOriginModelPart, unsigned int BufferSize);
    template void ExplicitMeshMovingUtilities::ProjectVirtualValues<3>(ModelPart &rOriginModelPart, unsigned int BufferSize);
}

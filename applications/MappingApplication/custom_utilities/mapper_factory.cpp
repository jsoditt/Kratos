//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Philipp Bucher, Jordi Cotela
//
// See Master-Thesis P.Bucher
// "Development and Implementation of a Parallel
//  Framework for Non-Matching Grid Mapping"


// System includes


// External includes


// Project includes
#include "mapper_factory.h"


namespace Kratos
{
    Mapper::Pointer MapperFactory::CreateMapper(ModelPart& rModelPartOrigin,
                                                ModelPart& rModelPartDestination,
                                                Parameters JsonParameters)
    {
        ModelPart& r_interface_model_part_origin = ReadInterfaceModelPart(rModelPartOrigin, JsonParameters, "origin");
        ModelPart& r_interface_model_part_destination = ReadInterfaceModelPart(rModelPartDestination, JsonParameters, "destination");

        const std::string mapper_name = JsonParameters["mapper_type"].GetString();

        const auto& mapper_list = GetRegisteredMappersList();

        if (mapper_list.find(mapper_name) != mapper_list.end())
        {
            const bool is_mpi_execution = GetIsMPIExecution();

            // Removing Parameters that are not needed by the Mapper
            JsonParameters.RemoveValue("mapper_type");
            JsonParameters.RemoveValue("interface_submodel_part_origin");
            JsonParameters.RemoveValue("interface_submodel_part_destination");

            return mapper_list.at(mapper_name)->Clone(r_interface_model_part_origin,
                                                      r_interface_model_part_destination,
                                                      JsonParameters,
                                                      is_mpi_execution);
        }
        else
        {
            std::stringstream err_msg;
            err_msg << "The requested Mapper \"" << mapper_name <<"\" is not registered! The following mappers are registered:" << std::endl;
            for (auto const& registered_mapper : mapper_list)
            {
                err_msg << registered_mapper.first << ", ";
            }
            KRATOS_ERROR << err_msg.str() << std::endl;
        }
    }

    void MapperFactory::Register(const std::string& MapperName,
                                 Mapper::Pointer pMapperPrototype)
    {
        GetRegisteredMappersList().insert(make_pair(MapperName, pMapperPrototype));
    }

    ModelPart& MapperFactory::ReadInterfaceModelPart(ModelPart& rModelPart,
                                                     Parameters InterfaceParameters,
                                                     const std::string& InterfaceSide)
    {
        int echo_level = 0;
        // read the echo_level temporarily, bcs the mJsonParameters have not yet been validated and defaults assigned
        if (InterfaceParameters.Has("echo_level"))
        {
            echo_level = std::max(echo_level, InterfaceParameters["echo_level"].GetInt());
        }

        int comm_rank = rModelPart.GetCommunicator().MyPID();

        std::string key_sub_model_part = "interface_submodel_part_";
        key_sub_model_part.append(InterfaceSide);

        if (InterfaceParameters.Has(key_sub_model_part))
        {
            const std::string name_interface_submodel_part = InterfaceParameters[key_sub_model_part].GetString();

            if (echo_level >= 3 && comm_rank == 0)
            {
                std::cout << "Mapper: SubModelPart used for " << InterfaceSide << "-ModelPart" << std::endl;
            }

            return rModelPart.GetSubModelPart(name_interface_submodel_part);
        }
        else
        {
            if (echo_level >= 3 && comm_rank == 0)
            {
                std::cout << "Mapper: Main ModelPart used for " << InterfaceSide << "-ModelPart" << std::endl;
            }

            return rModelPart;
        }
    }

    // CommRank is used as input bcs the MyPID function of the non-MPI MapperCommunicator is used
    // since this function is called before the MapperMPICommunicato is initialized
    void CheckInterfaceModelParts(const int CommRank)
    {
        const int num_nodes_origin = MapperUtilities::ComputeNumberOfNodes(mrModelPartOrigin);
        const int num_conditions_origin = MapperUtilities::ComputeNumberOfConditions(mrModelPartOrigin);
        const int num_elements_origin = MapperUtilities::ComputeNumberOfElements(mrModelPartOrigin);

        const int num_nodes_destination = MapperUtilities::ComputeNumberOfNodes(mrModelPartDestination);
        const int num_conditions_destination = MapperUtilities::ComputeNumberOfConditions(mrModelPartDestination);
        const int num_elements_destination = MapperUtilities::ComputeNumberOfElements(mrModelPartDestination);

        // Check if the ModelPart contains entities
        KRATOS_ERROR_IF(num_nodes_origin + num_conditions_origin + num_elements_origin < 1)
            << "Neither Nodes nor Conditions nor Elements found "
            << "in the Origin ModelPart" << std::endl;

        KRATOS_ERROR_IF(num_nodes_destination + num_conditions_destination + num_elements_destination < 1)
            << "Neither Nodes nor Conditions nor Elements found "
            << "in the Destination ModelPart" << std::endl;

        // Check if the inpt ModelParts contain both Elements and Conditions
        // This is NOT possible, bcs the InterfaceObjects are constructed
        // with whatever exists in the Modelpart (see the InterfaceObjectManagerBase,
        // function "InitializeInterfaceGeometryObjectManager")
        KRATOS_ERROR_IF(num_conditions_origin > 0 && num_elements_origin > 0)
            << "Origin ModelPart contains both Conditions and Elements "
            << "which is not permitted" << std::endl;

        KRATOS_ERROR_IF(num_conditions_destination > 0 && num_elements_destination > 0)
            << "Destination ModelPart contains both Conditions and Elements "
            << "which is not permitted" << std::endl;

        if (mEchoLevel >= 2) {
            std::vector<double> model_part_origin_bbox = MapperUtilities::ComputeModelPartBoundingBox(mrModelPartOrigin);
            std::vector<double> model_part_destination_bbox = MapperUtilities::ComputeModelPartBoundingBox(mrModelPartDestination);

            bool bbox_overlapping = MapperUtilities::ComputeBoundingBoxIntersection(
                                                        model_part_origin_bbox,
                                                        model_part_destination_bbox);
            if(CommRank == 0)
            {
                if (!bbox_overlapping) {
                    std::cout << "MAPPER WARNING, the bounding boxes of the "
                              << "Modelparts do not overlap! "
                              << MapperUtilities::PrintModelPartBoundingBoxes(model_part_origin_bbox,
                                                                              model_part_destination_bbox)
                              << std::endl;
                } else if (mEchoLevel >= 3)
                {
                    std::cout << MapperUtilities::PrintModelPartBoundingBoxes(model_part_origin_bbox,
                                                                              model_part_destination_bbox)
                              << std::endl;
                }
            }
        }

    std::unordered_map<std::string, Mapper::Pointer>& MapperFactory::GetRegisteredMappersList()
    {
        static std::unordered_map<std::string, Mapper::Pointer> registered_mappers;

        return registered_mappers;
    }

    bool MapperFactory::GetIsMPIExecution()
    {
#ifdef KRATOS_USING_MPI // mpi-parallel compilation
        int mpi_initialized;
        MPI_Initialized(&mpi_initialized);
        if (mpi_initialized) // parallel execution, i.e. mpi imported in python
        {
            int comm_size;
            MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
            if (comm_size > 1) return true;
        }
        return false;

#else // serial compilation
        return false;
#endif
    }

}  // namespace Kratos.


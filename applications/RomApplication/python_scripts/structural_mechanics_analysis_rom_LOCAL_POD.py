import KratosMultiphysics
import KratosMultiphysics.RomApplication as romapp
import KratosMultiphysics.StructuralMechanicsApplication
from KratosMultiphysics.RomApplication.empirical_cubature_method import EmpiricalCubatureMethod
from KratosMultiphysics.RomApplication import python_solvers_wrapper_rom as solver_wrapper
from KratosMultiphysics.StructuralMechanicsApplication.structural_mechanics_analysis import StructuralMechanicsAnalysis

import json
import numpy as np

import pdb

class StructuralMechanicsAnalysisROM(StructuralMechanicsAnalysis):

    def __init__(self,model,project_parameters, hyper_reduction_element_selector = None):
        super().__init__(model,project_parameters)
        if hyper_reduction_element_selector != None :
            if hyper_reduction_element_selector == "EmpiricalCubature":
                self.hyper_reduction_element_selector = EmpiricalCubatureMethod()
                self.time_step_residual_matrix_container = []
            else:
                err_msg =  "The requested element selection method \"" + hyper_reduction_element_selector + "\" is not in the rom application\n"
                err_msg += "Available options are: \"EmpiricalCubature\""
                raise Exception(err_msg)
        else:
            self.hyper_reduction_element_selector = None

        self.save_for_comparison = []


    #### Internal functions ####
    def _CreateSolver(self):
        """ Create the Solver (and create and import the ModelPart if it is not alread in the model) """
        ## Solver construction
        with open('RomParameters.json') as rom_parameters:
            rom_settings = KratosMultiphysics.Parameters(rom_parameters.read())
            self.project_parameters["solver_settings"].AddValue("rom_settings", rom_settings["rom_settings"])
        return solver_wrapper.CreateSolverByParameters(self.model, self.project_parameters["solver_settings"],self.project_parameters["problem_data"]["parallel_type"].GetString())

    def _GetSimulationName(self):
        return "::[ROM Simulation]:: "

    # def ModifyInitialGeometry(self):
    #     """Here is the place where the BASIS_ROM and the AUX_ID are imposed to each node"""
    #     super().ModifyInitialGeometry()



    def ModifyAfterSolverInitialize(self):
        super().ModifyAfterSolverInitialize()
        computing_model_part = self._solver.GetComputingModelPart()
        with open('RomParameters.json') as f:
            data = json.load(f)
            nodal_dofs = len(data["rom_settings"]["nodal_unknowns"])
            nodal_modes = data["nodal_modes"]
            number_of_bases = len(nodal_modes['1'])
            POD_BASES = []
            rom_dofs=[]
            for i in range(number_of_bases):
                POD_BASES.append(romapp.RomBasis())
            for i in range(number_of_bases):
                rom_dofs.append(self.project_parameters["solver_settings"]["rom_settings"]["number_of_rom_dofs"][i].GetInt())
            for node in computing_model_part.Nodes:
                for k in range(number_of_bases):
                    aux = KratosMultiphysics.Matrix(nodal_dofs, rom_dofs[k])
                    for j in range(nodal_dofs):
                        Counter=str(node.Id)
                        for i in range(rom_dofs[k]):
                            print(nodal_modes[Counter][str(k)][j][i])
                            aux[j,i] = nodal_modes[Counter][str(k)][j][i]
                    #pdb.set_trace()
                    #print(aux)
                    POD_BASES[k].SetNodalBasis(node.Id, aux)
                    if node.Id in data["nodes_to_print"]:
                        print(k,'th basis, node ',node.Id,'this basis is: ',POD_BASES[k].GetNodalBasis(node.Id))

            distance_to_clusters = romapp.DistanceToClusters(number_of_bases)
            w = data["w"]
            z0 = data["z0"]
            for i in range(number_of_bases):
                for j in range(number_of_bases):
                    distance_to_clusters.SetZEntry(z0[f'{i}'][f'{j}'],i,j)

                    for k in range(number_of_bases):
                        entries = len(w[f'{i}'][f'{j}'][f'{k}'])
                        temp_vector = KratosMultiphysics.Vector(entries)
                        for entry in range(entries):
                            temp_vector[entry] = w[f'{i}'][f'{j}'][f'{k}'][entry]
                        distance_to_clusters.SetWEntry(temp_vector,i,j,k)
                if i==0:
                    self.ClusterCentroids = data["cluster_centroids"][i]
                else:
                    self.ClusterCentroids = np.c_[self.ClusterCentroids , data["cluster_centroids"][i]]


            self.Delta_q = data["delta_q"]
            self.current_cluster = data["correct_cluster"]

            #pdb.set_trace()
            print(distance_to_clusters.GetZMatrix())

        Bases = romapp.RomBases()
        for i in range(number_of_bases):
            Bases.AddBasis(i, POD_BASES[i])

        self.Number_Of_Clusters = number_of_bases


        for i in range(len(data["nodes_to_print"])):
            self._GetSolver().get_builder_and_solver().SetNodeToPrint( data["nodes_to_print"][i] )
        for i in range(len(data["elements_to_print"])):
            self._GetSolver().get_builder_and_solver().SetElementToPrint( data["elements_to_print"][i] )

        self._GetSolver().get_builder_and_solver().SetUpBases(Bases)
        self._GetSolver().get_builder_and_solver().SetUpDistances(distance_to_clusters)

        self.step_index = 0

        if self.hyper_reduction_element_selector != None:
            if self.hyper_reduction_element_selector.Name == "EmpiricalCubature":
                self.ResidualUtilityObject = romapp.RomResidualsUtility(self._GetSolver().GetComputingModelPart(), self.project_parameters["solver_settings"]["rom_settings"], KratosMultiphysics.ResidualBasedIncrementalUpdateStaticScheme())


    def InitializeSolutionStep(self):
        super().InitializeSolutionStep()
        #select the current cluster ...
        self._GetSolver().get_builder_and_solver().UpdateCurrentCluster()
        #self._GetSolver().get_builder_and_solver().HardSetCurrentCluster(self.current_cluster[self.step_index])
        self.step_index +=1



    def FinalizeSolutionStep(self):
        #pdb.set_trace()
        super().FinalizeSolutionStep()
        #update the zmatrix ...
        #self.distance_to_clusters.UpdateZMatrix(self._GetSolver().get_builder_and_solver().GetCurrentReducedCoefficients())
        self._GetSolver().get_builder_and_solver().UpdateZMatrix()
        self.CurrentFullDimensionalVector = np.array(self._GetSolver().get_builder_and_solver().GetCurrentFullDimensionalVector())
        print('The solution I got by summing is: ',np.linalg.norm(self.CurrentFullDimensionalVector))
        self.save_for_comparison.append(self.CurrentFullDimensionalVector)
        print('according to full dimnesional reconstruction, nearest cluster is: ', self.FindNearestClusterToFullDimensionalVector())

        if self.hyper_reduction_element_selector != None:
            if self.hyper_reduction_element_selector.Name == "EmpiricalCubature":
                print('\n\n\n\nGenerating matrix of residuals')
                ResMat = self.ResidualUtilityObject.GetResiduals()
                NP_ResMat = np.array(ResMat, copy=False)
                self.time_step_residual_matrix_container.append(NP_ResMat)


    def FindNearestClusterToFullDimensionalVector(self):
        #identify the nearest cluster centroid to state i
        this_matrix = self.ClusterCentroids - self.CurrentFullDimensionalVector.reshape( len(self.CurrentFullDimensionalVector), 1)
        distance = np.zeros((self.Number_Of_Clusters))
        for j in range(self.Number_Of_Clusters):
            distance[j] = np.linalg.norm(this_matrix[:,j])
        nearest_cluster = np.argsort(distance)[0]
        return nearest_cluster


    def Finalize(self):
        super().FinalizeSolutionStep()
        if self.hyper_reduction_element_selector != None:
            if self.hyper_reduction_element_selector.Name == "EmpiricalCubature":
                OriginalNumberOfElements = self._GetSolver().GetComputingModelPart().NumberOfElements()
                ModelPartName = self._GetSolver().settings["model_import_settings"]["input_filename"].GetString()
                self. hyper_reduction_element_selector.SetUp(self.time_step_residual_matrix_container, OriginalNumberOfElements, ModelPartName)
                self.hyper_reduction_element_selector.Run()





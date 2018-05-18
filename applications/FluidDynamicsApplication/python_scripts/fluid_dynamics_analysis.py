from __future__ import absolute_import, division #makes KratosMultiphysics backward compatible with python 2.6 and 2.7

import KratosMultiphysics as Kratos
import KratosMultiphysics.FluidDynamicsApplication
try:
    import KratosMultiphysics.ExternalSolversApplication
except ImportError:
    pass

from analysis_stage import AnalysisStage

class FluidDynamicsAnalysis(AnalysisStage):
    '''Main script for fluid dynamics simulations using the navier_stokes family of python solvers.'''

    def __init__(self,model,parameters):
        super(FluidDynamicsAnalysis,self).__init__(model,parameters)

    def _CreateSolver(self):
        import python_solvers_wrapper_fluid
        return python_solvers_wrapper_fluid.CreateSolver(self.main_model_part, self.project_parameters)


    def OutputSolutionStep(self):
        super(FluidDynamicsAnalysis, self).OutputSolutionStep()

        # if self.save_restart:
        #     self.restart_utility.SaveRestart()

    def _CreateProcesses(self, parameter_name, initialization_order):
        """Create a list of Processes
        This method is temporary to not break existing code
        It will be removed in the future
        """
        list_of_processes = super(FluidDynamicsAnalysis, self)._CreateProcesses(parameter_name, initialization_order)

        # The list of processes will contain a list with each individual process already constructed (boundary conditions, initial conditions and gravity)
        # Note 1: gravity is constructed first. Outlet process might need its information.
        # Note 2: initial conditions are constructed before BCs. Otherwise, they may overwrite the BCs information.
        if len(list_of_processes) == 0:
            Kratos.Logger.PrintInfo("FluidDynamicsAnalysis", "Using the old way to create the processes, this will be removed!")
            from process_factory import KratosProcessFactory
            factory = KratosProcessFactory(self.model)
            list_of_processes =  factory.ConstructListOfProcesses( self.project_parameters["gravity"] )
            list_of_processes += factory.ConstructListOfProcesses( self.project_parameters["initial_conditions_process_list"] )
            list_of_processes += factory.ConstructListOfProcesses( self.project_parameters["boundary_conditions_process_list"] )
            list_of_processes += factory.ConstructListOfProcesses( self.project_parameters["auxiliar_process_list"] )
        else:
            if self.project_parameters.Has("gravity"):
                raise Exception("Mixing of process initialization is not alowed!")
            if self.project_parameters.Has("initial_conditions_process_list"):
                raise Exception("Mixing of process initialization is not alowed!")
            if self.project_parameters.Has("boundary_conditions_process_list"):
                raise Exception("Mixing of process initialization is not alowed!")
            if self.project_parameters.Has("auxiliar_process_list"):
                raise Exception("Mixing of process initialization is not alowed!")

        return list_of_processes

    def _GetOrderOfProcessesInitialization(self):
        return ["gravity",
                "initial_conditions_process_list",
                "boundary_conditions_process_list",
                "auxiliar_process_list"]


    def _SetUpGiDOutput(self):
        '''Initialize a GiD output instance'''
        if self.parallel_type == "OpenMP":
            from gid_output_process import GiDOutputProcess as OutputProcess
        elif self.parallel_type == "MPI":
            from gid_output_process_mpi import GiDOutputProcessMPI as OutputProcess

        output = OutputProcess(self._GetSolver().GetComputingModelPart(),
                                self.project_parameters["problem_data"]["problem_name"].GetString() ,
                                self.project_parameters["output_configuration"])

        return output

    def _SetUpRestart(self):
        """Initialize self.restart_utility as a RestartUtility instance and check if we need to initialize the problem from a restart file."""
        if self.project_parameters.Has("restart_settings"):
            restart_settings = self.project_parameters["restart_settings"]
            self.load_restart = restart_settings["load_restart"].GetBool()
            self.save_restart = restart_settings["save_restart"].GetBool()
            restart_settings.RemoveValue("load_restart")
            restart_settings.RemoveValue("save_restart")
            restart_settings.AddValue("input_filename", self.project_parameters["problem_data"]["problem_name"])
            restart_settings.AddValue("echo_level", self.project_parameters["problem_data"]["echo_level"])

            if self.parallel_type == "OpenMP":
                from restart_utility import RestartUtility as Restart
            elif self.parallel_type == "MPI":
                from trilinos_restart_utility import TrilinosRestartUtility as Restart

            self.restart_utility = Restart(self.main_model_part,
                                           self.project_parameters["restart_settings"])
        else:
            self.load_restart = False
            self.save_restart = False

    def _GetSimulationName(self):
        return "Fluid Dynamics Analysis"

if __name__ == '__main__':
    from sys import argv

    if len(argv) > 2:
        err_msg =  'Too many input arguments!\n'
        err_msg += 'Use this script in the following way:\n'
        err_msg += '- With default parameter file (assumed to be called "ProjectParameters.json"):\n'
        err_msg += '    "python fluid_dynamics_analysis.py"\n'
        err_msg += '- With custom parameter file:\n'
        err_msg += '    "python fluid_dynamics_analysis.py <my-parameter-file>.json"\n'
        raise Exception(err_msg)

    if len(argv) == 2: # ProjectParameters is being passed from outside
        parameter_file_name = argv[1]
    else: # using default name
        parameter_file_name = "ProjectParameters.json"

    with open(parameter_file_name,'r') as parameter_file:
        parameters = Kratos.Parameters(parameter_file.read())

    model = Kratos.Model()
    simulation = FluidDynamicsAnalysis(model,parameters)
    simulation.Run()

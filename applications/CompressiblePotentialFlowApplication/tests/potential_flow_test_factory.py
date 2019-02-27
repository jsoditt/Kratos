from __future__ import print_function, absolute_import, division  # makes KratosMultiphysics backward compatible with python 2.6 and 2.7

# Importing the Kratos Library
import KratosMultiphysics

# Import KratosUnittest
import KratosMultiphysics.KratosUnittest as UnitTest

# Other imports
from KratosMultiphysics.CompressiblePotentialFlowApplication.potential_flow_analysis import PotentialFlowAnalysis
import KratosMultiphysics.kratos_utilities as kratos_utilities

import os
if kratos_utilities.IsApplicationAvailable("HDF5Application"):
    has_hdf5_application = True
else:
    has_hdf5_application = False
if kratos_utilities.IsApplicationAvailable("StructuralMechanicsApplication"):
    has_structural_application = True
else:
    has_structural_application = False

class WorkFolderScope:
    def __init__(self, work_folder):
        self.currentPath = os.getcwd()
        self.scope = os.path.abspath(os.path.join(os.path.dirname(os.path.realpath(__file__)),work_folder))

    def __enter__(self):
        os.chdir(self.scope)

    def __exit__(self, exc_type, exc_value, traceback):
        os.chdir(self.currentPath)

class PotentialFlowTestFactory(UnitTest.TestCase):

    def setUp(self):
        # Set to true to get post-process files for the test
        self.print_output = False

    def test_execution(self):

        with WorkFolderScope(self.work_folder):
            self._run_test()
            for file_name in os.listdir():
                if file_name.endswith(".h5") or file_name.endswith(".time"):
                    kratos_utilities.DeleteFileIfExisting(file_name)

class Naca0012SmallTest(PotentialFlowTestFactory):
    def __init__(self,argv):
        super(Naca0012SmallTest, self).__init__(argv)
        self.file_name = "naca0012_small"
        self.work_folder = "naca0012_small_test"

    def _run_test(self):
        model = KratosMultiphysics.Model()
        with open(self.file_name + "_parameters.json",'r') as parameter_file:
            ProjectParameters = KratosMultiphysics.Parameters(parameter_file.read())

        if self.print_output:
            ProjectParameters.AddValue("output_processes", KratosMultiphysics.Parameters(r'''{
                "gid_output" : [{
                    "python_module" : "gid_output_process",
                    "kratos_module" : "KratosMultiphysics",
                    "process_name"  : "GiDOutputProcess",
                    "help"          : "This process writes postprocessing files for GiD",
                    "Parameters"    : {
                        "model_part_name"        : "MainModelPart",
                        "output_name"            : "naca0012",
                        "postprocess_parameters" : {
                            "result_file_configuration" : {
                                "gidpost_flags"       : {
                                    "GiDPostMode"           : "GiD_PostBinary",
                                    "WriteDeformedMeshFlag" : "WriteDeformed",
                                    "WriteConditionsFlag"   : "WriteConditions",
                                    "MultiFileFlag"         : "SingleFile"
                                },
                                "file_label"          : "step",
                                "output_control_type" : "step",
                                "output_frequency"    : 1,
                                "body_output"         : true,
                                "node_output"         : false,
                                "skin_output"         : false,
                                "plane_output"        : [],
                                "nodal_results"       : ["VELOCITY_POTENTIAL","AUXILIARY_VELOCITY_POTENTIAL","DISTANCE"],
                                "nodal_nonhistorical_results": ["TRAILING_EDGE"],
                                "elemental_conditional_flags_results": ["STRUCTURE"],
                                "gauss_point_results" : ["PRESSURE","VELOCITY","VELOCITY_LOWER","PRESSURE_LOWER","WAKE","ELEMENTAL_DISTANCES","KUTTA"]
                            },
                            "point_data_configuration"  : []
                        }
                    }
                }]
            }'''))

        potential_flow_analysis = PotentialFlowAnalysis(model,ProjectParameters)
        potential_flow_analysis.Run()

@UnitTest.skipUnless(has_hdf5_application,"Missing required application: HDF5Application")
@UnitTest.skipUnless(has_structural_application,"Missing required application: StructuralMechanicsApplication")
class Naca0012SmallAdjointTest(PotentialFlowTestFactory):
    def __init__(self,argv):
        super(Naca0012SmallAdjointTest, self).__init__(argv)
        self.file_name = "naca0012_small_adjoint"
        self.work_folder = "naca0012_small_adjoint_test"

    def _run_test(self):
        model_primal = KratosMultiphysics.Model()
        with open(self.file_name + "_primal_parameters.json",'r') as parameter_file:
            ProjectParametersPrimal = KratosMultiphysics.Parameters(parameter_file.read())

        primal_analysis = PotentialFlowAnalysis(model_primal,ProjectParametersPrimal)
        primal_analysis.Run()

        model_adjoint = KratosMultiphysics.Model()
        with open(self.file_name + "_parameters.json",'r') as parameter_file:
            ProjectParametersAdjoint = KratosMultiphysics.Parameters(parameter_file.read())

        adjoint_analysis = PotentialFlowAnalysis(model_adjoint,ProjectParametersAdjoint)
        adjoint_analysis.Run()

if __name__ == '__main__':
    UnitTest.main()

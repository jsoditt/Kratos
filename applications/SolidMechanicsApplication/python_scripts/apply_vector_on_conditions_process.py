from KratosMultiphysics import *
from KratosMultiphysics.SolidMechanicsApplication import *

import math

def Factory(settings, Model):
    if(type(settings) != Parameters):
        raise Exception("Expected input shall be a Parameters object, encapsulating a json string")
    return ApplyVectorOnConditionsProcess(Model, settings["Parameters"])

##all the processes python processes should be derived from "python_process"

class ApplyVectorOnConditionsProcess(Process):
    def __init__(self, Model, settings ):
        Process.__init__(self)
        
        self.model_part = Model[settings["model_part_name"].GetString()]
        self.var = globals()[settings["variable_name"].GetString()]

        #check if variable type is a vector
        if type(self.var) != type(DISPLACEMENT):
            raise Exception("Variable type is incorrect. Must be a three-component vector.")
    
        self.factor = settings["factor"].GetDouble();

        self.direction = [settings["direction"][0].GetDouble(),settings["direction"][1].GetDouble(),settings["direction"][2].GetDouble()]

        self.norm = math.sqrt(self.direction[0]*self.direction[0]+self.direction[1]*self.direction[1]+self.direction[2]*self.direction[2])
        
        if( self.norm != 0.0 ):
            self.value = [self.direction[0]*(self.factor/self.norm),self.direction[1]*(self.factor/self.norm),self.direction[2]*(self.factor/self.norm)]
        else:
            self.value = [0.0,0.0,0.0]
        
    def ExecuteInitialize(self):
        for cond in self.model_part.Conditions:
            cond.SetValue(self.var,self.value)

    
        
    

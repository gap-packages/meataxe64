DeclareGlobalFunction("UnderlyingMeataxe64Matrix");

DeclareRepresentation("IsMeataxe64VecMatObjRep", IsAttributeStoringRep, 
        ["UnderlyingMeataxe64Matrix"]);

DeclareSynonym("IsMeataxe64MatrixObj", IsMeataxe64VecMatObjRep and IsMatrixObj);
DeclareSynonym("IsMeataxe64VectorObj", IsMeataxe64VecMatObjRep and IsVectorObj);

DeclareAttribute("Meataxe64Echelonization", IsMeataxe64MatrixObj);



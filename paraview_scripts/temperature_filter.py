polydata = inputs[0]

output.ShallowCopy(polydata.VTKObject)

timestep = polydata.GetInformation().Get(output.DATA_TIME_STEP())
z = 200 * (1 - timestep / 625)

uu = polydata.GetPointData().GetArray("uu")

temperature = vtk.vtkDoubleArray()
temperature.SetName("Temperature")
temperature.SetNumberOfComponents(1)

numPoints = polydata.GetNumberOfPoints()

for i in range(0, numPoints):
    #  4.8e5 * uu / (1+z)^3
    this_uu = uu.GetTuple1(i)
    temperature.InsertNextValue(4.8e5 * this_uu / (1 + z)**3)

output.GetPointData().AddArray(temperature)

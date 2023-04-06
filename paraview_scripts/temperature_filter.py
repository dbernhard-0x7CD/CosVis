table = inputs[0].GetBlock(0)
polydata = inputs[0].GetBlock(1)

output.ShallowCopy(inputs[0].VTKObject)

time = table.GetRow(0)
z = time.GetValue(0).ToFloat()

uu = polydata.GetPointData().GetArray("uu")

temperature = vtk.vtkDoubleArray()
temperature.SetName("Temperature")
temperature.SetNumberOfComponents(1)

numPoints = polydata.GetNumberOfPoints()

for i in range(0, numPoints):
    #  4.8e5 * uu / (1+z)^3
    this_uu = uu.GetTuple1(i)
    temperature.InsertNextValue(4.8e5 * this_uu / (1 + z)**3)

polydata.GetPointData().AddArray(temperature)

output.RemoveBlock(0)
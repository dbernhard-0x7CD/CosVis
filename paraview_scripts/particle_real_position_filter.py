polydata = inputs[0]

timestep = polydata.GetInformation().Get(output.DATA_TIME_STEP())
numPoints = polydata.GetNumberOfPoints()

pts = vtk.vtkPoints()

a = numpy.linspace(1./201., 1., 626)[1:][int(timestep)]
 
n = 0
for i in range(0, numPoints):
    x,y,z = polydata.GetPoint(i)
    pts.InsertNextPoint(a * x,a * y,a * z)
    n+=1

output.ShallowCopy(polydata.VTKObject)
output.SetPoints(pts)

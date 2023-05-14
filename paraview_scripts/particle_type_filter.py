polydata = inputs[0]
# version 0.1
ALL =                      0b0 # This is not a bitmask
BARYON =               0b10
DARK_MATTER =         0b1000000000 # This is not a bitmask
BARYON_STAR =             0b100000
BARYON_WIND =            0b1000000
BARYON_STAR_FORMING =   0b10000000
DARK_AGN =             0b100000000

SELECTOR = DARK_MATTER

dsa = polydata.GetPointData()
mask = dsa.GetArray("mask")
numPoints = polydata.GetNumberOfPoints()
pts = vtk.vtkPoints()

# Create empty attribute arrays to copy over the selected particles' attributes
old_attributes = dict()
new_attributes = dict()
for x in dsa.keys():
    print("\tx: " + str(x))
    old_attributes[x]= dsa.GetArray(x)
    new_attributes[x] = dsa.GetArray(x).NewInstance()
    new_attributes[x].SetName(x)

n = 0
for i in range(0, numPoints):
    x,y,z = polydata.GetPoint(i)
    this_mask = mask[i]
    if SELECTOR == ALL:
        pass
    elif SELECTOR == DARK_MATTER:
        if this_mask & BARYON:
            continue
    else:
        if not (SELECTOR & this_mask):
            continue
    pts.InsertNextPoint(x,y,z)

    # Also copy the attributes
    for x in dsa.keys():
        new_attributes[x].InsertNextTuple1(old_attributes[x].GetTuple1(i))
    n+=1

output.SetPoints(pts)

# Add the new attribute arrays
for x in dsa.keys():
#    print("Adding attributes: " + str(x))
    output.GetPointData().AddArray(new_attributes[x])
# print("Amount of particles: " + str(n))